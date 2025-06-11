#include "MainWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QSqlQuery>
#include <QTextCodec>
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "DatabaseManager.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_userManager(new UserManager(this)),
    m_clubManager(new ClubManager(this)),
    m_activityManager(new ActivityManager(this)),
    m_currentUserId(-1)
{
    setWindowTitle("���Ź���ϵͳ");
    resize(1000, 700);

    setupUI();
    setupConnections();

    m_dbStatusLabel->setText(DatabaseManager::instance().isConnected() ?
        "�� ���ݿ���������" : "�� ���ݿ����ӶϿ�");
    m_dbStatusLabel->setStyleSheet(DatabaseManager::instance().isConnected() ?
        "color: green;" : "color: red;");

    m_dbCheckTimer = new QTimer(this);
    connect(m_dbCheckTimer, &QTimer::timeout, [this]() {
        bool connected = DatabaseManager::instance().isConnected();
        m_dbStatusLabel->setText(connected ? "�� ���ݿ���������" : "�� ���ݿ����ӶϿ�");
        m_dbStatusLabel->setStyleSheet(connected ? "color: green;" : "color: red;");
        });
    m_dbCheckTimer->start(5000);
}

MainWindow::~MainWindow() {
    delete m_dbCheckTimer;
}

UserManager* MainWindow::getUserManager() const {
    return m_userManager;
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    m_roleStackedWidget = new QStackedWidget(this);

    // ע�⴫��ҵ��������͵�ǰ�û�id������updateRoleDisplay�ᶯ̬�л�
    m_adminPanel = new AdminPanel(m_userManager, m_clubManager, this);
    m_presidentPanel = new PresidentPanel(m_clubManager, m_activityManager, m_currentUserId, this);
    m_studentPanel = new StudentPanel(m_activityManager, m_currentUserId, this);

    m_roleStackedWidget->addWidget(m_adminPanel);      // 0: ����Ա
    m_roleStackedWidget->addWidget(m_presidentPanel);  // 1: �糤
    m_roleStackedWidget->addWidget(m_studentPanel);    // 2: ѧ��

    mainLayout->addWidget(m_roleStackedWidget);

    setupMenuBar();
    setupStatusBar();
    updateRoleDisplay();
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();

    QMenu* fileMenu = menuBar->addMenu("�ļ�");
    QAction* exitAction = fileMenu->addAction("�˳�");
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    QMenu* userMenu = menuBar->addMenu("�û�");
    QAction* loginAction = userMenu->addAction("��¼");
    QAction* registerAction = userMenu->addAction("ע��");
    QAction* logoutAction = userMenu->addAction("ע��");
    connect(loginAction, &QAction::triggered, this, &MainWindow::onLoginClicked);
    connect(registerAction, &QAction::triggered, this, &MainWindow::onRegisterClicked);
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogoutClicked);

    QMenu* dataMenu = menuBar->addMenu("����");
    QAction* exportAction = dataMenu->addAction("���������(CSV)");
    connect(exportAction, &QAction::triggered, this, &MainWindow::onDataExportClicked);
}

void MainWindow::setupStatusBar() {
    m_statusBar = statusBar();
    m_userInfoLabel = new QLabel("δ��¼", this);
    m_statusBar->addWidget(m_userInfoLabel);
    m_dbStatusLabel = new QLabel(this);
    m_statusBar->addPermanentWidget(m_dbStatusLabel);
}

void MainWindow::setupConnections() {
    connect(&DatabaseManager::instance(), &DatabaseManager::connectionChanged, this, &MainWindow::handleDatabaseStatusChange);
}

void MainWindow::onLoginClicked() {
    LoginDialog dlg(m_userManager);
    if (dlg.exec() == QDialog::Accepted) {
        // ��UserManager��ȡ��ǰ��¼�û���Ϣ
        m_currentUserId = m_userManager->currentUserId();

        // ���¹���panel����Ϊÿ��panel��Ҫ����userId���룩
        delete m_presidentPanel;
        delete m_studentPanel;
        m_presidentPanel = new PresidentPanel(m_clubManager, m_activityManager, m_currentUserId, this);
        m_studentPanel = new StudentPanel(m_activityManager, m_currentUserId, this);
        m_roleStackedWidget->removeWidget(m_roleStackedWidget->widget(1));
        m_roleStackedWidget->insertWidget(1, m_presidentPanel);
        m_roleStackedWidget->removeWidget(m_roleStackedWidget->widget(2));
        m_roleStackedWidget->insertWidget(2, m_studentPanel);

        updateRoleDisplay();
        m_studentPanel->refreshActivityTable();
        m_studentPanel->refreshActiveClubTable();
        m_presidentPanel->refreshClubInfo();
        m_presidentPanel->refreshActivityTable();
        m_adminPanel->refreshUserTable();
        m_adminPanel->refreshClubTable();
    }
}

void MainWindow::onRegisterClicked() {
    RegisterDialog dlg(m_userManager);
    dlg.exec();
}

void MainWindow::onLogoutClicked() {
    m_userInfoLabel->setText("δ��¼");
    m_roleStackedWidget->setCurrentIndex(2); // Ĭ���е�ѧ��
    m_currentUserId = -1;
}

void MainWindow::onDataExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "����CSV", "", "CSV�ļ� (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "����ʧ��", "�޷�д���ļ�");
        return;
    }

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextStream out(&file);
    out.setGenerateByteOrderMark(true);
    out.setCodec(codec);

    out << "�ID,����,��ʼʱ��,����ʱ��,�ص�,�������,Ԥ��,״̬\n";
    QSqlQuery query("SELECT id, title, start_time, end_time, location, max_participants, budget, status FROM activities");
    while (query.next()) {
        for (int i = 0; i < 8; ++i)
            out << "\"" << query.value(i).toString().replace("\"", "\"\"") << (i < 7 ? "\"," : "\"\n");
    }
    out.flush();
    file.close();
    QMessageBox::information(this, "�����ɹ�", "������ѵ�����");
}


void MainWindow::updateRoleDisplay() {
    UserManager::Role role = m_userManager->currentRole();
    QString userName = m_userManager->currentUserName();

    switch (role) {
    case UserManager::Admin:
        m_userInfoLabel->setText(tr("����Ա: %1").arg(userName));
        m_roleStackedWidget->setCurrentIndex(0);
        break;
    case UserManager::President:
        m_userInfoLabel->setText(tr("�糤: %1").arg(userName));
        m_roleStackedWidget->setCurrentIndex(1);
        break;
    case UserManager::Student:
    default:
        m_userInfoLabel->setText(tr("ѧ��: %1").arg(userName));
        m_roleStackedWidget->setCurrentIndex(2);
        break;
    }
}

void MainWindow::handleDatabaseStatusChange(bool connected) {
    m_dbStatusLabel->setText(connected ? "�� ���ݿ���������" : "�� ���ݿ����ӶϿ�");
    m_dbStatusLabel->setStyleSheet(connected ? "color: green;" : "color: red;");
}
