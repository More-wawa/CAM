/**
 * @file main.cpp
 * @brief CAM 系统程序入口
 * @details 初始化 Qt 应用程序和 VTK 渲染环境，创建并显示主窗口
 */

#include <QApplication>
#include <qvtkopenglnativewidget.h>
#include "../include/mainwindow.h"

/**
 * @brief 程序入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 应用程序退出码
 */
int main(int argc, char* argv[]) {
    // 设置 VTK OpenGL 默认格式（必须在 QApplication 创建前调用）
    QSurfaceFormat::setDefaultFormat(QSurfaceFormat::defaultFormat());

    QApplication a(argc, argv);

    // 为弹出框（QMessageBox、QDialog）单独设置白色背景，防止继承深色菜单样式
    a.setStyleSheet(R"(
        QMessageBox        { background-color: white; }
        QMessageBox QLabel { color: #333333; }
        QDialog            { background-color: white; }
        QDialog QLabel     { color: #333333; }
        QDialogButtonBox QPushButton {
            background-color: #e0e0e0; color: #333; border-radius: 4px;
            padding: 4px 12px; min-width: 60px;
        }
        QDialogButtonBox QPushButton:hover { background-color: #c8c8c8; }
    )");

    MainWindow w;
    w.show();

    return QApplication::exec();
}
