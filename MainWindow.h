#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma execution_character_set("utf-8")

#include <QMainWindow>
#include <QLabel>
#include <QStatusBar>
#include <QStackedWidget>
#include <QTimer>
#include "StudentPanel.h"
#include "PresidentPanel.h"
#include "AdminPanel.h"
#include "UserManager.h"
#include "ClubManager.h"
#include "ActivityManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    UserManager* getUserManager() const;

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onLogoutClicked();
    void onDataExportClicked();
    void updateRoleDisplay();
    void handleDatabaseStatusChange(bool connected);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();

    // UI components
    QLabel* m_userInfoLabel;
    QLabel* m_dbStatusLabel;
    QStatusBar* m_statusBar;
    QStackedWidget* m_roleStackedWidget;

    // Panels
    StudentPanel* m_studentPanel;
    PresidentPanel* m_presidentPanel;
    AdminPanel* m_adminPanel;

    // Manager instances
    UserManager* m_userManager;
    ClubManager* m_clubManager;
    ActivityManager* m_activityManager;

    QTimer* m_dbCheckTimer;

    // 当前登录用户id（示例，正式开发需完善登录用户信息管理）
    int m_currentUserId;
};

#endif // MAINWINDOW_H




