#include "SpeakReal.h"
#include "pathmanage.h"
#include "myglobal.h"
#include "QTimer"

SpeakReal::SpeakReal(QObject *parent, const QString &bookname) :
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
    ////////////////////////////////////////////////////////////////
    //
    currUser = Global::Conf->getCurrentUser();
    qDebug() << QString("������������ĵ�ǰ�û���: %1").arg(currUser);
    bookName = bookname;
    qDebug() << QString("������������ĵ�ǰ�ʿ���: %1").arg(bookName);

    wordText = "";
    soundPath = "";
    soundTime = 0;
}

SpeakReal::~SpeakReal()
{

    mediaObject->deleteLater();
    mediaObject = NULL;
    audioOutput->deleteLater();
    audioOutput = NULL;
}

void SpeakReal::stateChanged(Phonon::State newState, Phonon::State oldstate)
{
    Q_UNUSED(oldstate);
    curState = newState;
    if(curState == Phonon::PausedState)
    {
        qDebug() << "״̬�ı�ΪPausedState����ʼ������������realSpeak()";
        realSpeak();
    }
    else if(curState == Phonon::StoppedState)
    {
        qDebug() << "״̬�ı�ΪStoppedState�����ͽ����ź�";
        emit soundEnded();
    }
    else if(curState == Phonon::ErrorState)
    {
        qDebug() << "״̬�ı�ΪErrorState�����ͷ��������ź�";
        emit speakError();
    }
}

void SpeakReal::realSpeak()
{
    qDebug() << QString("realSpeak()������������ʼ������λ�Ӻͷ���Դ����");
    mediaObject->setCurrentSource(Phonon::MediaSource(soundPath));
    realSpeakRestart();
    time++;
    if(time == 1)
    {
        realSpeakPlay();
    }
    else if(time > 1 && time <= soundTime)
    {
        qDebug() << QString("realSpeak()�����жϷ��������Ƿ�ﵽ����");
        qDebug() << QString("���Ŵ���%1δ�ﵽ%2����������").arg(time).arg(soundTime);
        qDebug() << QString("������������realSpeakPlay()����");

//        QTimer::singleShot(Interval, this, SLOT(realSpeakPlay()));
        realSpeakPlay();
    }
    else
    {
        qDebug() << QString("���Ŵ����ﵽ��ֹͣ����");
        realSpeakStop();
    }
}

void SpeakReal::setSoundTime(const int &soundtime)
{
    soundTime = soundtime;
}

void SpeakReal::setInterval(const int &interval)
{
     Interval = interval;
}

void SpeakReal::realPersonSpeak(const QString &text)
{
    qDebug() << QString("���������������˷������桭��");
    wordText = "";
    if(mediaObject->currentTime() == mediaObject->totalTime())
    {
        mediaObject->seek(0);
    }
    ///////////////////////////////////////////////////////
    //
    wordText = text;
    if(wordText.contains(QRegExp("[\?\\*\\:<>\\|]")))
    {
        wordText.remove(QRegExp("[\?\\*\\:<>\\|]"));
    }

    soundPath = PathManage::makePathStr(QString("/userdata/%1/RealSpeak/%2/%3.mp3")
                                        .arg(currUser)
                                        .arg(bookName)
                                        .arg(wordText));
    qDebug() << QString("������������Ƶ�ļ�Ϊ: %1").arg(soundPath);

    time = 0;
    qDebug() << "Ѱ�����˷����ļ�����";
    QFile mp3File(soundPath);
    if(mp3File.exists())
    {

        qDebug() << "���������ڶ�Ӧ�ʿ���Ƶ���д���,�����в���";
        realSpeak();
    }
    else
    {
        qDebug() << "���������ڶ�Ӧ�ʿ���Ƶ���в�����,�л�Ѱ��·��Ϊ��Ŀ¼��Ƶ��";
        soundPath = PathManage::makePathStr(QString("/Sound/%1.mp3")
                                            .arg(wordText));
        qDebug() << QString("������������Ƶ�ļ�Ϊ: %1").arg(soundPath);
        time = 0;
        QFile mp3FileSound(soundPath);
        if(mp3FileSound.exists())
        {
            qDebug() << "����������Ŀ¼��Ƶ���д���,������realSpeak()���в���";
            realSpeak();
        }
        else
        {
            qDebug() << "��������������,�����ź�����ת��������ʽ";
            wordText = "";
            soundPath = "";
            soundTime = 0;
            time = 0;
            emit UncontionsFile();
        }
    }
}

void SpeakReal::realSpeakRestart()
{
    if(mediaObject->currentTime() == mediaObject->totalTime())
    {
        mediaObject->seek(0);
    }
}

void SpeakReal::realSpeakPlay()
{
    qDebug() << "realSpeakPlay():������������,��ʼ������������";
    mediaObject->play();
}

void SpeakReal::realSpeakStop()
{
    qDebug() << "ֹͣ������������";
    mediaObject->stop();
}

Phonon::MediaObject *SpeakReal::getMediaObject()
{
    return mediaObject;
}
