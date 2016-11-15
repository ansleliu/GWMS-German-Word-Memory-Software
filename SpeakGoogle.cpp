#include "SpeakGoogle.h"
#include <QTextCodec>
#include <QTimer>
#include "myglobal.h"
#include "pathmanage.h"
#include "QDebug"
#include <QMutex>

SpeakGoogle::SpeakGoogle(QObject *parent) :
    QObject(parent)
{
    /////////////////////////////////////////////////////////////////
    //phonon
    mediaObject = new Phonon::MediaObject(this);
    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(mediaObject, audioOutput);

    connect(mediaObject,
            SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this,
            SLOT(stateChanged(Phonon::State, Phonon::State)));
    ///////////////////////////////////////////////////////////////////
    //
    manager = NULL;
    reply = NULL;
    file = NULL;
    soundPath = "";
    time = 0;
    tryTime = 0;
}

SpeakGoogle::~SpeakGoogle()
{
    mediaObject->deleteLater();
    mediaObject = NULL;
    audioOutput->deleteLater();
    audioOutput = NULL;

    if(manager != NULL)
    {
        manager->deleteLater();
        manager = NULL;
    }
    if(file != NULL)
    {

        file = NULL;
    }
    if(reply != NULL)
    {
        reply = NULL;
    }
}

void SpeakGoogle::stateChanged(Phonon::State newState,
                               Phonon::State oldstate)
{
    //////////////////////////////////////////////////////
    //
    qDebug() << QString("Phonon��״̬�Ƿ����ı�");
    qDebug() << QString("Phonon�ľ�״̬��: %1").arg(oldstate);
    qDebug() << QString("Phonon����״̬��: %1").arg(oldstate);
    ////////////////////////////////////////////
    //����״̬�ĸı�
    curState = newState;
    if(curState == Phonon::PausedState)
    {
        qDebug() << "״̬�ı�ΪPausedState����ʼ����";
        startSpeak();
    }
    else if(curState == Phonon::StoppedState)
    {
        qDebug() << "״̬�ı�ΪStoppedState�������źŷ���";
        tryTime = 0;
        time = 0;
        emit soundEnded();
    }
    else if(curState == Phonon::ErrorState)
    {
        qDebug() << QString("״̬�ı�ΪErrorState");
        qDebug() << QString("���ڷ����������ص���Ƶ�ļ������Ƴ�");
        file->remove(soundPath);
//        qDebug() << QString("���ص���Ƶ�ļ������Ƴ��������³�����Ƶ�����ļ�");
//        tryTime++;
//        if(tryTime <= 1)
//        {
//            qDebug() << QString("��������%1").arg(tryTime);
//            speakGoogleTTS(wordText);
//        }
//        else
        {
            qDebug() << "��������״̬�޷�ͨ����������������Ƶ�ļ����Ը��ģ����ͷ��������źŸ�����������";
            tryTime = 0;
            emit speakError();
        }
    }
}

void SpeakGoogle::startSpeak()
{
    qDebug() << QString("��ʼ���������÷���ԴΪ��Ƶ�ļ����ڵ�·��:%1").arg(soundPath);
    mediaObject->setCurrentSource(Phonon::MediaSource(soundPath));
    //��ʼ����
    googleTTS();
}

void SpeakGoogle::googleTTS()
{
    qDebug() << "��ʼ�������жϲ��Ŵ���";
    speakGoogleRestart();
    time++;
    if(time > soundTime)
    {
        qDebug() << "���Ŵ����ﵽ��ֹͣ����";       
        googleTTSStop();
    }
    else
    {
        qDebug() << "���Ŵ���δ�ﵽ����������";
//        QTimer::singleShot(Interval, this, SLOT(googleTTSPlay()));
        googleTTSPlay();
    }
}

void SpeakGoogle::setSoundTime(const int &soundtime)
{
    soundTime = soundtime;
}

void SpeakGoogle::setInterval(const int &interval)
{
    Interval = interval;
}

