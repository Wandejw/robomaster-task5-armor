cmake_minimum_required(VERSION 3.16)

project(task5_q7_thresholding LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV REQUIRED)

add_executable(thresholding
    main.cpp
    base.cpp
    function.cpp
)

target_include_directories(thresholding PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(thresholding PRIVATE ${OpenCV_LIBS})
