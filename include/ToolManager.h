#pragma once

#ifndef CAM_TOOLMANAGER_H
#define CAM_TOOLMANAGER_H

#include "Tool.h"
#include <QStandardPaths>

class QDockWidget;
class QTableWidget;

class ToolManager {
public:
    static ToolManager *New();

    ResultType init();

    ResultType openTool(const QString &fileName, FileType fileType, QString *message);

    ResultType saveToolToLocal(QString *message);

    ResultType loadToolFromLocal(QString *message);

    [[nodiscard]] int cur_tool_count() const {
        return curToolCount;
    }

    [[nodiscard]] QList<Tool> tool_list() const {
        return toolList;
    }

private:
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString saveToolLocation = appDataLocation + "/toolList.json";
    QList<Tool> toolList; // 所有刀具
    int curToolCount = 0; // 导入前刀具总数
    Tool *currentTool = nullptr; // 当前刀具
    QDockWidget *toolWidget = nullptr; // 刀具窗口
    QTableWidget *toolTable = nullptr; // 刀具列表

    bool initialized = false;

    static QString getUniqueKey(const Tool &t);

    bool isUniqueTool(const QString &uniKey);
};

#endif //CAM_TOOLMANAGER_H
