project ("llvmes")
cmake_minimum_required (VERSION 3.8)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Include dirs in: ${LLVM_INCLUDE_DIRS}")

foreach(lib ${LLVM_AVAILABLE_LIBS})
    message(STATUS ${lib})
endforeach()

set(SOURCEFILES
)

add_library (${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
    ${LLVM_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
)
add_definitions(${LLVM_DEFINITIONS})

target_link_libraries(${PROJECT_NAME} INTERFACE
    LLVM
)
