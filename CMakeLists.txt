project ("llvmes")
set (CMAKE_CXX_STANDARD 14)
cmake_minimum_required (VERSION 3.8)


list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
find_package(LLVM REQUIRED CONFIG)
find_package(GLFW 3.3 REQUIRED)
add_subdirectory(dependencies/glad)

if(VERBOSE)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    message(STATUS "Include dirs in: ${LLVM_INCLUDE_DIRS}")
    message(STATUS "INCLUDE DIR: ${GLFW3_INCLUDE_DIR}")

    foreach(lib ${LLVM_AVAILABLE_LIBS})
        message(STATUS ${lib})
    endforeach()
endif()


set(SOURCE
    src/llvmes/Graphics/Primitives/VertexArray.cpp
    src/llvmes/Graphics/Primitives/VertexBuffer.cpp
    src/llvmes/Graphics/Primitives/IndexBuffer.cpp
    src/llvmes/Graphics/Primitives/Texture2D.cpp
    src/llvmes/Graphics/Window.cpp
    src/llvmes/Graphics/Input.cpp
    src/llvmes/Graphics/Rectangle.cpp
    src/llvmes/Graphics/Shader.cpp
    src/llvmes/NES/ROMFile.cpp
    src/llvmes/NES/CPU.cpp
)

add_library (${PROJECT_NAME} ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${LLVM_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/dependecies/glad/include>
)
add_definitions(${LLVM_DEFINITIONS})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        ${LLVM_AVAILABLE_LIBS}
        ${GLFW_LIBRARIES}
        glad
    )
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC
        LLVM
        ${GLFW_LIBRARIES}
        dl
        glad
    )
endif()

if(VERBOSE)
    message("GLFW: ${GLFW_LIBRARIES}")
    message("GLFW: ${GLFW_INCLUDE_DIRS}")
endif()


add_subdirectory ("test")
add_subdirectory ("playground")
