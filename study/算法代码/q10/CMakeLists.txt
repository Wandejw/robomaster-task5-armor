cmake_minimum_required(VERSION 3.16)

project(task5_q10_median_filter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV REQUIRED)

add_executable(q10
    main.cpp
    base.cpp
    function.cpp
)

target_include_directories(q10 PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(q10 PRIVATE ${OpenCV_LIBS})
set_target_properties(q10 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
