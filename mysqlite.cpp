#include "mysqlite.h"
#include "QMessageBox"
#include <QObject>
MySQLite::MySQLite(const QString &dbpath, const QString &connName)
    : m_connName(connName)
{
    if (m_connName.isEmpty())
    {
        m_connName = dbpath;
    }
    db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", m_connName));
    db->setDatabaseName(dbpath);
    if (db->open())
    {
        qDebug() << QString("�ɹ������ݿ⣺ %1").arg(dbpath);
    }
    else
    {
        QMessageBox::warning(NULL, QObject::tr("���ݿ����"),
                                     QObject::tr("�޷������ݿ�: %1")
                                     .arg(db->lastError().text()));
        qDebug() << QString("�޷������ݿ�: %1����Ϊ���ݿ����%2").arg(dbpath).arg(db->lastError().text());
    }
}

MySQLite::~MySQLite()
{
    if(NULL == db)
    {
        qDebug() << "�ظ�ж��...";
    }
    qDebug() << "��ǰ�������ǣ�" << db->connectionName()
             <<" ,����Ҫ���Ƴ���";

    db->close();
    delete db;
    db = NULL;
    QSqlDatabase::removeDatabase(m_connName);

    QStringList connect = QSqlDatabase::connectionNames();
    for(int i=0;i<connect.size();i++)
    {
        qDebug() << "�Ƴ��󻹰�����Щ����" << connect.at(i);
    }
    qDebug() << "�Ѿ��Ƴ���һ������";
}

QVariant MySQLite::rexec(const QString &sql)
{
    QSqlQuery query = db->exec(sql);
    if (query.next())
    {
        return query.record().value(0);
    }
    else
    {
        if (query.lastError().number() != -1)	// û�ж�Ӧ������
        {
            QMessageBox::warning(NULL,QObject::tr("���ݿ����"),
                                         QObject::tr("û�ж�Ӧ������: %1 %2")
                                 .arg(query.lastError().number()).arg(query.lastError().type()));
        }

    }
    return "";
}

uint MySQLite::lastInsertID()
{
    return rexec("SELECT last_insert_rowid();").toUInt();
}

bool MySQLite::exec(const QString &sql)
{
    db->exec(sql);

    QSqlError::ErrorType type = db->lastError().type();

    if (type != QSqlError::NoError)
    {
        QString message = db->lastError().text();
        QMessageBox::warning(NULL,QObject::tr("���ݿ����"),
                                     QObject::tr("���ݿ����: %1").arg(message));
        qDebug() << "���ݿ����" << message;
        return false;
    }
    return true;
}

bool MySQLite::isTableExist(const QString &tableName)
{
    return db->tables(QSql::Tables).contains(tableName);
}

int MySQLite::recordCount(const QString &tableName)
{
    if(isTableExist(tableName))
    {
        QSqlQuery query(QString("select count(*) from %1").arg(tableName), *db);
        query.next();
        return query.value(0).toInt();
    }
    return 0;
}

