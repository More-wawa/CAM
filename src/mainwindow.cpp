//
// Created by More on 2026/3/5.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "../include/mainwindow.h"
#include "ui_MainWindow.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCompositePolyDataMapper.h>
#include <QFileDialog>
#include <vtkOCCTReader.h>
#include <vtkCamera.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    initVTK();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initVTK() {
    if (vtkInitialized) return;

    setWindowTitle("CAM");

    // 创建 VTK Widget
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    setCentralWidget(m_vtkWidget);

    // 创建渲染器
    m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_vtkRenderer->SetBackground(0.1, 0.2, 0.3); // 深蓝背景

    // 启用渐变背景
    m_vtkRenderer->GradientBackgroundOn();
    m_vtkRenderer->SetBackground(1, 1, 1);
    m_vtkRenderer->SetBackground2(0.5, 0.5, 0.8);

    // 把渲染器加到窗口
    m_vtkWidget->renderWindow()->AddRenderer(m_vtkRenderer);

    // 交互器风格（鼠标旋转、缩放、平移）
    m_vtkStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_vtkWidget->renderWindow()->GetInteractor()->SetInteractorStyle(m_vtkStyle);

    // Mapper
    m_vtkMapper = vtkSmartPointer<vtkCompositePolyDataMapper>::New();

    // 获取摄像机
    m_vtkCamera = m_vtkRenderer->GetActiveCamera();

    // 开启参考线
    m_vtkAxesActor = vtkSmartPointer<vtkAxesActor>::New();
    m_vtkAxesActor->SetShaftTypeToCylinder();
    m_vtkAxesActor->SetXAxisLabelText("X");
    m_vtkAxesActor->SetYAxisLabelText("Y");
    m_vtkAxesActor->SetZAxisLabelText("Z");

    // 创建 widget 并关联
    m_vtkOrientationMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    m_vtkOrientationMarkerWidget->SetOrientationMarker(m_vtkAxesActor);
    m_vtkOrientationMarkerWidget->SetInteractor(m_vtkWidget->renderWindow()->GetInteractor());

    // 开启参考线并放置屏幕左下角
    m_vtkOrientationMarkerWidget->SetEnabled(1);
    m_vtkOrientationMarkerWidget->InteractiveOn();

    // 确认已经初始化
    vtkInitialized = true;
}

void MainWindow::on_actionOpenFile_triggered() {
    // 确保初始化完成
    if (!vtkInitialized) initVTK();

    // 获取 STEP 文件路径
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        tr(""),
        tr("STEP 文件 (*.step *.stp);;所有文件 (*.*)")
    );

    if (fileName.isEmpty()) {
        qDebug() << "文件打开失败";
        return;
    }
    qDebug() << "文件打开成功";

    // 打开新 STEP 文件之前先清空缓存
    m_vtkRenderer->RemoveAllViewProps();

    // 创建 OCCTReader 读取 STEP 文件
    m_vtkOCCTReader = vtkSmartPointer<vtkOCCTReader>::New();
    m_vtkOCCTReader->SetFileName(fileName.toUtf8().constData());
    m_vtkOCCTReader->SetFileFormat(vtkOCCTReader::STEP);

    // 更新 pipeline
    m_vtkMapper->SetInputConnection(m_vtkOCCTReader->GetOutputPort());

    // 检查输出是否有效
    if (!m_vtkOCCTReader->GetOutput()) {
        qDebug() << "STEP 文件读取失败";
        return;
    }
    qDebug() << "STEP 文件读取成功";

    // 创建 Actor 并添加至渲染器
    m_vtkActor = vtkSmartPointer<vtkActor>::New();
    m_vtkActor->SetMapper(m_vtkMapper);
    m_vtkRenderer->AddActor(m_vtkActor);

    // 重置相机并渲染
    m_vtkRenderer->ResetCamera();
    m_vtkWidget->renderWindow()->Render();

    // 标记模型已加载
    modelLoaded = true;
}

void MainWindow::setStandardView(const double dx, const double dy, const double dz, const double ux, const double uy, const double uz) const {
    if (!modelLoaded || !m_vtkCamera || !m_vtkRenderer) {
        qDebug() << "model is still non-loaded";
        return;
    }

    // 获得当前模型最合适的距离和焦点
    m_vtkRenderer->ResetCamera();
    const double *focal = m_vtkCamera->GetFocalPoint();
    const double dist = m_vtkCamera->GetDistance() * 1.2;

    // 设置新视角
    m_vtkCamera->SetPosition(
        focal[0] + dx * dist,
        focal[1] + dy * dist,
        focal[2] + dz * dist
    );
    m_vtkCamera->SetFocalPoint(focal);
    m_vtkCamera->SetViewUp(ux, uy, uz);

    // 消除警告
    m_vtkCamera->OrthogonalizeViewUp();

    m_vtkRenderer->ResetCameraClippingRange();
    m_vtkWidget->renderWindow()->Render();
}

void MainWindow::on_actionViewTop_triggered() const {
    setStandardView(0, 1, 0, 0, 0, 1);
}

void MainWindow::on_actionViewBottom_triggered() const {
    setStandardView(0, -1, 0, 0, 0, 1);
}

void MainWindow::on_actionViewFront_triggered() const {
    setStandardView(0, 0, 1, 0, 1, 0);
}

void MainWindow::on_actionViewBack_triggered() const {
    setStandardView(0, 0, -1, 0, 1, 0);
}

void MainWindow::on_actionViewLeft_triggered() const {
    setStandardView(-1, 0, 0, 0, 1, 0);
}

void MainWindow::on_actionViewRight_triggered() const {
    setStandardView(1, 0, 0, 0, 1, 0);
}
