#ifndef LEARNWORDDIALOG_H
#define LEARNWORDDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include "myglobal.h"
#include "bcmgr.h"
#include "userbookdbmgr.h"
#include "learnmgr.h"
#include "learnwordconfigdialog.h"
#include <QtNetwork>
#include "SpeakMgr.h"
#include "AnsleDictDialog.h"
//#include "Thread/thread.h"
//#include "SpeakGoogle.h"
//#include "SpeakReal.h"
///////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
namespace Ui {
class LearnWordDialog;
}

class LearnWordDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LearnWordDialog(QWidget *parent = 0, const QString &bookpath = NULL,
                             const QString &learnOrreview = NULL,
                             const bool &learnbylektion = NULL, const int &Lektion = 0);
    ~LearnWordDialog();
/*
    static int SynthCallback(short *wav, int numsamples, espeak_EVENT *events)
    {
        //����Դ��������д�ⲿ�ִ���ʵ�����������ļ����ܣ�������Ե���
        return(0);
    }
*/
private slots:
    void initAll();
    void displayTime();
    //////////////////////////////////////////////////
    //
    void on_SoundButton_clicked();

    void on_NextButton_clicked();

    void on_RememButton_clicked();

    void on_NotSureButton_clicked();

    void on_ForgetButton_clicked();

    void on_RightButton_clicked();

    void on_WrongButton_clicked();

    void on_EditWordButton_clicked();

    void on_ConfigButton_clicked();

    void on_FullCheckBox_clicked(bool checked);

    void stopThread();

    void setButtonState();

    void on_TabWidget_currentChanged(int index);

    void replyFinished(QNetworkReply *);

    void setProgress(int value);

    void hideProgressBar();

    void on_SoundEnableCheckBox_clicked(bool checked);

    void on_WordShowLabel_customContextMenuRequested(const QPoint &pos);

    void on_action_Query_triggered();

    void on_action_Add2NewWordBook_triggered();

    void on_action_SpeakMethode_triggered();

    void on_action_EndSpeak_triggered();

    void on_action__ChangeSpeakMethode_triggered();

private:
    void getConfInfo();
    void getWordByLektion(const int &lektion);
    void getWord();

    void show();
    void otherShow();



private:
    Ui::LearnWordDialog *ui;
    //////////////////////////////
    //
    bool nextButtonEnable;
    bool afterSure;

    SpeakMgr * speakMgr;
//    SoundTread *sound;
//    SpeakGoogle *speakGoogleTTS;
//    SpeakReal *speakRealPerson;

    ///////////////////////////////////////
    //����
//    QNetworkAccessManager *manager;

    ///////////////////////////////////////
    //
    int time;
    QTimer *timer;
    QTimer *timerByLektion;

    /////////////////////////////////////
    //
    QString bookPath;
    QString bookName;
    int lektion;
    int outWordId;
    bool learnByLektion;
    QString learnOrReview;
    ////////////////////////////////
    //ȡ��
    int wordCount;
    int currWordCount;
    int haveLearnedCount;
    QList<int> wordID;
    int wid;
    int rid;
    int showWordCount;
    ////////////////////////////////
    //��ϰ
    int reviewWordID;
    QStringList word;
    ////////////////////////////////
    //�ж��Լ��ж����õ�ʱ��
    int judgeTime;
    QString judgeState;
    /////////////////////////////////
    //���ʵĲ���
    int Grade;        //���ʵȼ�
    int Repetition;   //�ظ�����
    int Interval;     //���ʱ��
    double EF;        //�Ѷ�����
    /////////////////////////////////
    //���ò���
    QString learnMethod;
    QString outWordMethod;
    QString reviewWordMethod;
    int soundTime;
    int interval;
    QMap<QString, QString> shortcuts;

    AnsleDictDialog *ansleDict;
};

#endif // LEARNWORDDIALOG_H
