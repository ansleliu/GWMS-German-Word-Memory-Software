#include "learnworddialog.h"
#include "ui_learnworddialog.h"
#include <QKeySequence>
#include "addnewrecorddialog.h"
#include "QDebug"
#include "QFileInfo"
#include "QDesktopServices"
#include "QMenu"
#include "QCursor"

LearnWordDialog::LearnWordDialog(QWidget *parent, const QString &bookpath,
                                 const QString &learnOrreview,
                                 const bool &learnbylektion,
                                 const int &Lektion) :
    QDialog(parent),
    ui(new Ui::LearnWordDialog)
{
    ui->setupUi(this);
    ui->TabWidget->setCurrentIndex(0);
    ui->progressBar->hide();
    ///////////////////////////////////////////////////
    //��ȡ��Ҫ��Ϣ����ʼ��
    bookPath = bookpath;    
    learnOrReview = learnOrreview;
    learnByLektion = learnbylektion;
    lektion = Lektion;
    QFileInfo bookInfo(bookPath);
    bookName = bookInfo.baseName();
    qDebug() << QString("��ǰ���еĴʿ�Ϊ: %1").arg(bookName);

    //////////////////////////////////////////////////////////////
    //��Ƶ�ź��������
    speakMgr = new SpeakMgr(0,bookName);
    connect(speakMgr,SIGNAL(SpeakFininshed()),this,SLOT(setButtonState()));
/*
    sound = new SoundTread();
    connect(sound,SIGNAL(soundEnded()),this,SLOT(setButtonState()));
    connect(this,SIGNAL(rejected()),this,SLOT(stopThread()));

    speakGoogleTTS = new SpeakGoogle();
    connect(speakGoogleTTS,SIGNAL(soundEnded()),this,SLOT(setButtonState()));

    speakRealPerson = new SpeakReal();
    connect(speakRealPerson,SIGNAL(soundEnded()),this,SLOT(setButtonState()));
*/
    //////////////////////////////////////////////////////////////////
    //��ȡ�ʿ��������ȡ�ôʿ��е���Ϣ
    if(userBookDBMgr)
    {
        delete userBookDBMgr;
    }
    userBookDBMgr = new UserBookDBMgr(bookPath,bcMgr->getBookName(bookPath));
    if(learnByLektion)
    {
        if(learnOrReview == "Learn")
        {
            wordCount = userBookDBMgr->getWordCountByLektion(lektion);
            haveLearnedCount = learnMgr->getHaveLearnedWordCountByLektion(bookPath,lektion);
            wordID = userBookDBMgr->getWordIDByLektion(lektion);
        }
        if(learnOrReview == "Review")
        {
            wordID = learnMgr->getReviewWordIDByLektion("StateTable",bookPath,lektion);
            wordCount = wordID.count();
        }
    }
    else
    {

        if(learnOrReview == "Learn")
        {
            LearnWordConfigDialog *config;
            config = new LearnWordConfigDialog();
            config->setOutOrderlyRadioButtonUnable();

            wordCount = userBookDBMgr->getWortCount();
            haveLearnedCount = learnMgr->getHaveLearnedWordCount(bookPath);

            config->deleteLater();
            config = NULL;
        }
        if(learnOrReview == "Review")
        {
            wordID = learnMgr->getReviewWordID("StateTable",bookPath);
            wordCount = wordID.count();
        }
    }
    //////////////////////////////////////////////////////
    //������ʱ��
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(displayTime()));

    timerByLektion = new QTimer(this);
    connect(timerByLektion, SIGNAL(timeout()), this, SLOT(displayTime()));

    /////////////////////////////////////////////////
    //��ʼ��
    QTimer::singleShot(1, this, SLOT(initAll()));
}

LearnWordDialog::~LearnWordDialog()
{
    qDebug() << QString("~LearnWordDialog���ansleDict");
    if(ansleDict != NULL)
    {
        if(ansleDict->isActiveWindow())
        {
            ansleDict->close();
        }
        ansleDict->deleteLater();
        ansleDict = NULL;
    }
    qDebug() << QString("~LearnWordDialog���speakMgr");
    speakMgr->deleteLater();
    speakMgr = NULL;

    if(timer != NULL)
    {
        delete timer;
    }
    if(timerByLektion != NULL)
    {
        delete timerByLektion;
    }
    delete ui;
}

void LearnWordDialog::initAll()
{
    ///////////////////////////
    //��ʼ������
    ansleDict = NULL;
    nextButtonEnable = false;
    afterSure = false;
    time = 0;
    outWordId = 0;

    word.clear();
    reviewWordID = 0;
    judgeTime = 0;
    judgeState = "";

    Grade = 0;        //���ʵȼ�
    Repetition = 0;   //�ظ�����
    Interval = 0;     //���ʱ��
    EF = 2.50;         //�Ѷ�����
    ///////////////////////////////////
    //�����ʱ״̬���м�¼,�����¼,�������ʱ״̬��
    learnMgr->transferRecord();
    showWordCount = 0;
    //////////////////////////////////
    //��ʼ��QLCDNumber��ʾ
    QString display = "00:00:00";
    ui->lcdNumber->display(display);
    //////////////////////////////////////////
    //��ʼ��������ʾ���
    ui->WordShowLabel->clear();
    ui->TextBrowser->clear();
    ui->OITextBrowser->clear();
    /////////////////////////////////////////////////
    //��ť��ʼ����ʾ
    ui->NextButton->setEnabled(false);

    ui->RightButton->setEnabled(false);
    ui->WrongButton->setEnabled(false);

    ui->EditWordButton->setEnabled(false);
    ui->ConfigButton->setEnabled(false);

    ui->RememButton->setEnabled(true);
    ui->NotSureButton->setEnabled(true);
    ui->ForgetButton->setEnabled(true);

    ////////////////////////////////
    //��ȡ������Ϣ
    getConfInfo();
    ///////////////////////////////
    //���γ���
    currWordCount = wordCount;
    if(learnByLektion)
    {
       getWordByLektion(lektion);
    }
    else
    {
        getWord();
    }
}