void SpeakGoogle::speakGoogleTTS(const QString &text)
{
    if(mediaObject->currentTime() == mediaObject->totalTime())
    {
        mediaObject->seek(0);
    }
    time = 0;
    ///////////////////////////////////////////////////////////////////////////////////
    //
    wordText = text;
    qDebug() << QString("�ȴ������ĵ���Ϊ: %1").arg(wordText);
    QString strUrl = QString("http://translate.google.com/translate_tts?tl=de&q=%1")
                            .arg(wordText);
    QUrl url(strUrl);
    qDebug() << QString("������Ƶ�ļ������ص�ַ:%1").arg(strUrl);
    //////////////////////////////////////////////////////////////
    //    
    QString userName = Global::Conf->getCurrentUser();

    if(wordText.contains(QRegExp("[\?\\*\\:<>\\|]")))
    {
        wordText.remove(QRegExp("[\?\\*\\:<>\\|]"));
    }

    soundPath = PathManage::makePathStr(QString("/userdata/%1/GoogleTTSVoice/%2.mp3")
                                        .arg(userName)
                                        .arg(wordText));
    qDebug() << QString("������Ƶ�ļ��Ĵ�ŵ�ַΪ:%1").arg(soundPath);

    if(file != NULL)
    {
        file->deleteLater();
        file = NULL;
    }
    file = new QFile(soundPath);

    if(file->exists())
    {
        qDebug() << "����Ƶ�ļ��Ѿ����ڣ�������Ƶ�ļ����أ�ֱ�ӽ��з���";
        startSpeak();
    }
    else
    {
        if(!file->open(QIODevice::WriteOnly))
        {
            qDebug() << "�ļ���ʧ��,�������أ����������������ļ���ʧ��ӡ��ת��������ʽ";
            file->deleteLater();
            file = NULL;

            emit fileOpenError();
            return;
        }
        /////////////////////////////////////////////////////////////////
        //׼��������Ƶ�ļ�
        /////////////////////////////////////////////////////////////////
        //����NetWork���
        qDebug() << QString("�ļ��Ѿ��򿪣�׼��������������");
        if(manager != NULL)
        {
            manager->deleteLater();
            manager = NULL;
        }
        qDebug() << QString("�����µ�QNetworkAccessManagerָ��manager");
        manager = new QNetworkAccessManager(this);
        ///////////////////////////////////////////////
        //���ض�Ӧ�������ļ�
//        QEventLoop loop;
        if(reply != NULL)
        {
//            reply->deleteLater();
            reply = NULL;
        }
        qDebug() << QString("�����µ�QNetworkRequestrָ��reply");
        reply = manager->get(QNetworkRequest(url));
        qDebug() << QString("��������ź���۵�����");
        connect(reply, SIGNAL(readyRead()),
                this, SLOT(httpReadyRead()));

        connect(reply, SIGNAL(finished()),
                this, SLOT(httpFinished()));
//        connect(reply, SIGNAL(finished()),
//                         &loop, SLOT(quit()));

        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(haveError(QNetworkReply::NetworkError)));
//        loop.exec();
        /////////////////////////////////////////////////////////////////
        /*
        DownloadControl *control = new DownloadControl();
        int PointCount = 1;

        QEventLoop loop;
        connect(control, SIGNAL(FileDownloadFinished()),
                         &loop, SLOT(quit()));
        connect(control, SIGNAL(FileDownloadFinished()),
                         this, SLOT(startSpeak()));
        connect(control, SIGNAL(DownloadFail()),
                         &loop, SLOT(quit()));
        connect(control, SIGNAL(DownloadFail()),
                         this, SLOT(googleTTSStop()));
        ////////////////////////////////////////////////////////
        //��ʼ�����ļ�
        starttime = QTime::currentTime();
        control->StartFileDownload(strUrl, PointCount,file);

        loop.exec();
        */
    }

    //////////////////////////////////////////////////////////////
    //
//    mediaObject->setCurrentSource(Phonon::MediaSource(url));
//    mediaObject->play();
//    time = 1;
}

void SpeakGoogle::speakGoogleRestart()
{
    qDebug() << "�ظ�����,���ò���λ��Ϊ��ʼλ��";
    if(mediaObject->currentTime() == mediaObject->totalTime())
    {
        mediaObject->seek(0);
    }
}

void SpeakGoogle::googleTTSPlay()
{
    qDebug() << QString("����������play()������Ƶ����");
    mediaObject->play();
}

void SpeakGoogle::googleTTSStop()
{
    qDebug() << QString("ֹͣ�ȸ�������������Ƶ����");
    if(manager != NULL)
    {
        qDebug() << QString("manager��Ϊ�գ������������");
        manager->deleteLater();
        manager = NULL;
    }
    //////////////////////
    if(reply != NULL)
    {
        qDebug() << QString("reply��Ϊ�գ������������");
        reply->deleteLater();
        reply = NULL;
    }
/*
    if(file != NULL)
    {
        qDebug() << QString("file��Ϊ�գ������������");
        file->flush();
        file->remove();
        if(file->isOpen())
        {
            file->close();
        }

        file->deleteLater();
        file = NULL;
    }
*/
    mediaObject->stop();
//    mediaObject->clear();
}

Phonon::MediaObject *SpeakGoogle::getMediaObject()
{
    return mediaObject;
}

void SpeakGoogle::httpFinished()
{
    ///////////////////////////
    //�������
    QMutex mutex;
    mutex.lock();
    if(reply->error() == QNetworkReply::NoError)
    {
        //////////////////////////////////////////
        //���û������Ļ��ر���ص��ź����
        qDebug() << QString("QNetworkReplyû�����⣬�ر���ص��ź����");
        disconnect(reply, SIGNAL(readyRead()),
                this, SLOT(httpReadyRead()));

        disconnect(reply, SIGNAL(finished()),
                this, SLOT(httpFinished()));

        disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(haveError(QNetworkReply::NetworkError)));
        ///////////////////////////////
        //�ź���۹رպ�������������
        file->flush();
        file->close();

        reply->deleteLater();
        reply = NULL;

        file->deleteLater();
        file = NULL;

        mutex.unlock();

        qDebug() << "��Ƶ�ļ�������ɿ�ʼ���з���";
        //////////////////////////////////////////////////////////////
        //����
       startSpeak();
    }
    else
    {
        qDebug() << "�����д��󣬴���Ϊ��"<< reply->error();
    }
}

void SpeakGoogle::httpReadyRead()
{
    if ( !file )
    {
        qDebug() << "�ļ�ָ�붪ʧ";
        emit fileOpenError();
        return;
    }
    QMutex mutex;
    mutex.lock();
    qDebug() << QString("��ȡHTTP��ֵ��д���ı��ĵ�����");
    QByteArray buffer = reply->readAll();
    file->write(buffer);
    mutex.unlock();
}

void SpeakGoogle::haveError(QNetworkReply::NetworkError error)
{
    //////////////////////////////////////////
    //����������ر���ص��ź����
    qDebug() << "������󣬴������Ϊ��"<< error;
    qDebug() << QString("�����д����ź���۵����ӽ����������������ź�");

    disconnect(reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()));

    disconnect(reply, SIGNAL(finished()),
            this, SLOT(httpFinished()));

    disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(haveError(QNetworkReply::NetworkError)));

    googleTTSStop();
    emit NetWorkUnavailable();
}

void SpeakGoogle::NetWorkIsBad()
{
    googleTTSStop();
}
