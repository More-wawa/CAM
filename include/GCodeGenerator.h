/**
 * @file GCodeGenerator.h
 * @brief G 代码生成器
 */

#pragma once

#ifndef CAM_GCODEGENERATOR_H
#define CAM_GCODEGENERATOR_H

#include "Toolpath.h"
#include "Tool.h"
#include "EnumType.h"
#include <QString>
#include <QFile>
#include <QTextStream>

/**
 * @class GCodeGenerator
 * @brief G 代码生成器
 */
class GCodeGenerator {
public:
    static GCodeGenerator* New() { return new GCodeGenerator(); }

    /**
     * @brief 将刀具路径导出为 G 代码字符串
     * @param toolpath 刀具路径
     * @param tool 刀具参数
     * @param feedRate 进给速度
     * @param message 输出消息
     * @return 操作结果
     */
    ResultType generate(const Toolpath& toolpath, const Tool& tool, int feedRate, QString* message);

    /**
     * @brief 将刀具路径导出为 G 代码字符串（简化版本）
     * @param toolpath 刀具路径
     * @return G 代码字符串
     */
    static QString generate(const Toolpath& toolpath);

    /**
     * @brief 将刀具路径导出为 G 代码字符串（简化版本）
     * @param toolpath 刀具路径
     * @param tool 刀具参数
     * @param feedRate 进给速度
     * @param spindle 主轴转速
     * @return G 代码字符串
     */
    static QString generate(const Toolpath& toolpath, const Tool& tool, int feedRate, int spindle);

    QString preview() const;

    QString preview(const Toolpath& toolpath, const Tool& tool, int feedRate, int spindle);

    ResultType exportToFile(const Toolpath& toolpath, const QString& filename);

    ResultType exportToFile(const QString& filename, QString* message);

private:
    QString m_lastPreview;
};

#endif // CAM_GCODEGENERATOR_H