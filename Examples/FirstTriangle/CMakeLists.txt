add_executable(FirstTriangle main.cpp)

target_include_directories(FirstTriangle PRIVATE ../../Walnut/src)

target_link_libraries(FirstTriangle PRIVATE walnut)


