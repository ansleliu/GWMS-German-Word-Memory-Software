#include "logindialog.h"
#include "ui_logindialog.h"
#include "creatnewuserdialog.h"
#include "QTimer"
#include "myglobal.h"

loginDialog::loginDialog(QWidget *parent,bool bDisableAutoLogin) :
    QDialog(parent), m_bDisableAutoLogin(bDisableAutoLogin),
    ui(new Ui::loginDialog)
{
    ui->setupUi(this);
/*
    QImage *userimage= new QImage("./rc/loginuser.png");
    ui->userlabel->setPixmap(QPixmap::fromImage(userimage->scaled(51,51)));
    QImage *mimaimage= new QImage("./rc/mimaicon.png");
    ui->mimalabel->setPixmap(QPixmap::fromImage(mimaimage->scaled(51,51)));
*/
    //��ʼ��ComboBox�е��û��б�
    initComboBoxUserList();

    if(ui->remembercheckBox->isChecked())
    {
        ui->forgetPasswordButton->setEnabled(false);
    }

    // ��������¼���û�
    QString userName = Global::Conf->getLastLoginUser();
    int index = ui->usernameComboBox->findText(userName);
    if (-1 != index)//�û�����
    {
        ui->usernameComboBox->setCurrentIndex(index);
        on_usernameComboBox_currentIndexChanged(userName);
        // �����Զ���¼��ť
        bool bAutoLogin = Global::Conf->getAutoLogin();
        ui->autocheckBox->setCheckState(bAutoLogin ? Qt::Checked : Qt::Unchecked);
        //�����Զ���¼ѡ��
        on_autocheckBox_clicked(bAutoLogin);

        if (bAutoLogin && !m_bDisableAutoLogin)
        {
            //����Զ���¼��һ��ʱ����Զ���¼
            QTimer::singleShot(1000, this, SLOT(on_loginButton_clicked()));
        }
    }

    ui->passwordlineEdit->setFocus();
}

loginDialog::~loginDialog()
{
    delete ui;
}

bool loginDialog::initComboBoxUserList()
{
    ui->usernameComboBox->clear();
    //��ȡ�û���Ϣ
    QStringList userList = Global::UserMag->getUserListOrderById();
    //�����ñ�
    QStringListIterator Iterator(userList);
    //��combobox������û�
    while(Iterator.hasNext())
    {
        ui->usernameComboBox->addItem(QIcon("./rc/loginuser.png"),Iterator.next());
    }
    return true;
}

void loginDialog::on_creatNewuserButton_clicked()
{
    CreatNewUserDialog newUserDlg(this);
    if (newUserDlg.exec()  == QDialog::Accepted)
    {
        ui->autocheckBox->setCheckState(Qt::Unchecked);
        ui->remembercheckBox->setCheckState(Qt::Unchecked);
        ui->forgetPasswordButton->setEnabled(true);

        ui->passwordlineEdit->setFocus();
    }
    initComboBoxUserList();
}

void loginDialog::on_forgetPasswordButton_clicked()
{
    QString username = ui->usernameComboBox->currentText();
    FindKeyDialog *findkey;
    findkey = new FindKeyDialog(this,username);
    if(findkey->exec() == QDialog::Accepted)
    {
        ui->passwordlineEdit->setFocus();
    }
    findkey->close();
    delete findkey;
    findkey = NULL;
}

void loginDialog::on_remembercheckBox_clicked(bool checked)
{
    QString userName = ui->usernameComboBox->currentText();
    Global::UserMag->updateSavePsw(userName, checked);
    if (!checked)
    {
        ui->autocheckBox->setCheckState(Qt::Unchecked);
        ui->forgetPasswordButton->setEnabled(true);
    }
    else
    {
        ui->forgetPasswordButton->setEnabled(false);
    }
}

void loginDialog::on_autocheckBox_clicked(bool checked)
{
    if (checked)
    {
        ui->remembercheckBox->setCheckState(Qt::Checked);
        ui->forgetPasswordButton->setEnabled(false);
    }

}

void loginDialog::on_loginButton_clicked()
{
    QString userName = ui->usernameComboBox->currentText();
    QString password = ui->passwordlineEdit->text();
    if (Global::UserMag->verifyPsw(userName, password))
    {
        //�������ļ�����������¼�û�������ǰ��¼�û�
        Global::Conf->setLastLoginUser(userName);
        //�����ݿ��и����û��ĵ�¼ʱ��
        Global::UserMag->updateLoginTime(userName);
        //�����ݿ��и��������¼״̬
        Global::UserMag->updateSavePsw(userName, ui->remembercheckBox->checkState());
        //�������ļ��������Զ���¼״̬
        Global::Conf->setAutoLogin(ui->autocheckBox->checkState());
        //�����ݿ��и����û���¼����
        Global::UserMag->updateLoginCount(userName);
        //�������ļ������õ�ǰ��¼�û�
        Global::Conf->setCurrentUser(userName);
        //�鿴�û��Ƿ�Ϊ���û�
        Global::Conf->setIsNewUser(Global::UserMag->getLoginCount(userName));

        accept();
    }
    else
    {
        QMessageBox::warning(this, tr("����"), tr("�������(ע���Сд),����������."));
        ui->passwordlineEdit->setFocus();
    }
}

void loginDialog::on_quitButton_clicked()
{
//    if (QMessageBox::information(this, tr("��ʾ��Ϣ"), tr("��ȷ��Ҫ�˳���?"),
//                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        QDialog::reject();
    }
}

void loginDialog::on_usernameComboBox_currentIndexChanged(const QString &userName)
{
    bool bSavePsw = Global::UserMag->getUserInfo(QString("savePsw"), userName).toInt();
    if (bSavePsw)
    {
        ui->passwordlineEdit->setText(Global::UserMag->getUserInfo(QString("password"), userName).toString());
        ui->forgetPasswordButton->setEnabled(false);
    }
    else
    {
        ui->passwordlineEdit->setText("");
        ui->forgetPasswordButton->setEnabled(true);
    }
    ui->remembercheckBox->setCheckState(bSavePsw ? Qt::Checked : Qt::Unchecked);
    ui->autocheckBox->setCheckState(Qt::Unchecked);
}
