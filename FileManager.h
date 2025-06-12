#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QByteArray>

class FileManager : public QObject {
    Q_OBJECT
public:
    static FileManager& instance();

    int saveClubFile(int clubId, int fileType, const QString& fileName, const QByteArray& data);
    QByteArray loadClubFile(int fileId);
    bool deleteClubFile(int fileId);

private:
    FileManager(QObject* parent = nullptr);
};

#endif // FILEMANAGER_H

