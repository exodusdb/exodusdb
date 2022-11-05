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
	#CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a"
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)

#
# Build type
# ##########
#
	# Can be initialised by 'cmake -DCMAKE_CXX_FLAGS=XYZ'
	#add_compile_options(-march=native)
	add_compile_options(-march=native)
	#add_compile_options(-no-pie)

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
	message("CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")
	message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
	message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
	message("CMAKE_CXX_FLAGS_MINSIZEREL is ${CMAKE_CXX_FLAGS_MINSIZEREL}")
	message("CMAKE_CXX_FLAGS_RELWITHDEBINFO is ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

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

	add_compile_options(-ffunction-sections -fdata-sections) # linker reduces size of output
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
# Warnings
# ########
#
	add_compile_options(-Wall -Wextra -Wconversion)
	#add_compile_options(-Wno-unused-result) #switch off nodiscard warnings

	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")

		# GCC
		add_compile_options(-Wno-cast-function-type)
		add_compile_options(-Wno-maybe-uninitialized)
		add_compile_options(-Wno-unknown-pragmas) #switched off to avoid various warnings

	else()

		add_compile_options(-Wno-unknown-warning-option)

		## CLANG
		# Suppress GNU warning about elvis operator ?:
		# warning: use of GNU ?: conditional expression extension, omitting middle operand [-Wgnu-conditional-omitted-operand]
		add_compile_options(-pedantic)
		add_compile_options(-Wno-gnu-conditional-omitted-operand)

		add_compile_options(-Wno-bad-function-cast)
		add_compile_options(-Wno-uninitialized)
		add_compile_options(-Wno-unused-value) ## allow things like 'osremove(temposfilename) or true;'
		add_compile_options(-Wno-sign-conversion) # We dont care about signed to unsigned?
		add_compile_options(-Wno-unknown-attributes) # likely unlikely
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
# Subdirs
# #######
#
	message("CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")

	add_subdirectory(exodus/libexodus/exodus)
	add_subdirectory(exodus/libpgexodus/src)
	add_subdirectory(cli/src)
	add_subdirectory(test/src)

#
# Uninstall
# #########
#
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
		IMMEDIATE @ONLY)

	add_custom_target(uninstall
		COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

#
# Exit
# ####
#
