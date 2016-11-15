#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "editkeydialog.h"
#include "findkeydialog.h"
#include "addbookdialog.h"
#include "deletebookdialog.h"
#include "SpeakTTS.h"

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    ///////////////////////////////////////
    //��ʼ������ֵ
    speakMethod = "";
    sysSpeakSpeed = 180;
    sysSpeakRate = 0;
    sysSpeakVolume = 100;
    sysSpeakPitch = 50;
    sysSpeakPitchRange = 50;
    sysSpeakWordGap = 1;
    testSound = NULL;
    speech = NULL;
    //////////////////////////////////
    //�������ֳ�ʼ��
    initAll();
}

OptionsDialog::~OptionsDialog()
{
    if(userBookDBMgr != NULL)
    {
        delete userBookDBMgr;
    }
    userBookDBMgr = NULL;

    if(speech != NULL)
    {
        speech->deleteLater();
    }
    speech = NULL;
    qDebug() << QString("���QtSpeech�����ɹ�");

    delete userDataModel;
    delete studyStateModel;

    delete ui;
}

void OptionsDialog::initAll()
{
    ///////////////////////////////////////////////////////
    //�û�����
    currUser = Global::Conf->getCurrentUser();
    ui->userLineEdit->setText(currUser);
    QString createTime = Global::UserMag->getUserInfo(QString("createTime")
                                                      ,Global::Conf->getCurrentUser())
                                                      .toString();
    QDateTime createtime = QDateTime::fromString(createTime,"yyyy-MM-dd hh:mm:ss");
    ui->creatLineEdit->setText(createTime);
    QString lastLoginTime  = Global::UserMag->getUserInfo(QString("lastLoginTime")
                                                          ,Global::Conf->getCurrentUser())
                                                          .toString();
    QDateTime lastlogintime = QDateTime::fromString(lastLoginTime,"yyyy-MM-dd hh:mm:ss");
    ui->lastLineEdit->setText(lastLoginTime);

    bool bAutoLogin = Global::Conf->getAutoLogin();
    ui->autoLoginCheckBox->setCheckState(bAutoLogin ? Qt::Checked : Qt::Unchecked);
    bool bSavePsw = Global::UserMag->getUserInfo(QString("savePsw"), currUser).toInt();
    ui->rememKeyCheckBox->setCheckState(bSavePsw ? Qt::Checked : Qt::Unchecked);
    int timeDiff = createtime.daysTo(lastlogintime);
//    QString timeDiffStr = QString::number(timeDiff,'i',0);
    int loginCount = Global::UserMag->getLoginCount(currUser);
    QString info = QString(tr("�˻�%1�Ѿ����� %2 ��,�ۼƵ�¼���� %3 ��"))
            .arg(currUser)
            .arg(timeDiff)
            .arg(loginCount);
    ui->infolabel->setText(info);
    ////////////////////////////////////////////////////////////
    //�ʿ����
    userDataModel = bcMgr->userDataDBModel(this);
    ui->bookTableView->setModel(userDataModel);
    ui->bookTableView->hideColumn(0);
    ui->bookTableView->hideColumn(1);
    ////////////////////////////////////////////////////////////
    //��¼����
    studyStateModel = learnMgr->studyStateDBModel(this);
    studyStateModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    ui->studyRecordTableView->setModel(studyStateModel);
    //����ĳЩ��ֻ��ί��
    ui->studyRecordTableView->setItemDelegateForColumn(0,new readonlyDelegate(this));
    ui->studyRecordTableView->setItemDelegateForColumn(1,new readonlyDelegate(this));
    ui->studyRecordTableView->setItemDelegateForColumn(2,new readonlyDelegate(this));
    ui->studyRecordTableView->setItemDelegateForColumn(3,new readonlyDelegate(this));
    ui->studyRecordTableView->setItemDelegateForColumn(4,new readonlyDelegate(this));
    ui->studyRecordTableView->setItemDelegateForColumn(5,new readonlyDelegate(this));
    ui->studyRecordTableView->setItemDelegateForColumn(6,new readonlyDelegate(this));
    ui->studyRecordTableView->setItemDelegateForColumn(8,new readonlyDelegate(this));
    ui->studyRecordTableView->setItemDelegateForColumn(9,new readonlyDelegate(this));
    ui->studyRecordTableView->setItemDelegateForColumn(11,new readonlyDelegate(this));
    //����Grade��ί��
    ui->studyRecordTableView->setItemDelegateForColumn(7,new GradeDelegate(this));
    //����Repetition��ί��
    ui->studyRecordTableView->setItemDelegateForColumn(10,new RepetitionDelegate(this));
    //�������һ������
    ui->studyRecordTableView->hideColumn(12);
    ////////////////////////////////////////////////////////////
    //ѧϰ����
    ui->ConfigTabWidget->setCurrentWidget(ui->LearnTab);

    QString LearnMethodInConf = bcMgr->userConf->getLearnMethod();
    int index = ui->LearnMethodComboBox->findText(LearnMethodInConf);
    ui->LearnMethodComboBox->setCurrentIndex(index);

    if(bcMgr->userConf->getOutWordMethod() == "Random")
    {
        ui->OutRandomRadioButton->click();
    }
    else
    {
        ui->OutOrderlyRadioButton->click();
    }
    if(bcMgr->userConf->getReviewWordMethod() == "Random")
    {
        ui->ReviewRandomRadioButton->click();
    }
    else
    {
        ui->ReviewOrderlyRadioButton->click();
    }
    /////////////////////////////////////////////////////////
    //��������
    //////////////////////////////////////////////////////
    //�����������
    bool isOpenSound = bcMgr->userConf->getIsOpenSound();
    ui->SoundCheckBox->setChecked(isOpenSound);
    ////////////////////////////////////////////////
    //��ȡһ��������ʽ
    speakMethod = bcMgr->userConf->getSpeakMethod();
    if(speakMethod == "LocalTTS")
    {
        ui->TTSRadioButton->click();
    }
    if(speakMethod == "googleTTS")
    {
        ui->OnlineRadioButton->click();
    }
    if(speakMethod == "realSpeak&eSpeakTTS")
    {
        ui->Real_TTSRadioButton->click();
    }
    //��������
    QString soundTime = bcMgr->userConf->getSoundTime();
    int soundIndex = ui->TimeComboBox->findText(soundTime);
    ui->TimeComboBox->setCurrentIndex(soundIndex);
    //�������
    QString interval = bcMgr->userConf->getInterval();
    int intervalIndex = ui->IntervalComboBox->findText(interval);    
    ui->IntervalComboBox->setCurrentIndex(intervalIndex);
    ////////////////////////////////////////////////////////////////////////
    //���ö�������
    //��ʼ���������������б�
    TTSComboBoxInit();
    //���ö�����������ĵ�ǰ����ֵ
    localTTS = bcMgr->userConf->getSpeakLocalTTS();
    qDebug() << QString("��ȡ����֮ǰ���õĶ�����������Ϊ��%1").arg(localTTS);
    qDebug() << QString("���õ�ǰ������������Ϊ��%1").arg(localTTS);
    ui->TTSComboBox->setCurrentIndex(ui->TTSComboBox->findText(localTTS));

    if(localTTS == QString("eSpeakTTS"))
    {
        eSpeakTTSInit();
        /////////////////////////////////////////////////////////////
        //��������
//        QString speakSpeed = bcMgr->userConf->getSpeakSpeed();
//        ui->SSpeedSlider->setValue(speakSpeed.toInt());
//        currSpeakSpeed = speakSpeed.toInt();
    }
    else
    {
        LocalTTSInit();
        /////////////////////////////////////////////////////////////
        //��������
 //       QString speakRate = bcMgr->userConf->getSpeakRate();
 //       ui->SSpeedSlider->setValue(speakRate.toInt());
 //       currSpeakRate = speakRate.toInt();
    }
    /////////////////////////////////////////////////////////////
    //��������
    QString speakVolume = bcMgr->userConf->getSpeakVolume();
    ui->SVolumeSlider->setValue(speakVolume.toInt());
    //��������
    QString speakPitch = bcMgr->userConf->getSpeakPitch();
    ui->SPitchSlider->setValue(speakPitch.toInt());
    //��������
    QString speakPitchRange = bcMgr->userConf->getSpeakPitchRange();
    ui->PRangeSlider->setValue(speakPitchRange.toInt());
    //���ü��
    QString wordGap = bcMgr->userConf->getSpeakWordGap();
    ui->GapSlider->setValue(wordGap.toInt());
    ////////////////////////////////////////////
    //
    currSpeakVolume = speakVolume.toInt();
    currSpeakPitch = speakPitch.toInt();
    currSpeakPitchRange = speakPitchRange.toInt();
    currSpeakWordGap = wordGap.toInt();

    /////////////////////////////////////////////////////////////////////
    //��ݼ�
    QMap<QString, QString> shortcutList = bcMgr->userConf->getShortcut();
    ui->RememLineEdit->setText(shortcutList.value("RememShortcut"));
    ui->ObscureLineEdit->setText(shortcutList.value("NotSureShortcut"));
    ui->ForgetLineEdit->setText(shortcutList.value("ForgetShortcut"));
    ui->RightLineEdit->setText(shortcutList.value("RightShortcut"));
    ui->WrongLineEdit->setText(shortcutList.value("WrongShortcut"));
    ui->EditWordLineEdit->setText(shortcutList.value("EditWortShortcut"));
    ui->SoundLineEdit->setText(shortcutList.value("SoundShortcut"));
    ui->NextLineEdit->setText(shortcutList.value("NextShortcut"));
    ui->ConfigLineEdit->setText(shortcutList.value("ConfigShortcut"));
}

