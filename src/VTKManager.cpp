/**
 * @file VTKManager.cpp
 * @brief VTK 三维可视化管理器实现
 */

#include "../include/vtkManager.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <IVtkOCC_Shape.hxx>
#include <IVtkTools_ShapeDataSource.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <vtkActor.h>
#include <QDebug>
#include <vtkProperty.h>

VTKManager* VTKManager::New()
{
    return new VTKManager();
}

/**
 * @brief 初始化 VTK 渲染管线
 * @details 创建渲染器、窗口、交互器、坐标轴等组件
 */
void VTKManager::init()
{
    if (initialized) return;

    m_vtkWidget = new QVTKOpenGLNativeWidget();
    m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();

    // 渐变背景（白色到淡蓝）
    m_vtkRenderer->GradientBackgroundOn();
    m_vtkRenderer->SetBackground(1, 1, 1);
    m_vtkRenderer->SetBackground2(0.5, 0.5, 0.8);

    m_vtkWidget->renderWindow()->AddRenderer(m_vtkRenderer);

    // 轨迹球交互风格
    m_vtkStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_vtkWidget->renderWindow()->GetInteractor()->SetInteractorStyle(m_vtkStyle);

    m_vtkMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_vtkCamera = m_vtkRenderer->GetActiveCamera();

    // 坐标轴指示器
    m_vtkAxesActor = vtkSmartPointer<vtkAxesActor>::New();
    m_vtkAxesActor->SetShaftTypeToCylinder();
    m_vtkAxesActor->SetXAxisLabelText("X");
    m_vtkAxesActor->SetYAxisLabelText("Y");
    m_vtkAxesActor->SetZAxisLabelText("Z");

    m_vtkOrientationMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    m_vtkOrientationMarkerWidget->SetOrientationMarker(m_vtkAxesActor);
    m_vtkOrientationMarkerWidget->SetInteractor(m_vtkWidget->renderWindow()->GetInteractor());
    m_vtkOrientationMarkerWidget->SetEnabled(1);
    m_vtkOrientationMarkerWidget->InteractiveOn();

    initialized = true;
}

/**
 * @brief 加载 STEP 模型文件
 * @details 使用 OCCT 读取并网格化，再通过 IVtk 桥接转为 VTK Actor 显示
 */
ResultType VTKManager::openModelFile(const QString& fileName)
{
    // 1. 环境准备
    if (fileName.isEmpty() || !m_vtkRenderer)
    {
        return ResultType::ModulError;
    }

    // 清空旧的渲染内容
    m_vtkRenderer->RemoveAllViewProps();

    // 使用 OCCT 原生接口读取 STEP 文件
    STEPControl_Reader reader;
    // 使用 toUtf8() 确保支持中文路径
    IFSelect_ReturnStatus status = reader.ReadFile(fileName.toUtf8().constData());

    if (status != IFSelect_RetDone)
    {
        qDebug() << "STEP 文件读取失败，状态码:" << (int)status;
        return ResultType::ModulError;
    }

    // 执行转换逻辑
    reader.TransferRoots();
    TopoDS_Shape occtShape = reader.OneShape();

    if (occtShape.IsNull())
    {
        qDebug() << "转换后的 Shape 为空";
        return ResultType::ModulError;
    }

    m_occtShape = occtShape;

    // 网格化
    BRepMesh_IncrementalMesh meshGenerator(occtShape, 0.1);
    if (!meshGenerator.IsDone())
    {
        qDebug() << "网格化失败";
        return ResultType::ModulError;
    }

    // 使用 IVtk 桥接将 OCCT 对象转为 VTK 数据源
    Handle(IVtkOCC_Shape) aShapeWrapper = new IVtkOCC_Shape(occtShape);

    // 创建 VTK 数据源并关联 OCCT 包装对象
    vtkNew<IVtkTools_ShapeDataSource> shapeSource;
    shapeSource->SetShape(aShapeWrapper);

    // 更新渲染管线 (Pipeline)
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(shapeSource->GetOutputPort());

    // 创建 Actor 并渲染
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    // 设置一些基本样式，方便观察
    actor->GetProperty()->SetColor(0.8, 0.8, 0.9); // 淡蓝色

    m_vtkRenderer->AddActor(actor);

    // 刷新界面
    m_vtkRenderer->ResetCamera();
    if (m_vtkWidget && m_vtkWidget->renderWindow())
    {
        m_vtkWidget->renderWindow()->Render();
    }

    qDebug() << "STEP 加载成功:" << fileName;
    return ResultType::Success;
}

void VTKManager::renderUpdate() const
{
    if (m_vtkWidget && m_vtkWidget->renderWindow())
        m_vtkWidget->renderWindow()->Render();
}

/**
 * @brief 设置标准视角
 * @details 根据方向向量和上方向调整相机位置，保持焦点不变
 */
void VTKManager::setStandardView(const double dx, const double dy, const double dz, const double ux, const double uy,
                                 const double uz) const
{
    if (!m_vtkCamera || !m_vtkRenderer)
    {
        qDebug() << "model is still non-loaded";
        return;
    }

    // 获取当前焦点和距离
    m_vtkRenderer->ResetCamera();
    const double* focal = m_vtkCamera->GetFocalPoint();
    const double dist = m_vtkCamera->GetDistance() * 1.2;

    // 设置新视角
    m_vtkCamera->SetPosition(
        focal[0] + dx * dist,
        focal[1] + dy * dist,
        focal[2] + dz * dist
    );
    m_vtkCamera->SetFocalPoint(focal);
    m_vtkCamera->SetViewUp(ux, uy, uz);

    m_vtkCamera->OrthogonalizeViewUp();
    m_vtkRenderer->ResetCameraClippingRange();
    m_vtkWidget->renderWindow()->Render();
}
