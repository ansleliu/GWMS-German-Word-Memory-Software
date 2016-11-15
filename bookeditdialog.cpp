#include "bookeditdialog.h"
#include "ui_bookeditdialog.h"
#include "addnewrecorddialog.h"
#include <QMessageBox>
#include <QSqlError>
#include <QIcon>
#include <QSqlQuery>
#include <QTime>
#include <QTimer>
#include "bcmgr.h"

BookEditDialog::BookEditDialog(QWidget *parent, const QString &BookPath) :
    QDialog(parent),
    ui(new Ui::BookEditDialog)
{
    ui->setupUi(this);

    QDateTime currtime = QDateTime::currentDateTime();//��ȡϵͳ���ڵ�ʱ��
    QString str = currtime.toString("yyyy-MM-dd dddd hh:mm:ss  "); //������ʾ��ʽ
    ui->label->setText(str);//�ڱ�ǩ����ʾʱ��

    //��ʱ����ÿ1000msִ��һ�βۺ���displayTime()
    QTimer *timer = new QTimer(this);
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(displayTime()));

    bookpath = BookPath;
    tableName = "WordsBook";
    bookEdit = new BookEdit(bookpath,"BOOKEDIT");
    model = bookEdit->wordListDBModel(this,tableName);
    ui->tableView->setModel(model);

    asIdSelect = "FALSE";//��id��ѯ��
    asWortSelect = "FALSE";//��wort��ѯ��
    descendingOrder = "FALSE";//�������

    ui->goTolineEdit->setDisabled(true);
}

BookEditDialog::~BookEditDialog()
{
    delete bookEdit;
    delete ui;
}

void BookEditDialog::displayTime()
{
    QDateTime currtime = QDateTime::currentDateTime();//��ȡϵͳ���ڵ�ʱ��
    QString str = currtime.toString("yyyy-MM-dd dddd hh:mm:ss "); //������ʾ��ʽ
    ui->label->setText(str);//�ڱ�ǩ����ʾʱ��
}

void BookEditDialog::on_newRecordButton_clicked()
{
    int rowNum = bookEdit->getRowCount();
    qDebug() << rowNum;

    AddNewRecordDialog *newRecord;
    newRecord = new AddNewRecordDialog(this);

    if(newRecord->exec() == QDialog::Accepted)
    {
/*
        model->insertRow(rowNum); //���һ��
        model->setData(model->index(rowNum,0),rowNum+1);//����id����
        model->setData(model->index(rowNum,1),newRecord->getWord());
        model->setData(model->index(rowNum,2),newRecord->getMean());
        model->setData(model->index(rowNum,3),newRecord->getOther());
        model->setData(model->index(rowNum,4),newRecord->getLektion());
        model->submitAll(); //ֱ���ύ
*/
        bookEdit->addNewRecord(newRecord->getWord(),
                               newRecord->getMean(),
                               newRecord->getOther(),
                               newRecord->getLektion());
        model->select();
    }
    ///////////////////////////////////////////////////////////////////
    //���ں�������
    newRecord->close();
    newRecord->deleteLater();
    newRecord = NULL;
    ////////////////////////////
    //���´ʿ�ʻ���
    updateWordCount();
}

void BookEditDialog::on_deleteRecordButton_clicked()
{
    int rowNum = bookEdit->getRowCount();
    qDebug() << rowNum;
    //ɾ������
//    model->removeRow(rowNum-1);
    int ok = QMessageBox::warning(this,tr("ɾ�����һ��"),
                                  tr("ѡ����Ҫɾ�����к��ٽ������������"
                                      "\n         "
                                      "��ȷ��ɾ���� %1 ����").arg(rowNum),
                                  QMessageBox::Yes,QMessageBox::No);
    if(ok == QMessageBox::No)
    {
       model->select(); //�����ɾ��������
    }
    else
    {
        bookEdit->removeRecord(rowNum); //�����ύ�������ݿ���ɾ������
        model->select();
        ////////////////////////////
        //���´ʿ�ʻ���
        updateWordCount();
    }
}

void BookEditDialog::on_showAllButton_clicked()
{
    descendingOrder = "FALSE";

    model->setTable(tableName);//���¹�����
    model->select();   //��ʾ�����������

    ui->newRecordButton->setDisabled(false);
    ui->deleteRecordButton->setEnabled(true);
}

void BookEditDialog::on_ascendingOrderButton_clicked()
{
    descendingOrder = "FALSE";

    model->setSort(0, Qt::AscendingOrder); //��Wid���Լ���0����������
    model->select();
}

void BookEditDialog::on_descendingOrderButton_clicked()
{
    descendingOrder = "TRUE";
    model->setSort(0, Qt::DescendingOrder);  //��Wid���Լ���0�н�������
    model->select();
}

void BookEditDialog::on_idSelectradioButton_clicked()
{
    asWortSelect = "FALSE";//��wort��ѯ��
    asIdSelect = "TRUE";//��id��ѯ��

    ui->goTolineEdit->setDisabled(false);

    deleteMethode == "FALSE";
    ui->deleteRecordButton->setDisabled(true);
    ui->newRecordButton->setDisabled(true);
}

void BookEditDialog::on_wortSelectradioButton_clicked()
{
    asIdSelect = "FALSE";//��id��ѯ��
    asWortSelect = "TRUE";//��wort��ѯ��

    ui->goTolineEdit->setDisabled(false);

    deleteMethode == "FALSE";
    ui->deleteRecordButton->setDisabled(true);
    ui->newRecordButton->setDisabled(true);
}

void BookEditDialog::updateWordCount()
{
    //////////////////////////
    //��ȡ�ʿ���
    int rowCount = bookEdit->getRowCount();
    bcMgr->updateTotalNumb(bookpath,rowCount);
}

void BookEditDialog::on_tableView_doubleClicked(const QModelIndex &index)
{
    EditorDialog *dialog;
    dialog = new EditorDialog(this);

    QString text;
    text = index.data().toString();
    dialog->setText(text);

    dialog->exec();
    text = dialog->getText();

    if(dialog->applyChange == "TRUE")
    {
        model->setData(index,text);
        model->submitAll();
    }
    else
    {
        model->submitAll();
    }
    dialog->close();
    delete dialog;
    dialog = NULL;
}

void BookEditDialog::on_goTolineEdit_textChanged(const QString &keyWort)
{
    ui->newRecordButton->setDisabled(true);
    //������������ɸѡ
    if(asWortSelect == "TRUE" && asIdSelect == "FALSE")
    {
        model->setFilter(QString("Wort like '%%1%'").arg(keyWort));
        //��ʾ���
        model->select();
    }
    else if(asWortSelect == "FALSE" && asIdSelect == "TRUE")
    {
        model->setFilter(QString("Wid = '%1'").arg(keyWort.toInt()));
        //��ʾ���
        model->select();
    }
}
