#
# Config
# ######
#
	cmake_minimum_required(VERSION 3.1)
	project(exodus_test)

#
# Init
# ####
#
	message("")
	message("--EXODUS TEST CMAKE INIT --")

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
	include_directories(../../exodus/libexodus ../../fast_float/include)
	#link_directories(${CMAKE_CURRENT_SOURCE_DIR}../../exodus/libexodus/exodus)
	link_directories(../../exodus/libexodus/exodus)

#
# Turn off optimisation since compile time of test_main currently becomes 60 seconds
# ##################################################################################
#
	# TODO look at breaking test_main main function into many smaller functions
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

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
	foreach(file ${files})
		string(REGEX REPLACE "(^.*/|\\.[^.]*$)" "" file_without_ext ${file})

		add_executable(${file_without_ext} ${file})
		target_link_libraries(${file_without_ext} ${PROJECT_LIBS} exodus)

		#setup a classic test i.e. requiring output of "Test passed"
		add_test(NAME ${file_without_ext} COMMAND ${file_without_ext} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
		set_tests_properties(${file_without_ext}
			PROPERTIES
			PASS_REGULAR_EXPRESSION "Test passed")
		set_tests_properties(${file_without_ext}
			PROPERTIES
			FAIL_REGULAR_EXPRESSION "(Test failed)")
		set_tests_properties(${file_without_ext}
			PROPERTIES
			TIMEOUT 120)

	#add_custom_target(run_toto COMMAND echo bla bla bla)
	#add_test(NAME test_toto COMMAND ${CMAKE_COMMAND} --build . --target run_toto)

	endforeach()

#
# Exit
# ####
#
	message("--EXODUS TEST CMAKE EXIT --")
