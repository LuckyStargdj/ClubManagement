#ifndef STUDENTPANEL_H
#define STUDENTPANEL_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include "ActivityManager.h"

class QTableWidget;
class QLineEdit;
class QComboBox;
class QDateEdit;
class QPushButton;

class StudentPanel : public QWidget {
    Q_OBJECT
public:
    explicit StudentPanel(ActivityManager* activityMgr, int userId, QWidget* parent = nullptr);

    void refreshActivityTable();
    void refreshActiveClubTable();

private slots:
    void onSearchClicked();
    void onFeedbackClicked();
    void onFeedbackResult(bool success, const QString& msg);

private:
    QTableWidget* activityTable;
    QTableWidget* activeClubTable;
    QLineEdit* searchEdit;
    QComboBox* typeCombo;
    QDateEdit* startDateEdit;
    QDateEdit* endDateEdit;
    QPushButton* searchBtn;
    QPushButton* feedbackBtn;

    ActivityManager* m_activityMgr;
    int m_userId;
    QVector<ActivityInfo> m_currentActs;
};

#endif // STUDENTPANEL_H


