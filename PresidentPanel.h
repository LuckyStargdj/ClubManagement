#ifndef PRESIDENTPANEL_H
#define PRESIDENTPANEL_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include "ClubManager.h"
#include "ActivityManager.h"

class QTableWidget;
class QLineEdit;
class QComboBox;
class QDateEdit;
class QPushButton;

class PresidentPanel : public QWidget {
    Q_OBJECT
public:
    explicit PresidentPanel(ClubManager* clubMgr, ActivityManager* actMgr, int userId, QWidget* parent = nullptr);

    void refreshClubInfo();
    void refreshActivityTable();

private slots:
    void onSaveClubClicked();
    void onApplyActivityClicked();
    void onDataChanged();

private:
    QLineEdit* clubNameEdit;
    QComboBox* clubTypeCombo;
    QDateEdit* establishDateEdit;
    QLineEdit* advisorEdit;
    QPushButton* saveClubBtn;
    QTableWidget* activityTable;
    QPushButton* applyActivityBtn;

    ClubManager* m_clubMgr;
    ActivityManager* m_actMgr;
    int m_userId;
    ClubInfo m_myClub;
    QVector<ActivityInfo> m_myActs;
};

#endif // PRESIDENTPANEL_H


