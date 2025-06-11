#include "PresidentPanel.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTableWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTime>
#include <QGroupBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>

PresidentPanel::PresidentPanel(ClubManager* clubMgr, ActivityManager* actMgr, int userId, QWidget* parent)
    : QWidget(parent), m_clubMgr(clubMgr), m_actMgr(actMgr), m_userId(userId)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* clubInfoGroup = new QGroupBox("社团信息管理", this);
    QFormLayout* formLayout = new QFormLayout(clubInfoGroup);
    clubNameEdit = new QLineEdit(clubInfoGroup);
    clubTypeCombo = new QComboBox(clubInfoGroup);
    clubTypeCombo->addItems({ "学术类", "文艺类", "体育类", "公益类" });
    establishDateEdit = new QDateEdit(QDate::currentDate(), clubInfoGroup);
    advisorEdit = new QLineEdit(clubInfoGroup);
    saveClubBtn = new QPushButton("保存信息", clubInfoGroup);
    formLayout->addRow("社团名称:", clubNameEdit);
    formLayout->addRow("社团类型:", clubTypeCombo);
    formLayout->addRow("成立日期:", establishDateEdit);
    formLayout->addRow("指导老师:", advisorEdit);
    formLayout->addRow(saveClubBtn);
    layout->addWidget(clubInfoGroup);

    QGroupBox* activityGroup = new QGroupBox("活动管理", this);
    QVBoxLayout* activityLayout = new QVBoxLayout(activityGroup);
    activityTable = new QTableWidget(0, 4, activityGroup);
    activityTable->setHorizontalHeaderLabels({ "活动ID", "名称", "时间", "状态" });
    activityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    applyActivityBtn = new QPushButton("申请新活动", activityGroup);
    activityLayout->addWidget(activityTable);
    activityLayout->addWidget(applyActivityBtn);
    layout->addWidget(activityGroup);

    connect(saveClubBtn, &QPushButton::clicked, this, &PresidentPanel::onSaveClubClicked);
    connect(applyActivityBtn, &QPushButton::clicked, this, &PresidentPanel::onApplyActivityClicked);
    connect(m_clubMgr, &ClubManager::clubInfoChanged, this, &PresidentPanel::onDataChanged);
    connect(m_actMgr, &ActivityManager::activityDataChanged, this, &PresidentPanel::onDataChanged);

    refreshClubInfo();
    refreshActivityTable();
}

void PresidentPanel::refreshClubInfo() {
    m_myClub = m_clubMgr->queryClubByPresident(m_userId);
    clubNameEdit->setText(m_myClub.name);
    clubTypeCombo->setCurrentText(m_myClub.type);
    establishDateEdit->setDate(m_myClub.establishDate);
    advisorEdit->setText(m_myClub.advisor);
}

void PresidentPanel::refreshActivityTable() {
    m_myActs.clear();
    // 按 club_id 查询活动
    QSqlQuery q;
    q.prepare("SELECT id, title, start_time, status FROM activities WHERE club_id=:cid");
    q.bindValue(":cid", m_myClub.id);
    q.exec();
    activityTable->setRowCount(0);
    int row = 0;
    while (q.next()) {
        activityTable->insertRow(row);
        for (int i = 0; i < 4; ++i)
            activityTable->setItem(row, i, new QTableWidgetItem(q.value(i).toString()));
        ++row;
    }
}

void PresidentPanel::onSaveClubClicked() {
    ClubInfo info = m_myClub;
    info.name = clubNameEdit->text().trimmed();
    info.type = clubTypeCombo->currentText();
    info.establishDate = establishDateEdit->date();
    info.advisor = advisorEdit->text().trimmed();
    if (info.name.isEmpty() || info.advisor.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "名称和指导老师不能为空");
        return;
    }
    if (m_clubMgr->updateClubInfo(info))
        QMessageBox::information(this, "成功", "已保存社团信息！");
    else
        QMessageBox::warning(this, "失败", "数据库错误");
}

void PresidentPanel::onApplyActivityClicked() {
    // 实际开发建议弹窗获取活动信息
    int clubId = m_myClub.id;
    QString title = QInputDialog::getText(this, "新活动", "活动标题：");
    if (title.isEmpty()) return;
    QDateTime start = QDateTime::currentDateTime();
    QDateTime end = QDateTime::currentDateTime().addSecs(3600);
    QString location = "社团活动室";
    int maxParticipants = 100;
    double budget = 500.0;
    QSqlQuery query;
    query.prepare("INSERT INTO activities (club_id, title, start_time, end_time, location, max_participants, budget, status) "
        "VALUES (:clubId, :title, :startTime, :endTime, :location, :maxParticipants, :budget, 0)");
    query.bindValue(":clubId", clubId);
    query.bindValue(":title", title);
    query.bindValue(":startTime", start);
    query.bindValue(":endTime", end);
    query.bindValue(":location", location);
    query.bindValue(":maxParticipants", maxParticipants);
    query.bindValue(":budget", budget);
    if (query.exec()) {
        QMessageBox::information(this, "申请成功", "活动已申请，等待审批");
        m_actMgr->activityDataChanged(); // 通知刷新
    }
    else {
        QMessageBox::warning(this, "失败", "申请失败: " + query.lastError().text());
    }
}

void PresidentPanel::onDataChanged() {
    refreshClubInfo();
    refreshActivityTable();
}
