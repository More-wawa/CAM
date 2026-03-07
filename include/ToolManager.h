#pragma once

#ifndef CAM_TOOLMANAGER_H
#define CAM_TOOLMANAGER_H

#include "Tool.h"
#include <QStandardPaths>

class QDockWidget;
class QTableWidget;

class ToolManager {
private:
    QString toolSavePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/toolList";
    QList<Tool> toolList; // 所有刀具
    size_t toolCount = 0; // 导入前刀具总数
    Tool *currentTool = nullptr; // 当前刀具
    QDockWidget *toolWidget = nullptr; // 刀具窗口
    QTableWidget *toolTable = nullptr; // 刀具列表

    int openTool(QString fileName);
};

#endif //CAM_TOOLMANAGER_H
