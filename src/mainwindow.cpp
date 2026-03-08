#include "../include/mainwindow.h"
#include "../ui/ui_MainWindow.h"
#include "../include/VTKManager.h"
#include "../include/ToolManager.h"
#include <QVTKOpenGLNativeWidget.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QDockWidget>

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

    // 绑定控件
    toolWidget = ui->dockWidgetToolTable;
    toolWidget->setVisible(false);
    toolTable = ui->tableWidgetToolTable;
    toolTable->setVisible(true);
    toolTable->setHorizontalHeaderLabels(
        QStringList() << "编号" << "名称" << "类型" << "直径"
        << "刃长" << "刃数" << "材质" << "状态");
    // 表格样式设置
    toolTable->setAlternatingRowColors(true); // 隔行变色
    toolTable->setSelectionMode(QAbstractItemView::ExtendedSelection); // 多行选择
    toolTable->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选择
    toolTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止编辑
    toolTable->verticalHeader()->setVisible(false); // 隐藏行号
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
        QMessageBox::warning(this, "删除成功", message);
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

void MainWindow::tableWidgetDoubleClicked(const int row, const int column) const {
    const Tool tool = getTool(row);

    toolManager->set_current_tool(tool);

    ui->labelCurrentTool->setText(QString("当前选中 %1 号刀具").arg(row + 1));
}
