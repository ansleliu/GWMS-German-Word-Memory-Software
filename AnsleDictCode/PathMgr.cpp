#include "PathMgr.h"
#include <QFile>

PathMgr::PathMgr()
{
}

QString PathMgr::makePathStr(const QString &fileName)
{
    QString appPath = QCoreApplication::applicationDirPath();
    //��pathName����ġ�/���ָ���ת��Ϊ�����ڵײ����ϵͳ�ķָ���������ת�����ַ�����
    return QDir::convertSeparators(appPath.append(fileName));
}

void PathMgr::mkPath(const QString &strPath)
{
    if (!QDir(strPath).exists())
    {
        QDir().mkpath(strPath);
    }
}

void PathMgr::createRelExeDir(const QString &path)
{
    QString strPath = PathMgr::makePathStr(path);
    PathMgr::mkPath(strPath);
}

QString PathMgr::makeRelativeFilePath(const QString &fileName)
{
    QDir dirfrom;
    return dirfrom.relativeFilePath(makePathStr(fileName));
}

bool PathMgr::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        fprintf(stderr, "Could not open %s for writing: %s\n",
            qPrintable(filename),
            qPrintable(file.errorString()));
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

bool PathMgr::saveToDisk(const QString &filename, QByteArray &data)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        fprintf(stderr, "Could not open %s for writing: %s\n",
            qPrintable(filename),
            qPrintable(file.errorString()));
        return false;
    }

    file.write(data);
    file.close();

    return true;
}