void OptionsDialog::TTSComboBoxInit()
{
    foreach(QtSpeech::VoiceName v, QtSpeech::voices())
    {
        qDebug() << "id:" << v.id;
        qDebug() << "name:" << v.name;
        //////////////////////////////////////////////////////
        //���Ե�Ʒ�ʷ�������Ͳ��ɹ����ķ�������
        if(v.name == QString("Microsoft Mary") ||
                v.name == QString("Microsoft Mike") ||
                v.name == QString("Sample TTS Voice"))
        {
            qDebug() << QString("����Microsoft Mary , Microsoft Mike �� Sample TTS Voice");
            qDebug() << QString("����: %1 �ɹ�").arg(v.name);
        }
        else
        {
            ui->TTSComboBox->addItem(v.name);
        }
    }
}

void OptionsDialog::LocalTTSInit()
{
    ui->SSpeedSlider->setRange(-10,10);
    ui->SSpeedSlider->setPageStep(2);

    ui->SSpeedSlider->setValue(bcMgr->userConf->getSpeakRate().toInt());
    currSpeakRate = bcMgr->userConf->getSpeakRate().toInt();

    ui->SPitchSlider->setEnabled(false);
//    ui->SSpeedSlider->setEnabled(false);
    ui->GapSlider->setEnabled(false);
    ui->PRangeSlider->setEnabled(false);
    ui->SVolumeSlider->setEnabled(false);

//    ui->TestSondButton->setEnabled(false);
//    ui->BackToSysButton->setEnabled(false);
}

