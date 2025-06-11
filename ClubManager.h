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

    ClubInfo queryClubByPresident(int userId); // ��ѯ��ǰ����������
    bool updateClubInfo(const ClubInfo& info);
    QVector<ClubInfo> queryPendingClubs(); // ����Ա�����д���������
    bool approveClub(int clubId);
    bool rejectClub(int clubId);

signals:
    void clubInfoChanged();
};

#endif // CLUBMANAGER_H


