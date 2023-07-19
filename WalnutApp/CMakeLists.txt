add_executable(App src/dummymain.cpp)

target_include_directories(App PRIVATE ../Walnut/src)

target_link_libraries(App PRIVATE walnut)