void LearnWordDialog::getConfInfo()
{
    //////////////////////////////////////////////////////////
    //��ȡ������Ϣ
    learnMethod = bcMgr->userConf->getLearnMethod();
    outWordMethod = bcMgr->userConf->getOutWordMethod();
    reviewWordMethod = bcMgr->userConf->getReviewWordMethod();

    bool isOpenSound = bcMgr->userConf->getIsOpenSound();
    ui->SoundEnableCheckBox->setChecked(isOpenSound);
    if(!ui->SoundButton->isChecked())
    {
        ui->SoundButton->setEnabled(false);
        nextButtonEnable = true;
    }
    else
    {
        ui->SoundButton->setEnabled(true);
    }
    soundTime = bcMgr->userConf->getSoundTime().toInt();
    interval = bcMgr->userConf->getInterval().toInt();

    QMap<QString, QString> shortcutList = bcMgr->userConf->getShortcut();
    ui->RememButton->setShortcut(QKeySequence(shortcutList.value("RememShortcut")));
    ui->NotSureButton->setShortcut(QKeySequence(shortcutList.value("NotSureShortcut")));
    ui->ForgetButton->setShortcut(QKeySequence(shortcutList.value("ForgetShortcut")));
    ui->RightButton->setShortcut(QKeySequence(shortcutList.value("RightShortcut")));
    ui->WrongButton->setShortcut(QKeySequence(shortcutList.value("WrongShortcut")));
    ui->EditWordButton->setShortcut(QKeySequence(shortcutList.value("EditWortShortcut")));
    ui->SoundButton->setShortcut(QKeySequence(shortcutList.value("SoundShortcut")));
    ui->NextButton->setShortcut(QKeySequence(shortcutList.value("NextShortcut")));
    ui->ConfigButton->setShortcut(QKeySequence(shortcutList.value("ConfigShortcut")));
}

void LearnWordDialog::displayTime()
{
    time++;
    int hour = time/3600;
    QString hourStr;
    if( hour < 10)
    {
        hourStr = "0" + QString::number(hour,'i',0);
    }
    else
    {
        hourStr = QString::number(hour,'i',0);
    }

    int min = (time%3600)/60;
    QString minStr;
    if(min < 10)
    {
        minStr = "0" + QString::number(min,'i',0);
    }
    else
    {
        minStr = QString::number(min,'i',0);
    }

    int sec = (time%3600)%60;
    QString secStr;
    if(sec < 10)
    {
        secStr = "0" + QString::number(sec,'i',0);
    }
    else
    {
        secStr = QString::number(sec,'i',0);
    }
    QString display = hourStr + ":" + minStr + ":" + secStr;
    ui->lcdNumber->display(display);
}

void LearnWordDialog::getWordByLektion(const int &lektion)
{
    /////////////////////////////////////////////////////////////////////
    //�Ƿ���Ҫ��ϰ�ĵ���
    QList<int> reviewWordIDByLektion = learnMgr->getReviewWordIDByLektion("CurrTimeStateTable",
                                                                          bookPath,lektion);
    int reviewWordCount = reviewWordIDByLektion.count();
    /////////////////////////////////////////
    //�����Ҫ��ϰ�ĵ���
    if(reviewWordCount > 0)
    {
        reviewWordID = reviewWordIDByLektion.at(0);
        word = learnMgr->getWordAll("CurrTimeStateTable",reviewWordID);
        wid = learnMgr->getImportantInfo("CurrTimeStateTable",reviewWordID).at(0).toInt();
    }
    else
    {
        wid = 0;
        rid = 0;
        if(outWordMethod == "Random")
        {
            if(learnOrReview == "Learn")
            {
                if(showWordCount >= (wordCount-haveLearnedCount))
                {
                    if(QMessageBox::information(this,tr("��ʾ"),
                                                QString(tr("�Ѿ�û����Ҫ���еĵ���\n���˳�ѡ��������Ԫ��ʿ�"))
                                                ,QMessageBox::Yes)
                            == QMessageBox::Yes)
                    {
                        this->reject();
                        return;
                    }
                }
                else
                {
                    int random = 0;
                    do
                    {
                        ////////////////////////////////////////
                        //���������
                        QTime time;
                        time = QTime::currentTime();
                        qsrand(time.msec()+time.second()*1000);
                        random = qrand()%currWordCount;//%currWordCount ����0-(currWordCount-1)��ѡ�������
                        wid = wordID.at(random);
                        wordID.removeAt(random);
                        currWordCount--;
                    }
                    while(learnMgr->isExist("StateTable",bookPath,wid)>0);
                    showWordCount++;
                    ui->CountLcdNumber->display(showWordCount);
                }
            }
            if(learnOrReview == "Review")
            {
                if(showWordCount >= wordCount)
                {
                    if(QMessageBox::information(this,tr("��ʾ"),
                                                QString(tr("�Ѿ�û����Ҫ���еĵ���\n���˳�ѡ��������Ԫ��ʿ�"))
                                                ,QMessageBox::Yes)
                            == QMessageBox::Yes)
                    {
                        this->reject();
                        return;
                    }
                }
                else
                {
                    int random = 0;
/*                    do
                    {
                        ////////////////////////////////////////
                        //���������
                        QTime time;
                        time = QTime::currentTime();
                        qsrand(time.msec()+time.second()*1000);
                        random = qrand()%currWordCount;//%wordCount ����0-(wordCount-1)��ѡ�������
                        rid = wordID.at(random);
                        QStringList wordInfo = learnMgr->getImportantInfo("StateTable",rid);
                        int widInState = wordInfo.at(0).toInt();
                        wordId = learnMgr->isExist("CurrTimeStateTable",bookPath,widInState);
                        wordID.removeAt(random);
                        currWordCount--;
                    }
                    while(wordId > 0);*/
                    ////////////////////////////////////////
                    //���������
                    QTime time;
                    time = QTime::currentTime();
                    qsrand(time.msec()+time.second()*1000);
                    random = qrand()%currWordCount;//%wordCount ����0-(wordCount-1)��ѡ�������
                    rid = wordID.at(random);
                    wordID.removeAt(random);
                    currWordCount--;
                    showWordCount++;
                    ui->CountLcdNumber->display(showWordCount);
                }
            }
        }
        else
        {
            if(learnOrReview == "Learn")
            {
                if(showWordCount >= (wordCount-haveLearnedCount))
                {
                    if(QMessageBox::information(this,tr("��ʾ"),
                                                QString(tr("�Ѿ�û����Ҫ���еĵ���\n���˳�ѡ��������Ԫ��ʿ�"))
                                                ,QMessageBox::Yes)
                            == QMessageBox::Yes)
                    {
                        this->reject();
                        return;
                    }
                }
                else
                {
                    do
                    {
                        wid = wordID.at(0);
                        wordID.removeAt(0);
                    }
                    while(learnMgr->isExist("StateTable",bookPath,wid)>0);
                    showWordCount++;
                    ui->CountLcdNumber->display(showWordCount);
                }
            }
            if(learnOrReview == "Review")
            {
                if(showWordCount >= wordCount)
                {
                    if(QMessageBox::information(this,tr("��ʾ"),
                                                QString(tr("�Ѿ�û����Ҫ���еĵ���\n���˳�ѡ��������Ԫ��ʿ�"))
                                                ,QMessageBox::Yes)
                            == QMessageBox::Yes)
                    {
                        this->reject();
                        return;
                    }
                }
                else
                {
                    rid = wordID.at(0);
                    wordID.removeAt(0);
                    showWordCount++;
                    ui->CountLcdNumber->display(showWordCount);
                }
            }
        }
        //////////////////////////////////////////////
        //��ȡ����
        if(learnOrReview == "Learn")
        {
            word = userBookDBMgr->getWordALL(wid);
        }
        if(learnOrReview == "Review")
        {
            word = learnMgr->getWordAll("StateTable",rid);
            wid = learnMgr->getImportantInfo("StateTable",rid).at(0).toInt();
        }
    }
    //////////////////////////////////////////////
    //��ʼ��ʱ
    timerByLektion->start(1000);
    show();
}

