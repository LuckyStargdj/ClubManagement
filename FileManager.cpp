#include "FileManager.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QVariant>

FileManager& FileManager::instance() {
    static FileManager instance;
    return instance;
}

FileManager::FileManager(QObject* parent) : QObject(parent) {}

int FileManager::saveClubFile(int clubId, int fileType, const QString& fileName, const QByteArray& data) {
    QMap<QString, QVariant> values;
    values[":club_id"] = clubId;
    values[":file_type"] = fileType;
    values[":file_name"] = fileName;
    values[":file_data"] = data;

    return DatabaseManager::instance().insertRecord("club_files", values);
}

QByteArray FileManager::loadClubFile(int fileId) {
    QSqlQuery query = DatabaseManager::instance().execQuery(
        "SELECT file_data FROM club_files WHERE id = :id",
        { {":id", fileId} }
    );

    if (query.next()) {
        return query.value(0).toByteArray();
    }
    return QByteArray();
}

bool FileManager::deleteClubFile(int fileId) {
    QSqlQuery query = DatabaseManager::instance().execQuery(
        "DELETE FROM club_files WHERE id = :id",
        { {":id", fileId} }
    );
    return query.numRowsAffected() > 0;
}