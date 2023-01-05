cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME TechnicalSketcher)
project(${TARGET_NAME} VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})



if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

if(CMAKE_CONFIGURATION_TYPES)
   list(APPEND CMAKE_CONFIGURATION_TYPES Deploy)
   list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
endif()

set(CMAKE_EXE_LINKER_FLAGS_DEPLOY ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL})



##################################
# Options: Library configuration #
##################################

option(STATIC_RUNTIME "want static MSVC runtime" on)





################
# Dependencies #
################

if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/modules/Battery/CMakeLists.txt")
set(GET_SUB true)
endif ()

if (GET_SUB)
find_package(Git)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update in ${CMAKE_CURRENT_LIST_DIR}")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()
endif()

if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/modules/Battery/CMakeLists.txt")
message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif ()

add_subdirectory("modules/Battery")




#####################
# Target definition #
#####################

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "include/*")
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*")
file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "resource/*")
list(REMOVE_ITEM RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resource/resource.aps")

add_executable(${TARGET_NAME} ${SRC_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

target_include_directories(${TARGET_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
set_target_properties(${TARGET_NAME} PROPERTIES CXX_EXTENSIONS OFF)

if(STATIC_RUNTIME)
    set_property(TARGET ${TARGET_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})
else()
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-psabi)
endif()


############################
# Preprocessor definitions #
############################

if (WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE
       WIN32_LEAN_AND_MEAN      # Prevents Windows.h from adding unnecessary includes
       NOMINMAX                 # Prevents Windows.h from defining min/max as macros 
       _CRT_SECURE_NO_WARNINGS
    )
endif()




###########
# Linking #
###########

target_link_libraries(${TARGET_NAME} Battery::Battery)




#######################
# Precompiled Headers #
#######################

target_precompile_headers(${TARGET_NAME} PRIVATE include/pch.h)




#######
# IDE #
#######

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${SRC_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/resource" PREFIX "Resource Files" FILES ${RESOURCE_FILES})


###########
# Install #
###########

install(
    TARGETS ${TARGET_NAME}
    EXPORT ${TARGET_NAME}Targets
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "include"
)
