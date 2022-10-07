#
# CONFIG
# ######
#
	cmake_minimum_required(VERSION 3.1)
	project(exodus)

	include(CTest)
	enable_testing()

	#use make VERBOSE=1 to debug make

#
# INIT
# ####
#

#
# CPack
# #####
#
	#update version in mv.h and libexodus/exodus/CMakeLists.txt
	set(CPACK_PACKAGE_CONTACT neo)
	set(CPACK_PACKAGE_VERSION 22.09.1)
	INCLUDE(CPack)

#
# C++ standard
# ############
#
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)

#
# Optimisation
# ############
#
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3") # 5% faster than -O2
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2") # little difference from -O1 running test_main
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O1") ##### remove -O1 for gdb to debug into libexodus #####
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og") # 5% slower than -01 but supposedly better debugging
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

#
# Turn off some warnings
# ######################
#
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-warning-option")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-cast-function-type -Wno-maybe-uninitialized")
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas") #switched off to avoid various warnings
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")           #switched off to avoid warning about elvis operator ?:
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-pie")

	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wextra -g -march=native")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden") # Force g++ to hide by default
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")               # Link time optimisation. very slow and no observed performance increase

#
# Coverage testing
# ################
#
	# See https://gcc.gnu.org/onlinedocs/gcc/Gcov-Data-Files.html
	#
	# test-coverage = generate gcno files at compile time
	# profile-arcs = generate gcda files at runtime
	#
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs")

#
# Sanitizer
# #########
#
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

#
# Profiling
# #########
#
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")

#
# Output
# ######
#
	message("CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
	message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
	message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")

	add_subdirectory(exodus/libexodus/exodus)
	add_subdirectory(exodus/libpgexodus/src)
	add_subdirectory(cli/src)
	add_subdirectory(test/src)

	#uninstall target
	configure_file(
	    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	    IMMEDIATE @ONLY)

	add_custom_target(uninstall
	    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


#
# EXIT
# ####
#
