cmake_minimum_required(VERSION 3.16)
project(TechnicalSketcher LANGUAGES C CXX)

#set(ENABLE_CLANG_TIDY OFF)

include(FetchContent)
FetchContent_Declare(battery
    GIT_REPOSITORY https://github.com/batterycenter/battery
    GIT_TAG        master
)
FetchContent_MakeAvailable(battery)

# Define the executable and all the source files
b_add_executable(${PROJECT_NAME}
        src/Shapes/ArcShape.cpp
        src/Shapes/CircleShape.cpp
        src/Shapes/GenericShape.cpp
        src/Shapes/LineShape.cpp

        src/Tools/ArcTool.cpp
        src/Tools/CircleTool.cpp
        src/Tools/LineStripTool.cpp
        src/Tools/LineTool.cpp
        src/Tools/SelectionTool.cpp

        src/ApplicationRenderer.cpp
        src/Layer.cpp
        src/LayerList.cpp
        src/LayerState.cpp
        src/main.cpp
        src/SelectionHandler.cpp
        src/TskDocument.cpp
        src/Tsk.cpp
        src/TskConfig.cpp
        src/TskHost.cpp
        src/TskRenderHost.cpp
        src/TskSettings.cpp
        src/TskSplashWindow.cpp
        src/TskWindow.cpp
        src/Updater.cpp
        src/UserInterface.cpp
)

# Include directory
target_include_directories(${PROJECT_NAME} PUBLIC include)

# What libraries to link to
b_add_module(graphics)
#b_add_module(json)
#b_add_module(network)
#b_add_module(filedialog)
target_link_libraries(${PROJECT_NAME} battery::core battery::graphics)

# Enable precompiled headers for faster compilation
b_precompile_headers(${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/include/pch.hpp")

# What project to start Visual Studio with
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Embed any resource files at build time
#battery_embed(${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/version.txt" "tsk_version" TEXT)
#battery_embed(${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/resources/splashscreen.png" "splashscreen_png" BINARY)
#battery_embed(${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/ui/TskWindow.py" "ui/TskWindow_py" TEXT)
#
#battery_embed(${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/resources/TechnicalSketcher.png" "assets/TechnicalSketcher_png" BINARY)
