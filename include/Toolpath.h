/**
 * @file Toolpath.h
 * @brief 刀具路径数据结构
 */

#pragma once

#ifndef CAM_TOOLPATH_H
#define CAM_TOOLPATH_H

#include <vector>
#include <QString>

/**
 * @class Toolpath
 * @brief 表示一条刀具路径
 */
class Toolpath {
public:
    QString name; ///< 路径名称
    bool visible = true; ///< 是否可见
    std::vector<double> points; ///< 路径点（临时占位）
    std::vector<int> segments; ///< 段数
};

#endif // CAM_TOOLPATH_H