void LearnWordDialog::getWord()
{
    /////////////////////////////////////////////////////////////////////
    //�Ƿ���Ҫ��ϰ�ĵ���
    QList<int> reviewWordIDList = learnMgr->getReviewWordID("CurrTimeStateTable",bookPath);
    int reviewWordCount = reviewWordIDList.count();
    /////////////////////////////////////////
    //�����Ҫ��ϰ�ĵ���
    if(reviewWordCount > 0)
    {
        reviewWordID = reviewWordIDList.at(0);
        word = learnMgr->getWordAll("CurrTimeStateTable",reviewWordID);
        wid = learnMgr->getImportantInfo("CurrTimeStateTable",reviewWordID).at(0).toInt();
    }
    else
    {
        wid = 0;
        rid = 0;
        if(outWordMethod == "Random")
        {
            if(learnOrReview == "Learn")
            {
                if(showWordCount >= (wordCount-haveLearnedCount))
                {
                    if(QMessageBox::information(this,tr("��ʾ"),
                                                QString(tr("�Ѿ�û����Ҫ���еĵ���\n���˳�ѡ��������Ԫ��ʿ�"))
                                                ,QMessageBox::Yes)
                            == QMessageBox::Yes)
                    {
                        this->reject();
                        return;
                    }
                }
                else
                {
                    do
                    {
                        /////////////////////////////////////////////////////////////////////////
                        //���������
                        QTime time;
                        time = QTime::currentTime();
                        qsrand(time.msec()+time.second()*1000);
                        int random = qrand()%wordCount;//%wordCount ����0-(wordCount-1)��ѡ�������
                        wid = random + 1;
                    }
                    while(learnMgr->isExist("StateTable",bookPath,wid)>0 ||
                           learnMgr->isExist("CurrTimeStateTable",bookPath,wid)>0);
                    showWordCount++;
                    ui->CountLcdNumber->display(showWordCount);
                }
            }
            if(learnOrReview == "Review")
            {
                if(showWordCount >= wordCount)
                {
                    if(QMessageBox::information(this,tr("��ʾ"),
                                                QString(tr("�Ѿ�û����Ҫ���еĵ���\n���˳�ѡ��������Ԫ��ʿ�"))
                                                ,QMessageBox::Yes)
                            == QMessageBox::Yes)
                    {
                        this->reject();
                        return;
                    }
                }
                else
                {
//                    int wordId = 0;
/*                    do
                    {
                        /////////////////////////////////////////////////////////////////////////
                        //���������
                        QTime time;
                        time = QTime::currentTime();
                        qsrand(time.msec()+time.second()*1000);
                        int random = qrand()%currWordCount;//%wordCount ����0-(wordCount-1)��ѡ�������
                        rid = wordID.at(random);
                        wordID.removeAt(random);
                        currWordCount--;
                        QStringList wordInfo = learnMgr->getImportantInfo("StateTable",rid);
                        int widInState = wordInfo.at(0).toInt();
                        wordId = learnMgr->isExist("CurrTimeStateTable",bookPath,widInState);
                    }
                    while(wordId > 0);
*/
                    /////////////////////////////////////////////////////////////////////////
                    //���������
                    QTime time;
                    time = QTime::currentTime();
                    qsrand(time.msec()+time.second()*1000);
                    int random = qrand()%currWordCount;//%wordCount ����0-(wordCount-1)��ѡ�������
                    rid = wordID.at(random);
                    wordID.removeAt(random);
                    currWordCount--;
                    showWordCount++;
                    ui->CountLcdNumber->display(showWordCount);
                }
            }
        }
        else
        {
            if(learnOrReview == "Learn")
            {
                if(outWordId >= wordCount)
                {
                    if(QMessageBox::information(this,tr("��ʾ"),
                                                QString(tr("�Ѿ�û����Ҫ���еĵ���\n���˳�ѡ��������Ԫ��ʿ�"))
                                                ,QMessageBox::Yes)
                            == QMessageBox::Yes)
                    {
                        this->reject();
                        return;
                    }
                }
                else
                {
                    do
                    {
                        outWordId++;
                    }
                    while(learnMgr->isExist("StateTable",bookPath,outWordId)>0);
                    wid = outWordId;
                    ui->CountLcdNumber->display(outWordId);
                }
            }
            if(learnOrReview == "Review")
            {
                if(showWordCount >= wordCount)
                {
                    if(QMessageBox::information(this,tr("��ʾ"),
                                                QString(tr("�Ѿ�û����Ҫ���еĵ���\n���˳�ѡ��������Ԫ��ʿ�"))
                                                ,QMessageBox::Yes)
                            == QMessageBox::Yes)
                    {
                        this->reject();
                    }
                }
                else
                {
                    rid = wordID.at(0);
                    wordID.removeAt(0);
                    showWordCount++;
                    ui->CountLcdNumber->display(showWordCount);
                }
            }
        }
        //////////////////////////////////////////////
        //��ȡ����
        if(learnOrReview == "Learn")
        {
            word = userBookDBMgr->getWordALL(wid);
        }
        if(learnOrReview == "Review")
        {
            word = learnMgr->getWordAll("StateTable",rid);
            wid = learnMgr->getImportantInfo("StateTable",rid).at(0).toInt();
        }
    }
    //////////////////////////////////////////////
    //��ʼ��ʱ
    timer->start(1000);
    show();
}

