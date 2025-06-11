#ifndef ACTIVITYMANAGER_H
#define ACTIVITYMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QStringList>
#include <QVariant>

struct ActivityInfo {
    int id;
    QString title;
    QDateTime startTime;
    QDateTime endTime;
    QString location;
    QString clubName;
    double rating; // 平均分
};

class ActivityManager : public QObject {
    Q_OBJECT
public:
    explicit ActivityManager(QObject* parent = nullptr);

    // 查询活动，带多条件
    QVector<ActivityInfo> queryActivities(const QString& keyword, const QString& type,
        const QDate& start, const QDate& end);

    // 添加活动反馈
    bool addFeedback(int activityId, int userId, int rating, const QString& comment);

    // 查询活跃度排名
    struct ClubActiveInfo {
        QString name;
        int activityCount;
        double partRate;
        double budgetRate;
    };
    QVector<ClubActiveInfo> queryActiveClubs();

signals:
    void activityDataChanged();
    void feedbackSubmitted(bool success, const QString& message);
};

#endif // ACTIVITYMANAGER_H



