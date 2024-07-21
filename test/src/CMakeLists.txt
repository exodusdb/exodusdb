#
# Config
# ######
#
	cmake_minimum_required(VERSION 3.10)
	project(exodus_test)

	#https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html

#
# Init
# ####
#
	message("-----------------------")
	message("-- CMAKE EXODUS TEST --")
	message("-----------------------")

#
# Ignore nodiscard warnings
# #########################
#
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result") #switch off nodiscard warnings
	#add_compile_options(-Wno-unused-result) #switch off nodiscard warnings
	#set(CMAKE_CXX_FLAGS qqqqqqqqq)
	#add_compile_definitions(-Wno-unused-result) #switch off nodiscard warnings
	#COMPILE_DEFINITIONS
#
# Configure test options
# ######################
#
	#CTEST_OUTPUT_ON_FAILURE=1
	#CTEST_PARALLEL_LEVEL=16
	#set(CMAKE_CTEST_ARGUMENTS "--parallel 32;--test-load 3;--output-on-failure")
	set(CMAKE_CTEST_ARGUMENTS -output-on-failure --parallel=32 --test-load=3)

#
# Enable testing
# ##############
#
	include(CTest)
	enable_testing()
	#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/tests)
	#set(CTEST_BINARY_DIRECTORY ${PROJECT_BINARY_DIR}/tests)

#
# Allow relative link directories
# ###############################
#
	cmake_policy(SET CMP0015 NEW)
#	include_directories(../../exodus/libexodus ../../fast_float/include ../../fmt/include)
	include_directories(../../exodus/libexodus ../../fast_float/include)
	#link_directories(${CMAKE_CURRENT_SOURCE_DIR}../../exodus/libexodus/exodus)
	link_directories(../../exodus/libexodus/exodus)

#
# Turn off optimisation since compile time of test_main currently becomes 60 seconds
# ##################################################################################
#
	# TODO look at breaking test_main main function into many smaller functions
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

#
# Build test programs not like test_*.cpp
# #######################################
#
	add_executable(testcommandline testcommandline.cpp)
	target_link_libraries(testcommandline exodus ${PROJECT_LIBS})

    # lib #
    add_library(testcommandlib SHARED testcommandlib.cpp)
	# REQUIRED only for c++ module discovery
	target_link_libraries(testcommandlib exodus ${PROJECT_LIBS})

#
# Trigger cmake update if any new test_*.cpp files appear
# #######################################################
#
	##file(GLOB files "test_*.cpp")
	file(GLOB_RECURSE files CONFIGURE_DEPENDS "test_*.cpp")
	list(SORT files)

#
# Add cmake commands for every test_*.cpp present
# ###############################################
#
	#message(PROJECT_LIBS="${PROJECT_LIBS}")

	foreach(file ${files})

		string(REGEX REPLACE "(^.*/|\\.[^.]*$)" "" file_without_ext ${file})

		add_executable(${file_without_ext} ${file})

		#target_link_libraries(${file_without_ext} ${PROJECT_LIBS} exodus)
#		target_link_libraries(${file_without_ext} ${FMT_LIBRARIES} exodus)
		target_link_libraries(${file_without_ext} exodus)
		target_link_libraries(${file_without_ext} ${PROJECT_LIBS})
		if (EXO_MODULE)
			target_link_libraries(${file_without_ext}
				exovar
			)
		endif()

		#setup a classic test i.e. requiring output of "Test passed"
		add_test(NAME ${file_without_ext} COMMAND ${file_without_ext} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

		set_tests_properties(
			${file_without_ext}
			PROPERTIES
			PASS_REGULAR_EXPRESSION "Test passed"
			FAIL_REGULAR_EXPRESSION "(Test failed)"
			TIMEOUT 120
		)

#		# NOT possible to easily customise tests until cmake 3.17 using CMAKE_CTEST_ARGUMENTS
#		# https://stackoverflow.com/questions/16158091/cmake-setting-an-environmental-variable-for-ctest-or-otherwise-getting-failed
#		set_tests_properties(
#			${file_without_ext}
#			PROPERTIES
#			CTEST_OUTPUT_ON_FAILURE 1
#		)
#		#https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html#environment-variables-for-ctest
#		set_tests_properties(
#			${file_without_ext}
#			PROPERTIES ENVIRONMENT CTEST_OUTPUT_ON_FAILURE CTEST_PARALLEL_LEVEL=32 CTEST_TEST_LOAD=2
#		)
	#add_custom_target(run_toto COMMAND echo bla bla bla)
	#add_test(NAME test_toto COMMAND ${CMAKE_COMMAND} --build . --target run_toto)

	endforeach()

	# test_regress must be built with optimisation otherwise it runs very slowly
	target_compile_options(test_Regress PUBLIC "-O2")

#
# Exit
# ####
#
#	message("--EXODUS TEST CMAKE EXIT --")
