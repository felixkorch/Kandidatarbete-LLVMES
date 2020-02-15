set(LLVM_TEST
    # list of test-files
    ReadROM
    ExecuteProgram
    ExecuteFile
)
set(LLVM_TEST_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/test)

foreach (test ${LLVM_TEST})
    set(LLVM_TEST_TARGET ${test})

    add_executable(${LLVM_TEST_TARGET} ${test}.cpp)
    target_link_libraries(${LLVM_TEST_TARGET} PUBLIC
        llvmes
    )

    set_target_properties(${LLVM_TEST_TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${LLVM_TEST_OUTPUT_DIR}
    )    
endforeach()