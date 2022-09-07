cmake_minimum_required(VERSION 3.1)

project(exodus_test)

message("")
message("--EXODUS TEST CMAKE INIT --")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/tests)
#set(CTEST_BINARY_DIRECTORY ${PROJECT_BINARY_DIR}/tests)

#allow relative link directories
cmake_policy(SET CMP0015 NEW)
include_directories(../../exodus/libexodus ../../fast_float/include)
#link_directories(${CMAKE_CURRENT_SOURCE_DIR}../../exodus/libexodus/exodus)
link_directories(../../exodus/libexodus/exodus)

#turn off optimisation since compile time of test_main currently becomes 60 seconds
#TODO look at breaking test_main main function into many smaller functions
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

##file(GLOB files "test_*.cpp")
file(GLOB_RECURSE files CONFIGURE_DEPENDS "test_*.cpp")
list(SORT files)
#set("files" "test_main2.cpp" "test_db.cpp" "test_main3.cpp" "test_multilang.cpp" "test_dim.cpp"
# "test_perf.cpp" "test_mvfuncs.cpp" "test_dict.cpp" "test_int.cpp" "test_sort.cpp" "test_zzzclean.cpp"
# "test_precision.cpp" "test_mvmv2.cpp" "test_isnum.cpp" "test_more.cpp" "test_comp.cpp"
# "test_precision2.cpp" "test_select.cpp" "test_numfmt.cpp" "test_math.cpp" "test_sortarray.cpp"
# "test_main1.cpp" "test_more2.cpp" "test_mvmv.cpp" "test_regress.cpp" "test_regex.cpp"
# "test_MD.cpp" "test_round.cpp" "test_except.cpp" "test_asyncupd.cpp" "test_osopen.cpp")

#file(GLOB files "test_mvmv.cpp")
foreach(file ${files})
	string(REGEX REPLACE "(^.*/|\\.[^.]*$)" "" file_without_ext ${file})

	add_executable(${file_without_ext} ${file})
	target_link_libraries(${file_without_ext} ${PROJECT_LIBS} exodus)

	#setup a classic test i.e. requiring output of "Test passed"
	add_test(${file_without_ext} ${file_without_ext})
	set_tests_properties(${file_without_ext}
		PROPERTIES
		PASS_REGULAR_EXPRESSION "Test passed")
	set_tests_properties(${file_without_ext}
		PROPERTIES
		FAIL_REGULAR_EXPRESSION "(Exception|Test failed)")
	set_tests_properties(${file_without_ext}
		PROPERTIES
		TIMEOUT 120)

#add_custom_target(run_toto COMMAND echo bla bla bla)
#add_test(NAME test_toto COMMAND ${CMAKE_COMMAND} --build . --target run_toto)

endforeach()

message("--EXODUS TEST CMAKE EXIT --")
