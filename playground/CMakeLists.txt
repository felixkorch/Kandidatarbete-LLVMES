set(PLAYGROUND_FILES
    Function
    6502
)
set(LLVM_PLAYGROUND_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/test/playground)

foreach (file ${PLAYGROUND_FILES})

    add_executable(${file} ${file}.cpp)
    target_link_libraries(${file} PUBLIC
        llvmes
    )

    set_target_properties(${file} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${LLVM_PLAYGROUND_OUTPUT_DIR}
    )    
endforeach()