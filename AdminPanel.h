#ifndef ADMINPANEL_H
#define ADMINPANEL_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include "UserManager.h"
#include "ClubManager.h"

class QTableWidget;
class QPushButton;

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
    void onDataChanged();

private:
    QTableWidget* userTable;
    QPushButton* resetPwdBtn;
    QPushButton* freezeBtn;
    QPushButton* activateBtn;
    QTableWidget* clubTable;
    QPushButton* approveBtn;
    QPushButton* rejectBtn;

    UserManager* m_userMgr;
    ClubManager* m_clubMgr;
};

#endif // ADMINPANEL_H




