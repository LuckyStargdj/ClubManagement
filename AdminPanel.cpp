#include "AdminPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QGroupBox>
#include <QMessageBox>
#include <QStatusBar>
#include <QLabel>
#include <QSpacerItem>
#include <QSqlQuery>
#include "DatabaseManager.h"

AdminPanel::AdminPanel(UserManager* userMgr, ClubManager* clubMgr, QWidget* parent)
    : QWidget(parent), m_userMgr(userMgr), m_clubMgr(clubMgr) {
    setupUI();
    DatabaseManager::instance().openDatabase();
    connect(&DatabaseManager::instance(), &DatabaseManager::connectionChanged,
        this, &AdminPanel::onDatabaseStatusChanged);
}

void AdminPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // �û�������
    QGroupBox* userGroup = new QGroupBox("�û�����", this);
    QVBoxLayout* userLayout = new QVBoxLayout(userGroup);

    createUserTable();
    userLayout->addWidget(userTable);

    // �û�������ť
    QHBoxLayout* userBtnLayout = new QHBoxLayout();
    resetPwdBtn = new QPushButton("��������", this);
    freezeBtn = new QPushButton("�����˻�", this);
    activateBtn = new QPushButton("�����˻�", this);

    connect(resetPwdBtn, &QPushButton::clicked, this, &AdminPanel::onResetPwdClicked);
    connect(freezeBtn, &QPushButton::clicked, this, &AdminPanel::onFreezeClicked);
    connect(activateBtn, &QPushButton::clicked, this, &AdminPanel::onActivateClicked);

    userBtnLayout->addWidget(resetPwdBtn);
    userBtnLayout->addWidget(freezeBtn);
    userBtnLayout->addWidget(activateBtn);

    userLayout->addLayout(userBtnLayout);
    mainLayout->addWidget(userGroup);

    // ������������
    QGroupBox* clubGroup = new QGroupBox("��������", this);
    QVBoxLayout* clubLayout = new QVBoxLayout(clubGroup);

    createClubTable();
    clubLayout->addWidget(clubTable);

    // ���Ų�����ť
    QHBoxLayout* clubBtnLayout = new QHBoxLayout();
    approveBtn = new QPushButton("��׼", this);
    rejectBtn = new QPushButton("�ܾ�", this);

    connect(approveBtn, &QPushButton::clicked, this, &AdminPanel::onApproveClubClicked);
    connect(rejectBtn, &QPushButton::clicked, this, &AdminPanel::onRejectClubClicked);

    clubBtnLayout->addWidget(approveBtn);
    clubBtnLayout->addWidget(rejectBtn);

    clubLayout->addLayout(clubBtnLayout);
    mainLayout->addWidget(clubGroup);

    // ״̬��
    statusBar = new QStatusBar(this);
    mainLayout->addWidget(statusBar);

    // ��ʼ������
    refreshUserTable();
    refreshClubTable();
    onDatabaseStatusChanged(DatabaseManager::instance().isConnected());
}

void AdminPanel::createUserTable() {
    userTable = new QTableWidget(this);
    userTable->setColumnCount(5);
    userTable->setHorizontalHeaderLabels(QStringList() << "ID" << "����" << "��ɫ" << "ѧ��/����" << "״̬");
    userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void AdminPanel::createClubTable() {
    clubTable = new QTableWidget(this);
    clubTable->setColumnCount(4);
    clubTable->setHorizontalHeaderLabels(QStringList() << "����ID" << "����" << "����" << "��������");
    clubTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    clubTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    clubTable->setSelectionMode(QAbstractItemView::SingleSelection);
    clubTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

int AdminPanel::getSelectedUserId() const {
    int row = userTable->currentRow();
    if (row < 0) return -1;
    return userTable->item(row, 0)->text().toInt();
}

int AdminPanel::getSelectedClubId() const {
    int row = clubTable->currentRow();
    if (row < 0) return -1;
    return clubTable->item(row, 0)->text().toInt();
}

void AdminPanel::refreshUserTable() {
    userTable->setRowCount(0);

    QSqlQuery query("SELECT id, name, role, student_id, status FROM users");
    int row = 0;
    while (query.next()) {
        userTable->insertRow(row);
        userTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        userTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));

        int role = query.value(2).toInt();
        QString roleStr = role == 0 ? "����Ա" : (role == 1 ? "���Ÿ�����" : "��ͨѧ��");
        userTable->setItem(row, 2, new QTableWidgetItem(roleStr));

        userTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));

        int status = query.value(4).toInt();
        userTable->setItem(row, 4, new QTableWidgetItem(status == 1 ? "����" : "����"));

        row++;
    }
}

