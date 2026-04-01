#include "../include/mainwindow.h"
#include "../ui/ui_MainWindow.h"
#include "../include/VTKManager.h"
#include "../include/ToolManager.h"
#include "../include/ToolpathPlanner.h"
#include "../include/Toolpath.h"
#include "../include/GCodeGenerator.h"
#include "../include/dialogaddtool.h"
#include <QVTKOpenGLNativeWidget.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QDockWidget>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QFont>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    connect(ui->tableWidgetToolTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::tableWidgetDoubleClicked);

    init();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::init() {
    setWindowTitle("CAM");
    setWindowIcon(QIcon("../image/ico.png"));

    // 全局样式表：统一字体、菜单栏、状态栏外观
    setStyleSheet(R"(
        QMainWindow { background-color: #f5f5f5; }
        QMenuBar { background-color: #2c2c2c; color: white; padding: 2px; }
        QMenuBar::item:selected { background-color: #444; }
        QMenu { background-color: #2c2c2c; color: white; }
        QMenu::item:selected { background-color: #555; }
        QStatusBar { background-color: #e0e0e0; color: #333; }
        QDockWidget::title { background: #3c3c3c; color: white; padding: 5px; font-weight: bold; }
        QGroupBox { font-weight: bold; border: 1px solid #ccc; border-radius: 6px; margin-top: 8px; padding-top: 6px; }
        QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }
        QTableWidget { border: 1px solid #ccc; border-radius: 4px; gridline-color: #e0e0e0; }
        QTableWidget::item:selected { background-color: #2196F3; color: white; }
        QHeaderView::section { background-color: #3c3c3c; color: white; padding: 4px; border: none; }
        QComboBox { padding: 4px; border: 1px solid #ccc; border-radius: 4px; }
        QDoubleSpinBox, QSpinBox { padding: 4px; border: 1px solid #ccc; border-radius: 4px; }
        QLineEdit { padding: 4px; border: 1px solid #ccc; border-radius: 4px; }
        QLabel { color: #333; }
        QMessageBox { background-color: white; }
        QMessageBox QLabel { color: #333; }
        QDialog { background-color: white; }
        QDialog QLabel { color: #333; }
    )");

    // 绑定控件
    toolWidget = ui->dockWidgetToolTable;
    toolWidget->setVisible(false);
    toolTable = ui->tableWidgetToolTable;
    toolTable->setVisible(true);
    toolTable->setHorizontalHeaderLabels(
        QStringList() << "编号" << "名称" << "类型" << "直径"
        << "刃长" << "刃数" << "材质" << "状态");
    // 表格样式设置
    toolTable->setAlternatingRowColors(true);                                // 隔行变色
    toolTable->setSelectionMode(QAbstractItemView::ExtendedSelection);       // 多行选择
    toolTable->setSelectionBehavior(QAbstractItemView::SelectRows);          // 整行选择
    toolTable->setEditTriggers(QAbstractItemView::NoEditTriggers);           // 禁止编辑
    toolTable->verticalHeader()->setVisible(false);                          // 隐藏行号
    toolTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    // toolTable->setSortingEnabled(true); // 允许点击表头排序

    // 创建并初始化刀具管理模块
    toolManager = ToolManager::New();
    if (const ResultType r = toolManager->init(); r == ResultType::CreateFileError) {
        QMessageBox::warning(this, tr("错误"), tr("初始化刀具目录失败"));
        return;
    }

    QString message;
    // 尝试从本地加载刀具
    toolManager->loadToolFromLocal(&message);
    ui->statusbar->showMessage(message, 3000);
    updateToolTable();

    // 创建并初始化 VTK 管理模块
    vtkManager = VTKManager::New();
    vtkManager->init();
    setCentralWidget(vtkManager->m_vtk_widget());

    // 创建刀具路径规划器
    toolpathPlanner = ToolpathPlanner::New();
    toolpathPlanner->setRenderer(vtkManager->getRenderer());

    // 连接异步信号
    connect(toolpathPlanner, &ToolpathPlanner::layerGenerated,
            this, &MainWindow::onToolpathLayerGenerated);
    connect(toolpathPlanner, &ToolpathPlanner::generationFinished,
            this, &MainWindow::onToolpathGenerationFinished);

    // 初始化加工操作 Dock
    initToolpathDock();
}

void MainWindow::on_actionOpenFile_triggered() {
    // 获取 STEP 文件路径
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "",
        tr("STEP 文件 (*.step *.stp);;所有文件 (*.*)")
    );

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
        file.close();
        return;
    }
    file.close();

    if (const ResultType r = vtkManager->openModelFile(fileName); r == ResultType::Success) {
        modelLoaded = true;
        // 将 OCCT shape 传给路径规划器，并清空旧路径
        toolpathPlanner->clearAll();
        toolpathPlanner->setShape(vtkManager->getShape());
        toolpathPlanner->setCurrentTool(toolManager->current_tool());
        updateToolpathList();
    } else if (r == ResultType::FileOpenError) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
    }

    // 标记模型已加载
    modelLoaded = true;
}

void MainWindow::on_actionViewTop_triggered() const {
    if (modelLoaded) vtkManager->setStandardView(0, 1, 0, 0, 0, 1);
}

void MainWindow::on_actionViewBottom_triggered() const {
    if (modelLoaded) vtkManager->setStandardView(0, -1, 0, 0, 0, 1);
}

void MainWindow::on_actionViewFront_triggered() const {
    if (modelLoaded) vtkManager->setStandardView(0, 0, 1, 0, 1, 0);
}

void MainWindow::on_actionViewBack_triggered() const {
    if (modelLoaded) vtkManager->setStandardView(0, 0, -1, 0, 1, 0);
}

void MainWindow::on_actionViewLeft_triggered() const {
    if (modelLoaded) vtkManager->setStandardView(-1, 0, 0, 0, 1, 0);
}

void MainWindow::on_actionViewRight_triggered() const {
    if (modelLoaded) vtkManager->setStandardView(1, 0, 0, 0, 1, 0);
}

void MainWindow::on_actionOpenTool_triggered() {
    const auto fileName = QFileDialog::getOpenFileName(
        this,
        tr("导入刀具库"),
        QString(),
        tr("支持的刀具文件 (*.json *.csv *.tsv);;JSON 文件 (*.json);;CSV/TSV 文件 (*.csv *.tsv);;所有文件 (*.*)")
    );
    if (fileName.isEmpty()) return; // 用户取消

    QString message;
    // 打开刀具文件
    if (const ResultType r = toolManager->openTool(fileName, FileType::Json, &message); r != ResultType::Success) {
        if (r == ResultType::FileOpenError) {
            QMessageBox::warning(this, tr("错误"), message);
        } else if (r == ResultType::JsonParseError) {
            QMessageBox::warning(this, tr("错误"), message);
        }
    } else {
        // 将刀具保存至本地
        saveToolToLocal();
        // 更新刀具列表
        updateToolTable();
    }
}

void MainWindow::updateToolTable() const {
    if (!toolTable) {
        qWarning() << "toolTable 未初始化";
        return;
    }

    // 清空旧内容，设置行数
    toolTable->clearContents();
    toolTable->setRowCount(toolManager->cur_tool_count());

    // 如果没有刀具，直接返回
    if (toolManager->tool_list().isEmpty()) {
        toolTable->setRowCount(0);
        return;
    }

    // 填充每一行
    for (int row = 0; row < toolManager->cur_tool_count(); ++row) {
        const Tool &t = toolManager->tool_list().at(row);
        int col = 0;

        // 编号
        auto *itemNo = new QTableWidgetItem(QString::number(row + 1));
        itemNo->setTextAlignment(Qt::AlignCenter);
        toolTable->setItem(row, col++, itemNo);

        // 名称
        toolTable->setItem(row, col++, new QTableWidgetItem(t.name));

        // 类型
        toolTable->setItem(row, col++, new QTableWidgetItem(t.type));

        // 直径
        auto *itemDia = new QTableWidgetItem(QString::number(t.diameter, 'f', 2) + " mm");
        itemDia->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        toolTable->setItem(row, col++, itemDia);

        // 刃长
        auto *itemFlute = new QTableWidgetItem(QString::number(t.fluteLength, 'f', 2) + " mm");
        itemFlute->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        toolTable->setItem(row, col++, itemFlute);

        // 刃数
        auto *itemFlutes = new QTableWidgetItem(QString::number(t.fluteCount));
        itemFlutes->setTextAlignment(Qt::AlignCenter);
        toolTable->setItem(row, col++, itemFlutes);

        // 材质
        toolTable->setItem(row, col++, new QTableWidgetItem(t.material));
    }

    // 优化显示
    toolTable->resizeColumnsToContents(); // 自动调整列宽
    toolTable->horizontalHeader()->setStretchLastSection(true); // 最后一列拉伸填充
}

void MainWindow::on_actionViewTool_triggered() const {
    toolWidget->setVisible(true);
}

void MainWindow::on_pushButtonDeleteTools_clicked() {
    // 确认删除确认框
    if (QMessageBox::question(this, "确认删除", "确定要删除选中的刀具吗？",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) // 默认焦点在“否”
        == QMessageBox::No) {
        return;
    }

    ResultType r;
    if (r = getSelectedToolList(); r == ResultType::NoToolSelected) {
        return;
    }

    QString message;
    if (r = toolManager->deleteToolSelected(&message); r == ResultType::Success) {
        ui->labelCurrentTool->setText(message);
        // 更新本地文件
        saveToolToLocal();
        // 更新刀具列表
        updateToolTable();
    } else if (r == ResultType::ToolListEmpty) {
        QMessageBox::warning(this, "警告", message);
    }
}

void MainWindow::saveToolToLocal() {
    QString message;
    if (const ResultType r = toolManager->saveToolToLocal(&message); r == ResultType::Success) {
        ui->statusbar->showMessage(message, 3000);
    } else if (r == ResultType::NoChangeTool) {
        ui->statusbar->showMessage(message, 3000);
    } else if (r == ResultType::FileOpenError) {
        QMessageBox::warning(this, tr("错误"), message);
    } else if (r == ResultType::ToolListEmpty) {
        ui->statusbar->showMessage(message, 3000);
    }
}

Tool MainWindow::getTool(const int row) const {
    Tool tool;

    // 列 1 - 名称
    const QTableWidgetItem *item = ui->tableWidgetToolTable->item(row, 1);
    tool.name = item ? item->text().trimmed() : "";

    // 列 2 - 类型
    item = ui->tableWidgetToolTable->item(row, 2);
    tool.type = item ? item->text().trimmed() : "";

    // 列 3 - 直径
    item = ui->tableWidgetToolTable->item(row, 3);
    if (item) {
        bool ok;
        const double val = item->text().toDouble(&ok);
        tool.diameter = ok ? val : 0.0;
    } else {
        tool.diameter = 0.0;
    }

    // 列 4 - 刃长
    item = ui->tableWidgetToolTable->item(row, 4);
    if (item) {
        bool ok;
        const double val = item->text().toDouble(&ok);
        tool.fluteLength = ok ? val : 0.0;
    } else {
        tool.fluteLength = 0.0;
    }

    // 列 5 - 刃数
    item = ui->tableWidgetToolTable->item(row, 5);
    if (item) {
        bool ok;
        const int val = item->text().toInt(&ok);
        tool.fluteCount = ok ? val : 2; // 默认 2 刃
    } else {
        tool.fluteCount = 2;
    }

    // 列 6 - 材质
    item = ui->tableWidgetToolTable->item(row, 6);
    tool.material = item ? item->text().trimmed() : "";

    // uniqueKey
    tool.uniqueKey = ToolManager::getUniqueKey(tool);

    return tool;
}

ResultType MainWindow::getSelectedToolList() {
    QList<QTableWidgetItem *> selectedItems = ui->tableWidgetToolTable->selectedItems();

    // 判断是否存在有效选中
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选中刀具");
        return ResultType::NoToolSelected;
    }

    // 获取所有选中的唯一行号
    QSet<int> selectedRows;
    for (const QTableWidgetItem *item: selectedItems) {
        if (item) {
            selectedRows.insert(item->row());
        }
    }

    QList<Tool> toolListSelected;
    for (const int row: selectedRows) {
        toolListSelected.append(getTool(row));
    }

    toolManager->set_tool_list_selected(toolListSelected);

    return ResultType::Success;
}

void MainWindow::on_pushButtonSelectTool_clicked() {
    QList<QTableWidgetItem *> selectedItems = ui->tableWidgetToolTable->selectedItems();

    // 判断是否存在有效选中
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选中刀具");
        return;
    }

    // 获取所有选中的唯一行号
    QSet<int> selectedRows;
    for (const QTableWidgetItem *item: selectedItems) {
        if (item) {
            selectedRows.insert(item->row());
        }
    }
    if (selectedRows.size() > 1) {
        QMessageBox::warning(this, "警告", "只能选择一把刀具");
        return;
    }

    // 获取选中的唯一行号
    const int selectedRow = selectedItems[0]->row();
    const Tool tool = getTool(selectedRow);

    toolManager->set_current_tool(tool);

    ui->labelCurrentTool->setText(QString("当前选中 %1 号刀具").arg(selectedRow + 1));
}

void MainWindow::on_pushButtonAddTool_clicked() {
    DialogAddTool dialog_add_tool(this);

    if (dialog_add_tool.exec() == QDialog::Accepted) {
        QString m_toolName = dialog_add_tool.get_tool_name();
        double m_diameter = dialog_add_tool.get_diameter();
        double m_fluteLength = dialog_add_tool.get_flute_length();
        double m_totalLength = dialog_add_tool.get_total_length();
        double m_cornerRadius = dialog_add_tool.get_corner_radius();
        int m_fluteCount = dialog_add_tool.get_flute_count();
        QString m_material = dialog_add_tool.get_material();
        QString m_type = dialog_add_tool.get_type();

        QString message;
        if (ResultType r = toolManager->addTool(
            m_toolName,
            m_diameter,
            m_fluteLength,
            m_fluteCount,
            m_totalLength,
            m_cornerRadius,
            m_material,
            m_type,
            &message
        ); r == ResultType::Success) {
            ui->labelCurrentTool->setText(QString("成功添加 %1 刀具").arg(m_toolName));
        }

        // 更新本地文件
        saveToolToLocal();
        // 更新刀具列表
        updateToolTable();
    } else {
        qDebug() << "用户取消了操作";
    }
}

void MainWindow::tableWidgetDoubleClicked(const int row, const int column) const {
    const Tool tool = getTool(row);

    toolManager->set_current_tool(tool);

    ui->labelCurrentTool->setText(QString("当前选中 %1 号刀具").arg(row + 1));
}

void MainWindow::initToolpathDock()
{
    toolpathDock = new QDockWidget(tr("加工操作"), this);
    toolpathDock->setVisible(false);

    auto* dockContent = new QWidget();
    auto* mainLayout = new QVBoxLayout(dockContent);

    // 加工类型选择
    mainLayout->addWidget(new QLabel(tr("加工类型")));
    comboBoxMachType = new QComboBox();
    comboBoxMachType->addItem(tr("面铣 (Planar Face Milling)"));
    comboBoxMachType->addItem(tr("轮廓铣 (Contour Milling)"));
    mainLayout->addWidget(comboBoxMachType);

    // 切削参数组
    auto* paramGroup = new QGroupBox(tr("切削参数"));
    auto* formLayout = new QFormLayout();

    spinBoxDepth = new QDoubleSpinBox();
    spinBoxDepth->setRange(0.1, 50.0);
    spinBoxDepth->setSingleStep(0.1);
    spinBoxDepth->setValue(1.0);
    spinBoxDepth->setSuffix(" mm");
    formLayout->addRow(tr("切深 (mm)"), spinBoxDepth);

    spinBoxStepover = new QDoubleSpinBox();
    spinBoxStepover->setRange(0.1, 50.0);
    spinBoxStepover->setSingleStep(0.1);
    spinBoxStepover->setValue(0.5);
    spinBoxStepover->setSuffix(" mm");
    formLayout->addRow(tr("行距 (mm)"), spinBoxStepover);

    spinBoxFeedRate = new QDoubleSpinBox();
    spinBoxFeedRate->setRange(1.0, 10000.0);
    spinBoxFeedRate->setSingleStep(10.0);
    spinBoxFeedRate->setValue(500.0);
    spinBoxFeedRate->setSuffix(" mm/min");
    formLayout->addRow(tr("进给率 (mm/min)"), spinBoxFeedRate);

    spinBoxSpindle = new QSpinBox();
    spinBoxSpindle->setRange(100, 30000);
    spinBoxSpindle->setSingleStep(100);
    spinBoxSpindle->setValue(8000);
    spinBoxSpindle->setSuffix(" RPM");
    formLayout->addRow(tr("主轴转速 (RPM)"), spinBoxSpindle);

    paramGroup->setLayout(formLayout);
    mainLayout->addWidget(paramGroup);

    // 生成按钮（改名避免 Qt 自动连接警告）
    btnGeneratePath = new QPushButton(tr("生成刀路"));
    btnGeneratePath->setObjectName("btnGenPath");
    btnGeneratePath->setMinimumHeight(36);
    btnGeneratePath->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border-radius: 4px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3d8b40; }"
        "QPushButton:disabled { background-color: #cccccc; color: #666666; }"
    );
    connect(btnGeneratePath, &QPushButton::clicked, this, &MainWindow::generateToolpathClicked);
    mainLayout->addWidget(btnGeneratePath);

    // 进度显示
    labelProgress = new QLabel(tr("就绪"));
    labelProgress->setStyleSheet("color: #555; font-style: italic;");
    mainLayout->addWidget(labelProgress);
    progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(false);
    progressBar->setTextVisible(true);
    progressBar->setStyleSheet(
        "QProgressBar { border: 1px solid #ccc; border-radius: 4px; height: 14px; text-align: center; }"
        "QProgressBar::chunk { background-color: #4CAF50; border-radius: 3px; }"
    );
    mainLayout->addWidget(progressBar);

    // 刀路列表
    mainLayout->addWidget(new QLabel(tr("已生成刀路")));
    listWidgetPaths = new QListWidget();
    listWidgetPaths->setAlternatingRowColors(true);
    listWidgetPaths->setStyleSheet(
        "QListWidget { border: 1px solid #ccc; border-radius: 4px; }"
        "QListWidget::item:selected { background-color: #2196F3; color: white; }"
    );
    mainLayout->addWidget(listWidgetPaths);

    // 刀路控制按钮（水平排列）
    auto* pathBtnLayout = new QHBoxLayout();
    btnToggleVisible = new QPushButton(tr("显示/隐藏"));
    btnToggleVisible->setObjectName("btnToggleVis");
    btnToggleVisible->setMinimumHeight(30);
    btnToggleVisible->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #F57C00; }"
        "QPushButton:pressed { background-color: #E65100; }"
    );
    connect(btnToggleVisible, &QPushButton::clicked, this, &MainWindow::toggleToolpathVisibleClicked);
    pathBtnLayout->addWidget(btnToggleVisible);

    btnClearPaths = new QPushButton(tr("清空刀路"));
    btnClearPaths->setObjectName("btnClearAll");
    btnClearPaths->setMinimumHeight(30);
    btnClearPaths->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #da190b; }"
        "QPushButton:pressed { background-color: #a01207; }"
    );
    connect(btnClearPaths, &QPushButton::clicked, this, &MainWindow::clearToolpathsClicked);
    pathBtnLayout->addWidget(btnClearPaths);
    mainLayout->addLayout(pathBtnLayout);

    // G代码导出区
    mainLayout->addWidget(new QLabel(tr("G代码导出")));

    btnExportGCode = new QPushButton(tr("导出G代码"));
    btnExportGCode->setObjectName("btnExportGCode");
    btnExportGCode->setMinimumHeight(36);
    btnExportGCode->setStyleSheet(
        "QPushButton { background-color: #9C27B0; color: white; border-radius: 4px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #7B1FA2; }"
        "QPushButton:pressed { background-color: #6A1B9A; }"
    );
    connect(btnExportGCode, &QPushButton::clicked, this, &MainWindow::exportGCodeClicked);
    mainLayout->addWidget(btnExportGCode);

    gCodePreview = new QTextEdit();
    gCodePreview->setReadOnly(true);
    gCodePreview->setMaximumHeight(160);
    gCodePreview->setFont(QFont("Courier New", 8));
    gCodePreview->setPlaceholderText(tr("G代码预览将显示在此处..."));
    gCodePreview->setStyleSheet(
        "QTextEdit { border: 1px solid #ccc; border-radius: 4px; background-color: #f9f9f9; }"
    );
    mainLayout->addWidget(gCodePreview);

    mainLayout->addStretch();

    toolpathDock->setWidget(dockContent);
    addDockWidget(Qt::RightDockWidgetArea, toolpathDock);

    // 在菜单中添加动作
    QAction* actionViewToolpath = new QAction(tr("加工操作"), this);
    connect(actionViewToolpath, &QAction::triggered, [this]() { toolpathDock->setVisible(true); });
    ui->menuView->addAction(actionViewToolpath);
}

void MainWindow::updateToolpathList() const
{
    if (!listWidgetPaths) return;
    listWidgetPaths->clear();
    for (int i = 0; i < toolpathPlanner->count(); ++i)
    {
        const Toolpath& p = toolpathPlanner->toolpaths().at(i);
        listWidgetPaths->addItem(QString("%1 — %2 段").arg(p.name).arg(p.segments.size()));
    }
}

void MainWindow::generateToolpathClicked()
{
    if (!modelLoaded)
    {
        QMessageBox::warning(this, tr("警告"), tr("请先加载 STEP 模型"));
        return;
    }

    if (toolpathPlanner->isGenerating())
    {
        QMessageBox::information(this, tr("提示"), tr("刀路正在生成中，请稍候"));
        return;
    }

    toolpathPlanner->setCurrentTool(toolManager->current_tool());

    double depth    = spinBoxDepth->value();
    double stepover = spinBoxStepover->value();
    double feed     = spinBoxFeedRate->value();
    int    spindle  = spinBoxSpindle->value();

    int idx = comboBoxMachType->currentIndex();

    // 显示进度条
    progressBar->setValue(0);
    progressBar->setVisible(true);
    labelProgress->setText(tr("正在生成刀路..."));
    btnGeneratePath->setEnabled(false);

    if (idx == 0)
        toolpathPlanner->generateFaceMillingAsync(depth, stepover, feed, spindle);
    else
        toolpathPlanner->generateContourMillingAsync(depth, stepover, feed, spindle);
}

void MainWindow::onToolpathLayerGenerated(int current, int total)
{
    int percent = (current * 100) / total;
    progressBar->setValue(percent);
    labelProgress->setText(QString("生成中：第 %1/%2 层").arg(current).arg(total));

    // 不触发渲染，避免主线程阻塞
    // vtkManager->renderUpdate();
}

void MainWindow::onToolpathGenerationFinished(ResultType result, QString message)
{
    progressBar->setVisible(false);
    btnGeneratePath->setEnabled(true);

    if (result == ResultType::Success)
    {
        labelProgress->setText(tr("生成完成"));
        updateToolpathList();
        // 只在完成时渲染一次
        vtkManager->renderUpdate();
    }
    else
    {
        labelProgress->setText(tr("生成失败"));
    }

    ui->statusbar->showMessage(message, 5000);
}

void MainWindow::toggleToolpathVisibleClicked()
{
    int idx = listWidgetPaths->currentRow();
    if (idx < 0)
    {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一条刀路"));
        return;
    }

    bool curVisible = toolpathPlanner->toolpaths().at(idx).visible;
    toolpathPlanner->setToolpathVisible(idx, !curVisible);
    vtkManager->renderUpdate();
}

void MainWindow::clearToolpathsClicked()
{
    toolpathPlanner->clearAll();
    updateToolpathList();
    labelProgress->setText(tr("就绪"));
    vtkManager->renderUpdate();
}

void MainWindow::exportGCodeClicked()
{
    const int idx = listWidgetPaths->currentRow();
    if (idx < 0)
    {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一条刀路"));
        return;
    }

    const Toolpath& path = toolpathPlanner->toolpaths().at(idx);
    const Tool tool = toolManager->current_tool();

    if (tool.name.isEmpty())
    {
        QMessageBox::warning(this, tr("警告"), tr("请先选择刀具"));
        return;
    }

    GCodeGenerator* gen = GCodeGenerator::New();
    QString message;

    if (gen->generate(path, tool, 1, &message) != ResultType::Success)
    {
        QMessageBox::warning(this, tr("G代码生成失败"), message);
        delete gen;
        return;
    }

    gCodePreview->setPlainText(gen->preview());

    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("导出G代码"),
        QString(),
        tr("SINUMERIK MPF (*.mpf);;NC 文件 (*.nc)")
    );

    if (!filePath.isEmpty())
    {
        if (gen->exportToFile(filePath, &message) == ResultType::Success)
            ui->statusbar->showMessage(message, 5000);
        else
            QMessageBox::warning(this, tr("导出失败"), message);
    }

    delete gen;
}
