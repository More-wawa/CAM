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

    init();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::init() {
    setWindowTitle("CAM");

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
    toolTable->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选择
    toolTable->setSelectionMode(QAbstractItemView::SingleSelection); // 单选
    toolTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止编辑
    toolTable->verticalHeader()->setVisible(false); // 隐藏行号
    toolTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    toolTable->setSortingEnabled(true); // 允许点击表头排序

    // 创建并初始化刀具管理模块
    toolManager = ToolManager::New();
    ResultType r = toolManager->init();
    if (r == ResultType::CreateFileError) {
        QMessageBox::warning(this, tr("错误"), tr("初始化刀具目录失败"));
    }

    QString message;
    // 尝试从本地加载刀具
    if (r = toolManager->loadToolFromLocal(&message); r == ResultType::Success) {
        ui->statusbar->showMessage(message, 3000);
    } else if (r == ResultType::JsonParseError) {
        QMessageBox::warning(this, tr("错误"), message);
    }
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
    if (ResultType r = toolManager->openTool(fileName, FileType::Json, &message); r != ResultType::Success) {
        if (r == ResultType::FileOpenError) {
            QMessageBox::warning(this, tr("错误"), message);
        } else if (r == ResultType::JsonParseError) {
            QMessageBox::warning(this, tr("错误"), message);
        }
    } else {
        // 将刀具保存至本地
        if (r = toolManager->saveToolToLocal(&message); r == ResultType::Success) {
            ui->statusbar->showMessage(message, 3000);
        } else if (r == ResultType::NoNewTools) {
            QMessageBox::warning(this, tr("警告"), message);
        } else if (r == ResultType::FileOpenError) {
            QMessageBox::warning(this, tr("错误"), message);
        }
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
    if (toolManager->cur_tool_count() == 0) {
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
