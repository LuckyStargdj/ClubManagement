#include "RegisterDialog.h"
#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QPushButton>

RegisterDialog::RegisterDialog(UserManager* userMgr, QWidget* parent)
    : QDialog(parent), m_userManager(userMgr), m_captchaValue(0) {
    setWindowTitle("社团管理系统 - 注册");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(500, 400);
    setupUI();
    generateCaptcha();
}

void RegisterDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 标题
    QLabel* titleLabel = new QLabel("用户注册", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    QGroupBox* formGroup = new QGroupBox(this);
    QFormLayout* formLayout = new QFormLayout(formGroup);

    // 学号/工号
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("请输入学号/工号（10位数字）");
    formLayout->addRow("学号/工号:", m_usernameEdit);

    // 姓名
    m_nameEdit = new QLineEdit(this);
    formLayout->addRow("姓名:", m_nameEdit);

    // 角色
    m_roleCombo = new QComboBox(this);
    m_roleCombo->addItem("普通学生", UserManager::Student);
    m_roleCombo->addItem("社团负责人", UserManager::President);
    m_roleCombo->addItem("管理员", UserManager::Admin);
    formLayout->addRow("角色:", m_roleCombo);

    // 密码
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("密码:", m_passwordEdit);

    // 确认密码
    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("确认密码:", m_confirmPasswordEdit);

    // 验证码
    QHBoxLayout* captchaLayout = new QHBoxLayout();
    m_captchaLabel = new QLabel(this);
    m_captchaLabel->setFixedSize(100, 40);
    m_captchaLabel->setStyleSheet("background-color: white; border: 1px solid gray;");
    m_captchaLabel->setAlignment(Qt::AlignCenter);

    m_captchaEdit = new QLineEdit(this);
    m_captchaEdit->setFixedWidth(100);

    QPushButton* refreshBtn = new QPushButton("刷新", this);
    connect(refreshBtn, &QPushButton::clicked, this, &RegisterDialog::generateCaptcha);

    captchaLayout->addWidget(m_captchaLabel);
    captchaLayout->addWidget(m_captchaEdit);
    captchaLayout->addWidget(refreshBtn);
    formLayout->addRow("验证码:", captchaLayout);

    formGroup->setLayout(formLayout);
    mainLayout->addWidget(formGroup);

    // 注册按钮
    m_registerButton = new QPushButton("注册", this);
    m_registerButton->setFixedHeight(40);
    connect(m_registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    mainLayout->addWidget(m_registerButton);
}

void RegisterDialog::generateCaptcha() {
    m_captchaValue = QRandomGenerator::global()->bounded(1000, 10000);
    m_captchaLabel->setText(QString::number(m_captchaValue));
    m_captchaLabel->setFont(QFont("Arial", 20, QFont::Bold));
}

void RegisterDialog::onRegisterClicked() {
    QString username = m_usernameEdit->text().trimmed();
    QString name = m_nameEdit->text().trimmed();
    int roleIndex = m_roleCombo->currentIndex();
    UserManager::Role role = static_cast<UserManager::Role>(m_roleCombo->itemData(roleIndex).toInt());
    QString password = m_passwordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();
    QString captcha = m_captchaEdit->text();

    // 输入验证
    if (username.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "学号/工号和姓名不能为空");
        return;
    }

    if (password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "密码和确认密码不能为空");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "输入错误", "两次输入的密码不一致");
        return;
    }

    if (captcha.isEmpty() || captcha != QString::number(m_captchaValue)) {
        QMessageBox::warning(this, "验证码错误", "验证码输入错误，请重试");
        generateCaptcha();
        m_captchaEdit->clear();
        return;
    }

    // 注册用户
    if (m_userManager->registerUser(name, role, username, password)) {
        m_username = username;
        accept();
        QMessageBox::information(this, "注册成功", "用户注册成功，请登录");
    }
    else {
        QMessageBox::warning(this, "注册失败", "用户注册失败，请重试");
        generateCaptcha();
    }
}
