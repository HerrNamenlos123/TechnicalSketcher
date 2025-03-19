cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(TechnicalSketcher)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compiler Flags
set(SANITIZERS "")
# set(SANITIZERS -fsanitize=address -fsanitize=leak -fsanitize=undefined)
# set(SANITIZERS -fsanitize=safe-stack)
# set(SANITIZERS -fsanitize=undefined -fsanitize=shadow-call-stack -fsanitize=integer -flto)
# set(SANITIZERS -fsanitize=undefined -fsanitize=memory -fsanitize=shadow-call-stack -fsanitize=integer -flto)

set(COMMON_COMPILER_FLAGS -fPIC -g -Wno-vla-cxx-extension -Wno-c++11-narrowing -I/usr/local/include -Wno-user-defined-literals)
set(COMMON_LINKER_FLAGS  -Wl,--no-undefined)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)
set(SDL_COMPILER_FLAGS ${SDL3_CFLAGS})
set(SDL_LINKER_FLAGS ${SDL3_LDFLAGS} -lSDL3_ttf -lSDL3_image)

# Shared Library
add_library(shared OBJECT src/shared/shared.cpp)
target_compile_options(shared PRIVATE ${COMMON_COMPILER_FLAGS} ${SANITIZERS})
target_link_options(shared PRIVATE ${COMMON_LINKER_FLAGS} ${SANITIZERS})

# Core Executable
add_executable(core src/core/main.cpp)
target_link_libraries(core PRIVATE shared)
target_compile_options(core PRIVATE ${COMMON_COMPILER_FLAGS} ${SANITIZERS} ${SDL_COMPILER_FLAGS})
target_link_options(core PRIVATE ${SDL_LINKER_FLAGS} ${COMMON_LINKER_FLAGS} ${SANITIZERS})

# App Shared Library
add_library(app EXCLUDE_FROM_ALL SHARED src/app/app.cpp $<TARGET_OBJECTS:shared>)
target_compile_options(app PRIVATE ${COMMON_COMPILER_FLAGS} ${SANITIZERS} ${SDL_COMPILER_FLAGS})
target_link_options(app PRIVATE ${SDL_LINKER_FLAGS} ${COMMON_LINKER_FLAGS} ${SANITIZERS})
set_target_properties(app PROPERTIES OUTPUT_NAME "app" LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")

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

# FMT library for proper string formatting
include(FetchContent)
FetchContent_Declare(
        fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
        GIT_TAG         11.1.4
        GIT_PROGRESS    TRUE
        USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(fmt)
target_link_libraries(core PRIVATE fmt::fmt)
target_link_libraries(shared PRIVATE fmt::fmt)
target_link_libraries(app PRIVATE fmt::fmt)
