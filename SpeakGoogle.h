#ifndef SPEAKGOOGLE_H
#define SPEAKGOOGLE_H

#include <QObject>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include "./DownloadControl/DownloadControl.h"
#include "phonon"
#include <QUrl>
#include <QFile>
#include <QTime>

class SpeakGoogle : public QObject
{
    Q_OBJECT
public:
    explicit SpeakGoogle(QObject *parent = 0);
    ~SpeakGoogle();
    
signals:
    void soundEnded();
    void NetWorkUnavailable();
    void speakError();
    void fileOpenError();

public slots:
    void NetWorkIsBad();

    void stateChanged(Phonon::State newState,
                      Phonon::State oldstate);
    void startSpeak();
    void googleTTS();
    void setSoundTime(const int &soundtime);
    void setInterval(const int &interval);

    void speakGoogleTTS(const QString &text);
    void speakGoogleRestart();
    void googleTTSPlay();
    void googleTTSStop();

    Phonon::MediaObject *getMediaObject();

private slots:
    void httpFinished();  //������غ�Ĵ���
    void httpReadyRead();  //���յ�����ʱ�Ĵ���
    void haveError(QNetworkReply::NetworkError error);//����������Ĵ���

private:
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QUrl url;   //�洢�����ַ
    QFile *file;  //�ļ�ָ��
    QString soundPath;

    QTime starttime;

    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
    Phonon::State curState;

    int soundTime;
    int Interval;
    int time;
    int tryTime;
    QString wordText;
};

#endif // SPEAKGOOGLE_H
