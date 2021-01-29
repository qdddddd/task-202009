cmake_minimum_required(VERSION 3.14)
project(task-202009)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
add_definitions(-D_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clangd
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "-pthread -march=native $ENV{CXXFLAGS}")
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(DEFAULT_BUILD_TYPE "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# External packages
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party)
set(BUILD_TESTS OFF CACHE BOOL "")
set(BUILD_SAMPLES OFF CACHE BOOL "")
# cpprest
add_subdirectory(${THIRD_PARTY_DIR}/cpprestsdk)
# azure-storage
set(CASABLANCA_DIR ${THIRD_PARTY_DIR}/cpprestsdk)
set(CASABLANCA_LIBRARY cpprest)
set(AZURESTORAGE_DIR ${THIRD_PARTY_DIR}/azure-storage-cpp/Microsoft.WindowsAzure.Storage)
add_subdirectory(${AZURESTORAGE_DIR})
include_directories(${AZURESTORAGE_DIR}/includes)

# Source code
# ROOT
set(ROOT_DIR src)
aux_source_directory(${ROOT_DIR} ROOT_SRCS)
list(REMOVE_ITEM ROOT_SRCS src/r2.cpp)
# UTILS
set(UTILS_DIR src/utils)
aux_source_directory(${UTILS_DIR} UTILS_SRCS)

include_directories(src
    ${ROOT_DIR}
    ${UTILS_DIR}
)

set(SRC_CODE
    ${ROOT_SRCS}
    ${UTILS_SRCS}
)

add_library(projcore STATIC ${SRC_CODE})

# Executables
set(SRC_MAIN src/r2.cpp)
add_executable(r2 ${SRC_MAIN})
target_link_libraries(projcore azurestorage)
target_link_libraries(projcore ${CASABLANCA_LIBRARY})
target_link_libraries(r2 projcore)
add_dependencies(r2 projcore)

# Install
install(TARGETS r2 DESTINATION bin)

# Display cmake options
MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
MESSAGE(STATUS "Compiler flags:" ${CMAKE_CXX_COMPILE_FLAGS})
MESSAGE(STATUS "Compiler cxx debug flags:" ${CMAKE_CXX_FLAGS_DEBUG})
MESSAGE(STATUS "Compiler cxx release flags:" ${CMAKE_CXX_FLAGS_RELEASE})
MESSAGE(STATUS "Compiler cxx flags:" ${CMAKE_CXX_FLAGS})
