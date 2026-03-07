#include "../include/ToolManager.h"
// #include <QFile>
//
// int ToolManager::openTool(QString fileName) {
//
//     QFile file(fileName);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
//         return;
//     }
//
//     const auto ext = QFileInfo(fileName).suffix().toLower();
//     const QByteArray data = file.readAll();
//     file.close();
//
//     if (ext == "json") {
//         const QJsonDocument doc = QJsonDocument::fromJson(data);
//         const QJsonArray array = doc.array();
//         if (doc.isNull() || !doc.isArray()) {
//             QMessageBox::warning(this, tr("错误"), tr("JSON 格式无效"));
//             return;
//         }
//         for (const auto &val: array) {
//             if (!val.isObject()) continue;
//             const QJsonObject obj = val.toObject();
//
//             Tool t;
//             t.fileType = MyFileType::JSON;
//             t.name = obj["name"].toString();
//             t.diameter = obj["diameter"].toDouble();
//             t.fluteLength = obj["fluteLength"].toDouble();
//             t.totalLength = obj["totalLength"].toDouble();
//             t.cornerRadius = obj["cornerRadius"].toDouble(0.0);
//             t.fluteCount = obj["fluteCount"].toInt(2);
//             t.material = obj["material"].toString();
//             t.type = obj["type"].toString();
//             t.uniqueKey = getUniqueKey(t);
//
//             qDebug() << t.uniqueKey;
//             if (isUniqueTool(t.uniqueKey)) toolList.append(t);
//
//             saveToolTable();
//         }
//     }
//
//     updateToolTable();
//     QMessageBox::information(this, tr("成功"), tr("已导入 %1 把刀具").arg(toolList.size() - toolCount));
//     toolCount = toolList.size();
// }
