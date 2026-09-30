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
    ../control/VideoControl.cpp
)
target_include_directories(preprocess_preview PRIVATE
    ../processing ../detection ../matching ../control ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(preprocess_preview PRIVATE ${OpenCV_LIBS})
set_target_properties(preprocess_preview PROPERTIES OUTPUT_NAME test)
