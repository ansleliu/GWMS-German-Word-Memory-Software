#include "SpeakMgr.h"
#include <QDebug>
#include "QTimer"

SpeakMgr::SpeakMgr(QObject *parent, const QString &bookname) :
    QObject(parent)
{
    sound = NULL;
    QtSpeechTTS = NULL;
    speakGoogleTTS = NULL;
    speakRealPerson = NULL;
    ////////////////////////////////////////////////
    textSpeak = "";
    bookName = bookname;
    speakTime = 1;
    Repetitions = 0;
    qDebug() << QString("��������������֪�Ĵʿ���Ϊ: %1").arg(bookName);
    ////////////////////////////////////////////////
    //��Ƶ�ź��������
    ////////////////////////////////////////////////
    //eSpeakTTS
    sound = new SoundTread();
    connect(sound,SIGNAL(soundEnded()),
            this,SLOT(EmitSpeakFininshSignal()));

    ////////////////////////////////////////////////
    //
    localTTS = bcMgr->userConf->getSpeakLocalTTS();
    qDebug() << QString("��������������Ϊ��%1").arg(localTTS);
    ////////////////////////////////////////////////
    //GoogleTTS
    speakGoogleTTS = new SpeakGoogle();
    connect(speakGoogleTTS,SIGNAL(soundEnded()),
            this,SLOT(EmitSpeakFininshSignal()));
    connect(speakGoogleTTS,SIGNAL(NetWorkUnavailable()),
            this,SLOT(changeSpeakEngineToESpeak()));
    connect(speakGoogleTTS,SIGNAL(speakError()),
            this,SLOT(changeSpeakEngineToESpeak()));
    connect(speakGoogleTTS,SIGNAL(fileOpenError()),
            this,SLOT(changeSpeakEngineToESpeak()));
    ////////////////////////////////////////////////
    //RealPerson
    speakRealPerson = new SpeakReal(0,bookName);
    connect(speakRealPerson,SIGNAL(soundEnded()),
            this,SLOT(EmitSpeakFininshSignal()));
    connect(speakRealPerson,SIGNAL(speakError()),
            this,SLOT(changeSpeakEngineToESpeak()));
    connect(speakRealPerson,SIGNAL(UncontionsFile()),
            this,SLOT(changeSpeakEngineToESpeak()));
}

SpeakMgr::~SpeakMgr()
{
    if(sound != NULL)
    {
        qDebug() << "~SpeakMgr���sound";
        sound->deleteLater();
        sound = NULL;
    }

    if(QtSpeechTTS != NULL)
    {
        QtSpeechTTS->deleteLater();
        QtSpeechTTS = NULL;
        qDebug() << QString("���QtSpeech�����ɹ�");
    }

    if(speakRealPerson != NULL)
    {
        qDebug() << "~SpeakMgr���speakRealPerson";
        speakRealPerson->deleteLater();
        speakRealPerson = NULL;
    }

    if(speakGoogleTTS != NULL)
    {
        qDebug() << "~SpeakMgr���speakGoogleTTS";
        speakGoogleTTS->deleteLater();
        speakGoogleTTS = NULL;
    }
}

void SpeakMgr::Speak(const QString &text,
                     const int &speakInterval,
                     const int &SpeakRepetitions)
{
    ///////////////////////////////////////////////////////////////
    //��ȡ������������
    speakMethod = bcMgr->userConf->getSpeakMethod();
    qDebug() << QString("��ǰ�����ļ����趨��Ĭ�Ϸ�����ʽΪ: %1").arg(speakMethod);
    currSpeakMethod = "";
    textSpeak = "";
    speakTime = 1;
    /////////////////////////////////////
    //
    textSpeak = text;
    Interval = speakInterval;
    Repetitions = SpeakRepetitions;
    /////////////////////////////////////////
    //LocalTTS
    if(speakMethod == "LocalTTS")
    {
        qDebug() << "Ĭ������Ϊ��LocalTTS , ��ʼ����";
        currSpeakMethod = "LocalTTS";
        if(localTTS == QString("eSpeakTTS"))
        {
            qDebug() << QString("Ĭ������Ϊ��%1, ��ʼ����").arg(localTTS);
            qDebug() << QString("��������Ϊ��%1").arg(Repetitions);
            sound->getSoundTime(Repetitions);
            sound->getWord(textSpeak);
            sound->getTimeDiff(Interval);
            sound->start();
        }
        else
        {
            QtSpeechTell();
        }
    }
    //////////////////////////////////////////
    //realSpeak&eSpeakTTS
    if(speakMethod == "realSpeak&eSpeakTTS")
    {
        qDebug() << "Ĭ������Ϊ��realSpeak&eSpeakTTS , ��ʼ����";
        qDebug() << QString("��������Ϊ��%1").arg(Repetitions);
        currSpeakMethod = "realSpeak&eSpeakTTS";
        speakRealPerson->setSoundTime(Repetitions);
        speakRealPerson->setInterval(Interval);
        speakRealPerson->realPersonSpeak(textSpeak);
    }
    //////////////////////////////////////////
    //googleTTS
    if(speakMethod == "googleTTS")
    {
        qDebug() << "Ĭ������Ϊ��googleTTS , ��ʼ����";
        qDebug() << QString("��������Ϊ��%1").arg(Repetitions);
        currSpeakMethod = "googleTTS";
        speakGoogleTTS->setSoundTime(Repetitions);
        speakGoogleTTS->setInterval(Interval);
        speakGoogleTTS->speakGoogleTTS(textSpeak);
    }
}

