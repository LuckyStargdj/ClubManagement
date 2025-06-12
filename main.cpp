#include <QApplication>
#include <QMessageBox>
#include "DatabaseManager.h"
#include "UserManager.h"
#include "LoginDialog.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // 初始化数据库
    DatabaseManager& dbMgr = DatabaseManager::instance();
    if (!dbMgr.openDatabase()) {
        QMessageBox::critical(nullptr, "数据库错误", "无法打开数据库");
        return -1;
    }

    UserManager userManager;
    LoginDialog loginDlg(&userManager);
    if (loginDlg.exec() == QDialog::Accepted) {
        MainWindow w(&userManager);
        w.show();
        return a.exec();
    }

    return 0;
}