/**
 * @file EnumType.h
 * @brief 全局枚举类型定义
 * @details 定义 CAM 系统中通用的操作结果类型和文件类型枚举
 */

#pragma once

#ifndef CAM_ENUMTYPE_H
#define CAM_ENUMTYPE_H

/**
 * @enum ResultType
 * @brief 操作结果返回码
 * @details 各模块函数通过此枚举返回执行状态，配合可选的 QString* message 输出详细信息
 */
enum class ResultType {
    Success,             ///< 操作成功
    ModulError,          ///< 模块内部错误
    FileOpenError,       ///< 文件打开失败
    JsonParseError,      ///< JSON 格式解析失败
    CreateFileError,     ///< 文件创建失败
    ToolListEmpty,       ///< 刀具列表为空
    NoChangeTool,        ///< 刀具无变化，无需保存
    NoToolSelected,      ///< 未选中任何刀具
    NoModelLoaded,       ///< 未加载模型
    ToolpathGenFailed,   ///< 刀具路径生成失败
};

/**
 * @enum FileType
 * @brief 刀具库文件格式
 */
enum class FileType {
    Json,   ///< JSON 格式刀具库
};

#endif //CAM_ENUMTYPE_H