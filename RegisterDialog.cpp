#include "RegisterDialog.h"
#include "LoginDialog.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRandomGenerator>

RegisterDialog::RegisterDialog(UserManager* user, QWidget* parent)
    : m_userManager(user), QDialog(parent) {
    setWindowTitle("社团管理系统 - 注册");
    setFixedSize(500, 400);
    setupUI();
    generateCaptcha();
}

void RegisterDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("用户注册", this);
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

    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("姓名:", this);
    m_nameEdit = new QLineEdit(this);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(nameLayout);

    QHBoxLayout* roleLayout = new QHBoxLayout();
    QLabel* roleLabel = new QLabel("角色:", this);
    m_roleCombo = new QComboBox(this);
    m_roleCombo->addItem("普通学生", UserManager::Student);
    m_roleCombo->addItem("社团负责人", UserManager::President);
    m_roleCombo->addItem("管理员", UserManager::Admin);
    roleLayout->addWidget(roleLabel);
    roleLayout->addWidget(m_roleCombo);
    mainLayout->addLayout(roleLayout);

    QHBoxLayout* passwordLayout = new QHBoxLayout();
    QLabel* passwordLabel = new QLabel("密码:", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(m_passwordEdit);
    mainLayout->addLayout(passwordLayout);

    QHBoxLayout* confirmLayout = new QHBoxLayout();
    QLabel* confirmLabel = new QLabel("确认密码:", this);
    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmLayout->addWidget(confirmLabel);
    confirmLayout->addWidget(m_confirmPasswordEdit);
    mainLayout->addLayout(confirmLayout);

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

    m_registerButton = new QPushButton("注册", this);
    mainLayout->addWidget(m_registerButton);

    connect(m_registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
}

void RegisterDialog::generateCaptcha() {
    m_captchaValue = QRandomGenerator::global()->bounded(1000, 10000);
    m_captchaLabel->setText(QString::number(m_captchaValue));
}

void RegisterDialog::onRegisterClicked() {
    m_username = m_usernameEdit->text().trimmed();
    QString name = m_nameEdit->text().trimmed();
    int roleIndex = m_roleCombo->currentIndex();
    UserManager::Role role = static_cast<UserManager::Role>(m_roleCombo->itemData(roleIndex).toInt());
    QString password = m_passwordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();
    QString captcha = m_captchaEdit->text();

    if (m_username.isEmpty() || name.isEmpty()) {
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
        QMessageBox::warning(this, "验证码错误", "验证码输入错误");
        generateCaptcha();
        m_captchaEdit->clear();
        return;
    }
    QRegularExpression regex("^\\d{10}$");
    if (!regex.match(m_username).hasMatch()) {
        QMessageBox::warning(this, "格式错误", "学号格式错误（需10位数字）");
        return;
    }
    if (password.length() < 6) {
        QMessageBox::warning(this, "密码强度不足", "密码长度至少为6位");
        return;
    }

    if (m_userManager->registerUser(name, role, m_username, password)) {
        QMessageBox::information(this, "注册成功", "用户注册成功，请登录");
        accept();
        LoginDialog dlg(m_userManager, m_userManager->currentUserName());
        dlg.exec();
    }
}

