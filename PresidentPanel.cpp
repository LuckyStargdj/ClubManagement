#include "PresidentPanel.h"
#include <QDateTime>
#include <QHeaderView>
#include <QSqlQuery>

PresidentPanel::PresidentPanel(ClubManager* clubMgr, ActivityManager* actMgr, int userId, QWidget* parent)
    : QWidget(parent), m_clubMgr(clubMgr), m_actMgr(actMgr), m_userId(userId)
{
    setupUI();
    DatabaseManager::instance().openDatabase();
    connect(&DatabaseManager::instance(), &DatabaseManager::connectionChanged, this, &PresidentPanel::onDatabaseStatusChanged);
}

void PresidentPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // ������Ϣ����
    QGroupBox* clubGroup = new QGroupBox("������Ϣ����", this);
    createClubForm();

    QVBoxLayout* clubLayout = new QVBoxLayout(clubGroup);
    clubLayout->addWidget(saveClubBtn);
    clubGroup->setLayout(clubLayout);

    mainLayout->addWidget(clubGroup);

    // �������
    QGroupBox* activityGroup = new QGroupBox("�����", this);
    QVBoxLayout* activityLayout = new QVBoxLayout(activityGroup);

    createActivityTable();
    activityLayout->addWidget(activityTable);

    applyActivityBtn = new QPushButton("�����»", this);
    connect(applyActivityBtn, &QPushButton::clicked, this, &PresidentPanel::onApplyActivityClicked);
    activityLayout->addWidget(applyActivityBtn);

    mainLayout->addWidget(activityGroup);

    // �ļ��ϴ�����
    QGroupBox* fileGroup = new QGroupBox("�ļ��ϴ�", this);
    QHBoxLayout* fileLayout = new QHBoxLayout(fileGroup);

    uploadDocBtn = new QPushButton("�ϴ������³�", this);
    connect(uploadDocBtn, &QPushButton::clicked, this, &PresidentPanel::onUploadDocumentClicked);

    uploadLogoBtn = new QPushButton("�ϴ�����LOGO", this);
    connect(uploadLogoBtn, &QPushButton::clicked, this, &PresidentPanel::onUploadLogoClicked);

    fileLayout->addWidget(uploadDocBtn);
    fileLayout->addWidget(uploadLogoBtn);
    fileGroup->setLayout(fileLayout);

    mainLayout->addWidget(fileGroup);

    // ״̬��
    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // ��ʼ������
    refreshClubInfo();
    refreshActivityTable();
    onDatabaseStatusChanged(DatabaseManager::instance().isConnected());
}

void PresidentPanel::createClubForm()
{
    // ����������Ϣ��
    QWidget* formWidget = new QWidget(this);
    QFormLayout* formLayout = new QFormLayout(formWidget);

    clubNameEdit = new QLineEdit(this);
    formLayout->addRow("��������:", clubNameEdit);

    clubTypeCombo = new QComboBox(this);
    clubTypeCombo->addItems({ "ѧ����", "������", "������", "������" });
    formLayout->addRow("��������:", clubTypeCombo);

    establishDateEdit = new QDateEdit(QDate::currentDate(), this);
    establishDateEdit->setCalendarPopup(true);
    formLayout->addRow("��������:", establishDateEdit);

    advisorEdit = new QLineEdit(this);
    formLayout->addRow("ָ����ʦ:", advisorEdit);

    saveClubBtn = new QPushButton("����������Ϣ", this);
    connect(saveClubBtn, &QPushButton::clicked, this, &PresidentPanel::onSaveClubClicked);

    // ������ӵ�������
    static_cast<QVBoxLayout*>(layout())->insertWidget(0, formWidget);
}

