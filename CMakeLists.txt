cmake_minimum_required(VERSION 3.16)

project(obsbot-meet2-control)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# SDK paths
set(SDK_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/sdk/include)
set(SDK_LIB_DIR ${CMAKE_SOURCE_DIR}/sdk/lib)

# GUI Application
add_executable(obsbot-meet2-gui
    src/gui/main.cpp
    src/gui/MainWindow.cpp
    src/gui/MainWindow.h
    src/gui/CameraController.cpp
    src/gui/CameraController.h
    src/gui/TrackingControlWidget.cpp
    src/gui/TrackingControlWidget.h
    src/gui/PTZControlWidget.cpp
    src/gui/PTZControlWidget.h
    src/gui/CameraSettingsWidget.cpp
    src/gui/CameraSettingsWidget.h
    src/common/Config.cpp
    src/common/Config.h
    resources/resources.qrc
)

target_include_directories(obsbot-meet2-gui PRIVATE
    ${SDK_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/common
)

target_link_directories(obsbot-meet2-gui PRIVATE
    ${SDK_LIB_DIR}
)

target_link_libraries(obsbot-meet2-gui PRIVATE
    Qt6::Widgets
    dev
)

# CLI Application
add_executable(obsbot-meet2-cli
    src/cli/meet2_test.cpp
    src/common/Config.cpp
    src/common/Config.h
)

target_include_directories(obsbot-meet2-cli PRIVATE
    ${SDK_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/common
)

target_link_directories(obsbot-meet2-cli PRIVATE
    ${SDK_LIB_DIR}
)

target_link_libraries(obsbot-meet2-cli PRIVATE
    dev
)

# Set RPATH for finding libdev.so
set_target_properties(obsbot-meet2-gui PROPERTIES
    BUILD_RPATH "${SDK_LIB_DIR}"
    INSTALL_RPATH "${SDK_LIB_DIR}"
)

set_target_properties(obsbot-meet2-cli PROPERTIES
    BUILD_RPATH "${SDK_LIB_DIR}"
    INSTALL_RPATH "${SDK_LIB_DIR}"
)