void LearnWordDialog::show()
{
    if(learnMethod == "��������")
    {
        ui->WordShowLabel->clear();
        if(reviewWordID > 0)
        {
            ui->WordShowLabel->setText(word.at(0)+"  #��ϰ");
        }
        else
        {
            ui->WordShowLabel->setText(word.at(0));
        }
        QTimer::singleShot(1, this, SLOT(on_SoundButton_clicked()));
    }
    else if(learnMethod == "�������")
    {
        ui->TextBrowser->clear();
        ui->TextBrowser->append(word.at(1));

        ui->OITextBrowser->clear();
        ui->OITextBrowser->append(word.at(2));

        QTimer::singleShot(1, this, SLOT(on_SoundButton_clicked()));
    }
    else
    {
        ui->WordShowLabel->clear();
        ui->TextBrowser->clear();
        ui->OITextBrowser->clear();
        QTimer::singleShot(1, this, SLOT(on_SoundButton_clicked()));
    }
}

void LearnWordDialog::otherShow()
{
    if(learnMethod == "��������")
    {
        ui->TextBrowser->clear();
        ui->TextBrowser->append(word.at(1));

        ui->OITextBrowser->clear();
        ui->OITextBrowser->append(word.at(2));
    }
    else if(learnMethod == "�������")
    {
        ui->WordShowLabel->clear();
        if(reviewWordID > 0)
        {
            ui->WordShowLabel->setText(word.at(0)+"  #��ϰ");
        }
        else
        {
            ui->WordShowLabel->setText(word.at(0));
        }
    }
    else
    {
        ui->WordShowLabel->clear();
        ui->TextBrowser->clear();
        ui->OITextBrowser->clear();

        if(reviewWordID > 0)
        {
            ui->WordShowLabel->setText(word.at(0)+"  #��ϰ");
        }
        else
        {
            ui->WordShowLabel->setText(word.at(0));
        }
        ui->TextBrowser->append(word.at(1));
        ui->OITextBrowser->append(word.at(2));
    }
}

void LearnWordDialog::on_SoundButton_clicked()
{    
    if(ui->SoundEnableCheckBox->isChecked())
    {
        ui->SoundButton->setEnabled(false);
        nextButtonEnable = false;
        ui->NextButton->setEnabled(false);
        //////////////////////////////////////////
        //
        speakMgr->Speak(word.at(0),interval,soundTime);

        //////////////////////////////////////////
        //
//        speakGoogleTTS->setSoundTime(soundTime);
//        speakGoogleTTS->speakGoogleTTS(word.at(0));

//        speakRealPerson->setSoundTime(soundTime);
//        speakRealPerson->realPersonSpeak(word.at(0));

        /////////////////////////////////////////
        //
//        ui->SoundButton->setEnabled(true);
//        sound->getSoundTime(soundTime);
//        sound->getWord(word.at(0));
//        sound->getTimeDiff(interval);
//        sound->start();
    }
}

void LearnWordDialog::on_NextButton_clicked()
{
    /////////////////////////////////////////
    //��ʼ����ť
    ui->RememButton->setEnabled(true);
    ui->NotSureButton->setEnabled(true);
    ui->ForgetButton->setEnabled(true);
    ui->action_Add2NewWordBook->setEnabled(true);

    ui->NextButton->setEnabled(false);

    ui->RightButton->setEnabled(false);
    ui->WrongButton->setEnabled(false);

    ui->EditWordButton->setEnabled(false);
    ui->ConfigButton->setEnabled(false);
    if(ui->SoundEnableCheckBox->isChecked())
    {
        nextButtonEnable = false;
    }
    afterSure = false;
    /////////////////////////////////////////
    //��ʼ��QLCDNumber��ʾ
    QString display = "00:00:00";
    ui->lcdNumber->display(display);
    /////////////////////////////////
    //ֹͣ��ʱ��
    if(learnByLektion)
    {
        timerByLektion->stop();
    }
    else
    {
        timer->stop();
    }
    time = 0;

    ui->WordShowLabel->clear();
    ui->TextBrowser->clear();
    ui->OITextBrowser->clear();

    word.clear();
    judgeTime = 0;
    judgeState = "";
    reviewWordID = 0;

    wid = 0;
    rid = 0;

    if(learnByLektion)
    {
        getWordByLektion(lektion);
    }
    else
    {
        getWord();
    }
}

void LearnWordDialog::on_RememButton_clicked()
{
    /////////////////////////////////
    //��ʼ����ť
    ui->RememButton->setEnabled(false);
    ui->NotSureButton->setEnabled(false);
    ui->ForgetButton->setEnabled(false);

    ui->RightButton->setEnabled(true);
    ui->WrongButton->setEnabled(true);
    ui->EditWordButton->setEnabled(false);
//    if(learnOrReview == "Learn")
    {
//       ui->EditWordButton->setEnabled(true);
    }
    /////////////////////////////////
    //ֹͣ��ʱ��
    if(learnByLektion)
    {
        timerByLektion->stop();
    }
    else
    {
        timer->stop();
    }
    judgeTime = time;
    time = 0;
    judgeState = "REMEMBER";
    ///////////////////////////////////////////////
    //��ʾ��������Ϣ
    ui->TabWidget->setCurrentWidget(ui->ConnectTab);
    otherShow();
}

void LearnWordDialog::on_NotSureButton_clicked()
{
    /////////////////////////////////
    //��ʼ����ť
    ui->RememButton->setEnabled(false);
    ui->NotSureButton->setEnabled(false);
    ui->ForgetButton->setEnabled(false);

    ui->RightButton->setEnabled(true);
    ui->WrongButton->setEnabled(true);
    ui->EditWordButton->setEnabled(false);
//    if(learnOrReview == "Learn")
    {
//        ui->EditWordButton->setEnabled(true);
    }
    /////////////////////////////////
    //ֹͣ��ʱ��
    if(learnByLektion)
    {
        timerByLektion->stop();
    }
    else
    {
        timer->stop();
    }

    judgeTime = time;
    time = 0;
    judgeState = "OBSCURE";
    ///////////////////////////////////////////////
    //��ʾ��������Ϣ
    ui->TabWidget->setCurrentWidget(ui->ConnectTab);
    otherShow();
}

