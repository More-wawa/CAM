/**
 * @file ToolpathPlanner.cpp
 * @brief 刀具路径规划器实现
 */

#include "ToolpathPlanner.h"
#include <QThread>
#include <QTimer>
#include <cmath>

void ToolpathPlanner::generateFaceMillingAsync(double depth, double stepover, int feedRate, int spindle)
{
    // 使用QTimer模拟异步生成，避免阻塞UI线程
    QTimer::singleShot(100, this, [this, depth, stepover, feedRate, spindle]() {
        emit layerGenerated(1, 3);

        // 生成模拟面铣路径
        Toolpath path;
        path.name = QString("面铣 - D%1mm - 深度%2mm").arg(m_currentTool.diameter).arg(depth);

        // 简单矩形路径模拟
        double toolRadius = m_currentTool.diameter / 2.0;
        double startX = -10.0, startY = -10.0;
        double width = 20.0, height = 20.0;

        // 生成Z字形路径
        std::vector<double> points;
        std::vector<int> segments;

        double z = -depth; // 切削深度
        double y = startY;
        bool direction = true; // true: +X方向, false: -X方向

        while (y <= startY + height) {
            if (direction) {
                // +X方向
                points.push_back(startX);
                points.push_back(y);
                points.push_back(z);
                points.push_back(startX + width);
                points.push_back(y);
                points.push_back(z);
            } else {
                // -X方向
                points.push_back(startX + width);
                points.push_back(y);
                points.push_back(z);
                points.push_back(startX);
                points.push_back(y);
                points.push_back(z);
            }
            segments.push_back(2); // 每段2个点

            y += stepover;
            direction = !direction;
        }

        // 添加安全高度移动
        points.push_back(startX + width/2);
        points.push_back(startY + height/2);
        points.push_back(z);
        points.push_back(startX + width/2);
        points.push_back(startY + height/2);
        points.push_back(10.0); // 安全高度
        segments.push_back(2);

        path.points = points;
        path.segments = segments;

        m_toolpaths.append(path);

        // 在路径上显示刀具形状
        placeToolAlongPath(path);

        emit layerGenerated(2, 3);
        QThread::currentThread()->msleep(100);
        emit layerGenerated(3, 3);
        QThread::currentThread()->msleep(100);

        emit generationFinished(ResultType::Success, "面铣刀路生成完成");
    });
}

void ToolpathPlanner::generateContourMillingAsync(double depth, double stepover, int feedRate, int spindle)
{
    QTimer::singleShot(100, this, [this, depth, stepover, feedRate, spindle]() {
        emit layerGenerated(1, 3);

        // 生成模拟轮廓铣路径
        Toolpath path;
        path.name = QString("轮廓铣 - D%1mm - 深度%2mm").arg(m_currentTool.diameter).arg(depth);

        // 简单矩形轮廓路径
        double toolRadius = m_currentTool.diameter / 2.0;
        double startX = -10.0, startY = -10.0;
        double width = 20.0, height = 20.0;

        std::vector<double> points;
        std::vector<int> segments;

        double z = -depth; // 切削深度

        // 矩形轮廓，从右下角开始，逆时针方向
        // 点1: 右下角
        points.push_back(startX + width);
        points.push_back(startY);
        points.push_back(z);
        // 点2: 右上角
        points.push_back(startX + width);
        points.push_back(startY + height);
        points.push_back(z);
        // 点3: 左上角
        points.push_back(startX);
        points.push_back(startY + height);
        points.push_back(z);
        // 点4: 左下角
        points.push_back(startX);
        points.push_back(startY);
        points.push_back(z);
        // 点5: 回到右下角
        points.push_back(startX + width);
        points.push_back(startY);
        points.push_back(z);

        segments.push_back(5); // 整个轮廓5个点

        // 添加安全高度移动
        points.push_back(startX + width/2);
        points.push_back(startY + height/2);
        points.push_back(z);
        points.push_back(startX + width/2);
        points.push_back(startY + height/2);
        points.push_back(10.0); // 安全高度
        segments.push_back(2);

        path.points = points;
        path.segments = segments;

        m_toolpaths.append(path);

        // 在路径上显示刀具形状
        placeToolAlongPath(path);

        emit layerGenerated(2, 3);
        QThread::currentThread()->msleep(100);
        emit layerGenerated(3, 3);
        QThread::currentThread()->msleep(100);

        emit generationFinished(ResultType::Success, "轮廓铣刀路生成完成");
    });
}

