/**
 * @file ToolManager.h
 * @brief 刀具库管理器
 * @details 负责刀具的增删查、文件持久化（JSON），以及当前选中刀具的状态维护
 */

#pragma once

#ifndef CAM_TOOLMANAGER_H
#define CAM_TOOLMANAGER_H

#include "Tool.h"
#include <QStandardPaths>

/**
 * @class ToolManager
 * @brief 刀具库管理器
 * @details 使用工厂模式创建。刀具数据以 JSON 格式持久化到用户数据目录，
 *          并在运行时维护内存中的刀具列表和当前选中状态。
 */
class ToolManager {
public:
    /** @brief 工厂方法，创建 ToolManager 实例 */
    static ToolManager *New();

    /**
     * @brief 根据刀具字段生成唯一标识符
     * @param t 刀具结构体
     * @return 由名称、直径、类型拼接的唯一键
     */
    static QString getUniqueKey(const Tool &t);

    /**
     * @brief 初始化数据目录，检查并创建刀具库存储路径
     * @return ResultType::Success 或 ResultType::CreateFileError
     */
    ResultType init();

    /**
     * @brief 从外部文件导入刀具（去重后追加）
     * @param fileName  文件路径
     * @param fileType  文件格式
     * @param message   操作结果说明
     */
    ResultType openTool(const QString &fileName, FileType fileType, QString *message);

    /**
     * @brief 将当前刀具列表序列化保存到本地 JSON 文件
     * @param message 操作结果说明
     */
    ResultType saveToolToLocal(QString *message);

    /**
     * @brief 从本地 JSON 文件加载刀具列表
     * @param message 操作结果说明
     */
    ResultType loadToolFromLocal(QString *message);

    /**
     * @brief 删除 toolListSelected 中的刀具
     * @param message 操作结果说明（含删除数量或当前刀具信息）
     */
    ResultType deleteToolSelected(QString *message);

    /**
     * @brief 手动添加一把刀具
     * @param m_name         名称
     * @param m_diameter     直径 (mm)
     * @param m_fluteLength  刃长 (mm)
     * @param m_totalLength  总长 (mm)
     * @param m_cornerRadius 刀尖圆角 (mm)
     * @param m_fluteCount   刃数
     * @param m_material     材质
     * @param m_type         类型
     * @param message        操作结果说明
     */
    ResultType addTool(
        QString m_name,
        double m_diameter,
        double m_fluteLength,
        double m_totalLength,
        double m_cornerRadius,
        int m_fluteCount,
        QString m_material,
        QString m_type,
        QString *message
    );

    /** @brief 返回当前刀具总数 */
    [[nodiscard]] int cur_tool_count() const { return curToolCount; }

    /** @brief 返回完整刀具列表 */
    [[nodiscard]] QList<Tool> tool_list() const { return toolList; }

    /** @brief 设置用户在表格中选中的刀具列表 */
    void set_tool_list_selected(const QList<Tool> &tool_list_selected) {
        toolListSelected = tool_list_selected;
    }

    /** @brief 设置当前活跃刀具（用于路径生成和 G 代码导出）*/
    void set_current_tool(const Tool &current_tool) { currentTool = current_tool; }

    /** @brief 返回当前活跃刀具 */
    [[nodiscard]] Tool current_tool() const { return currentTool; }

private:
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString saveToolLocation = appDataLocation + "/toolList.json"; ///< 本地刀具库文件路径

    Tool currentTool = Tool();          ///< 当前选中的活跃刀具
    QList<Tool> toolList;               ///< 完整刀具列表
    QList<Tool> toolListSelected;       ///< 用户选中待操作的刀具
    int curToolCount = 0;               ///< 当前刀具总数
    bool initialized = false;

    /**
     * @brief 检查某个唯一键是否已存在于 toolList
     * @param uniKey 唯一键
     * @return true 表示已存在（重复）
     */
    bool isUniqueTool(const QString &uniKey);
};

#endif //CAM_TOOLMANAGER_H
