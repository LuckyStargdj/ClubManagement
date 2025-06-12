#ifndef PRESIDENTPANEL_H
#define PRESIDENTPANEL_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

#include "ClubManager.h"
#include "ActivityManager.h"
#include "FileManager.h"
#include "DatabaseManager.h"

class PresidentPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PresidentPanel(ClubManager* clubMgr, ActivityManager* actMgr, int userId, QWidget* parent = nullptr);
    void refreshClubInfo();
    void refreshActivityTable();

private slots:
    void onSaveClubClicked();
    void onApplyActivityClicked();
    void onUploadDocumentClicked();
    void onUploadLogoClicked();
    void onDatabaseStatusChanged(bool connected);

private:
    void setupUI();
    void createClubForm();
    void createActivityTable();

    QLineEdit* clubNameEdit;
    QComboBox* clubTypeCombo;
    QDateEdit* establishDateEdit;
    QLineEdit* advisorEdit;
    QPushButton* saveClubBtn;
    QPushButton* uploadDocBtn;
    QPushButton* uploadLogoBtn;
    QTableWidget* activityTable;
    QPushButton* applyActivityBtn;
    QLabel* statusLabel;

    ClubManager* m_clubMgr;
    ActivityManager* m_actMgr;
    int m_userId;
    ClubInfo m_myClub;
};

#endif // PRESIDENTPANEL_H
