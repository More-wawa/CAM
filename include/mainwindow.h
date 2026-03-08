#pragma once

#ifndef CAM_MAINWINDOW_H
#define CAM_MAINWINDOW_H

#include <QMainWindow>
#include <vtkSmartPointer.h>

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class VTKManager;
class ToolManager;
class QDockWidget;
class QTableWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    bool modelLoaded = false;
    QDockWidget *toolWidget = nullptr; // 刀具窗口
    QTableWidget *toolTable = nullptr; // 刀具列表

    VTKManager *vtkManager;
    ToolManager *toolManager;

    void init();

    void updateToolTable() const;

private slots:
    void on_actionOpenFile_triggered();

    void on_actionViewTop_triggered() const;

    void on_actionViewBottom_triggered() const;

    void on_actionViewFront_triggered() const;

    void on_actionViewBack_triggered() const;

    void on_actionViewLeft_triggered() const;

    void on_actionViewRight_triggered() const;

    void on_actionViewTool_triggered() const;

    void on_actionOpenTool_triggered();
};

#endif //CAM_MAINWINDOW_H
