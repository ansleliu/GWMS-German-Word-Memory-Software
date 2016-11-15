#include "addbookdialog.h"
#include "ui_addbookdialog.h"
#include "QMessageBox"
#include "QFileDialog"
#include "QFileInfo"
#include "pathmanage.h"
#include "myglobal.h"

AddBookDialog::AddBookDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddBookDialog)
{
    ui->setupUi(this);

    bookName = "";
    bookPath = "";
}

AddBookDialog::~AddBookDialog()
{
    delete ui;
}

QString AddBookDialog::getClassName()
{
    return ui->classLineEdit->text();
}

QString AddBookDialog::getBookName()
{
    return ui->bookLineEdit->text();
}

QString AddBookDialog::getBookPath()
{
    return bookPath;
}

void AddBookDialog::setClassName(const QString &bookclass)
{
    ui->classLineEdit->setText(bookclass);
}

void AddBookDialog::setBookName(const QString &bookname)
{
    ui->bookLineEdit->setText(bookname);
}

void AddBookDialog::on_pushButton_clicked()
{
    QString userBookPath = PathManage::makePathStr("/userdata/" +
                                                   Global::Conf->getCurrentUser() + "/book/");
    QString filename = QFileDialog::getOpenFileName(this,
            tr("����´ʿ�"), userBookPath, tr("���ݿ��ļ�(*.db)"));
    if (filename.isEmpty())
        return;
    else if(!filename.contains(".db", Qt::CaseInsensitive))
    {
        QMessageBox::warning(this, tr("������"),
         QString(tr("��Ѵʿ��ļ��ŵ�%1Ŀ¼�²�����򿪣�")).arg(userBookPath));
        return;
    }
    else
    {
        QFileInfo fd(filename);
        bookName = fd.baseName();
        //bookPath = fd.filePath();
        bookPath = userBookPath.append(bookName);
        setBookName(bookName);
    }
}

void AddBookDialog::on_OKButton_clicked()
{
    if(getClassName().isEmpty() || getBookName().isEmpty())
    {
        QMessageBox::warning(this, tr("����"),
                             tr("\n�����ʹʿ���������Ϊ��!"),
                             QMessageBox::Ok);
    }
    else
    {
        accept();
    }
}

void AddBookDialog::on_QuitButton_clicked()
{
    close();
}
