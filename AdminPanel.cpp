#include "AdminPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

AdminPanel::AdminPanel(UserManager* userMgr, ClubManager* clubMgr, QWidget* parent)
    : QWidget(parent), m_userMgr(userMgr), m_clubMgr(clubMgr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    // 用户管理表格
    userTable = new QTableWidget(0, 4, this);
    userTable->setHorizontalHeaderLabels({ "ID", "姓名", "角色", "状态" });
    userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(userTable);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    resetPwdBtn = new QPushButton("重置密码", this);
    freezeBtn = new QPushButton("冻结账户", this);
    activateBtn = new QPushButton("激活账户", this);
    buttonLayout->addWidget(resetPwdBtn);
    buttonLayout->addWidget(freezeBtn);
    buttonLayout->addWidget(activateBtn);
    layout->addLayout(buttonLayout);

    // 社团审批表格
    clubTable = new QTableWidget(0, 3, this);
    clubTable->setHorizontalHeaderLabels({ "社团ID", "名称", "申请日期" });
    clubTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(clubTable);

    QHBoxLayout* clubBtnLayout = new QHBoxLayout();
    approveBtn = new QPushButton("批准", this);
    rejectBtn = new QPushButton("拒绝", this);
    clubBtnLayout->addWidget(approveBtn);
    clubBtnLayout->addWidget(rejectBtn);
    layout->addLayout(clubBtnLayout);

    // 信号槽连接
    connect(resetPwdBtn, &QPushButton::clicked, this, &AdminPanel::onResetPwdClicked);
    connect(freezeBtn, &QPushButton::clicked, this, &AdminPanel::onFreezeClicked);
    connect(activateBtn, &QPushButton::clicked, this, &AdminPanel::onActivateClicked);
    connect(approveBtn, &QPushButton::clicked, this, &AdminPanel::onApproveClubClicked);
    connect(rejectBtn, &QPushButton::clicked, this, &AdminPanel::onRejectClubClicked);

    connect(m_clubMgr, &ClubManager::clubInfoChanged, this, &AdminPanel::onDataChanged);

    refreshUserTable();
    refreshClubTable();
}

void AdminPanel::refreshUserTable() {
    userTable->setRowCount(0);
    QSqlQuery query("SELECT id, name, role, status FROM users");
    int row = 0;
    while (query.next()) {
        userTable->insertRow(row);
        userTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        userTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        int roleVal = query.value(2).toInt();
        QString roleStr = "普通学生";
        if (roleVal == 0) roleStr = "管理员";
        else if (roleVal == 1) roleStr = "社团负责人";
        userTable->setItem(row, 2, new QTableWidgetItem(roleStr));
        QString statusStr = (query.value(3).toInt() == 1 ? "正常" : "冻结");
        userTable->setItem(row, 3, new QTableWidgetItem(statusStr));
        ++row;
    }
}

void AdminPanel::refreshClubTable() {
    QVector<ClubInfo> clubs = m_clubMgr->queryPendingClubs();
    clubTable->setRowCount(0);
    for (int row = 0; row < clubs.size(); ++row) {
        const ClubInfo& c = clubs[row];
        clubTable->insertRow(row);
        clubTable->setItem(row, 0, new QTableWidgetItem(QString::number(c.id)));
        clubTable->setItem(row, 1, new QTableWidgetItem(c.name));
        clubTable->setItem(row, 2, new QTableWidgetItem(c.establishDate.toString("yyyy-MM-dd")));
    }
}

// 用户操作
void AdminPanel::onResetPwdClicked() {
    int row = userTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请选择要重置密码的用户");
        return;
    }
    int userId = userTable->item(row, 0)->text().toInt();
    if (m_userMgr->resetPassword(userId, "123456"))
        QMessageBox::information(this, "提示", "密码已重置为123456");
    else
        QMessageBox::warning(this, "失败", "重置失败");
}

void AdminPanel::onFreezeClicked() {
    int row = userTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请选择要冻结的账户");
        return;
    }
    int userId = userTable->item(row, 0)->text().toInt();
    if (m_userMgr->freezeAccount(userId)) {
        QMessageBox::information(this, "提示", "已冻结账户");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "失败", "冻结失败");
    }
}

void AdminPanel::onActivateClicked() {
    int row = userTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请选择要激活的账户");
        return;
    }
    int userId = userTable->item(row, 0)->text().toInt();
    QSqlQuery q;
    q.prepare("UPDATE users SET status=1 WHERE id=:id");
    q.bindValue(":id", userId);
    if (q.exec()) {
        QMessageBox::information(this, "提示", "账户已激活");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "失败", "激活失败");
    }
}

// 社团审批
void AdminPanel::onApproveClubClicked() {
    int row = clubTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请选择要审批的社团");
        return;
    }
    int clubId = clubTable->item(row, 0)->text().toInt();
    if (m_clubMgr->approveClub(clubId)) {
        QMessageBox::information(this, "审批成功", "社团已批准！");
        refreshClubTable();
    }
    else {
        QMessageBox::warning(this, "失败", "审批失败");
    }
}

void AdminPanel::onRejectClubClicked() {
    int row = clubTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请选择要拒绝的社团");
        return;
    }
    int clubId = clubTable->item(row, 0)->text().toInt();
    if (m_clubMgr->rejectClub(clubId)) {
        QMessageBox::information(this, "操作成功", "社团已拒绝并删除！");
        refreshClubTable();
    }
    else {
        QMessageBox::warning(this, "失败", "删除失败");
    }
}

// 数据变更信号刷新
void AdminPanel::onDataChanged() {
    refreshUserTable();
    refreshClubTable();
}