void OptionsDialog::eSpeakTTSInit()
{
    ui->SSpeedSlider->setRange(80,450);
    ui->SSpeedSlider->setPageStep(10);
    ui->SSpeedSlider->setValue(bcMgr->userConf->getSpeakSpeed().toInt());
    currSpeakSpeed = bcMgr->userConf->getSpeakSpeed().toInt();

    ui->SPitchSlider->setEnabled(true);
    ui->SSpeedSlider->setEnabled(true);
    ui->GapSlider->setEnabled(true);
    ui->PRangeSlider->setEnabled(true);
    ui->SVolumeSlider->setEnabled(true);

//    ui->TestSondButton->setEnabled(true);
//    ui->BackToSysButton->setEnabled(true);
}

void OptionsDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->tabWidget->setCurrentIndex(currentRow);
}

void OptionsDialog::on_tabWidget_currentChanged(int index)
{
    ui->listWidget->setCurrentRow(index);
}

void OptionsDialog::on_reviseKeyButton_clicked()
{
    EditKeyDialog *editKey;
    editKey = new EditKeyDialog(this,currUser);
    if(editKey->exec() == QDialog::Accepted)
    {

    }
}

void OptionsDialog::on_forgetKeyButton_clicked()
{
    FindKeyDialog *findKey;
    findKey = new FindKeyDialog(this,currUser);
    if(findKey->exec() == QDialog::Accepted)
    {
        findKey->close();
    }
    else
    {
        findKey->close();
    }
    delete findKey;
    findKey = NULL;
}

