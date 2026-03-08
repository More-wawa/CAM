#pragma once

#ifndef CAM_TOOLMANAGER_H
#define CAM_TOOLMANAGER_H

#include "Tool.h"
#include <QStandardPaths>

class ToolManager {
public:
    static ToolManager *New();

    static QString getUniqueKey(const Tool &t);

    ResultType init();

    ResultType openTool(const QString &fileName, FileType fileType, QString *message);

    ResultType saveToolToLocal(QString *message);

    ResultType loadToolFromLocal(QString *message);

    ResultType deleteToolSelected(QString *message);

    [[nodiscard]] int cur_tool_count() const {
        return curToolCount;
    }

    [[nodiscard]] QList<Tool> tool_list() const {
        return toolList;
    }

    void set_tool_list_selected(const QList<Tool> &tool_list_selected) {
        toolListSelected = tool_list_selected;
    }

    void set_current_tool(const Tool &current_tool) {
        currentTool = current_tool;
    }

private:
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString saveToolLocation = appDataLocation + "/toolList.json";
    Tool currentTool = Tool(); // 当前选中刀具
    QList<Tool> toolList; // 所有刀具
    QList<Tool> toolListSelected; // 已选中的刀具数组
    int curToolCount = 0; // 导入前刀具总数

    bool initialized = false;

    bool isUniqueTool(const QString &uniKey);
};

#endif //CAM_TOOLMANAGER_H
