# Builds a minimal app against an installed Walnut package, to check that
# find_package(Walnut) and the exported Walnut::walnut target work.
cmake_minimum_required(VERSION 3.22)
project(WalnutPackageTest CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Walnut CONFIG REQUIRED)

add_executable(walnut-package-test main.cpp)
target_link_libraries(walnut-package-test PRIVATE Walnut::walnut)
