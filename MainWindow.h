#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma execution_character_set("utf-8")

#include <QMainWindow>
#include <QStackedWidget>
#include <QStatusBar>
#include "UserManager.h"
#include "DatabaseManager.h"

class AdminPanel;
class PresidentPanel;
class StudentPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(UserManager* userMgr, QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    void updateStatusBar();
    void onDatabaseStatusChanged(bool connected);

    QStackedWidget* stackedWidget;
    AdminPanel* adminPanel;
    PresidentPanel* presidentPanel;
    StudentPanel* studentPanel;
    QStatusBar* statusBar;

    UserManager* m_userMgr;
};

#endif // MAINWINDOW_H




