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
    setWindowTitle("���Ź���ϵͳ - ע��");
    setFixedSize(500, 400);
    setupUI();
    generateCaptcha();
}

void RegisterDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("�û�ע��", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(20);

    QHBoxLayout* usernameLayout = new QHBoxLayout();
    QLabel* usernameLabel = new QLabel("ѧ��/����:", this);
    m_usernameEdit = new QLineEdit(this);
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addWidget(m_usernameEdit);
    mainLayout->addLayout(usernameLayout);

    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("����:", this);
    m_nameEdit = new QLineEdit(this);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(nameLayout);

    QHBoxLayout* roleLayout = new QHBoxLayout();
    QLabel* roleLabel = new QLabel("��ɫ:", this);
    m_roleCombo = new QComboBox(this);
    m_roleCombo->addItem("��ͨѧ��", UserManager::Student);
    m_roleCombo->addItem("���Ÿ�����", UserManager::President);
    m_roleCombo->addItem("����Ա", UserManager::Admin);
    roleLayout->addWidget(roleLabel);
    roleLayout->addWidget(m_roleCombo);
    mainLayout->addLayout(roleLayout);

    QHBoxLayout* passwordLayout = new QHBoxLayout();
    QLabel* passwordLabel = new QLabel("����:", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(m_passwordEdit);
    mainLayout->addLayout(passwordLayout);

    QHBoxLayout* confirmLayout = new QHBoxLayout();
    QLabel* confirmLabel = new QLabel("ȷ������:", this);
    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmLayout->addWidget(confirmLabel);
    confirmLayout->addWidget(m_confirmPasswordEdit);
    mainLayout->addLayout(confirmLayout);

    QHBoxLayout* captchaLayout = new QHBoxLayout();
    QLabel* captchaLabel = new QLabel("��֤��:", this);
    m_captchaLabel = new QLabel(this);
    m_captchaLabel->setStyleSheet("border: 1px solid gray; background: white;");
    m_captchaLabel->setFixedSize(60, 30);
    m_captchaEdit = new QLineEdit(this);
    m_captchaEdit->setFixedWidth(100);
    captchaLayout->addWidget(captchaLabel);
    captchaLayout->addWidget(m_captchaLabel);
    captchaLayout->addWidget(m_captchaEdit);
    mainLayout->addLayout(captchaLayout);

    m_registerButton = new QPushButton("ע��", this);
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
        QMessageBox::warning(this, "�������", "ѧ��/���ź���������Ϊ��");
        return;
    }
    if (password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "�������", "�����ȷ�����벻��Ϊ��");
        return;
    }
    if (password != confirmPassword) {
        QMessageBox::warning(this, "�������", "������������벻һ��");
        return;
    }
    if (captcha.isEmpty() || captcha != QString::number(m_captchaValue)) {
        QMessageBox::warning(this, "��֤�����", "��֤���������");
        generateCaptcha();
        m_captchaEdit->clear();
        return;
    }
    QRegularExpression regex("^\\d{10}$");
    if (!regex.match(m_username).hasMatch()) {
        QMessageBox::warning(this, "��ʽ����", "ѧ�Ÿ�ʽ������10λ���֣�");
        return;
    }
    if (password.length() < 6) {
        QMessageBox::warning(this, "����ǿ�Ȳ���", "���볤������Ϊ6λ");
        return;
    }

    if (m_userManager->registerUser(name, role, m_username, password)) {
        QMessageBox::information(this, "ע��ɹ�", "�û�ע��ɹ������¼");
        accept();
        LoginDialog dlg(m_userManager, m_userManager->currentUserName());
        dlg.exec();
    }
}

