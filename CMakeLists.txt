# Minimal CMake for CourseProject
cmake_minimum_required(VERSION 3.5)
project(CourseProject C)

set(CMAKE_C_STANDARD 99)

include_directories(${CMAKE_SOURCE_DIR})

set(SOURCES
	main.c
	md5.c
	usrManager.c
    fileHelper.c
    passwordInputSimulator.c
)

add_executable(course_project ${SOURCES})

target_link_libraries(course_project m)
