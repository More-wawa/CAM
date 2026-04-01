/**
 * @file mainwindow.h
 * @brief CAM 系统主窗口
 * @details 集成 VTK 三维显示、刀具库管理、刀具路径规划和 G 代码导出功能
 */

#pragma once

#ifndef CAM_MAINWINDOW_H
#define CAM_MAINWINDOW_H

#include <QMainWindow>
#include <vtkSmartPointer.h>
#include "EnumType.h"
#include "Tool.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class VTKManager;
class ToolManager;
class ToolpathPlanner;
class QDockWidget;
class QTableWidget;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QPushButton;
class QListWidget;
class QProgressBar;
class QLabel;
class QTextEdit;
class DialogAddTool;

/**
 * @class MainWindow
 * @brief CAM 系统主窗口
 * @details 包含以下功能模块：
 *          - 菜单栏：文件打开、视图切换、刀具库管理
 *          - 中央区域：VTK 三维显示窗口
 *          - 刀具停靠面板：刀具表格、增删改查
 *          - 加工停靠面板：路径规划参数、路径列表、G 代码导出
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    bool modelLoaded = false;                   ///< 是否已加载模型
    QDockWidget *toolWidget = nullptr;          ///< 刀具库停靠面板
    QTableWidget *toolTable = nullptr;          ///< 刀具表格

    VTKManager *vtkManager{};                   ///< VTK 可视化管理器
    ToolManager *toolManager{};                 ///< 刀具库管理器
    ToolpathPlanner *toolpathPlanner = nullptr; ///< 刀具路径规划器

    // ---- 加工操作 Dock 控件 ----
    QDockWidget*    toolpathDock      = nullptr;  ///< 加工操作停靠面板
    QComboBox*      comboBoxMachType  = nullptr;  ///< 加工类型下拉框（面铣/轮廓铣）
    QDoubleSpinBox* spinBoxDepth      = nullptr;  ///< 每层切削深度 (mm)
    QDoubleSpinBox* spinBoxStepover   = nullptr;  ///< 行距/步距 (mm)
    QDoubleSpinBox* spinBoxFeedRate   = nullptr;  ///< 进给速度 (mm/min)
    QSpinBox*       spinBoxSpindle    = nullptr;  ///< 主轴转速 (RPM)
    QPushButton*    btnGeneratePath   = nullptr;  ///< 生成刀路按钮
    QListWidget*    listWidgetPaths   = nullptr;  ///< 刀路列表
    QPushButton*    btnToggleVisible  = nullptr;  ///< 显示/隐藏刀路按钮
    QPushButton*    btnClearPaths     = nullptr;  ///< 清空刀路按钮
    QProgressBar*   progressBar       = nullptr;  ///< 路径生成进度条
    QLabel*         labelProgress     = nullptr;  ///< 进度文本标签
    QPushButton*    btnExportGCode    = nullptr;  ///< 导出 G 代码按钮
    QTextEdit*      gCodePreview      = nullptr;  ///< G 代码预览框

    /** @brief 初始化 VTK、刀具库、路径规划器，创建停靠面板 */
    void init();

    /** @brief 刷新刀具表格显示 */
    void updateToolTable() const;

    /** @brief 保存刀具库到本地 JSON 文件 */
    void saveToolToLocal();

    /**
     * @brief 获取表格中选中的刀具列表
     * @return ResultType::Success 或 ResultType::NoToolSelected
     */
    ResultType getSelectedToolList();

    /**
     * @brief 从表格指定行提取刀具数据
     * @param row 行号
     * @return Tool 结构体
     */
    Tool getTool(int row) const;

    /** @brief 刷新刀路列表显示 */
    void updateToolpathList() const;

    /** @brief 创建加工操作停靠面板及其所有控件 */
    void initToolpathDock();

private slots:
    // ---- 菜单栏动作 ----
    void on_actionOpenFile_triggered();         ///< 打开 STEP 模型
    void on_actionViewTop_triggered() const;    ///< 切换到俯视图
    void on_actionViewBottom_triggered() const; ///< 切换到仰视图
    void on_actionViewFront_triggered() const;  ///< 切换到正视图
    void on_actionViewBack_triggered() const;   ///< 切换到后视图
    void on_actionViewLeft_triggered() const;   ///< 切换到左视图
    void on_actionViewRight_triggered() const;  ///< 切换到右视图
    void on_actionViewTool_triggered() const;   ///< 显示/隐藏刀具库面板
    void on_actionOpenTool_triggered();         ///< 导入刀具库 JSON 文件

    // ---- 刀具库操作 ----
    void on_pushButtonDeleteTools_clicked();    ///< 删除选中刀具
    void on_pushButtonSelectTool_clicked();     ///< 设置当前活跃刀具
    void on_pushButtonAddTool_clicked();        ///< 打开添加刀具对话框
    void tableWidgetDoubleClicked(int row, int column) const; ///< 双击表格行选中刀具

    // ---- 刀具路径操作 ----
    void generateToolpathClicked();             ///< 生成刀具路径
    void toggleToolpathVisibleClicked();        ///< 切换选中刀路的可见性
    void clearToolpathsClicked();               ///< 清空所有刀路

    // ---- 异步生成进度回调 ----
    void onToolpathLayerGenerated(int current, int total);          ///< 更新进度条
    void onToolpathGenerationFinished(ResultType result, QString message); ///< 完成回调

    // ---- G 代码导出 ----
    void exportGCodeClicked();                  ///< 导出选中刀路为 G 代码
};

#endif //CAM_MAINWINDOW_H
