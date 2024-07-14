#
# Config
# ######
#
	cmake_minimum_required(VERSION 3.16)
	project(libexodus)

	message("---------------------")
	message("-- CMAKE LIBEXODUS --")
	message("---------------------")

#
# Compiler flags
# ##############
#
	# Use CMakeList.txt options in parent directory
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undef")

	# only for g++ 9.4.0 etc. on Ubuntu 20.04
	if (CMAKE_CXX_COMPILER_VERSION MATCHES "^9.")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inline -Wno-attributes")
	endif()

#
# Timebank option -DEXODUS_TIMEBANK=ON
# ###############
#
	OPTION(EXODUS_TIMEBANK "Option Debug timers" OFF) # Enabled by default
	IF(EXODUS_TIMEBANK)
		ADD_DEFINITIONS(-DEXODUS_TIMEBANK=${EXODUS_TIMEBANK})
	ENDIF(EXODUS_TIMEBANK)

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
	#
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

#
# Postgresql
# ##########
#
	# See /usr/share/cmake-3.16/Modules/FindPostgreSQL.cmake for documentation

	message("POSTGRES")
	find_package(PostgreSQL)
	message(" PostgreSQL_LIBRARIES=${PostgreSQL_LIBRARIES}")
	message(" PostgreSQL_INCLUDEDIRS=${PostgreSQL_INCLUDE_DIRS}")
	message(" PostgreSQL_VERSION_STRING=${PostgreSQL_VERSION_STRING}")
	include_directories(${PostgreSQL_INCLUDE_DIRS})

#
# Boost
# #####
#
	# See /usr/share/cmake-3.16/Modules/FindBoost.cmake for documentation

	message("BOOST")
	set(Boost_USE_STATIC_LIBS         OFF)
	set(Boost_USE_DEBUG_LIBS          OFF)
	set(Boost_USE_MULTITHREADED       ON)
	set(Boost_USE_STATIC_RUNTIME      OFF)
	set(Boost_Boost_USE_DEBUG_RUNTIME OFF)
	find_package(Boost REQUIRED COMPONENTS date_time system regex thread locale)

	#message(" Boost_LIBRARY_DIRS=${Boost_LIBRARY_DIRS}")
	message(" Boost_LIBRARIES=${Boost_LIBRARIES}")
	message(" Boost_INCLUDEDIR=${Boost_INCLUDE_DIR}")
	message(" Boost_VERSION=${Boost_VERSION}")
	include_directories(${BOOST_INCLUDE_DIR})

#
# fast_float
############
#
	include_directories(. .. ../../../fast_float/include)

#
# Ryu
# ###
#
	find_library(RYU_LIBRARIES NAMES ryu)
	message(" RYU_LIBRARIES=${RYU_LIBRARIES}")

#
# ICU
# ###
#
#	message("OTHER")
#	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
#	#note that displayed path confusingly shows no space or ; between multiple paths if multiple are specified either by space or ;
#	message(" CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")

# cmake MODULE options?
# #####################
#
# strings `which cmake`|grep CXX_MODULE
#
#CXX_MODULES
#INTERFACE_CXX_MODULE_SETS
#CXX_MODULE_DIRS
#CXX_MODULE_SET
#CXX_MODULE_DIRS_
#CXX_MODULE_SET_
#IMPORTED_CXX_MODULES_
#CXX_MODULES_DIRECTORY
#CXX_MODULES_BMI
#CXX_MODULE_HEADERS
#CXX_MODULE
#IMPORTED_CXX_MODULES_INCLUDE_DIRECTORIES
#IMPORTED_CXX_MODULES_COMPILE_DEFINITIONS
#IMPORTED_CXX_MODULES_COMPILE_FEATURES
#IMPORTED_CXX_MODULES_COMPILE_OPTIONS
#IMPORTED_CXX_MODULES_LINK_LIBRARIES
#CMAKE_CXX_MODULE_BMI_ONLY_FLAG

