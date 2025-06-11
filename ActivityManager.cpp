#include "ActivityManager.h"
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QDebug>

ActivityManager::ActivityManager(QObject* parent) : QObject(parent) {}

QVector<ActivityInfo> ActivityManager::queryActivities(const QString& keyword, const QString& type,
    const QDate& start, const QDate& end) {
    QVector<ActivityInfo> res;
    QString sql =
        "SELECT a.id, a.title, a.start_time, a.end_time, a.location, c.name, "
        "(SELECT AVG(rating) FROM activity_feedback WHERE activity_id=a.id) "
        "FROM activities a JOIN clubs c ON a.club_id=c.id WHERE 1=1";
    if (!keyword.isEmpty())
        sql += QString(" AND (a.title LIKE '%%1%' OR c.name LIKE '%%1%')").arg(keyword);
    if (!type.isEmpty() && type != "全部类型")
        sql += QString(" AND a.type='%1'").arg(type);
    sql += QString(" AND a.start_time>='%1' AND a.end_time<='%2'")
        .arg(start.toString(Qt::ISODate), end.toString(Qt::ISODate));

    QSqlQuery q(sql);
    while (q.next()) {
        ActivityInfo info;
        info.id = q.value(0).toInt();
        info.title = q.value(1).toString();
        info.startTime = q.value(2).toDateTime();
        info.endTime = q.value(3).toDateTime();
        info.location = q.value(4).toString();
        info.clubName = q.value(5).toString();
        info.rating = q.value(6).toDouble();
        res.append(info);
    }
    return res;
}

bool ActivityManager::addFeedback(int activityId, int userId, int rating, const QString& comment) {
    QSqlQuery q;
    q.prepare("INSERT INTO activity_feedback (activity_id, user_id, rating, comment) "
        "VALUES (:aid,:uid,:rating,:comment)");
    q.bindValue(":aid", activityId);
    q.bindValue(":uid", userId);
    q.bindValue(":rating", rating);
    q.bindValue(":comment", comment);
    bool success = q.exec();
    emit feedbackSubmitted(success, success ? "反馈成功" : "提交失败：" + q.lastError().text());
    emit activityDataChanged();
    return success;
}

QVector<ActivityManager::ClubActiveInfo> ActivityManager::queryActiveClubs() {
    QVector<ClubActiveInfo> res;
    QSqlQuery q(
        "SELECT c.name, "
        "(SELECT COUNT(*) FROM activities a WHERE a.club_id=c.id AND a.status=1), "
        "(SELECT AVG(atcnt.real_participants*1.0/atcnt.max_participants) FROM "
        "(SELECT a.id, a.max_participants, (SELECT COUNT(*) FROM activity_feedback af WHERE af.activity_id=a.id) as real_participants "
        "FROM activities a WHERE a.club_id=c.id AND a.status=1) atcnt), "
        "(SELECT AVG(a.actual_budget*1.0/a.budget) FROM activities a WHERE a.club_id=c.id AND a.status=1 AND a.budget>0) "
        "FROM clubs c WHERE c.status=1"
    );
    while (q.next()) {
        ClubActiveInfo info;
        info.name = q.value(0).toString();
        info.activityCount = q.value(1).toInt();
        info.partRate = q.value(2).toDouble();
        info.budgetRate = q.value(3).toDouble();
        res.append(info);
    }
    return res;
}
