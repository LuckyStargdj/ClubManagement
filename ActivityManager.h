#ifndef ACTIVITYMANAGER_H
#define ACTIVITYMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QStringList>
#include <QVariant>
#include <QSqlQuery>

struct ActivityInfo {
    int id;
    QString title;
    QDateTime startTime;
    QDateTime endTime;
    QString location;
    QString clubName;
    double rating; // ƽ����
    int maxParticipants;
    double budget;
    int status;
};

class ActivityManager : public QObject {
    Q_OBJECT
public:
    explicit ActivityManager(QObject* parent = nullptr);

    // �����ӿ�
    QVector<ActivityInfo> queryActivities(const QString& keyword, const QString& type,
        const QDate& start, const QDate& end);

    QVector<ActivityInfo> queryActivitiesByPinyin(const QString& pinyinAbbr,
        const QString& type, const QDate& start, const QDate& end);

    bool addActivity(int clubId, const QString& title, const QDateTime& start,
        const QDateTime& end, const QString& location,
        int maxParticipants, double budget);

    // �����
    bool addFeedback(int activityId, int userId, int rating, const QString& comment);

    // ��Ծ������
    struct ClubActiveInfo {
        QString name;
        int activityCount;
        double participationRate;
        double budgetEfficiency;
        double activityScore;
    };
    QVector<ClubActiveInfo> calculateClubActiveRanking();

signals:
    void activityDataChanged();
    void feedbackSubmitted(bool success, const QString& message);
    void conflictDetected(const QString& message);

private:
    ActivityInfo parseActivityFromQuery(const QSqlQuery& query) const;
};

#endif // ACTIVITYMANAGER_H


