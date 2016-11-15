#include "edititemdialog.h"
#include "ui_edititemdialog.h"
#include "QMessageBox"

editItemDialog::editItemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::editItemDialog)
{
    ui->setupUi(this);
}

editItemDialog::~editItemDialog()
{
    delete ui;
}

QString editItemDialog::getNewItemText()
{
    return ui->newNamelineEdit->text();
}

void editItemDialog::setGroupText(const QString &text)
{
    ui->groupBox->setTitle(text);
}

void editItemDialog::on_OKPushButton_clicked()
{
    if(getNewItemText().isEmpty())
    {
        QMessageBox::warning(this, tr("����"),
                             tr("\n�����������������Ҫ���ĵ�����!"),
                             QMessageBox::Ok);
    }
    else
    {
        accept();
    }
}

void editItemDialog::on_NOPushButton_clicked()
{
    close();
}
