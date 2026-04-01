/**
 * @file Tool.h
 * @brief 刀具数据结构定义
 * @details 定义了CAM系统中使用的刀具参数结构体
 */

#pragma once

#ifndef CAM_TOOL_H
#define CAM_TOOL_H

#include "EnumType.h"
#include <QString>

/**
 * @struct Tool
 * @brief 刀具参数结构体
 * @details 包含刀具的几何参数、材质信息和唯一标识
 */
struct Tool {
    FileType fileType;           ///< 文件格式类型
    QString name;                ///< 刀具名称，例如 "D6 平底铣刀"
    double diameter = 0.0;       ///< 刀具直径 (mm)
    double fluteLength = 0.0;    ///< 刃长 (mm)
    double totalLength = 0.0;    ///< 总长 (mm)
    double cornerRadius = 0.0;   ///< 刀尖圆角半径 (mm)，用于球刀、圆鼻刀
    int fluteCount = 2;          ///< 刃数
    QString material;            ///< 刀具材质，例如 "硬质合金"、"高速钢"
    QString type;                ///< 刀具类型，例如 "平底铣刀"、"球头铣刀"、"钻头"
    QString uniqueKey;           ///< 刀具唯一标识符
};

#endif //CAM_TOOL_H
