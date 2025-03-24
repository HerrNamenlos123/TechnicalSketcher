cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(TechnicalSketcher)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compiler Flags
# set(SANITIZERS "")
# set(SANITIZERS -fsanitize=address -fsanitize=leak -fsanitize=undefined)
# set(SANITIZERS -fsanitize=safe-stack)
# set(SANITIZERS -fsanitize=undefined -fsanitize=shadow-call-stack -fsanitize=integer -flto)
# set(SANITIZERS -fsanitize=memory)
# set(SANITIZERS -fsanitize=undefined -fsanitize=memory -fsanitize=shadow-call-stack -fsanitize=integer -flto)

if (NOT MSVC)
set(CLANG_FLAGS -g -Wall -Wextra -fdeclspec -Wno-ignored-attributes -Wno-vla-cxx-extension -Wno-c++11-narrowing -Wno-user-defined-literals -Werror -Wreturn-type -Wno-missing-designated-field-initializers -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare -Wno-unused-but-set-variable -Wno-unused-function)
endif()

set(COMMON_COMPILER_FLAGS -I/usr/local/include ${CLANG_FLAGS})
if (NOT MSVC)
set(COMMON_LINKER_FLAGS  -Wl,--no-undefined -lGL)
endif()
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(SDL3 REQUIRED sdl3)
set(SDL_COMPILER_FLAGS ${SDL3_CFLAGS})
set(SDL_LINKER_FLAGS ${SDL3_LDFLAGS})

# Shared Library
add_library(shared OBJECT src/shared/shared.cpp src/GL/glad.c)
target_compile_options(shared PRIVATE ${COMMON_COMPILER_FLAGS} ${SANITIZERS})
target_link_options(shared PRIVATE ${COMMON_LINKER_FLAGS} ${SANITIZERS})

# Core Executable
add_executable(core src/core/main.cpp)
target_link_libraries(core PRIVATE shared)
target_compile_options(core PRIVATE ${COMMON_COMPILER_FLAGS} ${SANITIZERS} ${SDL_COMPILER_FLAGS})
target_link_options(core PRIVATE ${SDL_LINKER_FLAGS} ${COMMON_LINKER_FLAGS} ${SANITIZERS})

# App Shared Library
add_library(app EXCLUDE_FROM_ALL SHARED src/app/app.cpp src/app/clay/clay_renderer.c $<TARGET_OBJECTS:shared>)
target_compile_options(app PRIVATE ${COMMON_COMPILER_FLAGS} ${SANITIZERS} ${SDL_COMPILER_FLAGS})
target_link_options(app PRIVATE ${SDL_LINKER_FLAGS} ${COMMON_LINKER_FLAGS} ${SANITIZERS})
set_target_properties(app PROPERTIES OUTPUT_NAME "app" LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")

if (MSVC)
target_compile_definitions(shared PUBLIC TSK_WINDOWS UNICODE)
target_compile_options(shared PUBLIC /wd4244 /wd4267 /wd4838 /wd4305)
else()
target_compile_definitions(shared PUBLIC TSK_LINUX)
endif ()

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_image REQUIRED)

target_link_libraries(core PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image)
target_link_libraries(shared PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image)
target_link_libraries(app PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image)

target_link_libraries(core PRIVATE shared)
target_link_libraries(app PRIVATE shared)

# Custom Run Target
add_custom_target(run
    COMMAND core
    DEPENDS core
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Output Directories
set_target_properties(core PROPERTIES OUTPUT_NAME "TechnicalSketcher")
set_target_properties(core PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(shared PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(app PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
