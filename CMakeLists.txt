cmake_minimum_required(VERSION 3.10)
project(Lab3_Signals C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

add_executable(lab3 laba3.c)
