add_executable(BasicCompute
            main.cpp
)

target_link_libraries(BasicCompute PRIVATE ComputeSys)

target_compile_definitions(BasicCompute PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../Resources"
)


