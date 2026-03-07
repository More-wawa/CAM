//
// Created by More on 2026/3/5.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "../include/mainwindow.h"
#include "ui_MainWindow.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCompositePolyDataMapper.h>
#include <QFileDialog>
#include <vtkOCCTReader.h>
#include <vtkCamera.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
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

    initVTK();
}

void MainWindow::initVTK() {
    if (vtkInitialized) return;

    // 创建 VTK Widget
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    setCentralWidget(m_vtkWidget);

    // 创建渲染器
    m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_vtkRenderer->SetBackground(0.1, 0.2, 0.3); // 深蓝背景

    // 启用渐变背景
    m_vtkRenderer->GradientBackgroundOn();
    m_vtkRenderer->SetBackground(1, 1, 1);
    m_vtkRenderer->SetBackground2(0.5, 0.5, 0.8);

    // 把渲染器加到窗口
    m_vtkWidget->renderWindow()->AddRenderer(m_vtkRenderer);

    // 交互器风格
    m_vtkStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_vtkWidget->renderWindow()->GetInteractor()->SetInteractorStyle(m_vtkStyle);

    // Mapper
    m_vtkMapper = vtkSmartPointer<vtkCompositePolyDataMapper>::New();

    // 获取摄像机
    m_vtkCamera = m_vtkRenderer->GetActiveCamera();

    // 开启参考线
    m_vtkAxesActor = vtkSmartPointer<vtkAxesActor>::New();
    m_vtkAxesActor->SetShaftTypeToCylinder();
    m_vtkAxesActor->SetXAxisLabelText("X");
    m_vtkAxesActor->SetYAxisLabelText("Y");
    m_vtkAxesActor->SetZAxisLabelText("Z");

    // 创建 widget 并关联
    m_vtkOrientationMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    m_vtkOrientationMarkerWidget->SetOrientationMarker(m_vtkAxesActor);
    m_vtkOrientationMarkerWidget->SetInteractor(m_vtkWidget->renderWindow()->GetInteractor());

    // 开启参考线并放置屏幕左下角
    m_vtkOrientationMarkerWidget->SetEnabled(1);
    m_vtkOrientationMarkerWidget->InteractiveOn();

    // 确认已经初始化
    vtkInitialized = true;
}

void MainWindow::on_actionOpenFile_triggered() {
    // 确保初始化完成
    if (!vtkInitialized) initVTK();

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

    qDebug() << "文件打开成功";

    // 打开新 STEP 文件之前先清空缓存
    m_vtkRenderer->RemoveAllViewProps();

    // 创建 OCCTReader 读取 STEP 文件
    m_vtkOCCTReader = vtkSmartPointer<vtkOCCTReader>::New();
    m_vtkOCCTReader->SetFileName(fileName.toUtf8().constData());
    m_vtkOCCTReader->SetFileFormat(vtkOCCTReader::STEP);

    // 更新 pipeline
    m_vtkMapper->SetInputConnection(m_vtkOCCTReader->GetOutputPort());

    // 检查输出是否有效
    if (!m_vtkOCCTReader->GetOutput()) {
        qDebug() << "STEP 文件读取失败";
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
        return;
    }
    qDebug() << "STEP 文件读取成功";

    // 创建 Actor 并添加至渲染器
    m_vtkActor = vtkSmartPointer<vtkActor>::New();
    m_vtkActor->SetMapper(m_vtkMapper);
    m_vtkRenderer->AddActor(m_vtkActor);

    // 重置相机并渲染
    m_vtkRenderer->ResetCamera();
    m_vtkWidget->renderWindow()->Render();

    // 标记模型已加载
    modelLoaded = true;
}

void MainWindow::setStandardView(const double dx, const double dy, const double dz, const double ux, const double uy,
                                 const double uz) const {
    if (!modelLoaded || !m_vtkCamera || !m_vtkRenderer) {
        qDebug() << "model is still non-loaded";
        return;
    }

    // 获得当前模型最合适的距离和焦点
    m_vtkRenderer->ResetCamera();
    const double *focal = m_vtkCamera->GetFocalPoint();
    const double dist = m_vtkCamera->GetDistance() * 1.2;

    // 设置新视角
    m_vtkCamera->SetPosition(
        focal[0] + dx * dist,
        focal[1] + dy * dist,
        focal[2] + dz * dist
    );
    m_vtkCamera->SetFocalPoint(focal);
    m_vtkCamera->SetViewUp(ux, uy, uz);

    // 消除警告
    m_vtkCamera->OrthogonalizeViewUp();

    m_vtkRenderer->ResetCameraClippingRange();
    m_vtkWidget->renderWindow()->Render();
}

void MainWindow::on_actionViewTop_triggered() const {
    setStandardView(0, 1, 0, 0, 0, 1);
}

void MainWindow::on_actionViewBottom_triggered() const {
    setStandardView(0, -1, 0, 0, 0, 1);
}

void MainWindow::on_actionViewFront_triggered() const {
    setStandardView(0, 0, 1, 0, 1, 0);
}

void MainWindow::on_actionViewBack_triggered() const {
    setStandardView(0, 0, -1, 0, 1, 0);
}

void MainWindow::on_actionViewLeft_triggered() const {
    setStandardView(-1, 0, 0, 0, 1, 0);
}

void MainWindow::on_actionViewRight_triggered() const {
    setStandardView(1, 0, 0, 0, 1, 0);
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
            t.fileType = FileType::JSON;
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
        if (tool.fileType == FileType::JSON) {
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
            t.fileType = FileType::JSON;
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

        // 当前使用的刀具标绿
        auto *statusItem = new QTableWidgetItem();
        if (&t == currentTool) {
            // 如果这是当前选中的刀具
            statusItem->setText("当前使用");
            statusItem->setForeground(Qt::darkGreen);
            statusItem->setFont(QFont("Microsoft YaHei", 9, QFont::Bold));
        } else {
            statusItem->setText("");
        }
        statusItem->setTextAlignment(Qt::AlignCenter);
        toolTable->setItem(row, col++, statusItem);
    }

    // 优化显示
    toolTable->resizeColumnsToContents(); // 自动调整列宽
    toolTable->horizontalHeader()->setStretchLastSection(true); // 最后一列拉伸填充
}

void MainWindow::on_actionViewTool_triggered() const {
    toolWidget->setVisible(true);
}
