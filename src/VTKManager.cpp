#include "../include/vtkManager.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCompositePolyDataMapper.h>
#include <vtkCamera.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOCCTReader.h>


VTKManager* VTKManager::New()
{
    return new VTKManager();
}

void VTKManager::init() {
    if (initialized) return;

    // 创建 VTK Widget
    m_vtkWidget = new QVTKOpenGLNativeWidget();

    // 创建渲染器
    m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_vtkRenderer->SetBackground(0.1, 0.2, 0.3); // 深蓝背景

    // 启用渐变背景
    m_vtkRenderer->GradientBackgroundOn();
    m_vtkRenderer->SetBackground(1, 1, 1);
    m_vtkRenderer->SetBackground2(0.5, 0.5, 0.8);

    // 把渲染器加到窗口
    m_vtkWidget->renderWindow()->AddRenderer(m_vtkRenderer);

    // 交互器风格
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
    initialized = true;
}

ErrorType VTKManager::openModelFile(QString fileName) {
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
        return ErrorType::ModulError;
    }
    qDebug() << "STEP 文件读取成功";

    // 创建 Actor 并添加至渲染器
    m_vtkActor = vtkSmartPointer<vtkActor>::New();
    m_vtkActor->SetMapper(m_vtkMapper);
    m_vtkRenderer->AddActor(m_vtkActor);

    // 重置相机并渲染
    m_vtkRenderer->ResetCamera();
    m_vtkWidget->renderWindow()->Render();

    return ErrorType::Success;
}

void VTKManager::setStandardView(const double dx, const double dy, const double dz, const double ux, const double uy,
                                 const double uz) const {
    if (!m_vtkCamera || !m_vtkRenderer) {
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