void OptionsDialog::on_autoLoginCheckBox_clicked(bool checked)
{
    if (checked)
    {
        ui->rememKeyCheckBox->setCheckState(Qt::Checked);
    }
    //�������ļ��������Զ���¼״̬
    Global::Conf->setAutoLogin(ui->autoLoginCheckBox->checkState());
}

void OptionsDialog::on_rememKeyCheckBox_clicked(bool checked)
{
    Global::UserMag->updateSavePsw(currUser, checked);
    //�����ݿ��и��������¼״̬
    if (!checked)
    {
        ui->autoLoginCheckBox->setCheckState(Qt::Unchecked);
    }
}

void OptionsDialog::on_addButton_clicked()
{
    AddBookDialog *addDiag;
    addDiag = new AddBookDialog(this);
    if(addDiag->exec() == QDialog::Accepted)
    {
        if (QMessageBox::information(this, tr("��ʾ��Ϣ"), tr("��ȷ��Ҫ����´ʿ���?"),
                                     QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            QString bookName = addDiag->getBookName();
            QString className = addDiag->getClassName();
            QString bookPath = addDiag->getBookPath();

            if(userBookDBMgr != NULL)
            {
                delete userBookDBMgr;
            }
            userBookDBMgr = new UserBookDBMgr(bookPath,bcMgr->getBookName(bookPath));
            int wordNum = userBookDBMgr->getWortCount();

            if(bcMgr->addNewBook(className,bookName,bookPath,wordNum))
            {
                ui->bookTableView->setModel(userDataModel);
                addDiag->close();
                delete addDiag;
                addDiag = NULL;
            }
            else
            {
                QMessageBox::warning(this,tr("����"),tr("\n�ôʿ��Ѿ�����!"));

                addDiag->close();
                delete addDiag;
                addDiag = NULL;
            }
        }
    }
    else
    {
        addDiag->close();
        delete addDiag;
        addDiag = NULL;
    }
}

void OptionsDialog::on_removeButton_clicked()
{
    QModelIndex currIndex = ui->bookTableView->currentIndex();
    if(currIndex.isValid())
    {
        if (QMessageBox::information(this, tr("��ʾ��Ϣ"), tr("��ȷ��Ҫɾ������ʿ���?"),
                                     QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            int row = currIndex.row();

            int x = ui->bookTableView->columnViewportPosition(2);
            int y = ui->bookTableView->rowViewportPosition(row);
            QPoint point = QPoint(x,y);
            QModelIndex index = ui->bookTableView->indexAt(point);
            const QString &bookClass = QString(index.data().toString());

            int xn = ui->bookTableView->columnViewportPosition(3);
            QPoint pointn = QPoint(xn,y);
            QModelIndex indexn = ui->bookTableView->indexAt(pointn);
            const QString &bookName = QString(indexn.data().toString());

            QString bookpath = bcMgr->getBookPaths(bookClass,bookName);
            if(learnMgr->isExist("StateTable",bookpath))
            {
                learnMgr->deleteRecord("StateTable",bookpath);
            }

            bcMgr->deleteBook(bookClass,bookName);
            userDataModel->select();
//            ui->bookTableView->setModel(userDataModel);

            studyStateModel->select();
//            userDataModel->removeRow(row);
//            userDataModel->submitAll();
        }
        else
        {

        }
    }
    else
    {
        QMessageBox::warning(this,tr("����"),tr("\n��ѡ����ɾ���Ĵʿ�!"));
    }
}

void OptionsDialog::on_deleteRecordButton_clicked()
{
    QModelIndex currIndex = ui->studyRecordTableView->currentIndex();
    if(currIndex.isValid())
    {
        if (QMessageBox::information(this, tr("��ʾ��Ϣ"), tr("��ȷ��Ҫɾ�������¼��?"),
                                     QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            int row = currIndex.row();

            studyStateModel->removeRow(row);
            studyStateModel->submitAll();
        }
        else
        {

        }
    }
    else
    {
        QMessageBox::warning(this,tr("����"),tr("\n��ѡ����ɾ���ļ�¼!"));
    }
}

void OptionsDialog::on_studyRecordTableView_doubleClicked(const QModelIndex & /*index*/)
{

}

QString OptionsDialog::getLearnMethod()
{
    learnMethod = ui->LearnMethodComboBox->currentText();
    bcMgr->userConf->setLearnMethod(learnMethod);
    return learnMethod;
}

QString OptionsDialog::getOutWordMethod()
{
    if(ui->OutOrderlyRadioButton->isChecked())
    {
        outWordMethod = "Orderly";
    }
    else if(ui->OutRandomRadioButton->isChecked())
    {
        outWordMethod = "Random";
    }
    else
    {
        outWordMethod = "Random";
    }
    bcMgr->userConf->setOutWordMethod(outWordMethod);
    return outWordMethod;
}

QString OptionsDialog::getReviewWordMethod()
{
    if(ui->ReviewOrderlyRadioButton->isChecked())
    {
        reviewWordMethod = "Orderly";
    }
    else if(ui->ReviewRandomRadioButton->isChecked())
    {
        reviewWordMethod = "Random";
    }
    else
    {
        reviewWordMethod = "Random";
    }
    bcMgr->userConf->setReviewWordMethod(reviewWordMethod);
    return reviewWordMethod;
}

int OptionsDialog::getSoundTime()
{
    QString time = ui->TimeComboBox->currentText();
    soundTime = time.toInt();
    bcMgr->userConf->setSoundTime(time);
    return soundTime;
}

int OptionsDialog::getInterval()
{
    QString soundInterval = ui->IntervalComboBox->currentText();
    interval = soundInterval.toInt();
    bcMgr->userConf->setInterval(soundInterval);
    return interval;
}

QMap<QString, QString> OptionsDialog::getShortcuts()
{
    shortcuts["RememShortcut"] = ui->RememLineEdit->text();
    shortcuts["NotSureShortcut"] = ui->ObscureLineEdit->text();
    shortcuts["ForgetShortcut"] = ui->ForgetLineEdit->text();
    shortcuts["RightShortcut"] = ui->RightLineEdit->text();
    shortcuts["WrongShortcut"] = ui->WrongLineEdit->text();
    shortcuts["EditWortShortcut"] = ui->EditWordLineEdit->text();
    shortcuts["SoundShortcut"] = ui->SoundLineEdit->text();
    shortcuts["NextShortcut"] = ui->NextLineEdit->text();
    shortcuts["ConfigShortcut"] = ui->ConfigLineEdit->text();
    bcMgr->userConf->setShortcut(shortcuts);
    return shortcuts;
}
/////////////////////////////////////////////////////
//������ļ�¼
void OptionsDialog::saveConfig()
{
    //////////////////////////////////////////////////////////
    //ѧϰ����
    learnMethod = ui->LearnMethodComboBox->currentText();
    bcMgr->userConf->setLearnMethod(learnMethod);

    if(ui->OutOrderlyRadioButton->isChecked())
    {
        outWordMethod = "Orderly";
    }
    else if(ui->OutRandomRadioButton->isChecked())
    {
        outWordMethod = "Random";
    }
    else
    {
        outWordMethod = "Random";
    }
    bcMgr->userConf->setOutWordMethod(outWordMethod);

    if(ui->ReviewOrderlyRadioButton->isChecked())
    {
        reviewWordMethod = "Orderly";
    }
    else if(ui->ReviewRandomRadioButton->isChecked())
    {
        reviewWordMethod = "Random";
    }
    else
    {
        reviewWordMethod = "Random";
    }
    bcMgr->userConf->setReviewWordMethod(reviewWordMethod);

    /////////////////////////////////////////////////////
    //ѭ������
    QString time = ui->TimeComboBox->currentText();
    soundTime = time.toInt();
    bcMgr->userConf->setSoundTime(time);

    if(ui->SoundCheckBox->isChecked())
    {
        bcMgr->userConf->setIsOpenSound(true);
    }
    else
    {
        bcMgr->userConf->setIsOpenSound(false);
    }
    ///////////////////////////////////////////////////////////
    //ѭ�����ʱ��
    QString soundInterval = ui->IntervalComboBox->currentText();
    interval = soundInterval.toInt();
    bcMgr->userConf->setInterval(soundInterval);
    //////////////////////////////////////////////////////////////
    //������������
    bcMgr->userConf->setSpeakSpeed(QString::number(currSpeakSpeed,'i',0));
    bcMgr->userConf->setSpeakRate(QString::number(currSpeakRate,'i',0));
    bcMgr->userConf->setSpeakVolume(QString::number(currSpeakVolume,'i',0));
    bcMgr->userConf->setSpeakPitch(QString::number(currSpeakPitch,'i',0));
    bcMgr->userConf->setSpeakPitchRange(QString::number(currSpeakPitchRange,'i',0));
    bcMgr->userConf->setSpeakWordGap(QString::number(currSpeakWordGap,'i',0));

    bcMgr->userConf->setSpeakMethod(speakMethod);
    bcMgr->userConf->setSpeakLocalTTS(ui->TTSComboBox->currentText());
    //////////////////////////////////////////////////////////////
    //��ݼ�
    shortcuts["RememShortcut"] = ui->RememLineEdit->text();
    shortcuts["NotSureShortcut"] = ui->ObscureLineEdit->text();
    shortcuts["ForgetShortcut"] = ui->ForgetLineEdit->text();
    shortcuts["RightShortcut"] = ui->RightLineEdit->text();
    shortcuts["WrongShortcut"] = ui->WrongLineEdit->text();
    shortcuts["EditWortShortcut"] = ui->EditWordLineEdit->text();
    shortcuts["SoundShortcut"] = ui->SoundLineEdit->text();
    shortcuts["NextShortcut"] = ui->NextLineEdit->text();
    shortcuts["ConfigShortcut"] = ui->ConfigLineEdit->text();
    bcMgr->userConf->setShortcut(shortcuts);
}

void OptionsDialog::on_applyButton_clicked()
{
    saveConfig();
    QDialog::accept();
}

void OptionsDialog::on_quitButton_clicked()
{
    QDialog::reject();
}

void OptionsDialog::on_SearchLineEdit_textChanged(const QString &arg1)
{
    studyStateModel->setFilter(QString("word Like '%%1%'").arg(arg1));
    //��ʾ���
    studyStateModel->select();
}

void OptionsDialog::on_ShowAllPushButton_clicked()
{
    ui->SearchLineEdit->clear();
//    QString tableName = QString("StateTable");
//    studyStateModel->setTable(tableName);
//    studyStateModel->select(); //ѡȡ�������������
}

void OptionsDialog::on_SSpeedSlider_valueChanged(int value)
{
    if(localTTS == QString("eSpeakTTS"))
    {
        qDebug() << QString("��ǰ������������Ϊ:%1,����ֵΪ:%2").arg(localTTS).arg(value);
        currSpeakRate = bcMgr->userConf->getSpeakRate().toInt();
        currSpeakSpeed = value;
    }
    else
    {
        qDebug() << QString("��ǰ������������Ϊ:%1,����ֵΪ:%2").arg(localTTS).arg(value);
        currSpeakSpeed = bcMgr->userConf->getSpeakSpeed().toInt();
        currSpeakRate = value;
    }
}

void OptionsDialog::on_SVolumeSlider_valueChanged(int value)
{
    currSpeakVolume = value;
}

void OptionsDialog::on_SPitchSlider_valueChanged(int value)
{
    currSpeakPitch = value;
}

void OptionsDialog::on_PRangeSlider_valueChanged(int value)
{
    currSpeakPitchRange = value;
}

void OptionsDialog::on_GapSlider_valueChanged(int value)
{
    currSpeakWordGap = value;
}

void OptionsDialog::on_TestSondButton_clicked()
{
    ui->TestSondButton->setEnabled(false);
    ///////////////////////////////////////////////
    //eSpeakTTS
    if(localTTS == QString("eSpeakTTS"))
    {
        if(testSound != NULL)
        {
            connect(testSound,SIGNAL(soundEnded()),
                        this,SLOT(SpeakFinished()));
            testSound->stopSpeakTTS();

            testSound->setSpeakParameter(currSpeakSpeed,
                                          currSpeakVolume,
                                          currSpeakPitch,
                                          currSpeakPitchRange,
                                          currSpeakWordGap);
            QString text = ui->TestSoundLineEdit->text();
            testSound->getTextToSpeak(text);
            testSound->start();
        }
        else
        {
            testSound = new TestSoundTread(this);
            connect(testSound,SIGNAL(soundEnded()),
                        this,SLOT(SpeakFinished()));
            testSound->setSpeakParameter(currSpeakSpeed,
                                          currSpeakVolume,
                                          currSpeakPitch,
                                          currSpeakPitchRange,
                                          currSpeakWordGap);
            QString text = ui->TestSoundLineEdit->text();
            testSound->getTextToSpeak(text);
            testSound->start();
        }
    }
    ////////////////////////////////////////////////////////////
    //localTTS
    else
    {
        if(speech != NULL)
        {
            speech->stop();
            speech->deleteLater();
            speech = NULL;
            ///////////////////////////////////
            //QtSpeech
            foreach(QtSpeech::VoiceName v, QtSpeech::voices())
            {
                qDebug() << "id:" << v.id;
                qDebug() << "name:" << v.name;
                if(v.name == localTTS)
                {
                    qDebug() << QString("�ҵ�ƥ��ķ������棺%1").arg(v.name);

                    speech = new QtSpeech(v,this);
                    qDebug() << QString("����QtSpeech�����ɹ�");

                    speech->setRate(currSpeakRate);
                    break;
                }
            }
            QString text = ui->TestSoundLineEdit->text();
            speech->tell(text,this,SLOT(SpeakFinished()));
        }
        else
        {
            ///////////////////////////////////
            //QtSpeech
            foreach(QtSpeech::VoiceName v, QtSpeech::voices())
            {
                qDebug() << "id:" << v.id;
                qDebug() << "name:" << v.name;
                if(v.name == localTTS)
                {
                    qDebug() << QString("�ҵ�ƥ��ķ������棺%1").arg(v.name);

                    speech = new QtSpeech(v,this);
                    qDebug() << QString("����QtSpeech�����ɹ�");

                    speech->setRate(currSpeakRate);
                    break;
                }
            }
            QString text = ui->TestSoundLineEdit->text();
            speech->tell(text,this,SLOT(SpeakFinished()));
        }
    }
}

void OptionsDialog::on_BackToSysButton_clicked()
{
    //////////////////////////////////////////////
    //
    if(localTTS == QString("eSpeakTTS"))
    {
        ui->SSpeedSlider->setValue(sysSpeakSpeed);
    }
    else
    {
        ui->SSpeedSlider->setValue(sysSpeakRate);
    }
    ui->SVolumeSlider->setValue(sysSpeakVolume);
    ui->SPitchSlider->setValue(sysSpeakPitch);
    ui->PRangeSlider->setValue(sysSpeakPitchRange);
    ui->GapSlider->setValue(sysSpeakWordGap);
    ///////////////////////////////////////////////
    //
    currSpeakSpeed = sysSpeakSpeed;
    currSpeakVolume = sysSpeakVolume;
    currSpeakPitch = sysSpeakPitch;
    currSpeakPitchRange = sysSpeakPitchRange;
    currSpeakWordGap = sysSpeakWordGap;
}

void OptionsDialog::on_TTSRadioButton_clicked(bool checked)
{
    if(checked)
    {
        speakMethod = "LocalTTS";
    }
}

void OptionsDialog::on_Real_TTSRadioButton_clicked(bool checked)
{
    if(checked)
    {
        speakMethod = "realSpeak&eSpeakTTS";
    }
}

void OptionsDialog::on_OnlineRadioButton_clicked(bool checked)
{
    if(checked)
    {
        speakMethod = "googleTTS";
    }
}

void OptionsDialog::on_TTSComboBox_currentIndexChanged(const QString &localtts)
{
    ///////////////////////////////////////
    //�����������汻����������δ��������
    localTTS = localtts;
    qDebug() << QString("�������������л�Ϊ��%1").arg(localTTS);
    if(localTTS == QString("eSpeakTTS"))
    {
        eSpeakTTSInit();
    }
    else
    {
        LocalTTSInit();
    }
}

void OptionsDialog::SpeakFinished()
{
    ui->TestSondButton->setEnabled(true);
//    disconnect(testSound,SIGNAL(soundEnded()),
//                this,SLOT(SpeakFinished()));
}