# strings `which cmake`|grep _MODULE
#
#CMAKE_MODULE_LINKER_FLAGS
#CMAKE_MODULE_PATH
#CXX_MODULES
#INTERFACE_CXX_MODULE_SETS
#CXX_MODULE_DIRS
#CXX_MODULE_SET
#CXX_MODULE_DIRS_
#CXX_MODULE_SET_
#IMPORTED_CXX_MODULES_
#CMAKE_SHARED_MODULE_SUFFIX
#CMAKE_SHARED_MODULE_PREFIX
#Fortran_MODULE_DIRECTORY
#Swift_MODULE_DIRECTORY
#CXX_MODULES_DIRECTORY
#_FIND_MODULE
#NO_MODULE
#CXX_MODULES_BMI
#_MODULE_MAP_FORMAT
#_MODULE_MAP_FLAG
#$DYNDEP_MODULE_MAP_FILE
#$SWIFT_MODULE
#$SWIFT_MODULE_NAME
#CMAKE_SHARED_MODULE_LOADER_
#CXX_MODULE_HEADERS
#LOCAL_MODULE := 
#_CREATE_SHARED_MODULE
#CXX_MODULE
#set(CMAKE_MODULE_PATH "%s")
#CMAKE_CXX_SCAN_FOR_MODULES
#CMAKE_SHARED_MODULE_CREATE_${LANG}_FLAGS
#CMAKE_SHARED_MODULE_${LANG}_FLAGS
# (found via CMAKE_MODULE_PATH) which shadows 
#UseSWIG honors SWIG_MODULE_NAME via -module flag.
#IMPORTED_CXX_MODULES_INCLUDE_DIRECTORIES
#IMPORTED_CXX_MODULES_COMPILE_DEFINITIONS
#IMPORTED_CXX_MODULES_COMPILE_FEATURES
#IMPORTED_CXX_MODULES_COMPILE_OPTIONS
#IMPORTED_CXX_MODULES_LINK_LIBRARIES
#.cmake" in CMAKE_MODULE_PATH this project has asked CMake to find a package configuration file provided by "
#.cmake" found in CMAKE_MODULE_PATH.
#.cmake must either be part of this project itself, in this case adjust CMAKE_MODULE_PATH so that it points to the correct location inside its source tree.
#Or it must be installed by a package which has already been found via find_package().  In this case make sure that package has indeed been found and adjust CMAKE_MODULE_PATH to contain the location where that package has installed Find
#CMAKE_FIND_PACKAGE_WARN_NO_MODULE
#.cmake module is in CMAKE_MODULE_PATH.  Add MODULE to exclusively request Module mode and fail if Find
#find_package called without NO_MODULE option and no Find
#.cmake module is in CMAKE_MODULE_PATH.  Add NO_MODULE to exclusively request Config mode and search for a package configuration file provided by 
#.cmake available in CMAKE_MODULE_PATH.
#(Variable CMAKE_FIND_PACKAGE_WARN_NO_MODULE enabled this warning.)
#CMAKE_CXX_MODULE_BMI_ONLY_FLAG
#in a "FILE_SET TYPE CXX_MODULES" but it is not scheduled for compilation.
#  PROPERTY IMPORTED_CXX_MODULES_
#` module but it is not found in a `FILE_SET` of type `CXX_MODULES`
# is of type `CXX_MODULES` but does not provide a module interface unit or partition
#` module but must be of type `CXX_MODULES`
#Modules are supported only by Ninja, Ninja Multi-Config, and Visual Studio generators for VS 17.4 and newer.  See the cmake-cxxmodules(7) manual for details.  Use the CMAKE_CXX_SCAN_FOR_MODULES variable to enable or disable scanning.
#" has C++ sources that may use modules, but the compiler does not provide a way to discover the import graph dependencies.  See the cmake-cxxmodules(7) manual for details.  Use the CMAKE_CXX_SCAN_FOR_MODULES variable to enable or disable scanning.
#set(CMAKE_Fortran_TARGET_MODULE_DIR "
#set(CMAKE_Fortran_TARGET_BUILDING_INSTRINSIC_MODULES ON) 
#'; must be one of 'SOURCE' or 'CXX_MODULE'
#  PRIVATE FILE_SET %s TYPE CXX_MODULES BASE_DIRS "%s" FILES

#add_library(Constants)
#target_sources(Constants
#  PUBLIC
#    FILE_SET cxx_modules TYPE CXX_MODULES FILES
#    Constants.ixx
#)

#add_library(vartyp)
#target_sources(vartyp
#  PUBLIC
#    FILE_SET cxx_modules TYPE CXX_MODULES FILES
#	vartyp.cpp
#)

