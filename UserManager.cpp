#include "UserManager.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QDebug>

UserManager::UserManager(QObject* parent) : QObject(parent) {}
UserManager::UserManager(int userid, const QString& username, Role role, QObject* parent) {
    m_currentUserId = userid;
    m_currentUserName = username;
    m_currentRole = role;
}
UserManager::~UserManager() {}

bool UserManager::registerUser(const QString& name, Role role, const QString& studentId, const QString& password)
{
    QRegularExpression regex("^\\d{10}$");
    if (!regex.match(studentId).hasMatch()) {
        emit registerStatus(false, "ѧ�Ÿ�ʽ������10λ���֣�");
        return false;
    }
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE student_id = :studentId");
    query.bindValue(":studentId", studentId);
    if (!query.exec() || (query.next() && query.value(0).toInt() > 0)) {
        emit registerStatus(false, "ѧ���Ѵ��ڻ��ѯʧ��");
        return false;
    }
    // ���û�
    query.prepare("INSERT INTO users (name, role, student_id, password, status) VALUES (:name, :role, :studentId, :password, 1)");
    query.bindValue(":name", name);
    query.bindValue(":role", static_cast<int>(role));
    query.bindValue(":studentId", studentId);
    query.bindValue(":password", password);
    if (!query.exec()) {
        emit registerStatus(false, "ע��ʧ��: " + query.lastError().text());
        return false;
    }
    emit registerStatus(true, "ע��ɹ�");
    return true;
}

bool UserManager::login(const QString& username, const QString& password)
{
    if (username.isEmpty() || password.isEmpty()) {
        emit loginStatusChanged(false, "�û��������벻��Ϊ��");
        return false;
    }
    QSqlQuery query;
    query.prepare("SELECT id, name, role, password, status FROM users WHERE student_id = :username");
    query.bindValue(":username", username);
    if (!query.exec() || !query.next()) {
        emit loginStatusChanged(false, "�û�������");
        return false;
    }
    if (query.value("status").toInt() == 0) {
        emit loginStatusChanged(false, "�˻��Ѷ���");
        return false;
    }
    QString savedPassword = query.value("password").toString();
    if (password != savedPassword) {
        emit loginStatusChanged(false, "�������");
        return false;
    }
    m_currentUserId = query.value("id").toInt();
    m_currentUserName = query.value("name").toString();
    m_currentRole = static_cast<Role>(query.value("role").toInt());
    emit loginStatusChanged(true, "��¼�ɹ�");
    return true;
}

bool UserManager::resetPassword(int userId, const QString& newPassword) {
    if (newPassword.isEmpty()) return false;
    QSqlQuery query;
    query.prepare("UPDATE users SET password = :password WHERE id = :userId");
    query.bindValue(":password", newPassword);
    query.bindValue(":userId", userId);
    return query.exec();
}
bool UserManager::deleteAccount(int userId) {
    if (userId <= 0) return false;
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE id = :userId");
    query.bindValue(":userId", userId);
    if (!query.exec() || query.numRowsAffected() == 0) return false;
    if (userId == m_currentUserId) {
        m_currentUserId = -1;
        m_currentUserName.clear();
        m_currentRole = Student;
    }
    return true;
}
bool UserManager::freezeAccount(int userId) {
    QSqlQuery query;
    query.prepare("UPDATE users SET status = 0 WHERE id = :userId");
    query.bindValue(":userId", userId);
    return query.exec();
}

bool UserManager::updateUserInfo(int userId, const QString& field, const QVariant& value) {
    QStringList allowedFields = { "name", "password" };
    if (!allowedFields.contains(field)) return false;
    QSqlQuery query;
    query.prepare("UPDATE users SET " + field + "=:value WHERE id=:userId");
    query.bindValue(":value", value);
    query.bindValue(":userId", userId);
    return query.exec();
}

int UserManager::currentUserId() const {
    return m_currentUserId;
}

QString UserManager::currentUserName() const{
    return m_currentUserName;
}

UserManager::Role UserManager::currentRole() const {
    return m_currentRole;
}
