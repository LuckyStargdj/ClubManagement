#include "ClubManager.h"
#include <QSqlQuery>
#include <QVariant>

ClubManager::ClubManager(QObject* parent) : QObject(parent) {}

ClubInfo ClubManager::queryClubByPresident(int userId) {
    ClubInfo info;
    QSqlQuery q;
    q.prepare("SELECT c.id, c.name, c.type, c.establish_date, c.advisor, c.status "
        "FROM clubs c JOIN users u ON c.id=u.club_id WHERE u.id=:uid");
    q.bindValue(":uid", userId);
    if (q.exec() && q.next()) {
        info.id = q.value(0).toInt();
        info.name = q.value(1).toString();
        info.type = q.value(2).toString();
        info.establishDate = QDate::fromString(q.value(3).toString(), Qt::ISODate);
        info.advisor = q.value(4).toString();
        info.status = q.value(5).toInt();
    }
    return info;
}

bool ClubManager::updateClubInfo(const ClubInfo& info) {
    QSqlQuery q;
    q.prepare("UPDATE clubs SET name=:n, type=:t, establish_date=:d, advisor=:a WHERE id=:id");
    q.bindValue(":n", info.name);
    q.bindValue(":t", info.type);
    q.bindValue(":d", info.establishDate.toString(Qt::ISODate));
    q.bindValue(":a", info.advisor);
    q.bindValue(":id", info.id);
    bool ok = q.exec();
    if (ok) emit clubInfoChanged();
    return ok;
}

QVector<ClubInfo> ClubManager::queryPendingClubs() {
    QVector<ClubInfo> res;
    QSqlQuery q("SELECT id, name, type, establish_date, advisor, status FROM clubs WHERE status=0");
    while (q.next()) {
        ClubInfo info;
        info.id = q.value(0).toInt();
        info.name = q.value(1).toString();
        info.type = q.value(2).toString();
        info.establishDate = QDate::fromString(q.value(3).toString(), Qt::ISODate);
        info.advisor = q.value(4).toString();
        info.status = q.value(5).toInt();
        res.append(info);
    }
    return res;
}

bool ClubManager::approveClub(int clubId) {
    QSqlQuery q;
    q.prepare("UPDATE clubs SET status=1 WHERE id=:id");
    q.bindValue(":id", clubId);
    bool ok = q.exec();
    if (ok) emit clubInfoChanged();
    return ok;
}
bool ClubManager::rejectClub(int clubId) {
    QSqlQuery q;
    q.prepare("DELETE FROM clubs WHERE id=:id");
    q.bindValue(":id", clubId);
    bool ok = q.exec();
    if (ok) emit clubInfoChanged();
    return ok;
}
