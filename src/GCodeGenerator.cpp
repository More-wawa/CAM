/**
 * @file GCodeGenerator.cpp
 * @brief G代码生成器实现
 */

#include "GCodeGenerator.h"
#include <QDateTime>
#include <QThread>

ResultType GCodeGenerator::generate(const Toolpath& toolpath, const Tool& tool, int feedRate, QString* message)
{
    if (message) *message = "G代码生成成功";

    // 生成简单的G代码
    QString gcode;

    // 程序头
    gcode += QString("(CAM系统生成 - %1)\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    gcode += QString("(刀具: %1, 直径: %2mm)\n").arg(tool.name).arg(tool.diameter);
    gcode += QString("(进给速度: %1 mm/min)\n\n").arg(feedRate);

    // G代码初始化
    gcode += "G90 G94 G17 G40 G49 G80\n"; // 绝对坐标，每分钟进给，XY平面，取消补偿
    gcode += "G21\n"; // 毫米模式
    gcode += "G54\n"; // 工件坐标系

    // 快速移动到安全高度
    gcode += "G00 Z10.0\n";

    // 解析刀具路径点
    const std::vector<double>& points = toolpath.points;
    const std::vector<int>& segments = toolpath.segments;

    int pointIndex = 0;
    bool firstMove = true;

    for (size_t i = 0; i < segments.size(); i++) {
        int segmentPointCount = segments[i];

        for (int j = 0; j < segmentPointCount; j++) {
            if (pointIndex + 2 >= points.size()) break;

            double x = points[pointIndex];
            double y = points[pointIndex + 1];
            double z = points[pointIndex + 2];
            pointIndex += 3;

            if (firstMove) {
                // 第一个点：快速定位到XY，然后下刀到Z
                gcode += QString("G00 X%1 Y%2\n").arg(x, 0, 'f', 3).arg(y, 0, 'f', 3);
                gcode += QString("G01 Z%1 F%2\n").arg(z, 0, 'f', 3).arg(feedRate);
                firstMove = false;
            } else {
                // 后续点：直线插补
                gcode += QString("G01 X%1 Y%2 Z%3 F%4\n")
                            .arg(x, 0, 'f', 3)
                            .arg(y, 0, 'f', 3)
                            .arg(z, 0, 'f', 3)
                            .arg(feedRate);
            }
        }
    }

    // 程序结束
    gcode += "G00 Z10.0\n"; // 抬刀到安全高度
    gcode += "M05\n"; // 主轴停止
    gcode += "M30\n"; // 程序结束

    m_lastPreview = gcode;
    return ResultType::Success;
}

QString GCodeGenerator::generate(const Toolpath& toolpath)
{
    Tool defaultTool;
    defaultTool.name = "默认刀具";
    defaultTool.diameter = 10.0;
    return generate(toolpath, defaultTool, 1000, 0);
}

QString GCodeGenerator::generate(const Toolpath& toolpath, const Tool& tool, int feedRate, int spindle)
{
    QString gcode;

    // 程序头
    gcode += QString("(CAM系统生成 - %1)\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    gcode += QString("(刀具: %1, 直径: %2mm)\n").arg(tool.name).arg(tool.diameter);
    gcode += QString("(进给速度: %1 mm/min, 主轴转速: %2 RPM)\n\n").arg(feedRate).arg(spindle);

    // G代码初始化
    gcode += "G90 G94 G17 G40 G49 G80\n"; // 绝对坐标，每分钟进给，XY平面，取消补偿
    gcode += "G21\n"; // 毫米模式
    gcode += "G54\n"; // 工件坐标系

    // 主轴启动
    if (spindle > 0) {
        gcode += QString("M03 S%1\n").arg(spindle);
    }

    // 快速移动到安全高度
    gcode += "G00 Z10.0\n";

    // 解析刀具路径点
    const std::vector<double>& points = toolpath.points;
    const std::vector<int>& segments = toolpath.segments;

    int pointIndex = 0;
    bool firstMove = true;

    for (size_t i = 0; i < segments.size(); i++) {
        int segmentPointCount = segments[i];

        for (int j = 0; j < segmentPointCount; j++) {
            if (pointIndex + 2 >= points.size()) break;

            double x = points[pointIndex];
            double y = points[pointIndex + 1];
            double z = points[pointIndex + 2];
            pointIndex += 3;

            if (firstMove) {
                // 第一个点：快速定位到XY，然后下刀到Z
                gcode += QString("G00 X%1 Y%2\n").arg(x, 0, 'f', 3).arg(y, 0, 'f', 3);
                gcode += QString("G01 Z%1 F%2\n").arg(z, 0, 'f', 3).arg(feedRate);
                firstMove = false;
            } else {
                // 后续点：直线插补
                gcode += QString("G01 X%1 Y%2 Z%3 F%4\n")
                            .arg(x, 0, 'f', 3)
                            .arg(y, 0, 'f', 3)
                            .arg(z, 0, 'f', 3)
                            .arg(feedRate);
            }
        }
    }

    // 程序结束
    gcode += "G00 Z10.0\n"; // 抬刀到安全高度
    gcode += "M05\n"; // 主轴停止
    gcode += "M30\n"; // 程序结束

    return gcode;
}

QString GCodeGenerator::preview(const Toolpath& toolpath, const Tool& tool, int feedRate, int spindle)
{
    m_lastPreview = generate(toolpath, tool, feedRate, spindle);
    return m_lastPreview;
}

ResultType GCodeGenerator::exportToFile(const Toolpath& toolpath, const QString& filename)
{
    QString gcode = generate(toolpath);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return ResultType::FileOpenError;
    }
    QTextStream out(&file);
    out << gcode;
    return ResultType::Success;
}

ResultType GCodeGenerator::exportToFile(const QString& filename, QString* message)
{
    if (m_lastPreview.isEmpty()) {
        if (message) *message = "没有可导出的G代码";
        return ResultType::ModulError;
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (message) *message = "无法打开文件";
        return ResultType::FileOpenError;
    }
    QTextStream out(&file);
    out << m_lastPreview;
    if (message) *message = "导出成功";
    return ResultType::Success;
}

QString GCodeGenerator::preview() const
{
    return m_lastPreview;
}