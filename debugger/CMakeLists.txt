cmake_minimum_required(VERSION 3.1.0)

project(Debugger)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(Debugger
        mainwindow.ui
        mainwindow.cpp
        main.cpp
        )

#qt5_wrap_ui(SOURCES mainwindow.ui) Needed?

target_include_directories(Debugger PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        )

target_link_libraries(Debugger
        llvmes
        Qt5::Widgets)