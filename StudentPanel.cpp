#include "StudentPanel.h"
#include "ClubManager.h"
#include <QHeaderView>
#include <QFormLayout>
#include <QMessageBox>

StudentPanel::StudentPanel(ActivityManager* activityMgr, int userId, QWidget* parent)
    : QWidget(parent), m_activityMgr(activityMgr), m_userId(userId)
{
    setupUI();
    DatabaseManager::instance().openDatabase();
    connect(&DatabaseManager::instance(), &DatabaseManager::connectionChanged, this, &StudentPanel::onDatabaseStatusChanged);
}

void StudentPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // ��������
    QGroupBox* searchGroup = new QGroupBox("���ѯ", this);
    QFormLayout* formLayout = new QFormLayout(searchGroup);

    searchEdit = new QLineEdit(this);
    formLayout->addRow("�ؼ���:", searchEdit);

    typeCombo = new QComboBox(this);
    typeCombo->addItems({ "ȫ������", "����", "����", "չ��", "�ݳ�" });
    formLayout->addRow("�����:", typeCombo);

    startDateEdit = new QDateEdit(QDate::currentDate(), this);
    endDateEdit = new QDateEdit(QDate::currentDate().addDays(30), this);
    formLayout->addRow("��ʼ����:", startDateEdit);
    formLayout->addRow("��������:", endDateEdit);

    searchBtn = new QPushButton("����", this);
    connect(searchBtn, &QPushButton::clicked, this, &StudentPanel::onSearchClicked);
    formLayout->addRow(searchBtn);

    searchGroup->setLayout(formLayout);
    mainLayout->addWidget(searchGroup);

    // ���
    createActivityTable();
    mainLayout->addWidget(activityTable);

    // ������ť
    feedbackBtn = new QPushButton("�ύ����", this);
    connect(feedbackBtn, &QPushButton::clicked, this, &StudentPanel::onFeedbackClicked);
    mainLayout->addWidget(feedbackBtn);

    // ��Ծ������
    QGroupBox* clubGroup = new QGroupBox("���Ż�Ծ������", this);
    QVBoxLayout* clubLayout = new QVBoxLayout(clubGroup);
    createClubTable();
    clubLayout->addWidget(clubTable);
    clubGroup->setLayout(clubLayout);
    mainLayout->addWidget(clubGroup);

    // ״̬��
    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // ��ʼ������
    refreshActivityTable();
    refreshActiveClubTable();
    onDatabaseStatusChanged(DatabaseManager::instance().isConnected());
}

void StudentPanel::createActivityTable()
{
    activityTable = new QTableWidget(this);
    activityTable->setColumnCount(6);
    activityTable->setHorizontalHeaderLabels(QStringList() << "�ID" << "�����" << "ʱ��" << "�ص�" << "����" << "����");
    activityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    activityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    activityTable->setSelectionMode(QAbstractItemView::SingleSelection);
    activityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void StudentPanel::createClubTable()
{
    clubTable = new QTableWidget(this);
    clubTable->setColumnCount(4);
    clubTable->setHorizontalHeaderLabels(QStringList() << "��������" << "�����" << "������" << "����ʹ��Ч��");
    clubTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    clubTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void StudentPanel::refreshActivityTable()
{
    m_currentActivities = m_activityMgr->queryActivities("", "ȫ������", QDate(), QDate());
    activityTable->setRowCount(0);

    for (int i = 0; i < m_currentActivities.size(); ++i) {
        const ActivityInfo& activity = m_currentActivities[i];

        activityTable->insertRow(i);
        activityTable->setItem(i, 0, new QTableWidgetItem(QString::number(activity.id)));
        activityTable->setItem(i, 1, new QTableWidgetItem(activity.title));
        activityTable->setItem(i, 2, new QTableWidgetItem(activity.startTime.toString("yyyy-MM-dd hh:mm")));
        activityTable->setItem(i, 3, new QTableWidgetItem(activity.location));
        activityTable->setItem(i, 4, new QTableWidgetItem(activity.clubName));
        activityTable->setItem(i, 5, new QTableWidgetItem(QString::number(activity.rating, 'f', 1)));
    }
}

void StudentPanel::refreshActiveClubTable()
{
    QVector<ActivityManager::ClubActiveInfo> clubs = m_activityMgr->calculateClubActiveRanking();
    clubTable->setRowCount(0);

    for (int i = 0; i < clubs.size(); ++i) {
        const ActivityManager::ClubActiveInfo& club = clubs[i];

        clubTable->insertRow(i);
        clubTable->setItem(i, 0, new QTableWidgetItem(club.name));
        clubTable->setItem(i, 1, new QTableWidgetItem(QString::number(club.activityCount)));
        clubTable->setItem(i, 2, new QTableWidgetItem(QString::number(club.participationRate * 100, 'f', 1) + "%"));
        clubTable->setItem(i, 3, new QTableWidgetItem(QString::number(club.budgetEfficiency * 100, 'f', 1) + "%"));
    }
}

void StudentPanel::onSearchClicked()
{
    QString keyword = searchEdit->text().trimmed();
    QString type = typeCombo->currentText();
    QDate start = startDateEdit->date();
    QDate end = endDateEdit->date();

    if (keyword.length() > 0 && ClubManager::toPinyinAbbr(keyword) == keyword) {
        m_currentActivities = m_activityMgr->queryActivitiesByPinyin(keyword, type, start, end);
    }
    else {
        m_currentActivities = m_activityMgr->queryActivities(keyword, type, start, end);
    }

    activityTable->setRowCount(0);

    for (int i = 0; i < m_currentActivities.size(); ++i) {
        const ActivityInfo& activity = m_currentActivities[i];

        activityTable->insertRow(i);
        activityTable->setItem(i, 0, new QTableWidgetItem(QString::number(activity.id)));
        activityTable->setItem(i, 1, new QTableWidgetItem(activity.title));
        activityTable->setItem(i, 2, new QTableWidgetItem(activity.startTime.toString("yyyy-MM-dd hh:mm")));
        activityTable->setItem(i, 3, new QTableWidgetItem(activity.location));
        activityTable->setItem(i, 4, new QTableWidgetItem(activity.clubName));
        activityTable->setItem(i, 5, new QTableWidgetItem(QString::number(activity.rating, 'f', 1)));
    }
}

void StudentPanel::onFeedbackClicked()
{
    int row = activityTable->currentRow();
    if (row < 0 || row >= m_currentActivities.size()) {
        QMessageBox::warning(this, "��ѡ��", "����ѡ��Ҫ�����Ļ");
        return;
    }

    int activityId = m_currentActivities[row].id;

    bool ok;
    int rating = QInputDialog::getInt(this, "�����", "��Ա��λ���� (1-5��):", 3, 1, 5, 1, &ok);
    if (!ok) return;

    QString comment = QInputDialog::getText(this, "�����", "�����������������:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    if (m_activityMgr->addFeedback(activityId, m_userId, rating, comment)) {
        QMessageBox::information(this, "�ύ�ɹ�", "��������ύ");
        refreshActivityTable();
    }
    else {
        QMessageBox::warning(this, "�ύʧ��", "������ύʧ��");
    }
}

void StudentPanel::onDatabaseStatusChanged(bool connected)
{
    statusLabel->setText(connected ? "���ݿ�������" : "���ݿ����ӶϿ�");
}