void AdminPanel::refreshClubTable() {
    clubTable->setRowCount(0);
    QVector<ClubInfo> clubs = m_clubMgr->queryPendingClubs();

    for (int i = 0; i < clubs.size(); ++i) {
        const ClubInfo& club = clubs[i];
        clubTable->insertRow(i);
        clubTable->setItem(i, 0, new QTableWidgetItem(QString::number(club.id)));
        clubTable->setItem(i, 1, new QTableWidgetItem(club.name));
        clubTable->setItem(i, 2, new QTableWidgetItem(club.type));
        clubTable->setItem(i, 3, new QTableWidgetItem(club.establishDate.toString("yyyy-MM-dd")));
    }
}

void AdminPanel::onDatabaseStatusChanged(bool connected) {
    statusBar->showMessage(connected ? "���ݿ�������" : "���ݿ����ӶϿ�");
}

void AdminPanel::onResetPwdClicked() {
    int userId = getSelectedUserId();
    if (userId == -1) {
        QMessageBox::warning(this, "����ʧ��", "��ѡ��Ҫ����������û�");
        return;
    }

    if (QMessageBox::question(this, "ȷ�ϲ���", "ȷ��Ҫ���ø��û��������������뽫����Ϊ123456") != QMessageBox::Yes) {
        return;
    }

    if (m_userMgr->resetPassword(userId, "123456")) {
        QMessageBox::information(this, "�����ɹ�", "����������Ϊ123456");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "����ʧ��", "��������ʧ��");
    }
}

void AdminPanel::onFreezeClicked() {
    int userId = getSelectedUserId();
    if (userId == -1) {
        QMessageBox::warning(this, "����ʧ��", "��ѡ��Ҫ������û�");
        return;
    }

    if (QMessageBox::question(this, "ȷ�ϲ���", "ȷ��Ҫ������û��˻���") != QMessageBox::Yes) {
        return;
    }

    if (m_userMgr->freezeAccount(userId)) {
        QMessageBox::information(this, "�����ɹ�", "�˻��Ѷ���");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "����ʧ��", "�˻�����ʧ��");
    }
}

void AdminPanel::onActivateClicked() {
    int userId = getSelectedUserId();
    if (userId == -1) {
        QMessageBox::warning(this, "����ʧ��", "��ѡ��Ҫ������û�");
        return;
    }

    if (QMessageBox::question(this, "ȷ�ϲ���", "ȷ��Ҫ������û��˻���") != QMessageBox::Yes) {
        return;
    }

    if (m_userMgr->activateAccount(userId)) {
        QMessageBox::information(this, "�����ɹ�", "�˻��Ѽ���");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "����ʧ��", "�˻�����ʧ��");
    }
}

void AdminPanel::onApproveClubClicked() {
    int clubId = getSelectedClubId();
    if (clubId == -1) {
        QMessageBox::warning(this, "����ʧ��", "��ѡ��Ҫ����������");
        return;
    }

    if (QMessageBox::question(this, "ȷ�ϲ���", "ȷ��Ҫ��׼�����ų�����") != QMessageBox::Yes) {
        return;
    }

    if (m_clubMgr->approveClub(clubId)) {
        QMessageBox::information(this, "�����ɹ�", "��������׼����");
        refreshClubTable();
    }
    else {
        QMessageBox::warning(this, "����ʧ��", "������׼ʧ��");
    }
}

void AdminPanel::onRejectClubClicked() {
    int clubId = getSelectedClubId();
    if (clubId == -1) {
        QMessageBox::warning(this, "����ʧ��", "��ѡ��Ҫ�ܾ�������");
        return;
    }

    if (QMessageBox::question(this, "ȷ�ϲ���", "ȷ��Ҫ�ܾ�������������������ݽ���ɾ��") != QMessageBox::Yes) {
        return;
    }

    if (m_clubMgr->rejectClub(clubId)) {
        QMessageBox::information(this, "�����ɹ�", "���������Ѿܾ�");
        refreshClubTable();
    }
    else {
        QMessageBox::warning(this, "����ʧ��", "���žܾ�����ʧ��");
    }
}