#include "AnsleDictSpeakGoogle.h"
#include <QTextCodec>
#include <QTimer>
#include "PathMgr.h"
#include <QDebug>
#include <QMutex>

AnsleDictSpeakGoogle::AnsleDictSpeakGoogle(QObject *parent) :
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
    time = 0;
}

AnsleDictSpeakGoogle::~AnsleDictSpeakGoogle()
{
    mediaObject->deleteLater();
    mediaObject = NULL;
    audioOutput->deleteLater();
    audioOutput = NULL;
}

void AnsleDictSpeakGoogle::stateChanged(Phonon::State newState,
                               Phonon::State oldstate)
{
    Q_UNUSED(oldstate);
    curState = newState;
    if(curState == Phonon::PausedState)
    {
        qDebug() << "״̬�ı䣬��ʼ����";
        startSpeak();
    }
    else if(curState == Phonon::StoppedState)
    {
        qDebug() << "״̬�ı䣬ֹͣ�����������źŷ���";
        emit soundEnded();
    }
    else if(curState == Phonon::ErrorState)
    {
        qDebug() << "״̬�ı䣬�������󣬽����źŷ���";

        emit speakError();
    }
}

void AnsleDictSpeakGoogle::startSpeak()
{
    qDebug() << "��ʼ����";
    mediaObject->setCurrentSource(Phonon::MediaSource(myurl));
    //��ʼ����
    googleTTS();
}

void AnsleDictSpeakGoogle::googleTTS()
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
        googleTTSPlay();
    }
}

void AnsleDictSpeakGoogle::setSoundTime(const int &soundtime)
{
    soundTime = soundtime;
}

void AnsleDictSpeakGoogle::speakGoogleTTS(const QString &text)
{
    if(mediaObject->currentTime() == mediaObject->totalTime())
    {
        mediaObject->seek(0);
    }
    time = 0;
    ///////////////////////////////////////////////////////////////////////////////////
    //
    wordText = text;
    QString strUrl = QString("http://translate.google.com/translate_tts?tl=de&q=%1")
                            .arg(wordText);
    QUrl url(strUrl); 
    myurl = url;
    startSpeak();
}

void AnsleDictSpeakGoogle::speakGoogleRestart()
{
    qDebug() << "�ظ��������ò���λ��";
    if(mediaObject->currentTime() == mediaObject->totalTime())
    {
        mediaObject->seek(0);
    }
}

void AnsleDictSpeakGoogle::googleTTSPlay()
{
    mediaObject->play();
}

void AnsleDictSpeakGoogle::googleTTSStop()
{
    mediaObject->stop();
//    mediaObject->clear();
}

Phonon::MediaObject *AnsleDictSpeakGoogle::getMediaObject()
{
    return mediaObject;
}

void AnsleDictSpeakGoogle::NetWorkIsBad()
{
    googleTTSStop();
}
