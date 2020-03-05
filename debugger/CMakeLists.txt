cmake_minimum_required(VERSION 3.1.0)

project(debugger)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
        mainwindow.ui
        mainwindow.cpp
        mainwindow.h
        main.cpp
        debugger.cpp
        debugger.h
        disassemblyview.cpp
        disassemblyview.h

        ext/framelesswindow/darkstyle.cpp
        ext/framelesswindow/framelesswindow.ui
        ext/framelesswindow/framelesswindow.cpp
        ext/framelesswindow/windowdragger.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/ext
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/ext/framelesswindow
)

target_link_libraries(${PROJECT_NAME}
        llvmes
        Qt5::Widgets
)


if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/debugger
)    