#
# libexodus source
##################
#
	add_library(exodus SHARED
		# Big ones first for efficient parallel compilation
		exoprog.cpp
		vardb.cpp varregex.cpp varos.cpp varos2.cpp varstr.cpp
		varb.cpp
		var.cpp dim.cpp varnum.cpp varmath.cpp exodebug.cpp
		varcompare.cpp
		varop.cpp
		varchrono.cpp
		varfuncs.cpp varioconv.cpp
		exoenv.cpp exofuncs.cpp
		varwait.cpp varunicode.cpp varposix.cpp exocallable.cpp
		#murmurhash2_64.cpp
		murmurhash3.cpp
		#naturalorder.cpp
		gettimeofday.cpp
		varoshandle.cpp vardbconn.cpp
		variter.cpp
		varput.cpp
		timebank.cpp
		extract_v2.cpp # Duplicated in pgexodus as extract.c
		extract_v3.cpp # Failed attempt at optimisation
		extract_v4.cpp # Failed attempt at optimisation
		# Little ones last for quick parallel finishing
		term_getkey.cpp term_haskey.cpp term_echo.cpp term_cursor.cpp
	)

#
# libexodus modules
# #################
#
	target_sources(exodus
		PUBLIC
		#MUST include ONLY IF using c++ modules
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		# exports vartyp
		vartyp.cpp
	)

#target_sources(exodus
#  PUBLIC
#    FILE_SET
#      CXX_MODULES
#      FILES
#        ${CMAKE_CURRENT_SOURCE_DIR}/vartyp.ixx
##        ${CMAKE_CURRENT_SOURCE_DIR}/Constants.ixx
#)

#
# link libs
# #########
#
	#dl is needed for dynamic link library code
	if (RYU_LIBRARIES STREQUAL "RYU_LIBRARIES-NOTFOUND")
		target_link_libraries(exodus ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} dl)
	else()
		target_link_libraries(exodus ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} dl ${RYU_LIBRARIES})
	endif()

	# This is probably not needed here and only required by cli compile.cpp linkoptions?
	target_link_libraries(exodus ${FMT_LIBRARIES})
	include_directories(exodus ${FMT_INCLUDE})

set_target_properties(exodus
  PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

#
# Request .so versioning e.g. /usr/local/lib/libexodus.so -> libexodus.so.24.07
# ######################
#
	#set_target_properties(exodus PROPERTIES SOVERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_VERSION}")
	set_target_properties(exodus PROPERTIES SOVERSION "24.07")

#
# Install libexodus.so
# ####################
#
	# CMAKE_INSTALL_PREFIX defaults to /usr/local/
	install(
		TARGETS exodus
		DESTINATION lib
	)
#
# Install modules
# ###############
#
	install(
		FILES
		${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/exodus.dir/vartyp.pcm
		DESTINATION lib
	)

#
# Install headers
# ###############
#
	install(
		FILES
			varb.h
			var.h
			vartyp.h
			varerr.h
			varfriends.h
			dim.h
			rex.h
			exodus.h
			range.h
			exoimpl.h
			exomacros.h
			exoenv.h
			exofuncs.h
			#mvimpl.h mvipc.h mvconfig.h mvlink.h
			exocallable.h
			program.h
			library.h
			dict.h
			common.h
			exoprog.h
			ioconv_custom.h
			timebank.h
			printtx.hpp
			htmllib2.h
		DESTINATION include/exodus
	)

#
# Install timebank info
# #####################
#
	install (
		FILES timebank.txt
		DESTINATION share/exodus
	)

#
# Trigger ldconfig
# ################
#
	# In libexodus and cli now that both install into lib
	# Is this really necessary or should cmake be doing it anyway?
	install(
		CODE "
	        execute_process(
	            COMMAND
	                ldconfig
	                OUTPUT_VARIABLE OUTVAR
	                ERROR_VARIABLE ERRVAR
	                RESULT_VARIABLE RESVAR
	        )
	        message(
				ldconfig <\${OUTVAR}>\n
	            <\${ERRVAR}>\n
	            <\${RESVAR}>\n
	        )
		"
	)

# Cleanup
# #######
#
	# Remove any old files that might shadow the files we installed in /usr/
	install(
		CODE "execute_process(COMMAND rm -f $ENV{HOME}/inc/printtx.hpp)"
	)

#
# Exit
# ####
#
#	message("--LIBEXODUS CMAKE EXIT --")
