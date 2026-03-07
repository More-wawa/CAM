cmake_minimum_required(VERSION 4.1)
project(CAM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/App/Qt/6.10.1/msvc2022_64")
set(DLL_DIR "C:/Users/More/ClionProjects/DLL")
set(VTK_DIR "${DLL_DIR}/${CMAKE_BUILD_TYPE}/vtk-with-occt/lib/cmake/vtk-9.6")
set(VTK_BIN_DIR "${DLL_DIR}/${CMAKE_BUILD_TYPE}/vtk-with-occt/bin")
set(OCCT_BIN_DIR "${DLL_DIR}/${CMAKE_BUILD_TYPE}/opencascade-7.9.3-vc14-64/win64/vc14/bin")
set(OCCT_3RDPARTY_ROOT_DIR "${DLL_DIR}/${CMAKE_BUILD_TYPE}/3rdparty-vc14-64")

set(QT_LIBS Core Gui Widgets OpenGLWidgets OpenGL)
set(OCCT_3RDPARTY_BIN_DIRS
        "${OCCT_3RDPARTY_ROOT_DIR}/tbb-2021.13.0-x64/bin"
        "${OCCT_3RDPARTY_ROOT_DIR}/openvr-1.14.15-64/bin/win64"
        "${OCCT_3RDPARTY_ROOT_DIR}/freeimage-3.18.0-x64/bin"
        "${OCCT_3RDPARTY_ROOT_DIR}/ffmpeg-3.3.4-64/bin"
)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        OpenGL
        OpenGLWidgets
        REQUIRED)
find_package(VTK REQUIRED COMPONENTS
        CommonCore              # 核心基础库
        CommonDataModel         # 核心数据结构
        CommonExecutionModel    # 核心机制
        CommonTransforms        # 坐标变换
        RenderingCore           # 渲染核心
        RenderingOpenGL2        # OpenGL 后端
        InteractionStyle        # 交互
        GUISupportQt            # Qt 支持核心
        ViewsQt                 # QVTKOpenGLNativeWidget 所在模块
        RenderingQt             # QT 渲染
        IOOCCT                  # OCCT 读取 STEP 格式支持
        # 参考线
        RenderingAnnotation
        InteractionWidgets
)

add_executable(${PROJECT_NAME} src/main.cpp
        src/mainwindow.cpp
        include/mainwindow.h
        ui/mainwindow.ui
        include/Tool.h
        include/FileType.h
        src/vtkManager.cpp
        include/vtkManager.h
)
target_link_libraries(${PROJECT_NAME}
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::OpenGL
        Qt::OpenGLWidgets
        ${VTK_LIBRARIES}
)

# 在 main.cpp 中设置默认格式
target_compile_definitions(${PROJECT_NAME} PRIVATE VTK_OPENGL2_BACKEND)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    # 判断是否 Debug
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif()

    # 判断是否找到 Qt 安装目录
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
        MESSAGE("QT_INSTALL_PATH not founded")
    endif()

    # 复制 Qt platforms 插件
    if(EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif()

    # 复制其他需要的 Qt DLL
    foreach(QT_LIB ${QT_LIBS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach()

    # 复制 VTK 所需 dll
    if(EXISTS "${VTK_BIN_DIR}" AND IS_DIRECTORY "${VTK_BIN_DIR}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${VTK_BIN_DIR}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                COMMENT "Copying VTK DLLs from ${VTK_BIN_DIR} to output directory"
        )
    else()
        message(WARNING
                "VTK bin directory not found or empty: ${VTK_BIN_DIR}\n"
                "VTK DLLs will NOT be copied automatically.\n"
                "Please manually copy from ${VTK_BIN_DIR} or add it to PATH."
        )
    endif()

    # 复制 OCCT 所有 dll
    if (EXISTS "${OCCT_BIN_DIR}" AND IS_DIRECTORY "${OCCT_BIN_DIR}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${OCCT_BIN_DIR}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                COMMENT "Copying OCCT DLLs from ${OCCT_BIN_DIR} to output directory"
        )
    else()
        message(WARNING
                "OCCT bin directory not found or empty: ${OCCT_BIN_DIR}\n"
                "OCCT DLLs will NOT be copied automatically.\n"
                "Please manually copy from ${OCCT_BIN_DIR} or add it to PATH."
        )
    endif()

    # 复制 OCCT_3RDPARTY 所需 dll
    if (EXISTS "${OCCT_3RDPARTY_ROOT_DIR}" AND IS_DIRECTORY "${OCCT_3RDPARTY_ROOT_DIR}")
        foreach (third_bin ${OCCT_3RDPARTY_BIN_DIRS})
            if (EXISTS "${third_bin}" AND IS_DIRECTORY "${third_bin}")
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${third_bin}"
                        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                        COMMENT "Copying OCCT DLLs from ${OCCT_BIN_DIR} to output directory"
                )
            else ()
                message(WARNING
                        "OCCT_3RDPARTY directory not found or empty: ${third_bin}"
                )
            endif ()
        endforeach ()
    else()
        message(WARNING
                "OCCT_3RDPARTY directory not found or empty: ${OCCT_3RDPARTY_ROOT_DIR}"
        )
    endif()
endif()
