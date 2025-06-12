#include "MainWindow.h"
#include "AdminPanel.h"
#include "PresidentPanel.h"
#include "StudentPanel.h"
#include "ClubManager.h"
#include "ActivityManager.h"
#include "DatabaseManager.h"

#include <QLabel>

MainWindow::MainWindow(UserManager* userMgr, QWidget* parent)
    : QMainWindow(parent), m_userMgr(userMgr)
{
    setWindowTitle("���Ź���ϵͳ");
    resize(800, 600);

    setupUI();
    DatabaseManager::instance().openDatabase();
    connect(&DatabaseManager::instance(), &DatabaseManager::connectionChanged, this, &MainWindow::onDatabaseStatusChanged);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    // �����û���ɫ���ض�Ӧ���
    UserManager::Role role = m_userMgr->currentRole();
    if (role == UserManager::Admin) {
        adminPanel = new AdminPanel(m_userMgr, new ClubManager(), this);
        stackedWidget->addWidget(adminPanel);
        stackedWidget->setCurrentWidget(adminPanel);
    }
    else if (role == UserManager::President) {
        presidentPanel = new PresidentPanel(new ClubManager(), new ActivityManager(), m_userMgr->currentUserId(), this);
        stackedWidget->addWidget(presidentPanel);
        stackedWidget->setCurrentWidget(presidentPanel);
    }
    else {
        studentPanel = new StudentPanel(new ActivityManager(), m_userMgr->currentUserId(), this);
        stackedWidget->addWidget(studentPanel);
        stackedWidget->setCurrentWidget(studentPanel);
    }

    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    QString roleStr;
    switch (m_userMgr->currentRole()) {
    case UserManager::Admin: roleStr = "����Ա"; break;
    case UserManager::President: roleStr = "���Ÿ�����"; break;
    default: roleStr = "��ͨѧ��";
    }

    statusBar->showMessage(QString("�û�: %1 (%2) | ���ݿ�: %3")
        .arg(m_userMgr->currentUserName())
        .arg(roleStr)
        .arg(DatabaseManager::instance().isConnected() ? "������" : "δ����"));
}

void MainWindow::onDatabaseStatusChanged(bool connected)
{
    updateStatusBar();
}