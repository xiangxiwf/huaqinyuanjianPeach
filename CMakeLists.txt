cmake_minimum_required(VERSION 3.16)

project(IndustrialAssemblyLineSortingSystem VERSION 0.1 LANGUAGES CXX CUDA)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt 模块
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network) # 添加 Network 模块
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network) # 添加 Network 模块

# 查找 OpenCV 和 DirectX
find_package(OpenCV REQUIRED)
find_library(DXGI_LIB dxgi)
find_library(D3D11_LIB d3d11)
find_library(USER32_LIB user32)
find_library(GDI32_LIB gdi32)
find_library(KERNEL32_LIB kernel32)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Mqtt)  # 添加 Mqtt 模块
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Mqtt)  # 添加 Mqtt 模块

# 检查 OpenCV 是否找到
if (NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found, please configure OpenCV properly.")
endif ()

# 检查 DirectX 库是否找到
if (NOT DXGI_LIB OR NOT D3D11_LIB OR NOT USER32_LIB OR NOT GDI32_LIB OR NOT KERNEL32_LIB)
    message(FATAL_ERROR "One or more required libraries for DXGI, D3D11, User32, GDI32, or Kernel32 are missing.")
endif ()

# 查找 CUDA 和 TensorRT
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})
set(CMAKE_CUDA_ARCHITECTURES 86)  # 针对 RTX 3060，使用 8.6 架构

set(TENSORRT_DIR "D:/TensorRT-8.6.1.6")  # 设置 TensorRT 安装路径
include_directories(${TENSORRT_DIR}/include)
link_directories(${TENSORRT_DIR}/lib)
set(TENSORRT_LIBS nvinfer nvinfer_plugin nvparsers nvonnxparser)

# 设置项目源文件
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ScreenCapture.cpp
        ScreenCapture.h
        common.h
        cuda_utils.h
        logging.h
        macros.h
        preprocess.h
        preprocess.cu
        YOLOv11.cpp
        YOLOv11.h

)

# 创建可执行文件
if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(IndustrialAssemblyLineSortingSystem
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            pushThread.h pushThread.cpp
    )
else ()
    if (ANDROID)
        add_library(IndustrialAssemblyLineSortingSystem SHARED
                ${PROJECT_SOURCES}
        )
    else ()
        add_executable(IndustrialAssemblyLineSortingSystem
                ${PROJECT_SOURCES}
        )
    endif ()
endif ()

# 链接 Qt 库、OpenCV、DirectX、TensorRT 和 CUDA
target_link_libraries(IndustrialAssemblyLineSortingSystem PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Mqtt  # 添加 Qt Mqtt 库
        ${OpenCV_LIBS}
        ${DXGI_LIB}
        ${D3D11_LIB}
        ${USER32_LIB}
        ${GDI32_LIB}
        ${KERNEL32_LIB}
        ${TENSORRT_LIBS}
        ${CUDA_LIBRARIES}
)


# 设置 MacOS X 和 Windows 的相关属性
if (${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.IndustrialAssemblyLineSortingSystem)
endif ()
set_target_properties(IndustrialAssemblyLineSortingSystem PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

# 安装目标
include(GNUInstallDirs)
install(TARGETS IndustrialAssemblyLineSortingSystem
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 完成 Qt 6 的最终化
if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(IndustrialAssemblyLineSortingSystem)
endif ()