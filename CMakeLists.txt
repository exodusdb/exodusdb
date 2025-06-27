#use make VERBOSE=1 to debug make
	message("------------------")
	message("-- CMAKE EXODUS --")
	message("------------------")
#
# CONFIG
# ######
#
	cmake_minimum_required(VERSION 3.10)
#	cmake_minimum_required(VERSION 3.28)
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

	message("")

	message(STATUS "CMAKE_CXX_COMPILER_ID      is ${CMAKE_CXX_COMPILER_ID}")
	message(STATUS "CMAKE_CXX_COMPILER_VERSION is ${CMAKE_CXX_COMPILER_VERSION}")
	message(STATUS "CMAKE_CXX_FLAGS            is ${CMAKE_CXX_FLAGS}")
	message("")

#
# Modules option -DEXO_MODULE=ON/OFF or enabled by default for clang >= 18
# ##############
#
	if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL 18.0)
		option(EXO_MODULE "Build as c++ modules if clang >= 18" ON) # Enabled clang >= 18
#	else()
		option(EXO_MODULE "Build as c++ modules if not clang >= 18" OFF) # Disabled by default
	endif()
	message("-- EXO_MODULE is ${EXO_MODULE}")

	add_compile_definitions(EXO_MODULE_INITIAL_BUILD=1)

	# man cmake-cxxmodules
	# See CMP0155 scanning defaults to yes for c++20 files from cmake 3.28 onwards
	# Why is this required then?
	if (EXO_MODULE)
		add_compile_definitions(EXO_MODULE=1)
		set(CMAKE_CXX_SCAN_FOR_MODULES ON)
	else()
#		add_compile_definitions(EXO_MODULE=0)
		set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
	endif()
	message(STATUS "CMAKE_CXX_SCAN_FOR_MODULES is ${CMAKE_CXX_SCAN_FOR_MODULES}")
	message("")

	# Allow lower C++ versions
	# TODO check minimum c++20
	#set(CMAKE_CXX_STANDARD_REQUIRED ON)

	set(CMAKE_CXX_EXTENSIONS OFF)

	#set(CMAKE_CXX_MODULE_EXTENSIONS ".cpp")
