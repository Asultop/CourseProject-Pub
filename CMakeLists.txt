# Minimal CMake for CourseProject
cmake_minimum_required(VERSION 3.5)
project(CourseProject C)

set(CMAKE_C_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR})

set(SOURCES
	src-extends/championHistoryColManager.c
    src-extends/passwordInputSimulator.c
	src-extends/problemBankManager.c
	src-extends/markdownPrinter.c
	src-extends/chineseSupport.c
	src-extends/ACMLocalJudger.c
	src-extends/screenManager.c
	src-extends/codeRender.c
	src-extends/colorPrint.c
	src-extends/usrManager.c
    src-extends/fileHelper.c
	src-extends/stack.c
	src-extends/md5.c
	main.c

)
set(TEST_RENDERER_SOURCES
	src-extends/chineseSupport.c
	src-test/test_codeRender.c
	src-extends/codeRender.c
	src-extends/fileHelper.c
)
set(TEST_CHINESE_SOURCES
	src-test/test_chineseSupport.c
	src-extends/chineseSupport.c
	src-extends/fileHelper.c
)
set(TEST_INPUT_SIMULATOR_SOURCES
	src-extends/passwordInputSimulator.c
	src-test/test_input_simulator.c
)
set(TEST_MARKDOWN_SOURCES
	src-extends/markdownPrinter.c
	src-extends/chineseSupport.c
	src-extends/fileHelper.c
	src-test/test_markdown.c
	src-extends/stack.c
)
set(TEST_MD5_SOURCES
	src-extends/fileHelper.c
	src-test/test_md5.c
	src-extends/md5.c
)
set(TEST_PRINTF_SOURCES
	src-extends/chineseSupport.c
	src-extends/fileHelper.c
	src-test/test_printf.c
)
set(TEST_SCREENMANAGER_SOURCES
	src-test/test_screenManager.c
	src-extends/chineseSupport.c
	src-extends/screenManager.c
	src-extends/colorPrint.c
)
set(TEST_STACK_SOURCES
	src-test/test_stack.c
	src-extends/stack.c
)

add_executable(course_project ${SOURCES})
add_executable(test_codeRender ${TEST_RENDERER_SOURCES})
add_executable(test_chineseSupport ${TEST_CHINESE_SOURCES})
add_executable(test_passwordInputSimulator ${TEST_INPUT_SIMULATOR_SOURCES})
add_executable(test_markdownPrinter ${TEST_MARKDOWN_SOURCES})
add_executable(test_md5 ${TEST_MD5_SOURCES})
add_executable(test_printf ${TEST_PRINTF_SOURCES})
add_executable(test_screenManager ${TEST_SCREENMANAGER_SOURCES})
add_executable(test_stack ${TEST_STACK_SOURCES})

target_link_libraries(course_project m)
target_link_libraries(test_codeRender m)
target_link_libraries(test_chineseSupport m)
target_link_libraries(test_passwordInputSimulator m)
target_link_libraries(test_markdownPrinter m)
target_link_libraries(test_md5 m)
target_link_libraries(test_printf m)
target_link_libraries(test_screenManager m)
target_link_libraries(test_stack m)

add_custom_target(test ALL
	DEPENDS test_codeRender
			test_chineseSupport
			test_passwordInputSimulator
			test_markdownPrinter
			test_md5
			test_printf
			test_screenManager
			test_stack
)

set_target_properties(course_project PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

set_target_properties(test_codeRender PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src-test/build)
set_target_properties(test_chineseSupport PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src-test/build)
set_target_properties(test_passwordInputSimulator PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src-test/build)
set_target_properties(test_markdownPrinter PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src-test/build)
set_target_properties(test_md5 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src-test/build)
set_target_properties(test_printf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src-test/build)
set_target_properties(test_screenManager PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src-test/build)
set_target_properties(test_stack PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src-test/build)