void SpeakMgr::Stop()
{
    /////////////////////////////////////////
    //LocalTTS
    if(speakMethod == "LocalTTS")
    {
        qDebug() << "Ĭ������Ϊ��LocaleSpeakTTS";
        if(localTTS == QString("eSpeakTTS"))
        {
            qDebug() << QString("ֹͣeSpeakTTS����");
            sound->stop();
            qDebug() << QString("�Ѿ�ֹͣeSpeakTTS����");
        }
        else
        {
            qDebug() << QString("ֹͣQtSpeechTTS����");
            QtSpeechTTS->stop();
            qDebug() << QString("�Ѿ�ֹͣQtSpeechTTS����");
        }
    }
    //////////////////////////////////////////
    //realSpeak&eSpeakTTS
    if(speakMethod == "realSpeak&eSpeakTTS")
    {
        qDebug() << "Ĭ������Ϊ��realSpeak&eSpeakTTS";
        qDebug() << QString("ֹͣrealSpeak&eSpeakTTS����");
        speakRealPerson->realSpeakStop();
        qDebug() << QString("�Ѿ�ֹͣrealSpeak&eSpeakTTS����");
    }
    //////////////////////////////////////////
    //googleTTS
    if(speakMethod == "googleTTS")
    {
        qDebug() << "Ĭ������Ϊ��googleTTS";
        qDebug() << QString("ֹͣgoogleTTS����");
        speakGoogleTTS->googleTTSStop();
        qDebug() << QString("�Ѿ�ֹͣgoogleTTS����");
    }
}

void SpeakMgr::QtSpeechTell()
{
    if(QtSpeechTTS != NULL)
    {
        QtSpeechTTS->deleteLater();
        QtSpeechTTS = NULL;
    }
    ////////////////////////////////////////////////
    //QtSpeechTTS
    foreach(QtSpeech::VoiceName v, QtSpeech::voices())
    {
        qDebug() << "id:" << v.id;
        qDebug() << "name:" << v.name;
        if(v.name == localTTS)
        {
            qDebug() << QString("�ҵ�ƥ��ķ������棺%1").arg(v.name);

            QtSpeechTTS = new QtSpeech(v,this);
            qDebug() << QString("����QtSpeech�����ɹ�");

            QtSpeechTTS->setRate(bcMgr->userConf->getSpeakRate().toInt());
            break;
        }
    }
    qDebug() << QString("Ĭ������Ϊ��%1, ��ʼ����").arg(localTTS);
    qDebug() << QString("��������Ϊ��%1").arg(Repetitions);
    QtSpeechTTS->tell(textSpeak,this,SLOT(judgeSpeakTime()));
}

void SpeakMgr::changeSpeakEngineToGoogle()
{
    qDebug() << "ת��������ʽΪ��googleTTS";
    currSpeakMethod = "googleTTS";
    speakGoogleTTS->setSoundTime(Repetitions);
    speakGoogleTTS->speakGoogleTTS(textSpeak);
}

void SpeakMgr::changeSpeakEngineToESpeak()
{
    qDebug() << QString("ת��������ʽΪ��LocalTTS");
    /////////////////////////////////////////
    //LocalTTS
    currSpeakMethod = "LocalTTS";
    if(localTTS == QString("eSpeakTTS"))
    {
        qDebug() << QString("Ĭ�϶�����������Ϊ��%1, ��ʼ����").arg(localTTS);
        sound->getSoundTime(Repetitions);
        sound->getWord(textSpeak);
        sound->getTimeDiff(Interval);
        sound->start();
    }
    else
    {
        QtSpeechTell();
    }
}

void SpeakMgr::judgeSpeakTime()
{

    if(QtSpeechTTS != NULL)
    {
        QtSpeechTTS->deleteLater();
        QtSpeechTTS = NULL;
    }

    speakTime++;
    qDebug() << QString("���Ŵ���Ϊ��%1").arg(speakTime);
    qDebug() << QString("��������Ϊ��%1").arg(Repetitions);
    if(speakTime <= Repetitions)
    {
        ////////////////////////////////
        //��������δ��
        qDebug() << QString("%1��������δ������������").arg(localTTS);
        QTimer::singleShot(Interval, this, SLOT(QtSpeechTell()));
    }
    else
    {
        EmitSpeakFininshSignal();
    }
}
/*
void SpeakMgr::setSpeakInterval()
{

}

void SpeakMgr::setSpeakRepetitions()
{

}
*/

void SpeakMgr::EmitSpeakFininshSignal()
{
//    speakTime++;
//    qDebug() << QString("���Ŵ���Ϊ��%1").arg(speakTime);
//    qDebug() << QString("��������Ϊ��%1").arg(Repetitions);
//    if(currSpeakMethod == "LocalTTS" &&
//            localTTS != QString("eSpeakTTS") &&
//            speakTime <= Repetitions)
//    {
        ////////////////////////////////
        //��������δ��
//        qDebug() << QString("%1��������δ������������").arg(localTTS);
//        QtSpeechTell();
//    }
//    else
    {
        qDebug() << QString("SpeakMgr�������������䷢�������źš���");
        emit SpeakFininshed();
    }
}