void ToolpathPlanner::generateToolpath(const Tool& tool, int machType, double depth,
                                      double stepover, int feedRate, int spindle)
{
    // 设置当前刀具
    m_currentTool = tool;

    if (machType == 0) { // 面铣
        generateFaceMillingAsync(depth, stepover, feedRate, spindle);
    } else { // 轮廓铣
        generateContourMillingAsync(depth, stepover, feedRate, spindle);
    }
}

vtkSmartPointer<vtkActor> ToolpathPlanner::createToolShape(const Tool& tool)
{
    // 创建圆柱体表示刀具
    vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
    cylinder->SetRadius(tool.diameter / 2.0);
    cylinder->SetHeight(tool.totalLength > 0 ? tool.totalLength : 50.0); // 默认长度50mm
    cylinder->SetResolution(24); // 分辨率

    // 创建变换：圆柱体默认沿Y轴，需要旋转为沿Z轴
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->RotateX(90.0); // 绕X轴旋转90度，使圆柱体沿Z轴

    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputConnection(cylinder->GetOutputPort());
    transformFilter->SetTransform(transform);

    // 创建mapper和actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(transformFilter->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // 设置刀具颜色（金色）
    actor->GetProperty()->SetColor(1.0, 0.84, 0.0); // 金色
    actor->GetProperty()->SetOpacity(0.7); // 半透明
    actor->GetProperty()->SetLighting(true);

    return actor;
}

void ToolpathPlanner::placeToolAlongPath(const Toolpath& path)
{
    if (!m_renderer || path.points.empty()) {
        return;
    }

    // 清除现有的刀具形状
    clearToolShapes();

    // 在路径的关键点放置刀具
    const std::vector<double>& points = path.points;

    // 每隔几个点放置一个刀具形状，避免过多
    // points.size()是坐标数量，每个点有3个坐标(x,y,z)
    int pointCount = static_cast<int>(points.size()) / 3;
    int step = std::max(1, pointCount / 10) * 3; // 大约放置10个刀具，乘以3转换为坐标索引步长

    for (size_t i = 0; i + 2 < points.size(); i += step) {
        double x = points[i];
        double y = points[i + 1];
        double z = points[i + 2];

        // 创建刀具形状
        vtkSmartPointer<vtkActor> toolActor = createToolShape(m_currentTool);

        // 设置刀具位置
        toolActor->SetPosition(x, y, z);

        // 添加到渲染器和列表
        m_renderer->AddActor(toolActor);
        m_toolActors.append(toolActor);
    }

    // 如果路径太短，至少放置一个刀具在起点
    if (m_toolActors.empty() && points.size() >= 3) {
        vtkSmartPointer<vtkActor> toolActor = createToolShape(m_currentTool);
        toolActor->SetPosition(points[0], points[1], points[2]);
        m_renderer->AddActor(toolActor);
        m_toolActors.append(toolActor);
    }
}

void ToolpathPlanner::clearToolShapes()
{
    if (!m_renderer) {
        return;
    }

    // 从渲染器移除所有刀具actor
    for (vtkSmartPointer<vtkActor>& actor : m_toolActors) {
        m_renderer->RemoveActor(actor);
    }

    m_toolActors.clear();
}

void ToolpathPlanner::clearAll()
{
    // 清除刀具形状
    clearToolShapes();

    // 清除刀路数据
    m_toolpaths.clear();
}