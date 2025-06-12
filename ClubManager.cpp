#include "ClubManager.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QChar>

ClubManager::ClubManager(QObject* parent) : QObject(parent) {}

QString ClubManager::toPinyinAbbr(const QString& text) {
    static QMap<QChar, QString> pinyinMap = {
        {'ÕÅ', "Z"}, {'Àî', "L"}, {'Íõ', "W"}, {'Áõ', "L"}, {'³Â', "C"},
        {'Ñî', "Y"}, {'ÕÔ', "Z"}, {'»Æ', "H"}, {'ÖÜ', "Z"}, {'Îâ', "W"},
        // Ìí¼Ó¸ü¶à³£¼ûºº×ÖÓ³Éä...
    };

    QString result;
    for (QChar ch : text) {
        if (ch.unicode() >= 0x4E00 && ch.unicode() <= 0x9FFF) { // ºº×Ö·¶Î§
            if (pinyinMap.contains(ch)) {
                result.append(pinyinMap[ch]);
            }
            else {
                result.append(ch);
            }
        }
        else if (ch.isLetter()) {
            result.append(ch.toUpper());
        }
    }
    return result;
}

bool ClubManager::checkTimeConflict(int clubId, const QDateTime& start, const QDateTime& end, const QString& location) {
    QMap<QString, QVariant> params;
    params[":location"] = location;
    params[":start"] = start;
    params[":end"] = end;
    params[":clubId"] = clubId;

    QString sql = "SELECT COUNT(*) FROM activities "
        "WHERE location = :location "
        "AND club_id != :clubId "
        "AND ((start_time >= :start AND start_time <= :end) "
        "OR (end_time >= :start AND end_time <= :end) "
        "OR (start_time <= :start AND end_time >= :end))";

    QSqlQuery query = DatabaseManager::instance().execQuery(sql, params);

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

ClubInfo ClubManager::queryClubByPresident(int userId) {
    QMap<QString, QVariant> params;
    params[":userId"] = userId;

    QString sql = "SELECT id, name, type, establish_date, advisor, status, py_abbr "
        "FROM clubs "
        "WHERE id IN (SELECT club_id FROM users WHERE id = :userId)";

    QSqlQuery query = DatabaseManager::instance().execQuery(sql, params);

    ClubInfo info;
    if (query.next()) {
        info.id = query.value("id").toInt();
        info.name = query.value("name").toString();
        info.type = query.value("type").toString();
        info.establishDate = query.value("establish_date").toDate();
        info.advisor = query.value("advisor").toString();
        info.status = query.value("status").toInt();
        info.pyAbbr = query.value("py_abbr").toString();
    }
    return info;
}

ClubInfo ClubManager::queryClubById(int clubId) {
    QMap<QString, QVariant> params;
    params[":clubId"] = clubId;

    QString sql = "SELECT * FROM clubs WHERE id = :clubId";

    QSqlQuery query = DatabaseManager::instance().execQuery(sql, params);

    ClubInfo info;
    if (query.next()) {
        info.id = query.value("id").toInt();
        info.name = query.value("name").toString();
        info.type = query.value("type").toString();
        info.establishDate = query.value("establish_date").toDate();
        info.advisor = query.value("advisor").toString();
        info.status = query.value("status").toInt();
        info.pyAbbr = query.value("py_abbr").toString();
    }
    return info;
}

bool ClubManager::updateClubInfo(const ClubInfo& info) {
    if (info.name.isEmpty() || info.advisor.isEmpty()) {
        return false;
    }

    QMap<QString, QVariant> params;
    params["name"] = info.name;
    params["type"] = info.type;
    params["establish_date"] = info.establishDate.toString(Qt::ISODate);
    params["advisor"] = info.advisor;

    QSqlQuery query;
    query.prepare("UPDATE clubs SET name = :name, type = :type, "
        "establish_date = :establish_date, advisor = :advisor "
        "WHERE id = :id");

    query.bindValue(":name", info.name);
    query.bindValue(":type", info.type);
    query.bindValue(":establish_date", info.establishDate.toString(Qt::ISODate));
    query.bindValue(":advisor", info.advisor);
    query.bindValue(":id", info.id);

    bool success = query.exec();
    if (success) {
        emit clubInfoChanged();
    }
    return success;
}

QVector<ClubInfo> ClubManager::queryPendingClubs() {
    QMap<QString, QVariant> params;
    params[":status"] = 0;

    QString sql = "SELECT * FROM clubs WHERE status = :status";

    QSqlQuery query = DatabaseManager::instance().execQuery(sql, params);

    QVector<ClubInfo> clubs;
    while (query.next()) {
        ClubInfo info;
        info.id = query.value("id").toInt();
        info.name = query.value("name").toString();
        info.type = query.value("type").toString();
        info.establishDate = query.value("establish_date").toDate();
        info.advisor = query.value("advisor").toString();
        info.status = query.value("status").toInt();
        info.pyAbbr = query.value("py_abbr").toString();
        clubs.append(info);
    }
    return clubs;
}

bool ClubManager::approveClub(int clubId) {
    QSqlQuery query;
    query.prepare("UPDATE clubs SET status = 1 WHERE id = :id");
    query.bindValue(":id", clubId);

    bool success = query.exec();
    if (success) emit clubInfoChanged();
    return success;
}

bool ClubManager::rejectClub(int clubId) {
    QSqlQuery query;
    query.prepare("DELETE FROM clubs WHERE id = :id");
    query.bindValue(":id", clubId);

    bool success = query.exec();
    if (success) emit clubInfoChanged();
    return success;
}