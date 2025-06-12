#ifndef STUDENTPANEL_H
#define STUDENTPANEL_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>

#include "ActivityManager.h"
#include "DatabaseManager.h"

class StudentPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StudentPanel(ActivityManager* activityMgr, int userId, QWidget* parent = nullptr);
    void refreshActivityTable();
    void refreshActiveClubTable();

private slots:
    void onSearchClicked();
    void onFeedbackClicked();
    void onDatabaseStatusChanged(bool connected);

private:
    void setupUI();
    void createActivityTable();
    void createClubTable();

    QLineEdit* searchEdit;
    QComboBox* typeCombo;
    QDateEdit* startDateEdit;
    QDateEdit* endDateEdit;
    QPushButton* searchBtn;
    QPushButton* feedbackBtn;
    QTableWidget* activityTable;
    QTableWidget* clubTable;

    ActivityManager* m_activityMgr;
    int m_userId;
    QVector<ActivityInfo> m_currentActivities;
    QLabel* statusLabel;
};

#endif // STUDENTPANEL_H

