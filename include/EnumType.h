#pragma once

#ifndef CAM_ENUMTYPE_H
#define CAM_ENUMTYPE_H

#endif //CAM_ENUMTYPE_H

enum class ResultType {
    Success,
    ModulError,
    FileOpenError,
    JsonParseError,
    CreateFileError,
    ToolListEmpty,
    NoNewTools,
};

enum class FileType {
    Json,
};