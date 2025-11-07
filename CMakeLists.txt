cmake_minimum_required(VERSION 3.14)
project(interlacer C)

find_package(TIFF)

add_executable(interlacer src/main.c)
target_link_libraries(interlacer PRIVATE TIFF::TIFF)
