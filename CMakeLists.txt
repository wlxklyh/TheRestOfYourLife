cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 11)
set(NAME Raytracing)
project(Raytracing)

file(GLOB_RECURSE file_list  *.h *.cc)



add_executable(Raytracing ${file_list})

#target_compile_definitions(${NAME} PRIVATE "drand48=rand")
target_compile_definitions(${NAME} PRIVATE "M_PI=3.1415926")