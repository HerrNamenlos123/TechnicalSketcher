cmake_minimum_required(VERSION 3.16)
project(TechnicalSketcher LANGUAGES C CXX)

# Add the CPM.cmake package manager
include(cmake/CPM.cmake)

# Download the battery library
CPMAddPackage("gh:batterycenter/battery#master")

# Define the executable and all the source files
battery_add_executable(${PROJECT_NAME}
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
        src/BatteryApp.cpp
        src/Layer.cpp
        src/LayerList.cpp
        src/LayerState.cpp
        src/Navigator.cpp
        src/SelectionHandler.cpp
        src/SketchFile.cpp
        src/Updater.cpp
        src/UserInterface.cpp
)

# Include directory
target_include_directories(${PROJECT_NAME} PUBLIC include)

# What libraries to link to
battery_add_module(graphics)
battery_add_module(json)
target_link_libraries(${PROJECT_NAME} battery::core battery::graphics battery::json)

# Enable precompiled headers for faster compilation
battery_precompile_headers(${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/include/pch.h")

# What project to start Visual Studio with
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Embed any resource files at build time
# battery_embed(${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/resources/test.txt" TEXT)
