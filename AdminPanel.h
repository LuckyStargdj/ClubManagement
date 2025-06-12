#ifndef ADMINPANEL_H
#define ADMINPANEL_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include "UserManager.h"
#include "ClubManager.h"

class QTableWidget;
class QPushButton;
class QStatusBar;

class AdminPanel : public QWidget {
    Q_OBJECT
public:
    explicit AdminPanel(UserManager* userMgr, ClubManager* clubMgr, QWidget* parent = nullptr);

    void refreshUserTable();
    void refreshClubTable();

private slots:
    void onResetPwdClicked();
    void onFreezeClicked();
    void onActivateClicked();
    void onApproveClubClicked();
    void onRejectClubClicked();
    void onDatabaseStatusChanged(bool connected);

private:
    void setupUI();
    void createUserTable();
    void createClubTable();
    int getSelectedUserId() const;
    int getSelectedClubId() const;

    QTableWidget* userTable;
    QPushButton* resetPwdBtn;
    QPushButton* freezeBtn;
    QPushButton* activateBtn;
    QTableWidget* clubTable;
    QPushButton* approveBtn;
    QPushButton* rejectBtn;
    QStatusBar* statusBar;

    UserManager* m_userMgr;
    ClubManager* m_clubMgr;
};

#endif // ADMINPANEL_H




