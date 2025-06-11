#ifndef USERMANAGER_H
#define USERMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QString>
#include <QVariant>

class UserManager : public QObject {
    Q_OBJECT
public:
    enum Role { Admin = 0, President = 1, Student = 2 };
    Q_ENUM(Role)

    explicit UserManager(QObject* parent = nullptr);
    explicit UserManager(int userid, const QString& username, Role role, QObject* parent = nullptr);
    virtual ~UserManager();

    bool registerUser(const QString& name, Role role, const QString& studentId, const QString& password);
    bool login(const QString& username, const QString& password);
    bool resetPassword(int userId, const QString& newPassword);
    bool deleteAccount(int userId);
    bool freezeAccount(int userId);
    bool updateUserInfo(int userId, const QString& field, const QVariant& value);

    int currentUserId() const;
    QString currentUserName() const;
    Role currentRole() const;

signals:
    void loginStatusChanged(bool success, const QString& message);
    void registerStatus(bool success, const QString& message);

private:
    int m_currentUserId = -1;
    QString m_currentUserName;
    Role m_currentRole = Student;
};

#endif // USERMANAGER_H


