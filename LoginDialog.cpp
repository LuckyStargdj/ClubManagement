#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRandomGenerator>

LoginDialog::LoginDialog(UserManager* user, QWidget* parent)
    : QDialog(parent), m_userManager(user) {
    setWindowTitle("社团管理系统 - 登录");
    setFixedSize(400, 300);
    setupUI();
    connect(m_userManager, &UserManager::loginStatusChanged,
        this, &LoginDialog::handleLoginResult);
    generateCaptcha();
}

LoginDialog::LoginDialog(UserManager* user, const QString& username, QWidget* parent) 
    : QDialog(parent), m_userManager(user), m_username(username) {
    setWindowTitle("社团管理系统 - 登录");
    setFixedSize(400, 300);
    setupUI();
    m_usernameEdit->setText(m_username);
    connect(m_userManager, &UserManager::loginStatusChanged,
        this, &LoginDialog::handleLoginResult);
    generateCaptcha();
}

void LoginDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("社团管理系统登录", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(20);

    QHBoxLayout* usernameLayout = new QHBoxLayout();
    QLabel* usernameLabel = new QLabel("学号/工号:", this);
    m_usernameEdit = new QLineEdit(this);
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addWidget(m_usernameEdit);
    mainLayout->addLayout(usernameLayout);

    QHBoxLayout* passwordLayout = new QHBoxLayout();
    QLabel* passwordLabel = new QLabel("密码:", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(m_passwordEdit);
    mainLayout->addLayout(passwordLayout);

    QHBoxLayout* captchaLayout = new QHBoxLayout();
    QLabel* captchaLabel = new QLabel("验证码:", this);
    m_captchaLabel = new QLabel(this);
    m_captchaLabel->setStyleSheet("border: 1px solid gray; background: white;");
    m_captchaLabel->setFixedSize(60, 30);
    m_captchaEdit = new QLineEdit(this);
    m_captchaEdit->setFixedWidth(100);
    captchaLayout->addWidget(captchaLabel);
    captchaLayout->addWidget(m_captchaLabel);
    captchaLayout->addWidget(m_captchaEdit);
    mainLayout->addLayout(captchaLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_loginButton = new QPushButton("登录", this);
    m_registerButton = new QPushButton("注册", this);
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_registerButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(m_captchaLabel, &QLabel::linkActivated, this, &LoginDialog::generateCaptcha);
}

void LoginDialog::generateCaptcha() {
    m_captchaValue = QRandomGenerator::global()->bounded(1000, 10000);
    m_captchaLabel->setText(QString::number(m_captchaValue));
}

void LoginDialog::onLoginClicked() {
    m_username = m_usernameEdit->text().trimmed();
    m_password = m_passwordEdit->text();
    QString captcha = m_captchaEdit->text();

    if (m_username.isEmpty() || m_password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名和密码不能为空");
        return;
    }
    if (captcha.isEmpty() || captcha != QString::number(m_captchaValue)) {
        QMessageBox::warning(this, "验证码错误", "验证码输入错误，请重试");
        generateCaptcha();
        m_captchaEdit->clear();
        return;
    }
    QRegularExpression regex("^\\d{10}$");
    if (!regex.match(m_username).hasMatch()) {
        QMessageBox::warning(this, "格式错误", "学号格式错误（需10位数字）");
        return;
    }
    m_userManager->login(m_username, m_password);
}

void LoginDialog::onRegisterClicked() {
    RegisterDialog dlg(m_userManager);
    if (dlg.exec() == QDialog::Accepted) {
        m_usernameEdit->setText(dlg.username());
        m_passwordEdit->setFocus();
    }
}

void LoginDialog::handleLoginResult(bool success, const QString& message) {
    if (success) {
        m_failCount = 0;
        accept();
    }
    else {
        m_failCount++;
        if (m_failCount >= 5) {
            m_userManager->freezeAccount(m_userManager->currentUserId());
            QMessageBox::critical(this, "账户已锁定", "多次登录失败，账户已锁定！");
        }
        generateCaptcha();
        m_captchaEdit->clear();
        QMessageBox::warning(this, "登录失败", message);
    }
}

