cmake_minimum_required(VERSION 2.8)
set(PROJ_NAME LensDistortionQt)
project(${PROJ_NAME})

# Prevent compilation in-source
if( ${CMAKE_BINARY_DIR} STREQUAL ${PROJECT_SOURCE_DIR} )
  Message( FATAL_ERROR "In-source build is not allowed. Create a build directory and run cmake in the directory.")
endif()

# enable C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# OpenCV
find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})

# Qt
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5PrintSupport)

# library
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(${Qt5Widgets_DEFINITIONS})

set(UI_HEADERS mainwindow.h)
set(UI_SOURCES mainwindow.cpp)
set(UI_FORMS mainwindow.ui)

qt5_wrap_cpp(UI_HEADERS_MOC ${UI_HEADERS})
qt5_wrap_ui(UI_FORMS_HEADERS ${UI_FORMS})

set(PROJ_CONFIG mainwindow)
add_library(${PROJ_CONFIG} ${UI_HEADERS_MOC} ${UI_FORMS_HEADERS})
qt5_use_modules(${PROJ_CONFIG} PrintSupport Widgets)

# main application
set(PROJ_HEADERS
    LensUndistortion.h
)
set(PROJ_SOURCES
    main.cpp
    LensUndistortion.cpp
)
add_executable(${PROJ_NAME}
    ${PROJ_SOURCES}
    ${PROJ_HEADERS}
    ${UI_FORMS_HEADERS}
    ${UI_SOURCES}
)
target_link_libraries(${PROJ_NAME}
    ${OpenCV_LIBS}
)
qt5_use_modules(${PROJ_NAME}
    Core
    Gui
    PrintSupport
    Widgets
)
