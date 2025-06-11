#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QSqlDatabase>
#include <QTimer>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance();
    bool openDatabase();
    bool isConnected() const;
    void checkConnection();

signals:
    void connectionChanged(bool connected);

private:
    DatabaseManager(QObject* parent = nullptr);
    void setConnected(bool connected);
    QSqlDatabase m_db;
    bool m_connected = false;
    QTimer m_connectionCheckTimer;
};

#endif // DATABASEMANAGER_H