void LearnWordDialog::on_ForgetButton_clicked()
{
    /////////////////////////////////
    //��ʼ����ť
    ui->RememButton->setEnabled(false);
    ui->NotSureButton->setEnabled(false);
    ui->ForgetButton->setEnabled(false);

    ui->RightButton->setEnabled(false);
    ui->WrongButton->setEnabled(false);
    ui->EditWordButton->setEnabled(true);
    ui->ConfigButton->setEnabled(true);
    afterSure = true;
    if(nextButtonEnable && afterSure)
    {
        ui->NextButton->setEnabled(true);
    }
    /////////////////////////////////
    //ֹͣ��ʱ��
    if(learnByLektion)
    {
        timerByLektion->stop();
    }
    else
    {
        timer->stop();
    }
    judgeTime = time;
    time = 0;
    judgeState = "FORGET";
    ///////////////////////////////////////////////
    //��ʾ��������Ϣ
    ui->TabWidget->setCurrentWidget(ui->ConnectTab);
    otherShow();
    //////////////////////////////////////
    //�жϵ��ʵ��Ѷȵȼ�
    Grade = 0; //���ǵõĵ��ʵȼ�Ϊ0
    //////////////////////////////////////
    //���µ���״̬��Ϣ
    /*
    int id = 0;
    if(learnOrReview == "Learn")
    {
        id = learnMgr->isExist("CurrTimeStateTable",bookPath,wid);
    }
    if(learnOrReview == "Review")
    {
        QStringList wordInfo = learnMgr->getImportantInfo("StateTable",rid);
        int widInState = wordInfo.at(0).toInt();

        id = learnMgr->isExist("CurrTimeStateTable",bookPath,widInState);
    }
*/    
    ////////////////////////////////////////////////////
    //
    if(reviewWordID != 0)
    {
        QList<QString> wordState = learnMgr->getWordState("CurrTimeStateTable",reviewWordID);
//        EF = learnMgr->countEF(wordState.at(1).toDouble(),Grade);
        ////////////////////////////////////////////////////////////////////////////////////////
        //ԭ�㷨
        /*
        QList<QString> countList = learnMgr->countInterval_Repetition(wordState.at(3).toInt(),
                                                                      wordState.at(1).toDouble()
                                                                      ,Grade,
                                                                      wordState.at(2).toInt());
        */
        QList<QString> countList = learnMgr->countInterval_Repetition_Curr_Review(wordState.at(3).toInt(),
                                                                                  wordState.at(1).toDouble()
                                                                                  ,Grade,
                                                                                  wordState.at(2).toInt());
        Interval = countList.at(0).toInt();
        Repetition = countList.at(1).toInt();
        EF = countList.at(2).toDouble();

        learnMgr->setWordState("CurrTimeStateTable",reviewWordID,Grade,EF,Repetition,Interval);
//        reviewWordID = 0;
    }
/*
    else if(id > 0)
    {
        QList<QString> wordState = learnMgr->getWordState("CurrTimeStateTable",id);
//        EF = learnMgr->countEF(wordState.at(1).toDouble(),Grade);
        QList<QString> countList = learnMgr->countInterval_Repetition(wordState.at(3).toInt(),
                                                                      wordState.at(1).toDouble()
                                                                      ,Grade,
                                                                      wordState.at(2).toInt());
        Interval = countList.at(0).toInt();
        Repetition = countList.at(1).toInt();
        EF = countList.at(2).toDouble();

        learnMgr->setWordState("CurrTimeStateTable",id,Grade,EF,Repetition,Interval);
    }
*/
    else
    {
        if(learnOrReview == "Learn")
        {
            int wordLektion = userBookDBMgr->getLektion(wid).toInt();
//            EF = learnMgr->countEF(2.50,Grade);
            QList<QString> countList = learnMgr->countInterval_Repetition_Curr_Review(0,
                                                                                      2.50
                                                                                      ,Grade,
                                                                                      0);
            Interval = countList.at(0).toInt();
            Repetition = countList.at(1).toInt();
            EF = countList.at(2).toDouble();

            learnMgr->addNewWordState("CurrTimeStateTable",bookPath,wid,word.at(0),word.at(1),word.at(2),
                                      wordLektion,EF,Grade,Repetition,Interval);
        }
        if(learnOrReview == "Review")
        {
            int wordLektion = learnMgr->getLektion("StateTable",rid);
            QStringList wordInfo = learnMgr->getImportantInfo("StateTable",rid);
            double currEF = learnMgr->getEF("StateTable",rid);
            int widInState = wordInfo.at(0).toInt();
//            EF = learnMgr->countEF(currEF,Grade);

            QList<QString> countList = learnMgr->countInterval_Repetition_Curr_Review(0,
                                                                                      currEF
                                                                                      ,Grade,
                                                                                      0);
            Interval = countList.at(0).toInt();
            Repetition = countList.at(1).toInt();
            EF = countList.at(2).toDouble();

            learnMgr->addNewWordState("CurrTimeStateTable",bookPath,widInState,word.at(0),word.at(1),word.at(2),
                                      wordLektion,EF,Grade,Repetition,Interval);
        }
    }
}

