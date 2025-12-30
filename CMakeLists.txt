# Minimal CMake for CourseProject
cmake_minimum_required(VERSION 3.5)
project(CourseProject C)

set(CMAKE_C_STANDARD 99)

include_directories(${CMAKE_SOURCE_DIR})

set(SOURCES
	main.c
	src-extends/md5.c
	src-extends/usrManager.c
    src-extends/fileHelper.c
    src-extends/passwordInputSimulator.c
	src-extends/championHistoryColManager.c
	src-extends/problemBankManager.c
	src-extends/stack.c
	src-extends/markdownPrinter.c
)

add_executable(course_project ${SOURCES})

target_link_libraries(course_project m)
