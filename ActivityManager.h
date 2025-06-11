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
    double rating; // ƽ����
};

class ActivityManager : public QObject {
    Q_OBJECT
public:
    explicit ActivityManager(QObject* parent = nullptr);

    // ��ѯ�����������
    QVector<ActivityInfo> queryActivities(const QString& keyword, const QString& type,
        const QDate& start, const QDate& end);

    // ��ӻ����
    bool addFeedback(int activityId, int userId, int rating, const QString& comment);

    // ��ѯ��Ծ������
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



