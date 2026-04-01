cmake_minimum_required(VERSION 3.20)
project(CAM)

# 基础编译设置
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 确保 CMAKE_BUILD_TYPE 有默认值
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 路径定义
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/msvc2022_64")
set(DLL_DIR "C:/Users/MoreW/CLionProjects/DLL")

set(VTK_DIR "${DLL_DIR}/vtk-9.4.0/lib/cmake/vtk-9.4")
set(VTK_BIN_DIR "${DLL_DIR}/vtk-9.4.0/bin")

set(OCCT_ROOT "${DLL_DIR}/opencascade-7.9.3-vc14-64")
set(OCCT_BIN_DIR "${OCCT_ROOT}/win64/vc14/bin")
set(OCCT_LIB_DIR "${OCCT_ROOT}/win64/vc14/lib")
set(OCCT_INC_DIR "${OCCT_ROOT}/inc")

set(OCCT_3RDPARTY_ROOT_DIR "${DLL_DIR}/3rdparty-vc14-64")

# 依赖项前置处理
set(TBB_DIR "${OCCT_3RDPARTY_ROOT_DIR}/tbb-2021.13.0-x64/lib/cmake/TBB")
find_package(TBB QUIET)

# 查找核心库
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

# 查找 VTK
find_package(VTK REQUIRED)

# 包含目录与库搜索路径
include_directories(
        "include"
        "${OCCT_INC_DIR}"
        "${OCCT_3RDPARTY_ROOT_DIR}/tbb-2021.13.0-x64/include"
        "${OCCT_3RDPARTY_ROOT_DIR}/freetype-2.13.3-x64/include/freetype2"
)

link_directories(
        "${OCCT_LIB_DIR}"
        "${OCCT_3RDPARTY_ROOT_DIR}/tbb-2021.13.0-x64/lib"
        "${OCCT_3RDPARTY_ROOT_DIR}/freetype-2.13.3-x64/lib"
)

# 定义 OCCT 链接列表
set(OCCT_LIBS
        TKIVtk TKOffset TKXSBase TKDESTEP TKV3d TKService
        TKHLR TKMesh TKPrim TKTopAlgo TKBRep TKGeomAlgo
        TKGeomBase TKMath TKernel freetype tbb
)

# 创建执行文件
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/mainwindow.cpp
        include/mainwindow.h
        ui/mainwindow.ui
        include/Tool.h
        src/vtkManager.cpp
        include/vtkManager.h
        src/ToolManager.cpp
        include/ToolManager.h
        include/EnumType.h
        src/dialogaddtool.cpp
        include/dialogaddtool.h
        ui/dialogaddtool.ui
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        ${VTK_LIBRARIES}
        ${OCCT_LIBS}
)

# 定义宏
target_compile_definitions(${PROJECT_NAME} PRIVATE VTK_OPENGL2_BACKEND)

# Windows 平台下的 DLL 自动拷贝
if (WIN32)
    set(DEST_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

    # 拷贝 Qt 核心 DLL 和 插件
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}/platforms"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_PREFIX_PATH}/bin/Qt6Core.dll" "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_PREFIX_PATH}/bin/Qt6Gui.dll" "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_PREFIX_PATH}/bin/Qt6Widgets.dll" "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_PREFIX_PATH}/bin/Qt6OpenGL.dll" "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_PREFIX_PATH}/bin/Qt6OpenGLWidgets.dll" "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_PREFIX_PATH}/plugins/platforms/qwindows.dll" "${DEST_DIR}/platforms"
            COMMENT "Copying Qt Runtime..."
    )

    # 拷贝 VTK DLL
    if(EXISTS "${VTK_BIN_DIR}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${VTK_BIN_DIR}" "${DEST_DIR}"
                COMMENT "Copying VTK DLLs..."
        )
    endif()

    # 拷贝 OCCT DLL
    if(EXISTS "${OCCT_BIN_DIR}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${OCCT_BIN_DIR}" "${DEST_DIR}"
                COMMENT "Copying OCCT DLLs..."
        )
    endif()

    # 拷贝重要的第三方库
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${OCCT_3RDPARTY_ROOT_DIR}/tbb-2021.13.0-x64/bin/tbb12.dll" "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${OCCT_3RDPARTY_ROOT_DIR}/freetype-2.13.3-x64/bin/freetype.dll" "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${OCCT_3RDPARTY_ROOT_DIR}/openvr-1.14.15-64/bin/win64/openvr_api.dll" "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${OCCT_3RDPARTY_ROOT_DIR}/freeimage-3.18.0-x64/bin/FreeImage.dll" "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${OCCT_3RDPARTY_ROOT_DIR}/ffmpeg-3.3.4-64/bin" "${DEST_DIR}"
            COMMENT "Copying 3rdparty DLLs..."
            COMMAND ${CMAKE_COMMAND} -E copy "${DLL_DIR}/jemalloc.dll" "${DEST_DIR}"
    )
endif()