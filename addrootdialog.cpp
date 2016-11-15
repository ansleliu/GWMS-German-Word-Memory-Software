#include "addrootdialog.h"
#include "ui_addrootdialog.h"

AddRootDialog::AddRootDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddRootDialog)
{
    ui->setupUi(this);
}

AddRootDialog::~AddRootDialog()
{
    delete ui;
}

QString AddRootDialog::getNewRoot()
{
    return ui->lineEdit->text();
}

void AddRootDialog::on_OKPushButton_clicked()
{
    if(getNewRoot().isEmpty())
    {
        QMessageBox::warning(this, tr("����"),
                             tr("\n����ӵķ���������Ϊ��!"),
                             QMessageBox::Ok);
    }
    else
    {
        accept();
    }
}

void AddRootDialog::on_NOPushButton_clicked()
{
    close();
}
