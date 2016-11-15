#include "myglobal.h"
#include "pathmanage.h"
#include <QTime>
#include <QCoreApplication>
#include <QEventLoop>

MyConfig *Global::Conf = NULL;
UserManage *Global::UserMag = NULL;

Global::Global(QObject *parent)
    :QObject(parent)
{
    /////////////////////////////////
    //

}

Global::~Global()
{

}

bool Global::init()
{
/*
    QLibrary eSpeakLib("./eSpeak/espeak_lib.dll");//�������õ���dll�ļ�
    if (!eSpeakLib.load())              //�ж��Ƿ���ȷ����
    {
        QMessageBox::information(NULL,QString("����"),QString("eSpeak���ӿ⵼�����!"));
    }
*/
    ////////////////////////////////////
    //�����ļ�·��
    PathManage::createRelExeDir("/Sound");
    //�����û�����·��
    PathManage::createRelExeDir("/userdata/");
    Conf = new MyConfig(PathManage::makePathStr("/Config.ini"));

    QString userdbPath = PathManage::makePathStr("/userdata/user.db");
    UserMag = new UserManage(userdbPath,"UserDB");

    return true;
}

bool Global::uninit()
{
    qDebug() << QString("������ر����ȫ��ָ�����");
    delete Conf;
    Conf = NULL;
    delete UserMag;
    UserMag = NULL;
    qDebug() << QString("������ر����ȫ��ָ������ɹ�");
    return true;
}

void Global::sleep(int secs)
{
    QTime dieTime = QTime::currentTime().addMSecs(secs);

    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
}
