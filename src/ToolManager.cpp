#include "../include/ToolManager.h"
#include <qfileinfo.h>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>

ToolManager *ToolManager::New() {
    return new ToolManager();
}

ResultType ToolManager::init() {
    // 检查是否已经初始化
    if (initialized) return ResultType::Success;

    // 初始化刀具保存目录
    if (!QDir().mkpath(appDataLocation)) {
        return ResultType::CreateFileError;
    }

    // 加载刀具
    curToolCount = toolList.size();

    // 初始化刀具数组


    // 确认已经初始化
    initialized = true;

    return ResultType::Success;
}

ResultType ToolManager::openTool(const QString &fileName, const FileType fileType, QString *message) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        *message = "无法打开文件：" + fileName;
        return ResultType::FileOpenError;
    }

    if (fileType == FileType::Json) {
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        const QJsonArray array = doc.array();
        if (doc.isNull() || !doc.isArray()) {
            *message = "Json 解析失败";
            return ResultType::JsonParseError;
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
        }
    }

    *message = QString("已导入 %1 把新刀具").arg(toolList.size() - curToolCount);

    return ResultType::Success;
}

QString ToolManager::getUniqueKey(const Tool &t) {
    return QString("%1").arg(t.name);
}

bool ToolManager::isUniqueTool(const QString &uniKey) {
    if (uniKey.isEmpty()) return false;
    foreach(const auto &t, toolList) {
        if (uniKey == t.uniqueKey) {
            return false;
        }
    }
    return true;
}

ResultType ToolManager::saveToolToLocal(QString *message) {
    QJsonArray toolArray;

    for (const auto &t: toolList) {
        QJsonObject obj;
        obj["name"] = t.name;
        obj["diameter"] = t.diameter;
        obj["fluteLength"] = t.fluteLength;
        obj["totalLength"] = t.totalLength;
        obj["cornerRadius"] = t.cornerRadius;
        obj["fluteCount"] = t.fluteCount;
        obj["material"] = t.material;
        obj["type"] = t.type;
        obj["uniqueKey"] = t.uniqueKey;

        toolArray.append(obj);
    }

    QJsonObject root;
    root["toolList"] = toolArray;
    root["version"] = "1.0";
    root["lastSaved"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QFile file(saveToolLocation);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        *message = QString("刀具文件保存失败：%1").arg(saveToolLocation);
        return ResultType::FileOpenError;
    }

    const QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    // if (toolList.isEmpty()) {
    //     *message = "刀库为空";
    //     curToolCount = toolList.size();
    //     return ResultType::ToolListEmpty;
    // }

    if (const auto c = abs(toolList.size() - curToolCount); c > 0) {
        *message = QString("%1 把刀具数据已更新至本地").arg(c);
        curToolCount = toolList.size();
        return ResultType::Success;
    }

    *message = "没有刀具数据更新";
    return ResultType::NoChangeTool;
}

ResultType ToolManager::loadToolFromLocal(QString *message) {
    QFile file(saveToolLocation);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        *message = "未发现刀具文件存储文件";
        file.close();
        return ResultType::FileOpenError;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

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

    if (toolList.isEmpty()) {
        *message = "本地 JSON 刀具文件为空";
        return ResultType::ToolListEmpty;
    }

    *message = QString("已从本地加载 %1 把刀具").arg(toolList.size());
    curToolCount = toolList.size();
    return ResultType::Success;
}

ResultType ToolManager::deleteToolSelected(QString *message) {
    for (const auto &tool: toolListSelected) {
        for (int i = 0; i < toolList.size(); i++) {
            // 根据刀具的 uniqueKey 从刀库中删除
            if (toolList.at(i).uniqueKey == tool.uniqueKey) {
                toolList.removeAt(i);
            }
        }
    }

    *message = QString("成功删除 %1 把刀具").arg(curToolCount - toolList.size());
    return ResultType::Success;
}










