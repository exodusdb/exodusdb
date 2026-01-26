##################################
# Create std.pcm in the build directory
# and target std_module
# use std_module as a dependency
# on all targets using "import std;"
##################################

#
# Assume variables like CLANG_VER, CPPSTD, STDLIB, STD_CPPM_FILE, INSTALL_PREFIX are defined
# in the parent CMakeLists.txt and propagated via variables or target properties.
# If not, you can set them here or pass them as cache variables.
#

	set(CPPSTD c++${CMAKE_CXX_STANDARD} CACHE STRING "C++ standard for std module")
	set(STDLIB libc++ CACHE STRING "Standard library (libc++ or libstdc++)")

	# Try to locate std.cppm intelligently, fall back to your known path
	string(REGEX MATCH "^[0-9]+" CLANG_MAJOR_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
	find_file(STD_CPPM_FILE std.cppm
		PATHS
			"/usr/lib/llvm-${CLANG_MAJOR_VERSION}/share/libc++/v1"
#			"/usr/share/libc++/v1"
#			"/usr/lib/llvm-22/share/libc++/v1"   # your original fallback
		DOC "Path to libc++ std module interface file (std.cppm)"
	)
	if(NOT STD_CPPM_FILE)
#		set(STD_CPPM_FILE "/usr/lib/llvm-22/share/libc++/v1/std.cppm" CACHE FILEPATH "...")
		if(NOT EXISTS "${STD_CPPM_FILE}")
			message(FATAL_ERROR "Cannot find std.cppm — please set STD_CPPM_FILE manually")
		endif()
	endif()

	# Use the compiler CMake actually selected
	set(CLANG_CXX "${CMAKE_CXX_COMPILER}")

	# Output paths — now inside the build directory
	set(PCM_OUTPUT	"${CMAKE_CURRENT_BINARY_DIR}/std.pcm")
	set(CPPSTD_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cppstd")

	# Export build-tree path for consumers
	set(STD_PCM_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)

#   set(TESTFILE "${CMAKE_CURRENT_BINARY_DIR}/test_clang")
	set(TESTFILE "${CMAKE_CURRENT_SOURCE_DIR}/test_clang")
	set(VERIFY_DONE "${CMAKE_CURRENT_BINARY_DIR}/verify.done")

#
# Custom command to precompile std.pcm (in build dir)
#
	add_custom_command(
		OUTPUT ${PCM_OUTPUT}
		COMMAND ${CLANG_CXX} -std=${CPPSTD} -stdlib=${STDLIB} -Wno-reserved-identifier -Wno-reserved-module-identifier --precompile -o ${PCM_OUTPUT} ${STD_CPPM_FILE}
		DEPENDS ${STD_CPPM_FILE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Precompiling std module"
		VERBATIM
	)

#
# Custom command to create cppstd file (in build dir)
#
	add_custom_command(
		OUTPUT ${CPPSTD_OUTPUT}
		COMMAND echo ${CPPSTD} > ${CPPSTD_OUTPUT}
		DEPENDS ${PCM_OUTPUT}  # Ensure it runs after PCM is ready
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Creating cppstd file"
		VERBATIM
	)

#
# Custom command to generate test.cpp, compile, run, verify, and clean up
#
	add_custom_command(
		OUTPUT ${VERIFY_DONE}
		COMMAND ${CMAKE_COMMAND} -E touch ${VERIFY_DONE}
		COMMAND ${CLANG_CXX} -std=${CPPSTD} -stdlib=${STDLIB} -fprebuilt-module-path=${CMAKE_CURRENT_BINARY_DIR} -o ${TESTFILE} ${TESTFILE}.cpp
		COMMAND ${TESTFILE}
		COMMAND ${TESTFILE} | grep "Clang-${CLANG_MAJOR_VERSION}"
		COMMAND ${CMAKE_COMMAND} -E touch ${VERIFY_DONE}
#	   COMMAND ${CMAKE_COMMAND} -E remove ${TESTFILE}.cpp ${TESTFILE}
		DEPENDS ${PCM_OUTPUT} ${CPPSTD_OUTPUT}  # Ensure PCM and cppstd are ready
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Verifying std module precompilation"
		VERBATIM
	)

#
# Custom target that other targets can depend on
#
	add_custom_target(std_module ALL
		DEPENDS ${PCM_OUTPUT} ${CPPSTD_OUTPUT} ${VERIFY_DONE}
	)

#
# Install the generated std.pcm and cppstd to ${CMAKE_INSTALL_PREFIX}/lib
# (run with cmake --install . or make install / ninja install)
#
	install(FILES
		${PCM_OUTPUT}
		${CPPSTD_OUTPUT}
		DESTINATION lib
		COMPONENT std_module
	)

# In your parent CMakeLists.txt, add_subdirectory(exostd)
# Then, for other targets: add_dependencies(your_other_target std_module)
#
# To tell your other targets where to find std.pcm during build:
#   target_compile_options(your_target PRIVATE
#	   "-fprebuilt-module-path=${STD_PCM_BUILD_DIR}"
#   )
#
# This ensures std_module is built first when building your_other_target.
# You can control ABI flags by setting CMAKE_CXX_FLAGS or target_compile_options on the relevant targets.
# Note: The verification runs during build time, so if it fails (e.g., grep doesn't find the string), the build will fail.