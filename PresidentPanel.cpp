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

    QGroupBox* clubInfoGroup = new QGroupBox("������Ϣ����", this);
    QFormLayout* formLayout = new QFormLayout(clubInfoGroup);
    clubNameEdit = new QLineEdit(clubInfoGroup);
    clubTypeCombo = new QComboBox(clubInfoGroup);
    clubTypeCombo->addItems({ "ѧ����", "������", "������", "������" });
    establishDateEdit = new QDateEdit(QDate::currentDate(), clubInfoGroup);
    advisorEdit = new QLineEdit(clubInfoGroup);
    saveClubBtn = new QPushButton("������Ϣ", clubInfoGroup);
    formLayout->addRow("��������:", clubNameEdit);
    formLayout->addRow("��������:", clubTypeCombo);
    formLayout->addRow("��������:", establishDateEdit);
    formLayout->addRow("ָ����ʦ:", advisorEdit);
    formLayout->addRow(saveClubBtn);
    layout->addWidget(clubInfoGroup);

    QGroupBox* activityGroup = new QGroupBox("�����", this);
    QVBoxLayout* activityLayout = new QVBoxLayout(activityGroup);
    activityTable = new QTableWidget(0, 4, activityGroup);
    activityTable->setHorizontalHeaderLabels({ "�ID", "����", "ʱ��", "״̬" });
    activityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    applyActivityBtn = new QPushButton("�����»", activityGroup);
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
    // �� club_id ��ѯ�
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
        QMessageBox::warning(this, "�������", "���ƺ�ָ����ʦ����Ϊ��");
        return;
    }
    if (m_clubMgr->updateClubInfo(info))
        QMessageBox::information(this, "�ɹ�", "�ѱ���������Ϣ��");
    else
        QMessageBox::warning(this, "ʧ��", "���ݿ����");
}

void PresidentPanel::onApplyActivityClicked() {
    // ʵ�ʿ������鵯����ȡ���Ϣ
    int clubId = m_myClub.id;
    QString title = QInputDialog::getText(this, "�»", "����⣺");
    if (title.isEmpty()) return;
    QDateTime start = QDateTime::currentDateTime();
    QDateTime end = QDateTime::currentDateTime().addSecs(3600);
    QString location = "���Ż��";
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
        QMessageBox::information(this, "����ɹ�", "������룬�ȴ�����");
        m_actMgr->activityDataChanged(); // ֪ͨˢ��
    }
    else {
        QMessageBox::warning(this, "ʧ��", "����ʧ��: " + query.lastError().text());
    }
}

void PresidentPanel::onDataChanged() {
    refreshClubInfo();
    refreshActivityTable();
}
