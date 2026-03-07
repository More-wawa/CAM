//
// Created by More on 2026/3/5.
//

#ifndef CAM_MAINWINDOW_H
#define CAM_MAINWINDOW_H

#include <QMainWindow>
#include <vtkSmartPointer.h>

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

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    bool vtkInitialized = false;
    bool modelLoaded = false;

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

    void initVTK();
    void setStandardView(double dx, double dy, double dz, double ux, double uy, double uz) const;

private slots:
    void on_actionOpenFile_triggered();

    void on_actionViewTop_triggered() const;
    void on_actionViewBottom_triggered() const;
    void on_actionViewFront_triggered() const;
    void on_actionViewBack_triggered() const;
    void on_actionViewLeft_triggered() const;
    void on_actionViewRight_triggered() const;
};

#endif //CAM_MAINWINDOW_H