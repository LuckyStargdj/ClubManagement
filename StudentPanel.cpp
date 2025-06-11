#include "StudentPanel.h"
#include <QVBoxLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>

StudentPanel::StudentPanel(ActivityManager* activityMgr, int userId, QWidget* parent)
    : QWidget(parent), m_activityMgr(activityMgr), m_userId(userId)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    // 搜索部分
    QGroupBox* searchGroup = new QGroupBox("活动查询", this);
    QFormLayout* searchLayout = new QFormLayout(searchGroup);
    searchEdit = new QLineEdit(searchGroup);
    typeCombo = new QComboBox(searchGroup);
    typeCombo->addItems({ "全部类型", "讲座", "比赛", "展览", "演出" });
    startDateEdit = new QDateEdit(QDate::currentDate(), searchGroup);
    endDateEdit = new QDateEdit(QDate::currentDate().addDays(30), searchGroup);
    searchLayout->addRow("关键词:", searchEdit);
    searchLayout->addRow("活动类型:", typeCombo);
    searchLayout->addRow("开始日期:", startDateEdit);
    searchLayout->addRow("结束日期:", endDateEdit);
    searchBtn = new QPushButton("搜索", searchGroup);
    searchLayout->addRow(searchBtn);
    layout->addWidget(searchGroup);

    // 活动表
    activityTable = new QTableWidget(0, 5, this);
    activityTable->setHorizontalHeaderLabels({ "活动名称", "时间", "地点", "社团", "评分" });
    activityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(activityTable);

    feedbackBtn = new QPushButton("反馈评分", this);
    layout->addWidget(feedbackBtn);

    // 活跃度排名
    QGroupBox* rankGroup = new QGroupBox("社团活跃度排名", this);
    QVBoxLayout* rankLayout = new QVBoxLayout(rankGroup);
    activeClubTable = new QTableWidget(0, 4, rankGroup);
    activeClubTable->setHorizontalHeaderLabels({ "社团名称", "活动次数", "参与率", "经费使用效率" });
    activeClubTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    rankLayout->addWidget(activeClubTable);
    layout->addWidget(rankGroup);

    connect(searchBtn, &QPushButton::clicked, this, &StudentPanel::onSearchClicked);
    connect(feedbackBtn, &QPushButton::clicked, this, &StudentPanel::onFeedbackClicked);
    connect(m_activityMgr, &ActivityManager::feedbackSubmitted, this, &StudentPanel::onFeedbackResult);
    connect(m_activityMgr, &ActivityManager::activityDataChanged, this, &StudentPanel::refreshActivityTable);

    refreshActivityTable();
    refreshActiveClubTable();
}

void StudentPanel::refreshActivityTable() {
    m_currentActs = m_activityMgr->queryActivities("", "", QDate::currentDate().addMonths(-1), QDate::currentDate().addMonths(1));
    activityTable->setRowCount(0);
    for (int row = 0; row < m_currentActs.size(); ++row) {
        const ActivityInfo& act = m_currentActs[row];
        activityTable->insertRow(row);
        activityTable->setItem(row, 0, new QTableWidgetItem(act.title));
        activityTable->setItem(row, 1, new QTableWidgetItem(act.startTime.toString("yyyy-MM-dd hh:mm")));
        activityTable->setItem(row, 2, new QTableWidgetItem(act.location));
        activityTable->setItem(row, 3, new QTableWidgetItem(act.clubName));
        activityTable->setItem(row, 4, new QTableWidgetItem(QString::number(act.rating, 'f', 2)));
    }
}

void StudentPanel::refreshActiveClubTable() {
    QVector<ActivityManager::ClubActiveInfo> clubs = m_activityMgr->queryActiveClubs();
    activeClubTable->setRowCount(0);
    for (int row = 0; row < clubs.size(); ++row) {
        const auto& c = clubs[row];
        activeClubTable->insertRow(row);
        activeClubTable->setItem(row, 0, new QTableWidgetItem(c.name));
        activeClubTable->setItem(row, 1, new QTableWidgetItem(QString::number(c.activityCount)));
        activeClubTable->setItem(row, 2, new QTableWidgetItem(QString::number(c.partRate, 'f', 2)));
        activeClubTable->setItem(row, 3, new QTableWidgetItem(QString::number(c.budgetRate, 'f', 2)));
    }
}

void StudentPanel::onSearchClicked() {
    QString keyword = searchEdit->text().trimmed();
    QString type = typeCombo->currentText();
    QDate start = startDateEdit->date();
    QDate end = endDateEdit->date();
    m_currentActs = m_activityMgr->queryActivities(keyword, type, start, end);
    activityTable->setRowCount(0);
    for (int row = 0; row < m_currentActs.size(); ++row) {
        const ActivityInfo& act = m_currentActs[row];
        activityTable->insertRow(row);
        activityTable->setItem(row, 0, new QTableWidgetItem(act.title));
        activityTable->setItem(row, 1, new QTableWidgetItem(act.startTime.toString("yyyy-MM-dd hh:mm")));
        activityTable->setItem(row, 2, new QTableWidgetItem(act.location));
        activityTable->setItem(row, 3, new QTableWidgetItem(act.clubName));
        activityTable->setItem(row, 4, new QTableWidgetItem(QString::number(act.rating, 'f', 2)));
    }
}

void StudentPanel::onFeedbackClicked() {
    int row = activityTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择一个活动");
        return;
    }
    int activityId = m_currentActs[row].id;
    bool ok = false;
    int rating = QInputDialog::getInt(this, "评分", "请输入评分(1-5):", 5, 1, 5, 1, &ok);
    if (!ok) return;
    QString comment = QInputDialog::getText(this, "评价", "请填写你的文字反馈：", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    m_activityMgr->addFeedback(activityId, m_userId, rating, comment);
}

void StudentPanel::onFeedbackResult(bool success, const QString& msg) {
    if (success) {
        QMessageBox::information(this, "反馈结果", msg);
        refreshActivityTable();
    }
    else {
        QMessageBox::warning(this, "反馈结果", msg);
    }
}
