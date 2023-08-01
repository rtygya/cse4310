project (program5)
cmake_minimum_required(VERSION 3.15)

# configure OpenCV
find_package(OpenCV REQUIRED)

# create create individual projects
add_executable(program5 program5.cpp)
target_link_libraries(program5 ${OpenCV_LIBS})
