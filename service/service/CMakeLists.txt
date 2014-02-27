cmake_minimum_required(VERSION 2.8)

find_package(Boost)

link_directories(../../exodus/libexodus/exodus ${Boost_LIBRARY_DIRS})
add_executable(main main.cpp mvwindow.cpp server.cpp)
target_link_libraries(main exodus boost_system boost_thread)
