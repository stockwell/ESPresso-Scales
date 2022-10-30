cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED PROJECT_NAME)
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(ESPresso-Scales)
else ()
    message(FATAL_ERROR "ESPresso-Scales: This must be a project's main CMakeLists.txt.")
endif ()