#	set(CMAKE_CXX_MODULE_EXTENSIONS ".cpp" ".h")
#	set(CMAKE_CXX_MODULE_EXTENSIONS ".ixx" ".h")

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

	message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
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
	message("")
	message("   CMAKE_CXX_FLAGS_DEBUG is          ${CMAKE_CXX_FLAGS_DEBUG}")
	message("   CMAKE_CXX_FLAGS_RELEASE is        ${CMAKE_CXX_FLAGS_RELEASE}")
	message("   CMAKE_CXX_FLAGS_MINSIZEREL is     ${CMAKE_CXX_FLAGS_MINSIZEREL}")
	message("   CMAKE_CXX_FLAGS_RELWITHDEBINFO is ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
	message("")

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

##		message("----------------")
##		message("-- FMT EXODUS --")
##		message("----------------")
#	find_package(fmt)
#	#target_link_libraries(<your-target> fmt::fmt)

###
### FMT - before setting complex compiler warnings for exodus subdirs/modules
### ###
###
##	if (${CMAKE_CXX_COMPILER_VERSION} STRLESS "13")
#		message("")
#		message("----------------")
#		message("-- FMT EXODUS --")
#		message("----------------")
#		# See https://fmt.dev/latest/usage.html
#		# How to build
#		set(BUILD_SHARED_LIBS true)
#		set(FMT_PEDANTIC true)
		add_subdirectory(fmt)
#		set(FMT_LIBRARIES fmt)
##	else()
##		message("Omitting subdir fmt")
##	endif()

#
# Warnings
# ########
#
#	add_compile_options(-Wall -Wextra -Wconversion)
#	add_compile_options(-Winline)
	#switch off nodiscard warnings
	#add_compile_options(-Wno-unused-result)

	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")

		# GCC
		add_compile_options(-Wno-cast-function-type)
		add_compile_options(-Wno-maybe-uninitialized)
		add_compile_options(-Wno-unknown-pragmas)

		# Suppress some warnings for fmt while using g++14
		add_compile_options(-Walloc-size-larger-than=1GB)
		add_compile_options(-Wno-stringop-overflow)

		# only for g++ 9.4.0 etc. on Ubuntu 20.04
		if (CMAKE_CXX_COMPILER_VERSION MATCHES "^9.")
			add_compile_options(-Wno-inline)
		endif()

	else()

		add_compile_options(-pedantic)

# Removed because causes unstoppable unsafe_buffer_usage warnings in exofuncs.h in clang 18 on Ubuntu 24.04 (c++ module bug?)
#		add_compile_options(-Weverything)
		add_compile_options(-Wno-unsafe_buffer_usage)
		add_compile_options(-Wno-reserved-id-macro)
#		add_compile_options(-fsafe-buffer-usage-suggestions)  # not available in clang 14 on Ubuntu 22.04

#	add_compile_options(-Wno-undefined-inline)
#	#/root/exodus/test/src/../../exodus/libexodus/exodus/var.h:1819:7: warning: inline function 'exodus::var::isnum' is not defined [-Wundefined-inline]
#	# 1819 |         bool isnum() const;
#	#      |              ^

		# only for older clang 10.* etc. on Ubuntu 20.04
#		if (CMAKE_CXX_COMPILER_VERSION MATCHES "^10.")
			add_compile_options(-Wno-undef)
#		endif()

		add_compile_options(-Wno-unknown-warning-option)

		## CLANG
		# Suppress GNU warning about elvis operator ?:
		# warning: use of GNU ?: conditional expression extension, omitting middle operand [-Wgnu-conditional-omitted-operand]
		add_compile_options (
								-Wno-c++98-compat
								-Wno-c++98-compat-pedantic
								-Wno-reserved-macro-identifier
								-Wno-disabled-macro-expansion
								-Wno-date-time
								-Wno-ctad-maybe-unsupported
								-Wno-padded
								-Wno-unreachable-code-break
								-Wno-weak-vtables
								-Wno-double-promotion
								-Wno-shadow
								-Wno-exit-time-destructors
								-Wno-global-constructors
								-Wno-missing-prototypes
								-Wno-shadow-field
								-Wno-float-equal
								-Wno-cast-align
								-Wno-unused-macros
								-Wno-missing-variable-declarations
								-Wno-return-std-move-in-c++11

								# building fmt
								-Wno-documentation-unknown-command
								-Wno-switch-enum
								-Wno-undefined-func-template
								#-Wno-reserved-id-macro
								-Wno-c++20-compat

							)
		add_compile_options(-Wno-gnu-conditional-omitted-operand)

		#add_compile_options(-Wno-bad-function-cast)
#		add_compile_options(-Wno-uninitialized)
		#add_compile_options(-Wno-unused-value) ## allow things like 'osremove(temposfilename) or true;'
		add_compile_options(-Wno-sign-conversion) # We dont care about signed to unsigned?
		add_compile_options(-Wno-unknown-attributes) # likely unlikely
		add_compile_options(-Wno-ignored-attributes) # likely unlikely
		add_compile_options(-Wno-float-conversion) # We dont care about float to bool?
	endif()

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
# Collect lots of git info
# ########################
#

# Check if in Git repo
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    # Local branch
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE GIT_RESULT
    )

    # Local commit timestamp
    execute_process(
        COMMAND git log -1 --format=%cd --date=iso
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_LOCAL_COMMIT_TIMESTAMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Local commit hash
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_LOCAL_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Repo URL from remote origin
    execute_process(
        COMMAND git config --get remote.origin.url
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_REPO_URL_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX REPLACE "^git@github.com:" "https://github.com/" GIT_REPO_URL "${GIT_REPO_URL_RAW}")
    string(REGEX REPLACE "\\.git$" "" GIT_REPO_URL "${GIT_REPO_URL}")

    # Remote commit hash (HEAD of origin/<branch>)
    execute_process(
        COMMAND git ls-remote origin HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_REMOTE_HEAD
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX REPLACE "^([0-9a-f]+)\tHEAD" "\\1" GIT_REMOTE_COMMIT_HASH "${GIT_REMOTE_HEAD}")
    string(SUBSTRING "${GIT_REMOTE_COMMIT_HASH}" 0 9 GIT_REMOTE_COMMIT_HASH_SHORT)  # Shorten to 9 chars

    # Remote commit timestamp
    execute_process(
        COMMAND git log -1 --format=%cd --date=iso origin/${GIT_BRANCH}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_REMOTE_COMMIT_TIMESTAMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE GIT_TS_RESULT
    )

    # Fallbacks if Git commands fail
    if(NOT GIT_RESULT EQUAL 0 OR NOT GIT_BRANCH)
        set(GIT_BRANCH "unknown")
    endif()
    if(NOT GIT_LOCAL_COMMIT_TIMESTAMP)
        set(GIT_LOCAL_COMMIT_TIMESTAMP "unknown")
    endif()
    if(NOT GIT_LOCAL_COMMIT_HASH)
        set(GIT_LOCAL_COMMIT_HASH "unknown")
    endif()
    if(NOT GIT_REPO_URL)
        set(GIT_REPO_URL "unknown")
    endif()
    if(NOT GIT_TS_RESULT EQUAL 0 OR NOT GIT_REMOTE_COMMIT_TIMESTAMP)
        set(GIT_REMOTE_COMMIT_TIMESTAMP "unknown")
    endif()
    if(NOT GIT_REMOTE_COMMIT_HASH_SHORT)
        set(GIT_REMOTE_COMMIT_HASH_SHORT "unknown")
    endif()
else()
    # Tarball build—no .git/
    set(GIT_BRANCH "unknown")
    set(GIT_LOCAL_COMMIT_TIMESTAMP "unknown")
    set(GIT_LOCAL_COMMIT_HASH "unknown")
    set(GIT_REPO_URL "unknown")
    set(GIT_REMOTE_COMMIT_TIMESTAMP "unknown")
    set(GIT_REMOTE_COMMIT_HASH_SHORT "unknown")
endif()

# Define macros
add_definitions(-DGIT_BRANCH="${GIT_BRANCH}")
add_definitions(-DGIT_LOCAL_COMMIT_TIMESTAMP="${GIT_LOCAL_COMMIT_TIMESTAMP}")
add_definitions(-DGIT_LOCAL_COMMIT_HASH="${GIT_LOCAL_COMMIT_HASH}")
add_definitions(-DGIT_REPO_URL="${GIT_REPO_URL}")
add_definitions(-DGIT_REMOTE_COMMIT_TIMESTAMP="${GIT_REMOTE_COMMIT_TIMESTAMP}")
add_definitions(-DGIT_REMOTE_COMMIT_HASH_SHORT="${GIT_REMOTE_COMMIT_HASH_SHORT}")

#
# Subdirs/submodules
# ##################

	# header only
    set(FASTFLOAT_DIR fast_float)

	message("")
    set(EXODUS_DIR exodus)
    set(SRC_EXODUS_DIR ${CMAKE_SOURCE_DIR}/${EXODUS_DIR})
    message("SRC_EXODUS_DIR ${CMAKE_SOURCE_DIR}/${EXODUS_DIR}")
	#target_compile_options(exodus PUBLIC -Wall)
	add_subdirectory(exodus)

	message("")
	#target_compile_options(pgexodus PUBLIC -Wall)
	add_subdirectory(pgexodus)

	message("")
    set(CLI_DIR cli)
    set(BIN_CLI_DIR ${CMAKE_BINARY_DIR}/${CLI_DIR})
    set(SRC_CLI_DIR ${CMAKE_SOURCE_DIR}/${CLI_DIR})
	#target_compile_options(nlist PUBLIC "-O2")
	add_subdirectory(cli)

	message("")
    set(TEST_DIR test)
	#target_compile_options(test_Regress PUBLIC "-O2")
	add_subdirectory(test)

#	message("")
#	add_subdirectory(backward-cpp)

#
# Exit
# ####
#
	message("---------------------------")
	message("-- CMAKE EXODUS FINISHED --")
	message("---------------------------")
