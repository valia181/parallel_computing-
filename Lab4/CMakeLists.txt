cmake_minimum_required(VERSION 3.30)
project(Lab4)

set(CMAKE_CXX_STANDARD 20)

add_executable(Lab4 main.cpp)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(server ws2_32)
target_link_libraries(client ws2_32)
