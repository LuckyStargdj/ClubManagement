#ifndef CLUBMANAGER_H
#define CLUBMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QDate>
#include <QVector>
#include <QString>

struct ClubInfo {
    int id;
    QString name;
    QString type;
    QDate establishDate;
    QString advisor;
    int status;
};

class ClubManager : public QObject {
    Q_OBJECT
public:
    explicit ClubManager(QObject* parent = nullptr);

    ClubInfo queryClubByPresident(int userId); // 查询当前负责人社团
    bool updateClubInfo(const ClubInfo& info);
    QVector<ClubInfo> queryPendingClubs(); // 管理员：所有待审批社团
    bool approveClub(int clubId);
    bool rejectClub(int clubId);

signals:
    void clubInfoChanged();
};

#endif // CLUBMANAGER_H


