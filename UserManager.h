#ifndef USERMANAGER_H
#define USERMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QString>
#include <QVariant>
#include <QRegularExpression>

class UserManager : public QObject {
    Q_OBJECT
public:
    enum Role { Admin = 0, President = 1, Student = 2 };
    Q_ENUM(Role)

    explicit UserManager(QObject* parent = nullptr);
    explicit UserManager(int userId, const QString& username, Role role, QObject* parent = nullptr);
    virtual ~UserManager();

    // 用户管理接口
    bool registerUser(const QString& name, Role role, const QString& studentId, const QString& password);
    bool login(const QString& username, const QString& password);
    bool resetPassword(int userId, const QString& newPassword);
    bool deleteAccount(int userId);
    bool freezeAccount(int userId);
    bool activateAccount(int userId);
    bool updateUserInfo(int userId, const QString& field, const QVariant& value);

    // 当前用户信息
    int currentUserId() const;
    QString currentUserName() const;
    Role currentRole() const;
    int currentClubId() const;

signals:
    void loginStatusChanged(bool success, const QString& message);
    void registerStatus(bool success, const QString& message);
    void userDataChanged();

private:
    bool validatePassword(const QString& password) const;
    bool validateStudentId(const QString& studentId) const;

    int m_currentUserId = -1;
    QString m_currentUserName;
    Role m_currentRole = Student;
    int m_currentClubId = -1;
};

#endif // USERMANAGER_H