void LearnWordDialog::on_RightButton_clicked()
{
    /////////////////////////////////////////
    //��ʼ����ť
    ui->WrongButton->setEnabled(false);
    ui->RightButton->setEnabled(false);
    ui->EditWordButton->setEnabled(true);
    ui->ConfigButton->setEnabled(true);
    afterSure = true;
    if(nextButtonEnable && afterSure)
    {
        ui->NextButton->setEnabled(true);
    }
    ///////////////////////////////////////////////
    //�жϵ��ʵ��Ѷȵȼ�
    if(judgeState == "REMEMBER")
    {
        if(judgeTime <= 4)
        {
            Grade = 5;
        }
        else if(judgeTime <= 8 && judgeTime > 4)
        {
            Grade = 4;
        }
        else if(judgeTime <= 12 && judgeTime > 8)
        {
            Grade = 3;
        }
        else
        {
            Grade = 2;
        }
    }
    if(judgeState == "OBSCURE")
    {
        if(judgeTime <= 8)
        {
            Grade = 3;
        }
        else if(judgeTime <= 16 && judgeTime > 8)
        {
            Grade = 2;
        }
        else
        {
            Grade = 1;
        }
    }
    //////////////////////////////////////
    //���µ���״̬��Ϣ
/*
    int id = 0;
    if(learnOrReview == "Learn")
    {
        id = learnMgr->isExist("CurrTimeStateTable",bookPath,wid);
    }
    if(learnOrReview == "Review")
    {
        QStringList wordInfo = learnMgr->getImportantInfo("StateTable",rid);
        int widInState = wordInfo.at(0).toInt();

        id = learnMgr->isExist("CurrTimeStateTable",bookPath,widInState);
    }
*/
    ////////////////////////////////////////////////////
    //
    if(reviewWordID != 0)
    {
        QList<QString> wordState = learnMgr->getWordState("CurrTimeStateTable",reviewWordID);
//        EF = learnMgr->countEF(wordState.at(1).toDouble(),Grade);
        ////////////////////////////////////////////////////////////////////////////////////////
        //ԭ�㷨
        /*
        QList<QString> countList = learnMgr->countInterval_Repetition(wordState.at(3).toInt(),
                                                                      wordState.at(1).toDouble()
                                                                      ,Grade,
                                                                      wordState.at(2).toInt());
        */
        QList<QString> countList = learnMgr->countInterval_Repetition_Curr_Review(wordState.at(3).toInt(),
                                                                                  wordState.at(1).toDouble()
                                                                                  ,Grade,
                                                                                  wordState.at(2).toInt());
        Interval = countList.at(0).toInt();
        Repetition = countList.at(1).toInt();
        EF = countList.at(2).toDouble();

        learnMgr->setWordState("CurrTimeStateTable",reviewWordID,Grade,EF,Repetition,Interval);
//        reviewWordID = 0;
    }
/*
    else if(id > 0)
    {
        QList<QString> wordState = learnMgr->getWordState("CurrTimeStateTable",id);
//        EF = learnMgr->countEF(wordState.at(1).toDouble(),Grade);
        QList<QString> countList = learnMgr->countInterval_Repetition(wordState.at(3).toInt(),
                                                                      wordState.at(1).toDouble()
                                                                      ,Grade,
                                                                      wordState.at(2).toInt());
        Interval = countList.at(0).toInt();
        Repetition = countList.at(1).toInt();
        EF = countList.at(2).toDouble();

        learnMgr->setWordState("CurrTimeStateTable",id,Grade,EF,Repetition,Interval);
    }
*/
    else
    {
        if(learnOrReview == "Learn")
        {
            int wordLektion = userBookDBMgr->getLektion(wid).toInt();
//            EF = learnMgr->countEF(2.50,Grade);
            QList<QString> countList = learnMgr->countInterval_Repetition_Curr_Review(0,
                                                                                      2.50
                                                                                      ,Grade,
                                                                                      0);
            Interval = countList.at(0).toInt();
            Repetition = countList.at(1).toInt();
            EF = countList.at(2).toDouble();

            learnMgr->addNewWordState("CurrTimeStateTable",bookPath,wid,word.at(0),word.at(1),word.at(2),
                                      wordLektion,EF,Grade,Repetition,Interval);
        }
        if(learnOrReview == "Review")
        {
            int wordLektion = learnMgr->getLektion("StateTable",rid);
            QStringList wordInfo = learnMgr->getImportantInfo("StateTable",rid);
            double currEF = learnMgr->getEF("StateTable",rid);
            int widInState = wordInfo.at(0).toInt();
//            EF = learnMgr->countEF(currEF,Grade);

            QList<QString> countList = learnMgr->countInterval_Repetition_Curr_Review(0,
                                                                                      currEF
                                                                                      ,Grade,
                                                                                      0);
            Interval = countList.at(0).toInt();
            Repetition = countList.at(1).toInt();
            EF = countList.at(2).toDouble();

            learnMgr->addNewWordState("CurrTimeStateTable",bookPath,widInState,word.at(0),word.at(1),word.at(2),
                                      wordLektion,EF,Grade,Repetition,Interval);
        }
    }
}

void LearnWordDialog::on_WrongButton_clicked()
{
    /////////////////////////////////////////
    //��ʼ����ť
    ui->RightButton->setEnabled(false);
    ui->WrongButton->setEnabled(false);
    ui->EditWordButton->setEnabled(true);
    ui->ConfigButton->setEnabled(true);
    afterSure = true;
    if(nextButtonEnable && afterSure)
    {
        ui->NextButton->setEnabled(true);
    }
    ///////////////////////////////////////////////
    //�жϵ��ʵ��Ѷȵȼ�
    Grade = 0;
    //////////////////////////////////////
    //���µ���״̬��Ϣ
    //////////////////////////////////////
    //���µ���״̬��Ϣ
/*
    int id = 0;
    if(learnOrReview == "Learn")
    {
        id = learnMgr->isExist("CurrTimeStateTable",bookPath,wid);
    }
    if(learnOrReview == "Review")
    {
        QStringList wordInfo = learnMgr->getImportantInfo("StateTable",rid);
        int widInState = wordInfo.at(0).toInt();

        id = learnMgr->isExist("CurrTimeStateTable",bookPath,widInState);
    }
*/
    ////////////////////////////////////////////////////
    //
    if(reviewWordID != 0)
    {
        QList<QString> wordState = learnMgr->getWordState("CurrTimeStateTable",reviewWordID);
//        EF = learnMgr->countEF(wordState.at(1).toDouble(),Grade);
        ////////////////////////////////////////////////////////////////////////////////////////
        //ԭ�㷨
        /*
        QList<QString> countList = learnMgr->countInterval_Repetition(wordState.at(3).toInt(),
                                                                      wordState.at(1).toDouble()
                                                                      ,Grade,
                                                                      wordState.at(2).toInt());
        */
        QList<QString> countList = learnMgr->countInterval_Repetition_Curr_Review(wordState.at(3).toInt(),
                                                                                  wordState.at(1).toDouble()
                                                                                  ,Grade,
                                                                                  wordState.at(2).toInt());
        Interval = countList.at(0).toInt();
        Repetition = countList.at(1).toInt();
        EF = countList.at(2).toDouble();

        learnMgr->setWordState("CurrTimeStateTable",reviewWordID,Grade,EF,Repetition,Interval);
//        reviewWordID = 0;
    }
/*
    else if(id > 0)
    {
        QList<QString> wordState = learnMgr->getWordState("CurrTimeStateTable",id);
//        EF = learnMgr->countEF(wordState.at(1).toDouble(),Grade);
        QList<QString> countList = learnMgr->countInterval_Repetition(wordState.at(3).toInt(),
                                                                      wordState.at(1).toDouble()
                                                                      ,Grade,
                                                                      wordState.at(2).toInt());
        Interval = countList.at(0).toInt();
        Repetition = countList.at(1).toInt();
        EF = countList.at(2).toDouble();

        learnMgr->setWordState("CurrTimeStateTable",id,Grade,EF,Repetition,Interval);
    }
*/
    else
    {
        if(learnOrReview == "Learn")
        {
            int wordLektion = userBookDBMgr->getLektion(wid).toInt();
//            EF = learnMgr->countEF(2.50,Grade);
            QList<QString> countList = learnMgr->countInterval_Repetition_Curr_Review(0,
                                                                                      2.50
                                                                                      ,Grade,
                                                                                      0);
            Interval = countList.at(0).toInt();
            Repetition = countList.at(1).toInt();
            EF = countList.at(2).toDouble();

            learnMgr->addNewWordState("CurrTimeStateTable",bookPath,wid,word.at(0),word.at(1),word.at(2),
                                      wordLektion,EF,Grade,Repetition,Interval);
        }
        if(learnOrReview == "Review")
        {
            int wordLektion = learnMgr->getLektion("StateTable",rid);
            QStringList wordInfo = learnMgr->getImportantInfo("StateTable",rid);
            double currEF = learnMgr->getEF("StateTable",rid);
            int widInState = wordInfo.at(0).toInt();
//            EF = learnMgr->countEF(currEF,Grade);

            QList<QString> countList = learnMgr->countInterval_Repetition_Curr_Review(0,
                                                                                      currEF
                                                                                      ,Grade,
                                                                                      0);
            Interval = countList.at(0).toInt();
            Repetition = countList.at(1).toInt();
            EF = countList.at(2).toDouble();

            learnMgr->addNewWordState("CurrTimeStateTable",bookPath,widInState,word.at(0),word.at(1),word.at(2),
                                      wordLektion,EF,Grade,Repetition,Interval);
        }
    }
}

