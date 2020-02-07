project(glad)

set(source
    src/glad.c
)

add_library(${PROJECT_NAME} ${source})
target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
            
)