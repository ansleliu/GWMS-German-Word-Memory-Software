#include "AnsleDictDialog.h"
#include "ui_AnsleDictDialog.h"
#include "../AnsleDictConfigDialog.h"
#include "../bcmgr.h"
#include <QMenu>

AnsleDictDialog::AnsleDictDialog(QString word, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnsleDictDialog)
{
    qDebug() << QString("����AnsleDictMini����");
    ui->setupUi(this);
    this->setAttribute (Qt::WA_DeleteOnClose);
    //////////////////////////////////////////////////////////////
    //��ʼ��
    ///////////////////////////////////////////////////////////////////////////////////////
    //������������
    speakConfig = new ConfigFile(PathManage::makePathStr("/AnsleDict/speakConfig.ini"));
    ui->SpeakTTSPushButton->setEnabled(false);
    ui->AddToNewWordPushButton->setEnabled(false);

    //////////////////////////////////////////////////////////////////////
    //����������·��
    PathManage::createRelExeDir("/AnsleDict/MyDict/");
    PathManage::createRelExeDir("/AnsleDict/MyDict/Dict/");
    //////////////////////////////////////
    //
    speakMgr = NULL;
    minimodel = NULL;
    myDict = NULL;

    QString myDictDB = PathManage::makePathStr("/AnsleDict/MyDict/Dict/�º��ʵ�.db");

    myDict = new CreatWordListDB(myDictDB,"MYDICT");

    minimodel = myDict->wordListDBModel(this,"WordsBook");
    ///////////////////////////////////////////////////////////
    //
    ui->WordTableView->setUpdatesEnabled(true);

    ui->WordTableView->setModel(minimodel);
    ui->WordTableView->hideColumn(0);
    ui->WordTableView->hideColumn(2);
    ui->WordTableView->hideColumn(3);
    ui->WordTableView->hideColumn(4);
    minimodel->select();

    ui->WordLineEdit->setFocus();
    ui->MeanTextBrowser->clear();

    qDebug() << QString("����AnsleDictMini�ɹ�");
    //////////////////////////////////////////
    //
    ui->WordLineEdit->setText(word);
    minimodel->select();
}

AnsleDictDialog::~AnsleDictDialog()
{
    qDebug() << QString("�ر�AnsleDictMini����");

    qDebug() << QString("~AnsleDictDialog���model");
    if(minimodel != NULL)
    {
        minimodel->deleteLater();
        minimodel = NULL;
    }
    qDebug() << QString("~AnsleDictDialog���speakMgr");
    if(speakMgr != NULL)
    {
        speakMgr->deleteLater();
        speakMgr = NULL;
    }
    qDebug() << QString("~AnsleDictDialogɾ�����ݿ�MYDICT");
    if(myDict != NULL)
    {
        delete myDict;
        myDict = NULL;
    }
    qDebug() << QString("~AnsleDictDialog���speakConfig");
    if(speakConfig != NULL)
    {
        delete speakConfig;
        speakConfig = NULL;
    }
    delete ui;
    qDebug() << QString("�ر�AnsleDictMini�ɹ�");
}

void AnsleDictDialog::on_ConfigPushButton_clicked()
{
    AnsleDictConfigDialog * config;
    config = new AnsleDictConfigDialog(this,speakConfig);

    if(config->exec() == QDialog::Accepted)
    {

    }
    config->close();
    config->deleteLater();
    config = NULL;
}

void AnsleDictDialog::on_AddToNewWordPushButton_clicked()
{
    QModelIndex currIndex = ui->WordTableView->currentIndex();
    QString word = currIndex.data().toString();
    if(bcMgr->myBook->isExist(word))
    {
        QMessageBox::information(this, "��ʾ��Ϣ", "�õ��������ʱ����Ѿ�����,�����ظ����");
        ui->AddToNewWordPushButton->setEnabled(false);
    }
    else
    {
        int row = currIndex.row();
        QString mean = minimodel->index(row,2).data().toString();
        int count = bcMgr->myBook->getRowCount();
        int lektion = (count+1)/200 + 1;
        bcMgr->myBook->addNewRecord(word,mean,"",lektion);
        ui->AddToNewWordPushButton->setEnabled(false);
    }
}

