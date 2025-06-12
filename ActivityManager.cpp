#include "ActivityManager.h"
#include "DatabaseManager.h"
#include "ClubManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

ActivityManager::ActivityManager(QObject* parent) : QObject(parent) {}

ActivityInfo ActivityManager::parseActivityFromQuery(const QSqlQuery& query) const {
    ActivityInfo info;
    info.id = query.value("id").toInt();
    info.title = query.value("title").toString();
    info.startTime = query.value("start_time").toDateTime();
    info.endTime = query.value("end_time").toDateTime();
    info.location = query.value("location").toString();
    info.clubName = query.value("club_name").toString();
    info.rating = query.value("avg_rating").toDouble();
    info.maxParticipants = query.value("max_participants").toInt();
    info.budget = query.value("budget").toDouble();
    info.status = query.value("status").toInt();
    return info;
}

QVector<ActivityInfo> ActivityManager::queryActivities(const QString& keyword, const QString& type,
    const QDate& start, const QDate& end) {
    QMap<QString, QVariant> params;

    QString sql = "SELECT a.id, a.title, a.start_time, a.end_time, a.location, "
        "c.name AS club_name, AVG(f.rating) AS avg_rating, "
        "a.max_participants, a.budget, a.status "
        "FROM activities a "
        "LEFT JOIN clubs c ON a.club_id = c.id "
        "LEFT JOIN activity_feedback f ON a.id = f.activity_id "
        "WHERE 1=1 ";

    if (!keyword.isEmpty()) {
        sql += "AND (a.title LIKE :keyword OR c.name LIKE :keyword) ";
        params[":keyword"] = "%" + keyword + "%";
    }

    if (!type.isEmpty() && type != "全部类型") {
        sql += "AND a.type = :type ";
        params[":type"] = type;
    }

    if (start.isValid()) {
        sql += "AND DATE(a.start_time) >= :start ";
        params[":start"] = start.toString(Qt::ISODate);
    }

    if (end.isValid()) {
        sql += "AND DATE(a.start_time) <= :end ";
        params[":end"] = end.toString(Qt::ISODate);
    }

    sql += "GROUP BY a.id";

    QSqlQuery query = DatabaseManager::instance().execQuery(sql, params);

    QVector<ActivityInfo> activities;
    while (query.next()) {
        activities.append(parseActivityFromQuery(query));
    }
    return activities;
}

QVector<ActivityInfo> ActivityManager::queryActivitiesByPinyin(const QString& pinyinAbbr,
    const QString& type, const QDate& start, const QDate& end) {
    QMap<QString, QVariant> params;
    params[":pinyin"] = pinyinAbbr + "%";

    QString sql = "SELECT a.id, a.title, a.start_time, a.end_time, a.location, "
        "c.name AS club_name, AVG(f.rating) AS avg_rating, "
        "a.max_participants, a.budget, a.status "
        "FROM activities a "
        "JOIN clubs c ON a.club_id = c.id "
        "LEFT JOIN activity_feedback f ON a.id = f.activity_id "
        "WHERE c.py_abbr LIKE :pinyin ";

    if (!type.isEmpty() && type != "全部类型") {
        sql += "AND a.type = :type ";
        params[":type"] = type;
    }

    if (start.isValid()) {
        sql += "AND DATE(a.start_time) >= :start ";
        params[":start"] = start.toString(Qt::ISODate);
    }

    if (end.isValid()) {
        sql += "AND DATE(a.start_time) <= :end ";
        params[":end"] = end.toString(Qt::ISODate);
    }

    sql += "GROUP BY a.id";

    QSqlQuery query = DatabaseManager::instance().execQuery(sql, params);

    QVector<ActivityInfo> activities;
    while (query.next()) {
        activities.append(parseActivityFromQuery(query));
    }
    return activities;
}

bool ActivityManager::addActivity(int clubId, const QString& title, const QDateTime& start,
    const QDateTime& end, const QString& location,
    int maxParticipants, double budget) {
    ClubManager clubMgr;
    if (clubMgr.checkTimeConflict(clubId, start, end, location)) {
        emit conflictDetected("该时间段和地点已被占用，请选择其他时间或地点");
        return false;
    }

    QMap<QString, QVariant> values;
    values["club_id"] = clubId;
    values["title"] = title;
    values["start_time"] = start;
    values["end_time"] = end;
    values["location"] = location;
    values["max_participants"] = maxParticipants;
    values["budget"] = budget;
    values["status"] = 0; // 待审核状态

    int newId = DatabaseManager::instance().insertRecord("activities", values);
    if (newId > 0) {
        emit activityDataChanged();
        return true;
    }
    return false;
}

bool ActivityManager::addFeedback(int activityId, int userId, int rating, const QString& comment) {
    if (rating < 1 || rating > 5) return false;

    QMap<QString, QVariant> values;
    values["activity_id"] = activityId;
    values["user_id"] = userId;
    values["rating"] = rating;
    values["comment"] = comment;

    int newId = DatabaseManager::instance().insertRecord("activity_feedback", values);
    if (newId > 0) {
        emit activityDataChanged();
        emit feedbackSubmitted(true, "反馈提交成功");
        return true;
    }
    else {
        emit feedbackSubmitted(false, "反馈提交失败");
        return false;
    }
}

QVector<ActivityManager::ClubActiveInfo> ActivityManager::calculateClubActiveRanking() {
    QVector<ClubActiveInfo> ranking;

    // 计算每个社团的活动次数、参与率、经费使用效率
    QString sql = R"(
        WITH club_stats AS (
            SELECT
                c.id AS club_id,
                c.name AS club_name,
                COUNT(a.id) AS activity_count,
                COALESCE(AVG(f.participation_rate), 0) AS participation_rate,
                COALESCE(AVG(a.actual_budget / a.budget), 0) AS budget_efficiency
            FROM clubs c
            LEFT JOIN activities a ON c.id = a.club_id AND a.status = 1
            LEFT JOIN (
                SELECT 
                    a.id AS activity_id,
                    COUNT(f.id) * 1.0 / a.max_participants AS participation_rate
                FROM activities a
                LEFT JOIN activity_feedback f ON a.id = f.activity_id
                WHERE a.status = 1
                GROUP BY a.id
            ) f ON a.id = f.activity_id
            WHERE c.status = 1
            GROUP BY c.id
        )
        SELECT
            club_name,
            activity_count,
            participation_rate,
            budget_efficiency,
            (activity_count * 0.4 + participation_rate * 0.3 + budget_efficiency * 0.3) AS activity_score
        FROM club_stats
        ORDER BY activity_score DESC
    )";

    QSqlQuery query = DatabaseManager::instance().execQuery(sql);

    while (query.next()) {
        ClubActiveInfo info;
        info.name = query.value("club_name").toString();
        info.activityCount = query.value("activity_count").toInt();
        info.participationRate = query.value("participation_rate").toDouble();
        info.budgetEfficiency = query.value("budget_efficiency").toDouble();
        info.activityScore = query.value("activity_score").toDouble();
        ranking.append(info);
    }

    return ranking;
}