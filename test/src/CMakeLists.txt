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
	add_executable(testcommandline testcommandline.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../exodus/libexodus/exodus/var.h)
	target_link_libraries(testcommandline exodus ${PROJECT_LIBS})

    # lib #
    add_library(testcommandlib SHARED testcommandlib.cpp)
	# REQUIRED only for c++ module discovery
	target_link_libraries(testcommandlib exodus ${PROJECT_LIBS})

#
# Generate testing_var.h.cpp
# ##########################
#
	add_executable(testing_var_h testing_var.h.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../exodus/libexodus/exodus/var.h)
	target_link_libraries(testing_var_h exodus ${PROJECT_LIBS})

	if (EXO_MODULE)
		target_link_libraries(testing_var_h
		exovar
	)
	endif()

	add_custom_target(PreBuild_testing_var_h
    	COMMAND test -x ${CMAKE_CURRENT_BINARY_DIR}/../../cli/src/gendoc && ${CMAKE_CURRENT_BINARY_DIR}/../../cli/src/gendoc ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../exodus/libexodus/exodus/var.h {m} > /dev/null || echo Skipped first build because gendoc not built yet
    	COMMENT "Running pre-build command gendoc to create or update testing_var.h.cpp"
	)

	add_dependencies(testing_var_h PreBuild_testing_var_h)

	#setup a classic test i.e. requiring output of "Test passed"
	add_test(NAME testing_var_h COMMAND testing_var_h WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
	set_tests_properties(
		testing_var_h
		PROPERTIES
		PASS_REGULAR_EXPRESSION "Test passed"
#		FAIL_REGULAR_EXPRESSION "(Test failed)"
		TIMEOUT 15
	)

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
#			FAIL_REGULAR_EXPRESSION "(Test failed)"
			TIMEOUT 15
		)

	endforeach()

	# test_regress must be built with optimisation otherwise it runs very slowly
	target_compile_options(test_Regress PUBLIC "-O2")

#
# Exit
# ####
#
#	message("--EXODUS TEST CMAKE EXIT --")