void AnsleDictDialog::on_SpeakTTSPushButton_clicked()
{
    ui->SpeakTTSPushButton->setEnabled(false);

    QModelIndex currIndex = ui->WordTableView->currentIndex();
    QString word = currIndex.data().toString();
    if(speakMgr != NULL)
    {
        speakMgr->deleteLater();
        speakMgr = NULL;
    }
    speakMgr = new AnsleDictSpeakMgr(speakConfig,this);
    connect(speakMgr,SIGNAL(SpeakFininshed()),this,SLOT(setButtonState()));
    speakMgr->Speak(word,500,1);
}

void AnsleDictDialog::on_WordTableView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex currIndex = ui->WordTableView->indexAt(pos);
    //����������Ҽ���λ�ò��ڷ�Χ�ڣ����ڿհ�λ���һ�
    if(!currIndex.isValid())
    {

    }
    else
    {
        QMenu *popMenu =new QMenu(this);//����һ���Ҽ������˵�
        popMenu->addAction(ui->action_AddToNewWord);
        popMenu->addAction(ui->action_Speak);

        popMenu->exec(QCursor::pos());//�����Ҽ��˵����˵�λ��Ϊ���λ��

        delete popMenu;
        popMenu = NULL;
    }
}

void AnsleDictDialog::on_WordLineEdit_textChanged(const QString &word)
{
    ui->WordLineEdit->setFocus();
    ui->MeanTextBrowser->clear();

    ui->SpeakTTSPushButton->setEnabled(false);
    ui->AddToNewWordPushButton->setEnabled(false);

    if(ui->ExpComboBox->currentIndex() == 0)
    {
        minimodel->setFilter(QString("Wort like '%1%'").arg(word));
        //��ʾ���
        minimodel->select();
    }
    else if(ui->ExpComboBox->currentIndex() == 1)
    {
        minimodel->setFilter(QString("Wort like '%%1%'").arg(word));
        //��ʾ���
        minimodel->select();
    }
    else if(ui->ExpComboBox->currentIndex() == 2)
    {
        minimodel->setFilter(QString("Wort like '%%1'").arg(word));
        //��ʾ���
        minimodel->select();
    }
}

void AnsleDictDialog::on_WordTableView_clicked(const QModelIndex &index)
{
    ui->SpeakTTSPushButton->setEnabled(true);
    ui->AddToNewWordPushButton->setEnabled(true);

    int row = index.row();
    QString mean = minimodel->index(row,2).data().toString();

    ui->MeanTextBrowser->clear();
    ui->MeanTextBrowser->setText(mean);
}

void AnsleDictDialog::on_ExpComboBox_currentIndexChanged(int index)
{
    ui->WordLineEdit->setFocus();
    ui->MeanTextBrowser->clear();
    ui->SpeakTTSPushButton->setEnabled(false);
    ui->AddToNewWordPushButton->setEnabled(false);

    if(index == 0)
    {
        minimodel->setFilter(QString("Wort like '%1%'").arg(ui->WordLineEdit->text()));
        //��ʾ���
        minimodel->select();
    }
    else if(index == 1)
    {
        minimodel->setFilter(QString("Wort like '%%1%'").arg(ui->WordLineEdit->text()));
        //��ʾ���
        minimodel->select();
    }
    else if(index == 2)
    {
        minimodel->setFilter(QString("Wort like '%%1'").arg(ui->WordLineEdit->text()));
        //��ʾ���
        minimodel->select();
    }
}

void AnsleDictDialog::on_action_AddToNewWord_triggered()
{
    on_AddToNewWordPushButton_clicked();
}

void AnsleDictDialog::on_action_Speak_triggered()
{
    on_SpeakTTSPushButton_clicked();
}

void AnsleDictDialog::setButtonState()
{
    ui->SpeakTTSPushButton->setEnabled(true);
    if(speakMgr != NULL)
    {
        speakMgr->deleteLater();
        speakMgr = NULL;
    }
}
