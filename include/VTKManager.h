/**
 * @file VTKManager.h
 * @brief VTK 三维可视化管理器
 * @details 封装 VTK 渲染管线与 OpenCASCADE 模型的加载、显示和相机控制
 */

#pragma once

#ifndef CAM_VTKMANAGER_H
#define CAM_VTKMANAGER_H

#include "EnumType.h"
#include <vtkSmartPointer.h>
#include <TopoDS_Shape.hxx>

class QString;
class QVTKOpenGLNativeWidget;
class vtkRenderer;
class vtkRenderWindow;
class vtkInteractorStyleTrackballCamera;
class vtkCompositePolyDataMapper;
class vtkPolyDataMapper;
class vtkOCCTReader;
class vtkActor;
class vtkCamera;
class vtkAxesActor;
class vtkOrientationMarkerWidget;

/**
 * @class VTKManager
 * @brief VTK 渲染管线管理器
 * @details 负责初始化渲染窗口、加载 STEP 模型、设置标准视角，
 *          并提供给 ToolpathPlanner 使用的 vtkRenderer 接口
 */
class VTKManager {
public:
    /** @brief 工厂方法，创建 VTKManager 实例 */
    static VTKManager* New();

    /** @brief 返回嵌入 Qt 窗口的 VTK 控件 */
    [[nodiscard]] QVTKOpenGLNativeWidget * m_vtk_widget() const {
        return m_vtkWidget;
    }

    /** @brief 初始化渲染管线（渲染器、窗口、交互器、坐标轴）*/
    void init();

    /**
     * @brief 加载 STEP/STP 模型文件
     * @param fileName 文件路径
     * @return ResultType::Success 或 ResultType::FileOpenError
     */
    ResultType openModelFile(const QString &fileName);

    /**
     * @brief 设置标准视角（正视、俯视等）
     * @param dx,dy,dz 相机方向向量
     * @param ux,uy,uz 相机上方向向量
     */
    void setStandardView(double dx, double dy, double dz, double ux, double uy, double uz) const;

    /** @brief 返回已加载的 OCCT 形体，供路径规划器使用 */
    [[nodiscard]] const TopoDS_Shape& getShape() const { return m_occtShape; }

    /** @brief 返回 VTK 渲染器，供刀路可视化使用 */
    [[nodiscard]] vtkSmartPointer<vtkRenderer> getRenderer() const { return m_vtkRenderer; }

    /** @brief 触发一次渲染刷新 */
    void renderUpdate() const;

private:
    bool initialized = false;
    TopoDS_Shape m_occtShape;                                                   ///< 已加载的 OCCT 几何体

    QVTKOpenGLNativeWidget *m_vtkWidget = nullptr;                              ///< Qt 嵌入式 VTK 控件
    vtkSmartPointer<vtkRenderer> m_vtkRenderer;                                 ///< 场景渲染器
    vtkSmartPointer<vtkRenderWindow> m_vtkRenderWindow;                         ///< 渲染窗口
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> m_vtkStyle;              ///< 轨迹球交互风格
    vtkSmartPointer<vtkPolyDataMapper> m_vtkMapper;                             ///< 模型数据映射器
    vtkSmartPointer<vtkOCCTReader> m_vtkOCCTReader;                             ///< OCCT 文件读取器
    vtkSmartPointer<vtkActor> m_vtkActor;                                       ///< 模型可视化 Actor
    vtkSmartPointer<vtkCamera> m_vtkCamera;                                     ///< 场景相机
    vtkSmartPointer<vtkAxesActor> m_vtkAxesActor;                               ///< 坐标轴显示 Actor
    vtkSmartPointer<vtkOrientationMarkerWidget> m_vtkOrientationMarkerWidget;   ///< 坐标轴控件
};

#endif //CAM_VTKMANAGER_H
