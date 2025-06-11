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

    // �û�������
    userTable = new QTableWidget(0, 4, this);
    userTable->setHorizontalHeaderLabels({ "ID", "����", "��ɫ", "״̬" });
    userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(userTable);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    resetPwdBtn = new QPushButton("��������", this);
    freezeBtn = new QPushButton("�����˻�", this);
    activateBtn = new QPushButton("�����˻�", this);
    buttonLayout->addWidget(resetPwdBtn);
    buttonLayout->addWidget(freezeBtn);
    buttonLayout->addWidget(activateBtn);
    layout->addLayout(buttonLayout);

    // �����������
    clubTable = new QTableWidget(0, 3, this);
    clubTable->setHorizontalHeaderLabels({ "����ID", "����", "��������" });
    clubTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(clubTable);

    QHBoxLayout* clubBtnLayout = new QHBoxLayout();
    approveBtn = new QPushButton("��׼", this);
    rejectBtn = new QPushButton("�ܾ�", this);
    clubBtnLayout->addWidget(approveBtn);
    clubBtnLayout->addWidget(rejectBtn);
    layout->addLayout(clubBtnLayout);

    // �źŲ�����
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
        QString roleStr = "��ͨѧ��";
        if (roleVal == 0) roleStr = "����Ա";
        else if (roleVal == 1) roleStr = "���Ÿ�����";
        userTable->setItem(row, 2, new QTableWidgetItem(roleStr));
        QString statusStr = (query.value(3).toInt() == 1 ? "����" : "����");
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

// �û�����
void AdminPanel::onResetPwdClicked() {
    int row = userTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "��ʾ", "��ѡ��Ҫ����������û�");
        return;
    }
    int userId = userTable->item(row, 0)->text().toInt();
    if (m_userMgr->resetPassword(userId, "123456"))
        QMessageBox::information(this, "��ʾ", "����������Ϊ123456");
    else
        QMessageBox::warning(this, "ʧ��", "����ʧ��");
}

void AdminPanel::onFreezeClicked() {
    int row = userTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "��ʾ", "��ѡ��Ҫ������˻�");
        return;
    }
    int userId = userTable->item(row, 0)->text().toInt();
    if (m_userMgr->freezeAccount(userId)) {
        QMessageBox::information(this, "��ʾ", "�Ѷ����˻�");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "ʧ��", "����ʧ��");
    }
}

void AdminPanel::onActivateClicked() {
    int row = userTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "��ʾ", "��ѡ��Ҫ������˻�");
        return;
    }
    int userId = userTable->item(row, 0)->text().toInt();
    QSqlQuery q;
    q.prepare("UPDATE users SET status=1 WHERE id=:id");
    q.bindValue(":id", userId);
    if (q.exec()) {
        QMessageBox::information(this, "��ʾ", "�˻��Ѽ���");
        refreshUserTable();
    }
    else {
        QMessageBox::warning(this, "ʧ��", "����ʧ��");
    }
}

// ��������
void AdminPanel::onApproveClubClicked() {
    int row = clubTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "��ʾ", "��ѡ��Ҫ����������");
        return;
    }
    int clubId = clubTable->item(row, 0)->text().toInt();
    if (m_clubMgr->approveClub(clubId)) {
        QMessageBox::information(this, "�����ɹ�", "��������׼��");
        refreshClubTable();
    }
    else {
        QMessageBox::warning(this, "ʧ��", "����ʧ��");
    }
}

void AdminPanel::onRejectClubClicked() {
    int row = clubTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "��ʾ", "��ѡ��Ҫ�ܾ�������");
        return;
    }
    int clubId = clubTable->item(row, 0)->text().toInt();
    if (m_clubMgr->rejectClub(clubId)) {
        QMessageBox::information(this, "�����ɹ�", "�����Ѿܾ���ɾ����");
        refreshClubTable();
    }
    else {
        QMessageBox::warning(this, "ʧ��", "ɾ��ʧ��");
    }
}

// ���ݱ���ź�ˢ��
void AdminPanel::onDataChanged() {
    refreshUserTable();
    refreshClubTable();
}
