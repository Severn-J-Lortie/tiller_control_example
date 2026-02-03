cmake_minimum_required(VERSION 3.8)
project(tiller_control_example)

find_library(labjack_libraries NAMES labjackusb HINTS /usr/local/lib REQUIRED)
add_library(u3 STATIC u3.c)
target_include_directories(u3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(u3 PUBLIC /usr/local/include)
target_link_libraries(u3 PUBLIC ${labjack_libraries})

add_executable(main main.cpp)
target_link_libraries(main PRIVATE u3)
set_target_properties(main PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)

