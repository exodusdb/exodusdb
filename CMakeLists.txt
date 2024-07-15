#use make VERBOSE=1 to debug make
	message("------------------")
	message("-- CMAKE EXODUS --")
	message("------------------")
#
# CONFIG
# ######
#
#	cmake_minimum_required(VERSION 3.10)
	cmake_minimum_required(VERSION 3.28)
	project(exodus)

#	cmake versions available 2024-06-10
#
#	Ubuntu       24.04   22.04   20.04
#
#	snap cmake  3.29.5  3.29.5  3.29.5
#	max c++         26      26     26
#
#	apt  cmake  3.28.3  3.22.1  3.16.3
#   max c++         26      23      20
#
#   g++             13      11       9
#   max c++         26      23      20
#
#   g++max          14      12      10
#
# CTest
# #####
#
	include(CTest)
	enable_testing()

#
# CPack
# #####
#
	#update version in mv.h and libexodus/exodus/CMakeLists.txt
	set(CPACK_PACKAGE_CONTACT neo)
	set(CPACK_PACKAGE_VERSION 24.07.1)
	INCLUDE(CPack)

#
# C++ standard
# ############
#
	# CMake max supporting C++ versions
	#https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
	#CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a"
	if(CMAKE_VERSION VERSION_LESS "3.12.0")
		#set(CMAKE_CXX_STANDARD 20)
		#try building without specifying c++ version
		# but exodus requires c++20 minimum

	elseif(CMAKE_VERSION VERSION_LESS "3.20.0")
		set(CMAKE_CXX_STANDARD 20)

	elseif(CMAKE_VERSION VERSION_LESS "3.25.0")
		set(CMAKE_CXX_STANDARD 23)

	else()
		set(CMAKE_CXX_STANDARD 26)

	endif()

	message("-------------------------------------------------")

	# man cmake-cxxmodules
	# See CMP0155 scanning defaults to yes for c++20 files from cmake 3.28 onwards
	# Why is this required then?
#	set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
	set(CMAKE_CXX_SCAN_FOR_MODULES ON)
	message("CMAKE_CXX_SCAN_FOR_MODULES is ${CMAKE_CXX_SCAN_FOR_MODULES}")

	# Allow lower C++ versions
	# TODO check minimum c++20
	#set(CMAKE_CXX_STANDARD_REQUIRED ON)

	set(CMAKE_CXX_EXTENSIONS OFF)

	message("CMAKE_CXX_COMPILER_VERSION REQUESTED is ${CMAKE_CXX_COMPILER_VERSION}")

	#set(CMAKE_CXX_MODULE_EXTENSIONS ".cpp")
#	set(CMAKE_CXX_MODULE_EXTENSIONS ".cpp" ".h")
#	set(CMAKE_CXX_MODULE_EXTENSIONS ".ixx" ".h")

	message("-------------------------------------------------")
