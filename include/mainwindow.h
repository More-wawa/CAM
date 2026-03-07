//
// Created by More on 2026/3/5.
//

#ifndef CAM_MAINWINDOW_H
#define CAM_MAINWINDOW_H

#include <QMainWindow>
#include <vtkSmartPointer.h>
#include "Tool.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class QVTKOpenGLNativeWidget;
class vtkRenderer;
class vtkRenderWindow;
class vtkInteractorStyleTrackballCamera;
class vtkCompositePolyDataMapper;
class vtkOCCTReader;
class vtkActor;
class vtkCamera;
class vtkAxesActor;
class vtkOrientationMarkerWidget;
class QDockWidget;
class QTableWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    bool vtkInitialized = false;
    bool modelLoaded = false;
    QString toolSavePath = "/toolList";
    QList<Tool> toolList; // 所有刀具
    size_t toolCount = 0; // 导入前刀具总数
    Tool *currentTool = nullptr; // 当前刀具
    QDockWidget *toolWidget = nullptr; // 刀具窗口
    QTableWidget *toolTable = nullptr; // 刀具列表

    QVTKOpenGLNativeWidget *m_vtkWidget = nullptr;
    vtkSmartPointer<vtkRenderer> m_vtkRenderer;
    vtkSmartPointer<vtkRenderWindow> m_vtkRenderWindow;
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> m_vtkStyle;
    vtkSmartPointer<vtkCompositePolyDataMapper> m_vtkMapper;
    vtkSmartPointer<vtkOCCTReader> m_vtkOCCTReader;
    vtkSmartPointer<vtkActor> m_vtkActor;
    vtkSmartPointer<vtkCamera> m_vtkCamera;
    vtkSmartPointer<vtkAxesActor> m_vtkAxesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_vtkOrientationMarkerWidget;

    void init();

    void initVTK();

    void setStandardView(double dx, double dy, double dz, double ux, double uy, double uz) const;

    void updateToolTable() const;

    void saveToolTable();

    void loadToolTable();

    static QString getUniqueKey(const Tool &t);

    bool isUniqueTool(const QString &uniKey);

private slots:
    void on_actionOpenFile_triggered();

    void on_actionViewTop_triggered() const;

    void on_actionViewBottom_triggered() const;

    void on_actionViewFront_triggered() const;

    void on_actionViewBack_triggered() const;

    void on_actionViewLeft_triggered() const;

    void on_actionViewRight_triggered() const;

    void on_actionViewTool_triggered() const;

    void on_actionImportTool_triggered();
};

#endif //CAM_MAINWINDOW_H
