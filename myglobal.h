#ifndef MYGLOBAL_H
#define MYGLOBAL_H
#include <QObject>
#include "usermanage.h"
#include "myconfig.h"

class MyConfig;
class UserManage;

class Global:public QObject
{
    Q_OBJECT
public:
    Global(QObject *parent = 0);
    ~Global();

    static bool init();
    static bool uninit();
    static void sleep(int secs);

    static MyConfig *Conf;//�����������Ϣ
    static UserManage *UserMag;//�û�����
};

#endif // MYGLOBAL_H
