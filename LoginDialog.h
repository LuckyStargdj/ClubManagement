#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#pragma execution_character_set("utf-8")

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class UserManager;
class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(UserManager* user, QWidget* parent = nullptr);
    explicit LoginDialog(UserManager* user, const QString& username, QWidget* parent = nullptr);
    QString username() const { return m_username; }
    QString password() const { return m_password; }

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void handleLoginResult(bool success, const QString& message);

private:
    void setupUI();
    void generateCaptcha();

    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLabel* m_captchaLabel;
    QLineEdit* m_captchaEdit;
    QPushButton* m_loginButton;
    QPushButton* m_registerButton;

    QString m_username;
    QString m_password;
    int m_captchaValue = 0;
    int m_failCount = 0;
    UserManager* m_userManager;
};

#endif // LOGINDIALOG_H

