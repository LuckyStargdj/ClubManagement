#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent), m_connected(false) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // 获取应用数据目录
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty()) {
        qWarning() << "Failed to get app data location";
        dataDir = ".";
    }

    QDir dir(dataDir);
    if (!dir.exists()) dir.mkpath(".");

    QString dbPath = dir.filePath("club_management.db");
    m_db.setDatabaseName(dbPath);

    connect(&m_connectionCheckTimer, &QTimer::timeout, this, &DatabaseManager::checkConnection);
    m_connectionCheckTimer.start(5000); // 每5秒检查一次
}

bool DatabaseManager::openDatabase() {
    if (m_connected) return true;
    if (!m_db.open()) {
        qDebug() << "Database Error: " << m_db.lastError().text();
        setConnected(false);
        return false;
    }

    initDatabase();
    return true;
}

void DatabaseManager::initDatabase() {
    // 创建必要的数据表
    QSqlQuery query;

    // 用户表
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "role INTEGER NOT NULL,"
        "student_id TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "status INTEGER DEFAULT 1,"
        "club_id INTEGER DEFAULT -1)")) {
        qWarning() << "Failed to create users table:" << query.lastError();
    }

    // 社团表
    if (!query.exec("CREATE TABLE IF NOT EXISTS clubs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "type TEXT NOT NULL,"
        "establish_date DATE NOT NULL,"
        "advisor TEXT,"
        "status INTEGER DEFAULT 0,"
        "py_abbr TEXT DEFAULT '')")) {
        qWarning() << "Failed to create clubs table:" << query.lastError();
    }

    // 社团文件表
    if (!query.exec("CREATE TABLE IF NOT EXISTS club_files ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "club_id INTEGER NOT NULL,"
        "file_type INTEGER NOT NULL,"  // 0=章程,1=LOGO,2=其他
        "file_name TEXT NOT NULL,"
        "file_data BLOB)")) {
        qWarning() << "Failed to create club_files table:" << query.lastError();
    }

    // 活动表
    if (!query.exec("CREATE TABLE IF NOT EXISTS activities ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "club_id INTEGER NOT NULL,"
        "title TEXT NOT NULL,"
        "start_time DATETIME NOT NULL,"
        "end_time DATETIME NOT NULL,"
        "location TEXT NOT NULL,"
        "max_participants INTEGER NOT NULL,"
        "budget REAL NOT NULL,"
        "actual_budget REAL DEFAULT 0,"
        "status INTEGER DEFAULT 0," // 0=待审批,1=已批准,2=已拒绝
        "CONSTRAINT location_time UNIQUE (location, start_time))")) {
        qWarning() << "Failed to create activities table:" << query.lastError();
    }

    // 活动反馈表
    if (!query.exec("CREATE TABLE IF NOT EXISTS activity_feedback ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "activity_id INTEGER NOT NULL,"
        "user_id INTEGER NOT NULL,"
        "rating INTEGER NOT NULL CHECK (rating BETWEEN 1 AND 5),"
        "comment TEXT)")) {
        qWarning() << "Failed to create activity_feedback table:" << query.lastError();
    }

    // 添加测试数据（可选）
    if (query.exec("SELECT COUNT(*) FROM users") && query.next() && query.value(0).toInt() == 0) {
        query.exec("INSERT INTO users (name, role, student_id, password) VALUES "
            "('Admin', 0, '1000000000', 'admin123'),"
            "('张三', 1, '2023000001', 'zhangsan'),"
            "('李四', 2, '2023000002', 'lisi')");
    }

    setConnected(true);
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

QSqlQuery DatabaseManager::execQuery(const QString& sql, const QMap<QString, QVariant>& params) {
    QSqlQuery query;
    query.prepare(sql);

    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    if (!query.exec()) {
        qWarning() << "Query failed:" << query.lastError() << "\nSQL:" << sql;
    }
    return query;
}

int DatabaseManager::insertRecord(const QString& table, const QMap<QString, QVariant>& values) {
    if (table.isEmpty() || values.isEmpty()) return -1;

    QString columns;
    QString placeholders;
    QMapIterator<QString, QVariant> it(values);

    while (it.hasNext()) {
        it.next();
        if (!columns.isEmpty()) {
            columns += ", ";
            placeholders += ", ";
        }
        columns += it.key();
        placeholders += ":" + it.key();
    }

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(table).arg(columns).arg(placeholders);
    QSqlQuery query = execQuery(sql, values);

    if (query.lastError().isValid()) return -1;
    return query.lastInsertId().toInt();
}