void LearnWordDialog::on_EditWordButton_clicked()
{
    AddNewRecordDialog *editWort;
    editWort = new AddNewRecordDialog(this);
    editWort->setWindowTitle(tr("  �༭�ô���  "));
    editWort->setWindowIcon(QIcon("./rc/edit_rename.png"));
    editWort->setGroupTitle(tr("�༭���Ѵ��ڴ���"));
    editWort->setLektionLineEditRradOnly();
    //////////////////////////////////////////////////////////
    //��ȡ����
    qDebug() << QString("Ҫ�޸ĵĵ��ʵı��Ϊ��");
    qDebug() << QString::number(wid,'i',0);
    qDebug() << QString("Ҫ�޸ĵĵ���Ϊ��");
    qDebug() << word.at(0);

    editWort->setWord(word.at(0));
    editWort->setMean(word.at(1));
    editWort->setOther(word.at(2));

    qDebug() << QString("Ҫ�޸ĵĵ���������ԪΪ��");
    qDebug() << userBookDBMgr->getLektion(wid);
    editWort->setLektion(userBookDBMgr->getLektion(wid).toInt());
    //////////////////////////////////////////////////////////////////////
    //��ʼ�޸�
    if(editWort->exec() == QDialog::Accepted)
    {
        //////////////////////////////////////////////////////////////////
        //�����¼
        word.clear();
        word.append(editWort->getWord());
        word.append(editWort->getMean());
        word.append(editWort->getOther());
        qDebug() << QString("���ʻ�����³ɹ���");
        qDebug() << word;

        {
            ////////////////////////////////////////////////////////
            //���´ʿ��еĵ�����Ϣ
            bool uw = userBookDBMgr->updateWord(editWort->getWord(),wid);
            bool um = userBookDBMgr->updateMean(editWort->getMean(),wid);
            bool uc = userBookDBMgr->updateConnection(editWort->getOther(),wid);
            bool ul = userBookDBMgr->updateLektion(QString::number(editWort->getLektion(),'i',0),wid);
            /////////////////////////////////////////////////
            //���ʻ��޸ĳɹ�
            if(uw == 1 && um == 1 && uc == 1 && ul ==1)
            {
                ////////////////////////////////////////////////////////
                //�ڱ����ʽ�������ʾ�޸Ĺ���ĵ���
                ui->WordShowLabel->clear();
                if(reviewWordID > 0)
                {
                    ui->WordShowLabel->setText(editWort->getWord()+"  #��ϰ");
                }
                else
                {
                    ui->WordShowLabel->setText(editWort->getWord());
                }

                ui->TextBrowser->clear();
                ui->TextBrowser->append(editWort->getMean());

                ui->OITextBrowser->clear();
                ui->OITextBrowser->append(editWort->getOther());
                qDebug() << QString("�޸ı����ʽ���ʻ���Ϣ�ɹ�");
                //////////////////////////////////////////////////////
                //
                qDebug() << QString("�޸Ĵʿ��е��ʼ�¼�ɹ�");
                ////////////////////////////////////////////////////////
                //������ʱѧϰ��¼�еĵ�����Ϣ
                int idCT = learnMgr->isExist("CurrTimeStateTable",bookPath,wid);
                if(idCT > 0)
                {
                    QStringList wordall;
                    wordall.append(editWort->getWord());
                    wordall.append(editWort->getMean());
                    wordall.append(editWort->getOther());
                    wordall.append(QString::number(editWort->getLektion(),'i',0));

                    bool swa = learnMgr->setWordAll("CurrTimeStateTable",wid,bookPath,wordall);
                    if(swa == 1)
                    {
                        qDebug() << QString("�޸���ʱ��¼���е��ʼ�¼�ɹ�");
                    }
                    else
                    {
                        qDebug() << QString("�޸���ʱ��¼���е��ʼ�¼ʧ��");
                    }
                }

                ////////////////////////////////////////////////////////
                //����ѧϰ��¼�еĵ�����Ϣ
                int idST = learnMgr->isExist("StateTable",bookPath,wid);
                if(idST > 0)
                {
                    QStringList wordall;
                    wordall.append(editWort->getWord());
                    wordall.append(editWort->getMean());
                    wordall.append(editWort->getOther());
                    wordall.append(QString::number(editWort->getLektion(),'i',0));

                    bool swa = learnMgr->setWordAll("StateTable",wid,bookPath,wordall);
                    if(swa == 1)
                    {
                        qDebug() << QString("�޸ļ�¼���е��ʼ�¼�ɹ�");
                    }
                    else
                    {
                        qDebug() << QString("�޸ļ�¼���е��ʼ�¼ʧ��");
                    }
                }
            }
            else
            {
                qDebug() << QString("�޸Ĵʿ��е��ʼ�¼ʧ��");
            }
            /////////////////////////////////
            //����
        }//
    }
    ///////////////////////////////////////////////////////////////////
    //���ں�������
    editWort->close();
    editWort->deleteLater();
    editWort = NULL;
}

