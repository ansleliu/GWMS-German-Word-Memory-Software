#ifndef PATHMGR_H
#define PATHMGR_H
#include <algorithm>
#include <QString>
#include <QDir>
#include <QCoreApplication>
#include <QIODevice>
#include <QByteArray>

class PathMgr
{
public:
    PathMgr();
    ~PathMgr();

public:
    static QString makePathStr(const QString &fileName);	// ����һ��·�� �����exe����·��,δ�ں������"\"
    static void mkPath(const QString &strPath);	// ����Ŀ¼,���Ŀ¼������
    static void createRelExeDir(const QString &path);
    static QString makeRelativeFilePath(const QString &fileName);
    static bool saveToDisk(const QString &filename, QIODevice *data);// �������ݵ��ļ�
    static bool saveToDisk(const QString &filename, QByteArray &data);
};

#endif // PATHMGR_H
