#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent), m_connected(false) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("club_management.db");
    connect(&m_connectionCheckTimer, &QTimer::timeout, this, &DatabaseManager::checkConnection);
    m_connectionCheckTimer.start(5000);
}

bool DatabaseManager::openDatabase() {
    if (m_connected) return true;
    if (!m_db.open()) {
        qDebug() << "Database Error: " << m_db.lastError().text();
        setConnected(false);
        return false;
    }

    QSqlQuery query;

    // 用户表
    query.exec("CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "role INTEGER NOT NULL,"
        "student_id TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "status INTEGER DEFAULT 1)");

    // 社团表
    query.exec("CREATE TABLE IF NOT EXISTS clubs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "type TEXT NOT NULL,"
        "establish_date DATE NOT NULL,"
        "advisor TEXT,"
        "status INTEGER DEFAULT 0,"
        "py_abbr TEXT DEFAULT '',"
        "activity_score REAL DEFAULT 0)");

    // 社团文档
    query.exec("CREATE TABLE IF NOT EXISTS club_documents ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "club_id INTEGER NOT NULL,"
        "file_name TEXT NOT NULL,"
        "file_type TEXT NOT NULL,"
        "file_data BLOB,"
        "FOREIGN KEY(club_id) REFERENCES clubs(id))");

    // 活动表
    query.exec("CREATE TABLE IF NOT EXISTS activities ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "club_id INTEGER NOT NULL,"
        "title TEXT NOT NULL,"
        "start_time DATETIME NOT NULL,"
        "end_time DATETIME NOT NULL,"
        "location TEXT NOT NULL,"
        "max_participants INTEGER NOT NULL,"
        "budget REAL NOT NULL,"
        "actual_budget REAL DEFAULT 0,"
        "status INTEGER DEFAULT 0,"
        "FOREIGN KEY(club_id) REFERENCES clubs(id))");

    // 活动反馈表
    query.exec("CREATE TABLE IF NOT EXISTS activity_feedback ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "activity_id INTEGER NOT NULL,"
        "user_id INTEGER NOT NULL,"
        "rating INTEGER NOT NULL,"
        "comment TEXT,"
        "FOREIGN KEY(activity_id) REFERENCES activities(id),"
        "FOREIGN KEY(user_id) REFERENCES users(id))");

    setConnected(true);
    return true;
}

bool DatabaseManager::isConnected() const {
    return m_connected;
}

void DatabaseManager::setConnected(bool connected) {
    if (m_connected != connected) {
        m_connected = connected;
        emit connectionChanged(connected);
    }
}

void DatabaseManager::checkConnection() {
    bool currentlyConnected = m_db.isOpen();
    if (currentlyConnected) {
        QSqlQuery query("SELECT 1");
        currentlyConnected = query.isActive();
    }
    if (currentlyConnected != m_connected) {
        setConnected(currentlyConnected);
    }
}