void LearnWordDialog::on_ConfigButton_clicked()
{
    /////////////////////////////////////////////////////
    //���ÿ�ʼ
    LearnWordConfigDialog *configDialog;
    configDialog = new LearnWordConfigDialog(this);
    if(learnByLektion)
    {
    }
    else
    {
        if(learnOrReview == "Learn")
        {
            configDialog->setOutOrderlyRadioButtonUnable();
        }
    }
    if(configDialog->exec() == QDialog::Accepted)
    {
        //////////////////////////////////////////////
        //��ȡ������Ϣ
        getConfInfo();
        ///////////////////////////////////////////////
        //ȡ��
        on_NextButton_clicked();

        configDialog->close();
        delete configDialog;
        configDialog = NULL;
    }
    else
    {
        configDialog->close();
        delete configDialog;
        configDialog = NULL;
    }
}

void LearnWordDialog::on_FullCheckBox_clicked(bool checked)
{
    if(checked)
    {
        QDialog::showFullScreen();
    }
    else
    {
        QDialog::showNormal();
    }
}

void LearnWordDialog::stopThread()
{
    /*
    if (sound->isRunning())
    {
        sound->stop();
        sound->quit();
    }
    */
}

void LearnWordDialog::setButtonState()
{
    if(ui->SoundEnableCheckBox->isChecked())
    {
//        sound->quit();
        ui->SoundButton->setEnabled(true);
    }
    nextButtonEnable = true;
    if(afterSure && nextButtonEnable)
    {
        ui->NextButton->setEnabled(true);
    }
}

void LearnWordDialog::on_TabWidget_currentChanged(int index)
{
    if(index == 1)
    {
       QNetworkProxyFactory::setUseSystemConfiguration(true);
       QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
       QString location =QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
       diskCache->setCacheDirectory(location);
       QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager(this);
       networkAccessManager->setCache(diskCache);

       QUrl url = QUrl::fromEncoded("http://www.godic.net");
       QString searchWord = QString("/dicts/de/%1").arg(word.at(0));
       QByteArray addWordText = searchWord.toLatin1();
       url.setEncodedPath(addWordText);
       ui->WebView->load(url);
       ui->WebView->show();
       connect(ui->WebView, SIGNAL(loadProgress(int)),this, SLOT(setProgress(int)));
       connect(ui->WebView, SIGNAL(loadFinished(bool)),this, SLOT(hideProgressBar()));
    }
}

void LearnWordDialog::replyFinished(QNetworkReply *)
{

}

void LearnWordDialog::setProgress(int value)
{
    ui->progressBar->setHidden(false);
    ui->progressBar->setValue(value);
}

void LearnWordDialog::hideProgressBar()
{
    ui->progressBar->hide();
    QWebPage *page = ui->WebView->page();
    page->setContentEditable(true);
}

void LearnWordDialog::on_SoundEnableCheckBox_clicked(bool checked)
{
    if(!checked)
    {
        ui->SoundButton->setEnabled(false);
        nextButtonEnable = true;
    }
    else
    {
        ui->SoundButton->setEnabled(true);
    }
}

void LearnWordDialog::on_WordShowLabel_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu *popMenu =new QMenu(this);//����һ���Ҽ������˵�
    popMenu->addAction(ui->action_Query);
    popMenu->addAction(ui->action_Add2NewWordBook);
    popMenu->addAction(ui->action_EndSpeak);
//    popMenu->addAction(ui->action_SpeakMethode);
//    popMenu->addAction(ui->action__ChangeSpeakMethode);
    popMenu->exec(QCursor::pos());//�����Ҽ��˵����˵�λ��Ϊ���λ��

    delete popMenu;
    popMenu = NULL;
}

void LearnWordDialog::on_action_Query_triggered()
{
    if(ansleDict != NULL)
    {
        ansleDict->close();
        ansleDict->deleteLater();
        ansleDict = NULL;
    }
    ansleDict = new AnsleDictDialog(word.at(0),this);
    ansleDict->setModal(false);
//    if(ansleDict->exec() == QDialog::Rejected)
//    {
//        ansleDict->close();
//        ansleDict->deleteLater();
//        ansleDict = NULL;
//    }
    ansleDict->setModal(false);
    ansleDict->show();

    ansleDict = NULL;
}

void LearnWordDialog::on_action_Add2NewWordBook_triggered()
{
    QString myWord = ui->WordShowLabel->text();
    qDebug() << QString("������ %1 �����ҵ����ʿ�").arg(myWord);
    if(bcMgr->myBook->isExist(myWord))
    {
        QMessageBox::information(this, "��ʾ��Ϣ", "�õ��������ʱ����Ѿ�����,�����ظ����");
        ui->action_Add2NewWordBook->setEnabled(false);
    }
    else
    {
        QString mean = word.at(1);
        QString other = word.at(2);

        int count = bcMgr->myBook->getRowCount();
        int lektion = (count+1)/200 + 1;

        bcMgr->myBook->addNewRecord(myWord,mean,other,lektion);
        ui->action_Add2NewWordBook->setEnabled(false);
    }
}

void LearnWordDialog::on_action_SpeakMethode_triggered()
{

}

void LearnWordDialog::on_action_EndSpeak_triggered()
{
    speakMgr->Stop();
    ui->SoundButton->setEnabled(true);
    if(ui->EditWordButton->isEnabled())
    {
        nextButtonEnable = true;
        ui->NextButton->setEnabled(true);
    }
    else
    {
        nextButtonEnable = true;
    }
}

void LearnWordDialog::on_action__ChangeSpeakMethode_triggered()
{

}
