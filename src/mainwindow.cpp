#include "../include/mainwindow.h"
#include "../ui/ui_MainWindow.h"
#include "../include/VTKManager.h"
#include "../include/ToolManager.h"
#include <QVTKOpenGLNativeWidget.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QDockWidget>
#include <QStandardPaths>

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

    // 初始化刀具保存目录
    if (const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); !QDir().
        mkpath(appDataDir)) {
        QMessageBox::warning(this, tr("错误"), QString("初始化刀具目录失败：%1").arg(appDataDir));
    }

    // 加载刀具
    toolSavePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + toolSavePath;
    loadToolTable();
    toolCount = toolList.size();

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
        return;
    }
    file.close();

    if (vtkManager->openModelFile(fileName) == ErrorType::ModulError) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
    }

    // 标记模型已加载
    modelLoaded = true;
}

void MainWindow::on_actionViewTop_triggered() const {
    vtkManager->setStandardView(0, 1, 0, 0, 0, 1);
}

void MainWindow::on_actionViewBottom_triggered() const {
    vtkManager->setStandardView(0, -1, 0, 0, 0, 1);
}

void MainWindow::on_actionViewFront_triggered() const {
    vtkManager->setStandardView(0, 0, 1, 0, 1, 0);
}

void MainWindow::on_actionViewBack_triggered() const {
    vtkManager->setStandardView(0, 0, -1, 0, 1, 0);
}

void MainWindow::on_actionViewLeft_triggered() const {
    vtkManager->setStandardView(-1, 0, 0, 0, 1, 0);
}

void MainWindow::on_actionViewRight_triggered() const {
    vtkManager->setStandardView(1, 0, 0, 0, 1, 0);
}

void MainWindow::on_actionImportTool_triggered() {
    const auto fileName = QFileDialog::getOpenFileName(
        this,
        tr("导入刀具库"),
        QString(),
        tr("支持的刀具文件 (*.json *.csv *.tsv);;JSON 文件 (*.json);;CSV/TSV 文件 (*.csv *.tsv);;所有文件 (*.*)")
    );

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
        return;
    }

    const auto ext = QFileInfo(fileName).suffix().toLower();
    const QByteArray data = file.readAll();
    file.close();

    if (ext == "json") {
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray array = doc.array();
        if (doc.isNull() || !doc.isArray()) {
            QMessageBox::warning(this, tr("错误"), tr("JSON 格式无效"));
            return;
        }
        for (const auto &val: array) {
            if (!val.isObject()) continue;
            const QJsonObject obj = val.toObject();

            Tool t;
            t.fileType = FileType::Json;
            t.name = obj["name"].toString();
            t.diameter = obj["diameter"].toDouble();
            t.fluteLength = obj["fluteLength"].toDouble();
            t.totalLength = obj["totalLength"].toDouble();
            t.cornerRadius = obj["cornerRadius"].toDouble(0.0);
            t.fluteCount = obj["fluteCount"].toInt(2);
            t.material = obj["material"].toString();
            t.type = obj["type"].toString();
            t.uniqueKey = getUniqueKey(t);

            qDebug() << t.uniqueKey;
            if (isUniqueTool(t.uniqueKey)) toolList.append(t);

            saveToolTable();
        }
    }

    updateToolTable();
    QMessageBox::information(this, tr("成功"), tr("已导入 %1 把刀具").arg(toolList.size() - toolCount));
    toolCount = toolList.size();
}

void MainWindow::saveToolTable() {
    if (toolList.isEmpty()) {
        return;
    }

    for (const auto &tool: toolList) {
        if (tool.fileType == FileType::Json) {
            QJsonArray toolArray;

            for (const auto &t: toolList) {
                QJsonObject toolJSON;
                toolJSON["name"] = t.name;
                toolJSON["diameter"] = t.diameter;
                toolJSON["fluteLength"] = t.fluteLength;
                toolJSON["totalLength"] = t.totalLength;
                toolJSON["cornerRadius"] = t.cornerRadius;
                toolJSON["fluteCount"] = t.fluteCount;
                toolJSON["material"] = t.material;
                toolJSON["type"] = t.type;
                toolJSON["uniqueKey"] = t.uniqueKey;

                toolArray.append(toolJSON);
            }

            QJsonObject root;
            root["toolList"] = toolArray;
            root["version"] = "1.0";
            root["lastSaved"] = QDateTime::currentDateTime().toString(Qt::ISODate);

            const QJsonDocument doc(root);

            QFile file(toolSavePath + ".json");
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "保存失败", "无法写入文件：" + file.fileName());
                return;
            }
            file.write(doc.toJson(QJsonDocument::Indented));
            qDebug() << "File Type JSON, save to " << file.fileName();
            file.close();
        }
    }

    if (const auto c = toolList.size() - toolCount; c > 0) {
        ui->statusbar->showMessage(QString("%1副刀具已保存至本地").arg(toolList.size() - toolCount), 3000);
    }
}

void MainWindow::loadToolTable() {
    // JSON
    if (QFile file(toolSavePath + ".json"); file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray data = file.readAll();
        file.close();

        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            return;
        }

        QJsonObject root = doc.object();

        for (const auto array = root["toolList"].toArray(); const auto &val: array) {
            if (!val.isObject()) continue;
            QJsonObject obj = val.toObject();

            Tool t;
            t.fileType = FileType::Json;
            t.name = obj["name"].toString();
            t.diameter = obj["diameter"].toDouble();
            t.fluteLength = obj["fluteLength"].toDouble();
            t.totalLength = obj["totalLength"].toDouble();
            t.cornerRadius = obj["cornerRadius"].toDouble(0.0);
            t.fluteCount = obj["fluteCount"].toInt(2);
            t.material = obj["material"].toString();
            t.type = obj["type"].toString();
            t.uniqueKey = obj["uniqueKey"].toString();

            toolList.append(t);
        }
    }

    updateToolTable();

    if (!toolList.isEmpty()) {
        ui->statusbar->showMessage(QString("已从本地成功加载%1副刀具").arg(toolList.size()), 3000);
    }
}

QString MainWindow::getUniqueKey(const Tool &t) {
    return QString("%1|%2|%3").arg(t.name).arg(t.cornerRadius).arg(t.diameter);
}

bool MainWindow::isUniqueTool(const QString &uniKey) {
    if (uniKey.isEmpty()) return false;
    foreach(const auto &t, toolList) {
        if (uniKey == t.uniqueKey) {
            return false;
        }
    }
    return true;
}

void MainWindow::updateToolTable() const {
    if (!toolTable) {
        qWarning() << "toolTable 未初始化";
        return;
    }

    // 清空旧内容，设置行数
    toolTable->clearContents();
    toolTable->setRowCount(toolList.size());

    // 如果没有刀具，直接返回
    if (toolList.isEmpty()) {
        toolTable->setRowCount(0);
        return;
    }

    // 填充每一行
    for (int row = 0; row < toolList.size(); ++row) {
        const Tool &t = toolList.at(row);
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
