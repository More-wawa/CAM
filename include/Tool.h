#pragma once

#ifndef CAM_TOOL_H
#define CAM_TOOL_H

#endif //CAM_TOOL_H

#include "EnumType.h"
#include <QString>

struct Tool {
    FileType fileType; // 文件格式
    QString name; // 刀具名称 e.g. "D6 平底铣刀"
    double diameter = 0.0; // 直径 mm
    double fluteLength = 0.0; // 刃长
    double totalLength = 0.0; // 总长
    double cornerRadius = 0.0; // 刀尖圆角（球刀、圆鼻刀用）
    int fluteCount = 2; // 刃数
    QString material; // 材质 e.g. "硬质合金"、"高速钢"
    QString type; // "平底铣刀"、"球头铣刀"、"钻头"等
    QString uniqueKey; // 刀具唯一标识
};
