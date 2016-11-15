#ifndef USERMANAGE_H
#define USERMANAGE_H
#include "mysqlite.h"

class UserManage : public MySQLite
{
public:
    UserManage(const QString &dbpath, const QString &connName);
    virtual ~UserManage();

public:
    bool addUser(QMap<QString, QVariant> &ui);//����û�
    bool exists(const QString &userName); // �ж��û��Ƿ����

    bool getUserInfo(QMap<QString, QVariant> &ui, const QString &userName); // ��ȡ�û���ص���Ϣ
    QVariant getUserInfo(const QString &itemName,const QString &userName); // ��ȡ�û���ص���Ϣ

    bool verifyPsw(const QString &userName,const QString &psw); // У������

    bool updateLoginTime(const QString &userName);	// ���µ�¼ʱ��
    bool updateLoginCount(const QString &userName); //���µ�¼����

    int getLoginCount(const QString &userName); //��ȡ��¼����

    bool updateSavePsw(const QString &userName, bool bSave = false); // ���±���״̬����
    bool updatePassword(const QString &userName, const QString &password); // ��������

    bool removeUser(const QString &userName);//ɾ���û�

    QStringList getUserList();	// �����û��б�,�Ѿ�������¼ʱ�������
    QStringList getUserListOrderById();	// �����û��б�, ��id�����ź�

protected:
    bool createTable();	// ������
};

#endif // USERMANAGE_H
