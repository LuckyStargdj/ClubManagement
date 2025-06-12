#include "UserManager.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QRegularExpression>

UserManager::UserManager(QObject* parent) : QObject(parent) {}
UserManager::UserManager(int userid, const QString& username, Role role, QObject* parent) {
    m_currentUserId = userid;
    m_currentUserName = username;
    m_currentRole = role;
}
UserManager::~UserManager() {}

bool UserManager::validateStudentId(const QString& studentId) const {
    QRegularExpression regex("^\\d{10}$");
    return regex.match(studentId).hasMatch();
}

bool UserManager::validatePassword(const QString& password) const {
    return password.length() >= 6;
}

bool UserManager::registerUser(const QString& name, Role role, const QString& studentId, const QString& password) {
    if (!validateStudentId(studentId)) {
        emit registerStatus(false, "学号格式错误（需10位数字）");
        return false;
    }

    if (!validatePassword(password)) {
        emit registerStatus(false, "密码长度至少为6位");
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE student_id = :studentId");
    query.bindValue(":studentId", studentId);
    if (!query.exec() || !query.next()) {
        emit registerStatus(false, "数据库查询失败");
        return false;
    }

    if (query.value(0).toInt() > 0) {
        emit registerStatus(false, "该学号已注册");
        return false;
    }

    // 加密密码（简单示例，实际应使用更安全的方法）
    QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    // 新用户
    QMap<QString, QVariant> values;
    values["name"] = name;
    values["role"] = static_cast<int>(role);
    values["student_id"] = studentId;
    values["password"] = passwordHash;

    int newId = DatabaseManager::instance().insertRecord("users", values);

    if (newId > 0) {
        emit registerStatus(true, "注册成功");
        return true;
    }
    else {
        emit registerStatus(false, "注册失败");
        return false;
    }
}

bool UserManager::login(const QString& username, const QString& password) {
    if (username.isEmpty() || password.isEmpty()) {
        emit loginStatusChanged(false, "用户名和密码不能为空");
        return false;
    }

    QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    QSqlQuery query;
    query.prepare("SELECT id, name, role, password, status, club_id "
        "FROM users WHERE student_id = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
        emit loginStatusChanged(false, "用户不存在");
        return false;
    }

    if (query.value("status").toInt() == 0) {
        emit loginStatusChanged(false, "账户已冻结");
        return false;
    }

    QString savedPassword = query.value("password").toString();
    if (passwordHash != savedPassword) {
        emit loginStatusChanged(false, "密码错误");
        return false;
    }

    m_currentUserId = query.value("id").toInt();
    m_currentUserName = query.value("name").toString();
    m_currentRole = static_cast<Role>(query.value("role").toInt());
    m_currentClubId = query.value("club_id").toInt();

    emit loginStatusChanged(true, "登录成功");
    return true;
}

bool UserManager::resetPassword(int userId, const QString& newPassword) {
    if (newPassword.isEmpty() || !validatePassword(newPassword)) return false;

    QString passwordHash = QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256).toHex();

    QSqlQuery query;
    query.prepare("UPDATE users SET password = :password WHERE id = :userId");
    query.bindValue(":password", passwordHash);
    query.bindValue(":userId", userId);
    return query.exec();
}

bool UserManager::deleteAccount(int userId) {
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE id = :userId");
    query.bindValue(":userId", userId);

    bool success = query.exec();
    if (success && userId == m_currentUserId) {
        m_currentUserId = -1;
        m_currentUserName.clear();
        m_currentRole = Student;
    }
    return success;
}

bool UserManager::freezeAccount(int userId) {
    QSqlQuery query;
    query.prepare("UPDATE users SET status = 0 WHERE id = :userId");
    query.bindValue(":userId", userId);
    bool success = query.exec();
    if (success) emit userDataChanged();
    return success;
}

bool UserManager::activateAccount(int userId) {
    QSqlQuery query;
    query.prepare("UPDATE users SET status = 1 WHERE id = :userId");
    query.bindValue(":userId", userId);
    bool success = query.exec();
    if (success) emit userDataChanged();
    return success;
}

bool UserManager::updateUserInfo(int userId, const QString& field, const QVariant& value) {
    QStringList allowedFields = { "name", "password" };
    if (!allowedFields.contains(field)) return false;

    QMap<QString, QVariant> updates;
    if (field == "password") {
        if (!validatePassword(value.toString())) return false;
        QString passwordHash = QCryptographicHash::hash(value.toString().toUtf8(), QCryptographicHash::Sha256).toHex();
        updates[field] = passwordHash;
    }
    else {
        updates[field] = value;
    }

    QSqlQuery query;
    QString sql = QString("UPDATE users SET %1 = :value WHERE id = :userId").arg(field);
    query.prepare(sql);
    query.bindValue(":value", updates[field]);
    query.bindValue(":userId", userId);

    bool success = query.exec();
    if (success) emit userDataChanged();
    return success;
}

int UserManager::currentUserId() const {
    return m_currentUserId;
}

QString UserManager::currentUserName() const {
    return m_currentUserName;
}

UserManager::Role UserManager::currentRole() const {
    return m_currentRole;
}

int UserManager::currentClubId() const {
    return m_currentClubId;
}