void PresidentPanel::createActivityTable()
{
    activityTable = new QTableWidget(this);
    activityTable->setColumnCount(5);
    activityTable->setHorizontalHeaderLabels(QStringList() << "�ID" << "����" << "ʱ��" << "�ص�" << "״̬");
    activityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    activityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    activityTable->setSelectionMode(QAbstractItemView::SingleSelection);
    activityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void PresidentPanel::refreshClubInfo()
{
    m_myClub = m_clubMgr->queryClubByPresident(m_userId);
    if (m_myClub.id == -1) {
        QMessageBox::warning(this, "����", "�޷���ȡ������Ϣ����ȷ���������Ÿ�����");
        return;
    }

    clubNameEdit->setText(m_myClub.name);
    clubTypeCombo->setCurrentText(m_myClub.type);
    establishDateEdit->setDate(m_myClub.establishDate);
    advisorEdit->setText(m_myClub.advisor);

    // ��������״̬����/���ÿؼ�
    bool isApproved = (m_myClub.status == 1);
    clubNameEdit->setEnabled(isApproved);
    clubTypeCombo->setEnabled(isApproved);
    establishDateEdit->setEnabled(isApproved);
    advisorEdit->setEnabled(isApproved);
    saveClubBtn->setEnabled(isApproved);
    uploadDocBtn->setEnabled(isApproved);
    uploadLogoBtn->setEnabled(isApproved);
    applyActivityBtn->setEnabled(isApproved);
}

void PresidentPanel::refreshActivityTable()
{
    activityTable->setRowCount(0);

    QMap<QString, QVariant> params;
    params[":club_id"] = m_myClub.id;

    QString sql = "SELECT id, title, start_time, location, status "
        "FROM activities "
        "WHERE club_id = :club_id";

    QSqlQuery query = DatabaseManager::instance().execQuery(sql, params);

    int row = 0;
    while (query.next()) {
        activityTable->insertRow(row);
        activityTable->setItem(row, 0, new QTableWidgetItem(query.value("id").toString()));
        activityTable->setItem(row, 1, new QTableWidgetItem(query.value("title").toString()));
        activityTable->setItem(row, 2, new QTableWidgetItem(query.value("start_time").toDateTime().toString("yyyy-MM-dd hh:mm")));
        activityTable->setItem(row, 3, new QTableWidgetItem(query.value("location").toString()));

        int status = query.value("status").toInt();
        QString statusStr;
        if (status == 0) statusStr = "������";
        else if (status == 1) statusStr = "����׼";
        else if (status == 2) statusStr = "�Ѿܾ�";
        activityTable->setItem(row, 4, new QTableWidgetItem(statusStr));

        row++;
    }
}

void PresidentPanel::onDatabaseStatusChanged(bool connected)
{
    statusLabel->setText(connected ? "���ݿ�������" : "���ݿ����ӶϿ�");
}

void PresidentPanel::onSaveClubClicked()
{
    ClubInfo info = m_myClub;
    info.name = clubNameEdit->text().trimmed();
    info.type = clubTypeCombo->currentText();
    info.establishDate = establishDateEdit->date();
    info.advisor = advisorEdit->text().trimmed();

    if (info.name.isEmpty() || info.advisor.isEmpty()) {
        QMessageBox::warning(this, "�������", "�������ƺ�ָ����ʦ����Ϊ��");
        return;
    }

    if (m_clubMgr->updateClubInfo(info)) {
        QMessageBox::information(this, "����ɹ�", "������Ϣ�Ѹ���");
        refreshClubInfo();
    }
    else {
        QMessageBox::warning(this, "����ʧ��", "������Ϣ����ʧ��");
    }
}

void PresidentPanel::onApplyActivityClicked()
{
    bool ok;
    QString title = QInputDialog::getText(this, "�»����", "����������:", QLineEdit::Normal, "", &ok);
    if (!ok || title.isEmpty()) return;

    QDateTime start = QDateTime::currentDateTime().addDays(7);
    QDateTime end = start.addSecs(3600 * 2); // 2Сʱ = 2 * 3600��

    QString location = QInputDialog::getText(this, "��ص�", "�������ص�:", QLineEdit::Normal, "", &ok);
    if (!ok || location.isEmpty()) return;

    int maxParticipants = QInputDialog::getInt(this, "��������", "����������������:", 100, 1, 1000, 1, &ok);
    if (!ok) return;

    double budget = QInputDialog::getDouble(this, "�Ԥ��", "������Ԥ��:", 500.0, 0, 10000, 2, &ok);
    if (!ok) return;

    if (m_actMgr->addActivity(m_myClub.id, title, start, end, location, maxParticipants, budget)) {
        QMessageBox::information(this, "����ɹ�", "��������ύ���ȴ�����");
        refreshActivityTable();
    }
    else {
        QMessageBox::warning(this, "����ʧ��", "������ύʧ��");
    }
}

void PresidentPanel::onUploadDocumentClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "ѡ�������³�", "", "PDF�ļ� (*.pdf)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "���ļ�ʧ��", "�޷���ѡ�����ļ�");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    if (FileManager::instance().saveClubFile(m_myClub.id, 0, QFileInfo(filePath).fileName(), data) > 0) {
        QMessageBox::information(this, "�ϴ��ɹ�", "�����³����ϴ�");
    }
    else {
        QMessageBox::warning(this, "�ϴ�ʧ��", "�����³��ϴ�ʧ��");
    }
}

void PresidentPanel::onUploadLogoClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "ѡ������LOGO", "", "ͼƬ�ļ� (*.png *.jpg *.bmp)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "���ļ�ʧ��", "�޷���ѡ�����ļ�");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    if (FileManager::instance().saveClubFile(m_myClub.id, 1, QFileInfo(filePath).fileName(), data) > 0) {
        QMessageBox::information(this, "�ϴ��ɹ�", "����LOGO���ϴ�");
    }
    else {
        QMessageBox::warning(this, "�ϴ�ʧ��", "����LOGO�ϴ�ʧ��");
    }
}