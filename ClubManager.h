#ifndef CLUBMANAGER_H
#define CLUBMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QDate>
#include <QVector>
#include <QString>

struct ClubInfo {
    int id = -1;
    QString name;
    QString type;
    QDate establishDate;
    QString advisor;
    int status = 0; // 0=�����,1=����׼,2=�Ѿܾ�
    QString pyAbbr; // ƴ������ĸ��д
};

class ClubManager : public QObject {
    Q_OBJECT
public:
    explicit ClubManager(QObject* parent = nullptr);

    // ���ֲ�����ӿ�
    ClubInfo queryClubByPresident(int userId);
    ClubInfo queryClubById(int clubId);
    bool updateClubInfo(const ClubInfo& info);
    QVector<ClubInfo> queryPendingClubs();
    bool approveClub(int clubId);
    bool rejectClub(int clubId);

    // ���߷���
    static QString toPinyinAbbr(const QString& text);
    bool checkTimeConflict(int clubId, const QDateTime& start, const QDateTime& end, const QString& location);

signals:
    void clubInfoChanged();

private:
    QString generateWhereClause(const QMap<QString, QVariant>& conditions) const;
};

#endif // CLUBMANAGER_H


