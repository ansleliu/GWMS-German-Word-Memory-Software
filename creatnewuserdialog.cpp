#include "creatnewuserdialog.h"
#include "ui_creatnewuserdialog.h"
#include "myglobal.h"
#include "QMessageBox"

CreatNewUserDialog::CreatNewUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreatNewUserDialog)
{
    ui->setupUi(this);

    ui->userNamelineEdit->setFocus();
}

CreatNewUserDialog::~CreatNewUserDialog()
{
    delete ui;
}

void CreatNewUserDialog::on_YesButton_clicked()
{
    QString userName = ui->userNamelineEdit->text();
    QString password = ui->passwordlineEdit->text();
    QString verifyPsw = ui->surePasswordlineEdit->text();

    if (userName.isEmpty())
    {
        QMessageBox::warning(this, tr("����"), tr("�û�������Ϊ��."), QMessageBox::Ok);
        ui->userNamelineEdit->setFocus();
        return;
    }

    if (password != verifyPsw)
    {
        QMessageBox::warning(this, tr("����"), tr("�����������벻һ��."), QMessageBox::Ok);
        ui->passwordlineEdit->setFocus();
        return;
    }

    if (Global::UserMag->exists(userName))
    {
        QMessageBox::warning(this, tr("����"), tr("���û��Ѵ���."), QMessageBox::Ok);
        ui->userNamelineEdit->setFocus();
        ui->userNamelineEdit->selectAll();
        return;
    }

    QMap<QString, QVariant> userinfo;
    userinfo["userName"] = userName;
    userinfo["password"] = verifyPsw;

    Global::UserMag->addUser(userinfo);
    accept();
}

void CreatNewUserDialog::on_NoButton_clicked()
{
    reject();
}
