#
# CONFIG
# ######
#
	cmake_minimum_required(VERSION 3.16)
	project(libexodus)

#
# INIT
# ####
#
	message("---------------------")
	message("-- CMAKE LIBEXODUS --")
	message("---------------------")

	#Use CMakeList.txt options in parent directory
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undef")

	# only for g++ 9.4.0 etc. on Ubuntu 20.04
	if (CMAKE_CXX_COMPILER_VERSION MATCHES "^9.")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inline -Wno-attributes")
	endif()
#
# Add TIMEBANK option
# ###################
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
# POSTGRES
# ########
#
	# See /usr/share/cmake-3.16/Modules/FindPostgreSQL.cmake for documentation

	message("POSTGRES")
	find_package(PostgreSQL)
	message(" PostgreSQL_LIBRARIES=${PostgreSQL_LIBRARIES}")
	message(" PostgreSQL_INCLUDEDIRS=${PostgreSQL_INCLUDE_DIRS}")
	message(" PostgreSQL_VERSION_STRING=${PostgreSQL_VERSION_STRING}")
	include_directories(${PostgreSQL_INCLUDE_DIRS})

#
# BOOST
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
# ICU
# ###
#
	message("OTHER")
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
	#note that displayed path confusingly shows no space or ; between multiple paths if multiple are specified either by space or ;
	message(" CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")

#
# RYU
# ###
#
	find_library(RYU_LIBRARIES NAMES ryu)
	message(" RYU_LIBRARIES=${RYU_LIBRARIES}")

#
# A shared library output called exodus - list all its source files
# #################################################################
#
	add_library(exodus SHARED
		# Big ones first for efficient parallel compilation
		exoprog.cpp
		vardb.cpp varregex.cpp varos.cpp varos2.cpp varstr.cpp
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
# Specify what libraries are to be linked in when building libexodus
# ##################################################################
#
	include_directories(. .. ../../../fast_float/include)
	#dl is needed for dynamic link library code
	if (RYU_LIBRARIES STREQUAL "RYU_LIBRARIES-NOTFOUND")
		target_link_libraries(exodus ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} dl)
	else()
		target_link_libraries(exodus ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} dl ${RYU_LIBRARIES})
	endif()

# This is probably not needed here and only required by cli compile.cpp linkoptions?
#
	target_link_libraries(exodus ${FMT_LIBRARIES})
	include_directories(exodus ${FMT_INCLUDE})

#
# State that the output libexodus should be installed in the default system library
# #################################################################################
#

	# CMAKE_INSTALL_PREFIX defaults to /usr/local/
	install(
		TARGETS exodus
		DESTINATION lib
	)

	#set_target_properties(exodus PROPERTIES SOVERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_VERSION}")
	set_target_properties(exodus PROPERTIES SOVERSION "24.07")

#
# Install some of exodus' include files so programs can be written in exodus c++
# ##############################################################################
#
	install(
		FILES
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

	install (
		FILES
			timebank.txt
		DESTINATION share/exodus
	)

	# In libexodus and cli now that both install into lib
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

# 6. CLEAN OUT ANY HOME DIR FILES THAT MAY SHADOW THE REAL FILE IN /usr/local/...
#################################################################################
#
    install(
        CODE "
            execute_process(
	            COMMAND
    	            rm -f $ENV{HOME}/inc/printtx.hpp
            )
        "
    )

#
# EXIT
# ####
#
#	message("--LIBEXODUS CMAKE EXIT --")
