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

    // 社团信息部分
    QGroupBox* clubGroup = new QGroupBox("社团信息管理", this);
    createClubForm();

    QVBoxLayout* clubLayout = new QVBoxLayout(clubGroup);
    clubLayout->addWidget(saveClubBtn);
    clubGroup->setLayout(clubLayout);

    mainLayout->addWidget(clubGroup);

    // 活动管理部分
    QGroupBox* activityGroup = new QGroupBox("活动管理", this);
    QVBoxLayout* activityLayout = new QVBoxLayout(activityGroup);

    createActivityTable();
    activityLayout->addWidget(activityTable);

    applyActivityBtn = new QPushButton("申请新活动", this);
    connect(applyActivityBtn, &QPushButton::clicked, this, &PresidentPanel::onApplyActivityClicked);
    activityLayout->addWidget(applyActivityBtn);

    mainLayout->addWidget(activityGroup);

    // 文件上传部分
    QGroupBox* fileGroup = new QGroupBox("文件上传", this);
    QHBoxLayout* fileLayout = new QHBoxLayout(fileGroup);

    uploadDocBtn = new QPushButton("上传社团章程", this);
    connect(uploadDocBtn, &QPushButton::clicked, this, &PresidentPanel::onUploadDocumentClicked);

    uploadLogoBtn = new QPushButton("上传社团LOGO", this);
    connect(uploadLogoBtn, &QPushButton::clicked, this, &PresidentPanel::onUploadLogoClicked);

    fileLayout->addWidget(uploadDocBtn);
    fileLayout->addWidget(uploadLogoBtn);
    fileGroup->setLayout(fileLayout);

    mainLayout->addWidget(fileGroup);

    // 状态栏
    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // 初始化数据
    refreshClubInfo();
    refreshActivityTable();
    onDatabaseStatusChanged(DatabaseManager::instance().isConnected());
}

void PresidentPanel::createClubForm()
{
    // 创建社团信息表单
    QWidget* formWidget = new QWidget(this);
    QFormLayout* formLayout = new QFormLayout(formWidget);

    clubNameEdit = new QLineEdit(this);
    formLayout->addRow("社团名称:", clubNameEdit);

    clubTypeCombo = new QComboBox(this);
    clubTypeCombo->addItems({ "学术类", "文艺类", "体育类", "公益类" });
    formLayout->addRow("社团类型:", clubTypeCombo);

    establishDateEdit = new QDateEdit(QDate::currentDate(), this);
    establishDateEdit->setCalendarPopup(true);
    formLayout->addRow("成立日期:", establishDateEdit);

    advisorEdit = new QLineEdit(this);
    formLayout->addRow("指导老师:", advisorEdit);

    saveClubBtn = new QPushButton("保存社团信息", this);
    connect(saveClubBtn, &QPushButton::clicked, this, &PresidentPanel::onSaveClubClicked);

    // 将表单添加到主布局
    static_cast<QVBoxLayout*>(layout())->insertWidget(0, formWidget);
}

void PresidentPanel::createActivityTable()
{
    activityTable = new QTableWidget(this);
    activityTable->setColumnCount(5);
    activityTable->setHorizontalHeaderLabels(QStringList() << "活动ID" << "名称" << "时间" << "地点" << "状态");
    activityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    activityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    activityTable->setSelectionMode(QAbstractItemView::SingleSelection);
    activityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void PresidentPanel::refreshClubInfo()
{
    m_myClub = m_clubMgr->queryClubByPresident(m_userId);
    if (m_myClub.id == -1) {
        QMessageBox::warning(this, "错误", "无法获取社团信息，请确保您是社团负责人");
        return;
    }

    clubNameEdit->setText(m_myClub.name);
    clubTypeCombo->setCurrentText(m_myClub.type);
    establishDateEdit->setDate(m_myClub.establishDate);
    advisorEdit->setText(m_myClub.advisor);

    // 根据社团状态启用/禁用控件
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
        if (status == 0) statusStr = "待审批";
        else if (status == 1) statusStr = "已批准";
        else if (status == 2) statusStr = "已拒绝";
        activityTable->setItem(row, 4, new QTableWidgetItem(statusStr));

        row++;
    }
}

void PresidentPanel::onDatabaseStatusChanged(bool connected)
{
    statusLabel->setText(connected ? "数据库已连接" : "数据库连接断开");
}

void PresidentPanel::onSaveClubClicked()
{
    ClubInfo info = m_myClub;
    info.name = clubNameEdit->text().trimmed();
    info.type = clubTypeCombo->currentText();
    info.establishDate = establishDateEdit->date();
    info.advisor = advisorEdit->text().trimmed();

    if (info.name.isEmpty() || info.advisor.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "社团名称和指导老师不能为空");
        return;
    }

    if (m_clubMgr->updateClubInfo(info)) {
        QMessageBox::information(this, "保存成功", "社团信息已更新");
        refreshClubInfo();
    }
    else {
        QMessageBox::warning(this, "保存失败", "社团信息更新失败");
    }
}

void PresidentPanel::onApplyActivityClicked()
{
    bool ok;
    QString title = QInputDialog::getText(this, "新活动申请", "请输入活动名称:", QLineEdit::Normal, "", &ok);
    if (!ok || title.isEmpty()) return;

    QDateTime start = QDateTime::currentDateTime().addDays(7);
    QDateTime end = start.addSecs(3600 * 2); // 2小时 = 2 * 3600秒

    QString location = QInputDialog::getText(this, "活动地点", "请输入活动地点:", QLineEdit::Normal, "", &ok);
    if (!ok || location.isEmpty()) return;

    int maxParticipants = QInputDialog::getInt(this, "参与人数", "请输入最大参与人数:", 100, 1, 1000, 1, &ok);
    if (!ok) return;

    double budget = QInputDialog::getDouble(this, "活动预算", "请输入活动预算:", 500.0, 0, 10000, 2, &ok);
    if (!ok) return;

    if (m_actMgr->addActivity(m_myClub.id, title, start, end, location, maxParticipants, budget)) {
        QMessageBox::information(this, "申请成功", "活动申请已提交，等待审批");
        refreshActivityTable();
    }
    else {
        QMessageBox::warning(this, "申请失败", "活动申请提交失败");
    }
}

void PresidentPanel::onUploadDocumentClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择社团章程", "", "PDF文件 (*.pdf)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "打开文件失败", "无法打开选定的文件");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    if (FileManager::instance().saveClubFile(m_myClub.id, 0, QFileInfo(filePath).fileName(), data) > 0) {
        QMessageBox::information(this, "上传成功", "社团章程已上传");
    }
    else {
        QMessageBox::warning(this, "上传失败", "社团章程上传失败");
    }
}

void PresidentPanel::onUploadLogoClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择社团LOGO", "", "图片文件 (*.png *.jpg *.bmp)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "打开文件失败", "无法打开选定的文件");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    if (FileManager::instance().saveClubFile(m_myClub.id, 1, QFileInfo(filePath).fileName(), data) > 0) {
        QMessageBox::information(this, "上传成功", "社团LOGO已上传");
    }
    else {
        QMessageBox::warning(this, "上传失败", "社团LOGO上传失败");
    }
}