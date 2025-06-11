#include <QApplication>
#include <QMessageBox>
#include "MainWindow.h"
#include "DatabaseManager.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // ��������Ϊ΢���ź�
    QFont font("Microsoft YaHei", 10); 
    app.setFont(font);

    // ���ݿ��ʼ��
    if (!DatabaseManager::instance().openDatabase()) {
        QMessageBox::critical(nullptr, "���ݿ����", "�޷��������ݿ⣬�������ݿ����á�");
        return 1;
    }

    MainWindow w;
    w.show();

    return app.exec();
}
