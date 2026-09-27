cmake_minimum_required(VERSION 3.16)
project(task5_preprocess_preview LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio highgui)
add_executable(preprocess_preview
    preview.cpp
    ../processing/ImageProcessor.cpp
    ../detection/LightDetector.cpp
    ../matching/ArmorMatcher.cpp
)
target_include_directories(preprocess_preview PRIVATE
    ../processing ../detection ../matching ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(preprocess_preview PRIVATE ${OpenCV_LIBS})
