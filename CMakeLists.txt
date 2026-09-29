cmake_minimum_required(VERSION 3.16)
project(task5_armor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio highgui)

add_executable(task5_armor
    main/main.cpp
    processing/ImageProcessor.cpp
    detection/LightDetector.cpp
    matching/ArmorMatcher.cpp
    InputHandle/InputHandle.cpp
    control/VideoControl.cpp
)

target_include_directories(task5_armor PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    common processing detection matching InputHandle control
)
target_link_libraries(task5_armor PRIVATE ${OpenCV_LIBS})
