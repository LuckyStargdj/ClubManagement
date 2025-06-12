#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QSqlDatabase>
#include <QTimer>
#include <QFile>
#include <QMap>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance();
    bool openDatabase();
    bool isConnected() const;
    void checkConnection();

    // Êý¾Ý¿â²Ù×÷
    QSqlQuery execQuery(const QString& sql, const QMap<QString, QVariant>& params = QMap<QString, QVariant>());
    int insertRecord(const QString& table, const QMap<QString, QVariant>& values);

signals:
    void connectionChanged(bool connected);

private:
    DatabaseManager(QObject* parent = nullptr);
    void setConnected(bool connected);
    void initDatabase();

    QSqlDatabase m_db;
    bool m_connected = false;
    QTimer m_connectionCheckTimer;
};

#endif // DATABASEMANAGER_H

