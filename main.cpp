#include <QApplication>
#include <QMessageBox>
#include "MainWindow.h"
#include "DatabaseManager.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // 设置字体为微软雅黑
    QFont font("Microsoft YaHei", 10); 
    app.setFont(font);

    // 数据库初始化
    if (!DatabaseManager::instance().openDatabase()) {
        QMessageBox::critical(nullptr, "数据库错误", "无法连接数据库，请检查数据库配置。");
        return 1;
    }

    MainWindow w;
    w.show();

    return app.exec();
}