#
# Build type
# ##########
#
#	# Can be initialised by 'cmake -DCMAKE_CXX_FLAGS=XYZ'
#	#add_compile_options(-march=native)
#	add_compile_options(-march=native)
#	#add_compile_options(-no-pie)

	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
			"Default build type: RelWithDebInfo" FORCE)
	endif()

	message("CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
	#-DCMAKE_BUILD_TYPE=DEBUG
	#-DCMAKE_BUILD_TYPE=MINSIZEREL
	#-DCMAKE_BUILD_TYPE=RELEASE
	#-DCMAKE_BUILD_TYPE=RELWITHDEBINFO
	#
	#From CMakeCache.txt
	#//Flags used by the CXX compiler during all build types.
	#CMAKE_CXX_FLAGS:STRING=
	#
	#//Flags used by the CXX compiler during DEBUG builds.
	#CMAKE_CXX_FLAGS_DEBUG:STRING=-g
	#
	#//Flags used by the CXX compiler during MINSIZEREL builds.
	#CMAKE_CXX_FLAGS_MINSIZEREL:STRING=-Os -DNDEBUG
	#
	#//Flags used by the CXX compiler during RELEASE builds.
	#CMAKE_CXX_FLAGS_RELEASE:STRING=-O3 -DNDEBUG
	#
	#//Flags used by the CXX compiler during RELWITHDEBINFO builds.
	#CMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=-O2 -g -DNDEBUG
	message("CMAKE_CXX_FLAGS is                ${CMAKE_CXX_FLAGS}")
	message("CMAKE_CXX_FLAGS_DEBUG is          ${CMAKE_CXX_FLAGS_DEBUG}")
	message("CMAKE_CXX_FLAGS_RELEASE is        ${CMAKE_CXX_FLAGS_RELEASE}")
	message("CMAKE_CXX_FLAGS_MINSIZEREL is     ${CMAKE_CXX_FLAGS_MINSIZEREL}")
	message("CMAKE_CXX_FLAGS_RELWITHDEBINFO is ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

	message("-------------------------------------------------")

	add_compile_options(-Wno-unknown-warning-option)
	#add_compile_options(-O3)   # 5% faster than -O2
	#add_compile_options(-O2)   # little difference from -O1 running test_main
	#add_compile_options(-O1)   # gdb might not step into libexodus?
	#add_compile_options(-Og)   # 5% slower than -01 but supposedly better debugging
	#add_compile_options(-O0)
	#add_compile_options(-flto) # Link time optimisation. Makes linking of libexodus.so very slow

#
# Optimise runtime linking speed
# ##############################
#
	#add_compile_options(-fvisibility=hidden) # Force g++ to hide all lib definitions by default and require PUBLIC
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fvisibility=hidden) # Force g++ to hide all lib definitions by default and require PUBLIC

#
# Optimise binary size
# ####################
#

#	add_compile_options(-ffunction-sections -fdata-sections) # linker reduces size of output
	#add_compile_options(-g)

#
# Libraries
# #########
#
	# clang
	if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		# This requires boost to be be compiled with clang
		#https://www.boost.org/doc/libs/1_80_0/more/getting_started/unix-variants.html
		#
		#./bootstrap.shwith-toolset=clang
		#./b2 clean
		#./b2 toolset=clang cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++"
		#add_compile_options(-stdlib=libc++) #24 bytes strings. apt install libc++-dev libc++abi-dev
	endif()

#
# Acquire and pass definitions for OS_NAME and OS_VERSION e.g. Ubuntu, 24.04

	execute_process(
		COMMAND lsb_release -rs
		OUTPUT_VARIABLE OS_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	MESSAGE(STATUS "OS_VERSION = ${OS_VERSION}")
	ADD_DEFINITIONS( -DEXO_OS_VERSION=\"${OS_VERSION}\" )

	execute_process(
		COMMAND lsb_release -is
		OUTPUT_VARIABLE OS_NAME
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	MESSAGE(STATUS "OS_NAME = ${OS_NAME}")
	ADD_DEFINITIONS( -DEXO_OS_NAME=\"${OS_NAME}\" )

##
## FMT - before setting complex compiler warnings for exodus subdirs/modules
## ###
##
#	if (${CMAKE_CXX_COMPILER_VERSION} STRLESS "13")
		message("----------------")
		message("-- FMT EXODUS --")
		message("----------------")
		# See https://fmt.dev/latest/usage.html
		# How to build
		set(BUILD_SHARED_LIBS true)
		set(FMT_PEDANTIC true)
		add_subdirectory(fmt)
		set(FMT_LIBRARIES fmt)
#	else()
#		message("Omitting subdir fmt")
#	endif()

#
# Warnings
# ########
#
#	add_compile_options(-Wall -Wextra -Wconversion)
#	add_compile_options(-Winline)
	#switch off nodiscard warnings
	#add_compile_options(-Wno-unused-result)

#	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
#
#		# GCC
#		add_compile_options(-Wno-cast-function-type)
#		add_compile_options(-Wno-maybe-uninitialized)
#		add_compile_options(-Wno-unknown-pragmas)
#
#		# only for g++ 9.4.0 etc. on Ubuntu 20.04
#		if (CMAKE_CXX_COMPILER_VERSION MATCHES "^9.")
#			add_compile_options(-Wno-inline)
#		endif()
#
#	else()
#
##	add_compile_options(-Wno-undefined-inline)
##	#/root/exodus/test/src/../../exodus/libexodus/exodus/var.h:1819:7: warning: inline function 'exodus::var::isnum' is not defined [-Wundefined-inline]
##	# 1819 |         bool isnum() const;
##	#      |              ^
#
#		# only for older clang 10.* etc. on Ubuntu 20.04
##		if (CMAKE_CXX_COMPILER_VERSION MATCHES "^10.")
#			add_compile_options(-Wno-undef)
##		endif()
#
#		add_compile_options(-Wno-unknown-warning-option)
#
#		## CLANG
#		# Suppress GNU warning about elvis operator ?:
#		# warning: use of GNU ?: conditional expression extension, omitting middle operand [-Wgnu-conditional-omitted-operand]
#		add_compile_options (
#								-Weverything
#
#								-Wno-c++98-compat
#								-Wno-c++98-compat-pedantic
#								-Wno-reserved-macro-identifier
#								-Wno-disabled-macro-expansion
#								-Wno-date-time
#								-Wno-ctad-maybe-unsupported
#								-Wno-padded
#								-Wno-unreachable-code-break
#								-Wno-weak-vtables
#								-Wno-double-promotion
#								-Wno-shadow
#								-Wno-exit-time-destructors
#								-Wno-global-constructors
#								-Wno-missing-prototypes
#							#-Wno-unsafe-buffer-usage
#							# -fsafe-buffer-usage-suggestions # not available in clang 14 on Ubuntu 22.04
#								-Wno-shadow-field
#								-Wno-float-equal
#								-Wno-cast-align
#								-Wno-unused-macros
#								-Wno-missing-variable-declarations
#								-Wno-return-std-move-in-c++11
#
#								# building fmt
#								-Wno-documentation-unknown-command
#								-Wno-switch-enum
#								-Wno-undefined-func-template
#								#-Wno-reserved-id-macro
#								-Wno-c++20-compat
#
#							)
#		add_compile_options(-pedantic)
#		add_compile_options(-Wno-gnu-conditional-omitted-operand)
#
#		#add_compile_options(-Wno-bad-function-cast)
#		add_compile_options(-Wno-uninitialized)
#		#add_compile_options(-Wno-unused-value) ## allow things like 'osremove(temposfilename) or true;'
#		add_compile_options(-Wno-sign-conversion) # We dont care about signed to unsigned?
#		add_compile_options(-Wno-unknown-attributes) # likely unlikely
#		add_compile_options(-Wno-ignored-attributes) # likely unlikely
#		add_compile_options(-Wno-float-conversion) # We dont care about float to bool?
#	endif()
#
#
# Coverage testing
# ################
#
	# See https://gcc.gnu.org/onlinedocs/gcc/Gcov-Data-Files.html
	#
	# test-coverage = generate gcno files at compile time
	# profile-arcs = generate gcda files at runtime
	#
	#add_compile_options(-ftest-coverage -fprofile-arcs)

#
# Sanitizer
# #########
#
	#add_compile_options(-fsanitize=address)
	#add_compile_options(-fsanitize=undefined)

#
# Profiling
# #########
#
	#add_compile_options(-pg)

#
# Uninstall
# #########
#
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
		IMMEDIATE @ONLY
	)

	add_custom_target(uninstall
		COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
	)

#
# Exit
# ####
#

#
# Subdirs/submodules
# ##################

	add_subdirectory(exodus/libexodus/exodus)
	#target_compile_options(exodus PUBLIC -Wall)

	#add_subdirectory(exodus/libpgexodus/src)
	add_subdirectory(exodus/pgexodus)
	#target_compile_options(pgexodus PUBLIC -Wall)

	add_subdirectory(cli/src)
	#target_compile_options(nlist PUBLIC "-O2")

	add_subdirectory(test/src)
	#target_compile_options(test_Regress PUBLIC "-O2")

	message("---------------------------")
	message("-- CMAKE EXODUS FINISHED --")
	message("---------------------------")
