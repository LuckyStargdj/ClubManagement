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

    // 用户管理部分
    QGroupBox* userGroup = new QGroupBox("用户管理", this);
    QVBoxLayout* userLayout = new QVBoxLayout(userGroup);

    createUserTable();
    userLayout->addWidget(userTable);

    // 用户操作按钮
    QHBoxLayout* userBtnLayout = new QHBoxLayout();
    resetPwdBtn = new QPushButton("重置密码", this);
    freezeBtn = new QPushButton("冻结账户", this);
    activateBtn = new QPushButton("激活账户", this);

    connect(resetPwdBtn, &QPushButton::clicked, this, &AdminPanel::onResetPwdClicked);
    connect(freezeBtn, &QPushButton::clicked, this, &AdminPanel::onFreezeClicked);
    connect(activateBtn, &QPushButton::clicked, this, &AdminPanel::onActivateClicked);

    userBtnLayout->addWidget(resetPwdBtn);
    userBtnLayout->addWidget(freezeBtn);
    userBtnLayout->addWidget(activateBtn);

    userLayout->addLayout(userBtnLayout);
    mainLayout->addWidget(userGroup);

    // 社团审批部分
    QGroupBox* clubGroup = new QGroupBox("社团审批", this);
    QVBoxLayout* clubLayout = new QVBoxLayout(clubGroup);

    createClubTable();
    clubLayout->addWidget(clubTable);

    // 社团操作按钮
    QHBoxLayout* clubBtnLayout = new QHBoxLayout();
    approveBtn = new QPushButton("批准", this);
    rejectBtn = new QPushButton("拒绝", this);

    connect(approveBtn, &QPushButton::clicked, this, &AdminPanel::onApproveClubClicked);
    connect(rejectBtn, &QPushButton::clicked, this, &AdminPanel::onRejectClubClicked);

    clubBtnLayout->addWidget(approveBtn);
    clubBtnLayout->addWidget(rejectBtn);

    clubLayout->addLayout(clubBtnLayout);
    mainLayout->addWidget(clubGroup);

    // 状态栏
    statusBar = new QStatusBar(this);
    mainLayout->addWidget(statusBar);

    // 初始化数据
    refreshUserTable();
    refreshClubTable();
    onDatabaseStatusChanged(DatabaseManager::instance().isConnected());
}

void AdminPanel::createUserTable() {
    userTable = new QTableWidget(this);
    userTable->setColumnCount(5);
    userTable->setHorizontalHeaderLabels(QStringList() << "ID" << "姓名" << "角色" << "学号/工号" << "状态");
    userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void AdminPanel::createClubTable() {
    clubTable = new QTableWidget(this);
    clubTable->setColumnCount(4);
    clubTable->setHorizontalHeaderLabels(QStringList() << "社团ID" << "名称" << "类型" << "申请日期");
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
        QString roleStr = role == 0 ? "管理员" : (role == 1 ? "社团负责人" : "普通学生");
        userTable->setItem(row, 2, new QTableWidgetItem(roleStr));

        userTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));

        int status = query.value(4).toInt();
        userTable->setItem(row, 4, new QTableWidgetItem(status == 1 ? "正常" : "冻结"));

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
    statusBar->showMessage(connected ? "数据库已连接" : "数据库连接断开");
}

void AdminPanel::onResetPwdClicked() {
    int userId = getSelectedUserId();
    if (userId == -1) {
        QMessageBox::warning(this, "操作失败", "请选择要重置密码的用户");
        return;
    }

    if (QMessageBox::question(this, "确认操作", "确定要重置该用户的密码吗？新密码将设置为123456") != QMessageBox::Yes) {
        return;
    }

    if (m_userMgr->resetPassword(userId, "123456")) {
        QMessageBox::information(this, "操作成功", "密码已重置为123456");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "操作失败", "密码重置失败");
    }
}

void AdminPanel::onFreezeClicked() {
    int userId = getSelectedUserId();
    if (userId == -1) {
        QMessageBox::warning(this, "操作失败", "请选择要冻结的用户");
        return;
    }

    if (QMessageBox::question(this, "确认操作", "确定要冻结该用户账户吗？") != QMessageBox::Yes) {
        return;
    }

    if (m_userMgr->freezeAccount(userId)) {
        QMessageBox::information(this, "操作成功", "账户已冻结");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "操作失败", "账户冻结失败");
    }
}

void AdminPanel::onActivateClicked() {
    int userId = getSelectedUserId();
    if (userId == -1) {
        QMessageBox::warning(this, "操作失败", "请选择要激活的用户");
        return;
    }

    if (QMessageBox::question(this, "确认操作", "确定要激活该用户账户吗？") != QMessageBox::Yes) {
        return;
    }

    if (m_userMgr->activateAccount(userId)) {
        QMessageBox::information(this, "操作成功", "账户已激活");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "操作失败", "账户激活失败");
    }
}

void AdminPanel::onApproveClubClicked() {
    int clubId = getSelectedClubId();
    if (clubId == -1) {
        QMessageBox::warning(this, "操作失败", "请选择要审批的社团");
        return;
    }

    if (QMessageBox::question(this, "确认操作", "确定要批准该社团成立吗？") != QMessageBox::Yes) {
        return;
    }

    if (m_clubMgr->approveClub(clubId)) {
        QMessageBox::information(this, "操作成功", "社团已批准成立");
        refreshClubTable();
    }
    else {
        QMessageBox::warning(this, "操作失败", "社团批准失败");
    }
}

void AdminPanel::onRejectClubClicked() {
    int clubId = getSelectedClubId();
    if (clubId == -1) {
        QMessageBox::warning(this, "操作失败", "请选择要拒绝的社团");
        return;
    }

    if (QMessageBox::question(this, "确认操作", "确定要拒绝该社团申请吗？相关数据将被删除") != QMessageBox::Yes) {
        return;
    }

    if (m_clubMgr->rejectClub(clubId)) {
        QMessageBox::information(this, "操作成功", "社团申请已拒绝");
        refreshClubTable();
    }
    else {
        QMessageBox::warning(this, "操作失败", "社团拒绝操作失败");
    }
}