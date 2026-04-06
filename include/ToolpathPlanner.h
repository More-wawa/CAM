/**
 * @file ToolpathPlanner.h
 * @brief 刀具路径规划器
 */

#pragma once

#ifndef CAM_TOOLPATHPLANNER_H
#define CAM_TOOLPATHPLANNER_H

#include "Toolpath.h"
#include "Tool.h"
#include "EnumType.h"
#include <QObject>
#include <QList>

#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <TopoDS_Shape.hxx>

/**
 * @class ToolpathPlanner
 * @brief 刀具路径规划器，负责生成加工路径
 */
class ToolpathPlanner : public QObject {
    Q_OBJECT

public:
    static ToolpathPlanner* New() { return new ToolpathPlanner(); }

    explicit ToolpathPlanner(QObject* parent = nullptr) : QObject(parent) {}

    void setRenderer(vtkRenderer* renderer) { m_renderer = renderer; }
    void clearAll();
    void setShape(const TopoDS_Shape& shape) { m_shape = shape; }
    void setCurrentTool(const Tool& tool) { m_currentTool = tool; }
    int count() const { return m_toolpaths.size(); }
    const QList<Toolpath>& toolpaths() const { return m_toolpaths; }
    bool isGenerating() const { return false; }
    void setToolpathVisible(int index, bool visible) {
        if (index >= 0 && index < m_toolpaths.size()) {
            m_toolpaths[index].visible = visible;
        }
    }

    void generateFaceMillingAsync(double depth, double stepover, int feedRate, int spindle);
    void generateContourMillingAsync(double depth, double stepover, int feedRate, int spindle);

    /**
     * @brief 生成刀具路径（异步）
     * @param tool 刀具参数
     * @param machType 加工类型
     * @param depth 每层切削深度
     * @param stepover 行距
     * @param feedRate 进给速度
     * @param spindle 主轴转速
     */
    void generateToolpath(const Tool& tool, int machType, double depth,
                          double stepover, int feedRate, int spindle);

signals:
    void layerGenerated(int current, int total); ///< 每层生成完成
    void generationFinished(ResultType result, QString message); ///< 全部生成完成

private:
    /**
     * @brief 创建刀具3D形状（圆柱体）
     * @param tool 刀具参数
     * @return 刀具actor
     */
    vtkSmartPointer<vtkActor> createToolShape(const Tool& tool);

    /**
     * @brief 沿路径放置刀具形状
     * @param path 刀具路径
     */
    void placeToolAlongPath(const Toolpath& path);

    /**
     * @brief 清除所有刀具形状
     */
    void clearToolShapes();

private:
    vtkRenderer* m_renderer = nullptr;
    TopoDS_Shape m_shape;
    Tool m_currentTool;
    QList<Toolpath> m_toolpaths;
    QList<vtkSmartPointer<vtkActor>> m_toolActors; ///< 刀具形状actor列表
};

#endif // CAM_TOOLPATHPLANNER_H