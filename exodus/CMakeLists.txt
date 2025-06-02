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
# BOOST FIBER OPTION -DEXO_MODULE=ON/OFF or enabled by default
# ##################
#
# Requires: apt install libboost-fiber-dev libboost-context-dev
#
	option(EXO_BOOST_FIBER "Build with Boost fiber async" ON)
	message("-- EXO_BOOST_FIBER is ${EXO_BOOST_FIBER}")

	if (EXO_BOOST_FIBER)
		add_compile_definitions(EXO_BOOST_FIBER=1)
	endif()

#
# -DEXO_PREPARED=ON/OFF
# #####################
#
	option(EXO_PREPARED "DB read/write/delete use SQL PREPARED statements" ON)
	message("-- EXO_PREPARED is ${EXO_PREPARED}")

	if (EXO_PREPARED)
		add_compile_definitions(EXO_PREPARED=1)
	endif()

#
# Compiler flags
# ##############
#
	# Use CMakeList.txt options in parent directory
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undef")

	# only for g++ 9.4.0 etc. on Ubuntu 20.04
	if (CMAKE_CXX_COMPILER_VERSION MATCHES "^9.")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inline -Wno-attributes")
	endif()

    #// Required to allow exodebug to get absolute addresses instead of function name + offset
    #// bad: "/root/lib/libnlist.so(_ZN11_ExoProgram4mainEv+0x403) [0x7aa6aa9c3ec3]"
    # "-fvisibility-inlines-hidden";
    add_compile_options("-fvisibility-inlines-hidden")

    add_compile_options("-fno-omit-frame-pointer" "-ffunction-sections" "-fdata-sections")

#
# Timebank option -DEXO_TIMEBANK=ON
# ###############
#
	option(EXO_TIMEBANK "Option Debug timers" OFF) # Disabled by default
	if(EXO_TIMEBANK)
		add_definitions(-DEXO_TIMEBANK=${EXO_TIMEBANK})
	endif(EXO_TIMEBANK)

#
# Patch ostream - See vardefs.h for more info.
# #############
#
# Similar code in install.sh and CMakeLists.txt - KEEP IN SYNC
# Warning. Using sudo which is strange inside cmake.
# Warning. Only doing c++ 14 here. TODO patch all.
#
#	execute_process(
#		COMMAND
#			sudo test -f  /usr/include/c++/14/ostream && sed -i "s|# include <format>|# ifndef EXO_FORMAT\\n#  include <format>\\n# endif|" /usr/include/c++/14/ostream
#		COMMAND
#			sudo test -f  /usr/include/c++/14/ostream && sed -i "s|#if __cpp_lib_print|#if !defined(EXO_FORMAT) \\&\\& __cpp_lib_print|" /usr/include/c++/14/ostream
#		VERBATIM
#	)
execute_process(
    COMMAND
        sh -c "sudo test -f /usr/include/c++/14/ostream && sudo sed -i 's|# include <format>|# ifndef EXO_FORMAT\\n#  include <format>\\n# endif|' /usr/include/c++/14/ostream"
    COMMAND
        sh -c "sudo test -f /usr/include/c++/14/ostream && sudo sed -i 's|#if __cpp_lib_print|#if !defined(EXO_FORMAT) \\&\\& __cpp_lib_print|' /usr/include/c++/14/ostream"
)

#
# Coverage testing
# ################
#
	# See https://gcc.gnu.org/onlinedocs/gcc/Gcov-Data-Files.html
	#
	# test-coverage = generate gcno files at compile time
	# profile-arcs = generate gcda files at runtime
	#
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs")

	#
	# Sanitizer
	#
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

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

#	find_package(Boost REQUIRED COMPONENTS date_time system regex thread locale)
    if (EXO_BOOST_FIBER)
		find_package(Boost REQUIRED COMPONENTS date_time system regex thread locale fiber context)
	else()
		find_package(Boost REQUIRED COMPONENTS date_time system regex thread locale)
	endif()

	#message(" Boost_LIBRARY_DIRS=${Boost_LIBRARY_DIRS}")
	message(" Boost_LIBRARIES=${Boost_LIBRARIES}")
	message(" Boost_INCLUDEDIR=${Boost_INCLUDE_DIR}")
	message(" Boost_VERSION=${Boost_VERSION}")

	include_directories(${BOOST_INCLUDE_DIR})

#	find_library(LIBUV_LIBRARY NAMES uv libuv uv1 libuv1)
#	link_libraries(uv)

#	find_package(PkgConfig REQUIRED)
#	pkg_check_modules(LIBUV REQUIRED libuv)
# find_library(LIBUV_LIBRARY NAMES uv libuv)
# find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
#
# fast_float
############
#
	include_directories(. .. ../fast_float/include)

##
## fmt
## ###
##
	include_directories(${FMT_INCLUDE})

#
# Ryu
# ###
#
#	Only needed for speed on older compilers pre Ubuntu 22.04 which dont have floating point to_chars
#	See varnum.cpp
#	#if __GNUC__ >= 11 || __clang_major__ >=  14
#		#define EXO_USE_TO_CHARS
	message("BOOST")

	find_library(RYU_LIBRARIES NAMES ryu)

	message(" RYU_LIBRARIES=${RYU_LIBRARIES}")

#
# exodus own include
# ##################
#
	include_directories(exodus)

#
# module std
# ##########
#
	if (EXO_MODULE)
		add_library(std)
		target_sources(std
			PUBLIC
			FILE_SET modules_public
			TYPE CXX_MODULES
			FILES
				std.cppm
		)
		target_compile_options(std PRIVATE -Wno-include-angled-in-module-purview -Wno-reserved-module-identifier)
	#	target_link_libraries(std allstd)
	endif()

#
# module?
# #######
#
	if (EXO_MODULE)
		add_library(exovar)
		target_include_directories(exovar PRIVATE ${PostgreSQL_INCLUDE_DIRS} ${BOOST_INCLUDE_DIR})
		target_include_directories(exovar PRIVATE ${FMT_INCLUDE})
		#target_include_directories(exovar PRIVATE exodus/include)
		#target_compile_features(${NAME} PUBLIC cxx_std_23)
		target_sources(exovar
			PUBLIC
			FILE_SET modules_public
			TYPE CXX_MODULES
			FILES
	#			exo.cppm
				var.cppm
	#			dim.cppm
		)
		target_link_libraries(exovar std)
#		target_link_libraries(exovar ${FMT_LIBRARIES})
		target_link_libraries(exovar fmt::fmt)
	endif()
#
# module?
# #######
#
	if (EXO_MODULE)
		add_library(exoprog)
		target_include_directories(exoprog PRIVATE ${PostgreSQL_INCLUDE_DIRS} ${BOOST_INCLUDE_DIR})
		target_include_directories(exoprog PRIVATE ${FMT_INCLUDE})
		#target_include_directories(exoprog PRIVATE exodus/include)
		#target_compile_features(exoprog PUBLIC cxx_std_23)
		target_sources(exoprog
			PUBLIC
			FILE_SET modules_public
			TYPE CXX_MODULES
			FILES
				exoprog.cppm
		)
		target_link_libraries(exoprog std)
		target_link_libraries(exoprog exovar)
#		target_link_libraries(exovar fmt::fmt)
	endif()
#
# libexodus source
##################
#
	add_library(exodus SHARED
#	)
#	target_sources(exodus
#		PUBLIC
		# Big ones first for efficient parallel compilation
		exoprog.cpp
		vardb.cpp varregex.cpp varos.cpp varos2.cpp varstr.cpp
		varb.cpp
		var.cpp dim.cpp
		varnum.cpp varmath.cpp exodebug.cpp
		varcompare.cpp
		varop.cpp
		varop2.cpp
		varprocess.cpp
		varchrono.cpp
		varfuncs.cpp varioconv.cpp
		exoenv.cpp exofuncs.cpp

		vardb_async.cpp
		task_scheduler.cpp
		task_mutex.cpp
		task_manager.cpp

		DBresult.cpp
		DBconn.cpp
		DBpool.cpp

		job.cpp
		job_manager.cpp
		threadsafequeue.cpp
		threadpool.cpp

		varlisted.cpp

		varwait.cpp varunicode.cpp varposix.cpp exocallable.cpp
		#murmurhash2_64.cpp
		murmurhash3.cpp
		#naturalorder.cpp
		gettimeofday.cpp
		varoshandle.cpp
		variter.cpp
#		dimiter.cpp
		varput.cpp
		timebank.cpp
		extract_v2.cpp # Duplicated in pgexodus as extract.c
		extract_v3.cpp # Failed attempt at optimisation
		extract_v4.cpp # Failed attempt at optimisation
		# Little ones last for quick parallel finishing
		term_getkey.cpp term_haskey.cpp term_echo.cpp term_cursor.cpp
		readline.cpp
		reset_range.cpp
	)

##
## libexodus modules
## #################
##
#	target_sources(exodus
#		PUBLIC
#		#MUST include ONLY IF using c++ modules
#		FILE_SET cxx_modules TYPE CXX_MODULES FILES
#		# exports vartyp
#		vartyp.cpp
#	)

#
# link libs
# #########
#
	# Boost and Postgresql
	target_link_libraries(exodus
		${Boost_LIBRARIES}
		${PostgreSQL_LIBRARIES}
	)

	if (EXO_MODULE)
		target_link_libraries(exodus
			exovar
			exoprog
		)
	endif()

	# dl
	# ##
	#
	# dl is needed for dynamic link library code supporting dlopen() and dysym()
	# that is used to load exoprog .so programs/functions/libraries as required at runtime
	target_link_libraries(exodus
		dl
	)

	# ryu
	# ###
	#
	if (NOT RYU_LIBRARIES STREQUAL "RYU_LIBRARIES-NOTFOUND")
		target_link_libraries(exodus
			${RYU_LIBRARIES}
		)
	endif()

#	# fmt
#	# ###
#	#
#	# This is probably not needed here. Maybe required by cli compile.cpp linkoptions?
#	target_link_libraries(exodus ${FMT_LIBRARIES})
	target_link_libraries(exodus fmt::fmt)


#
# readline
##########
#
	target_link_libraries(exodus
			readline
	)

#
# Request .so versioning e.g. /usr/local/lib/libexodus.so -> libexodus.so.24.07
# ######################
#
	#set_target_properties(exodus PROPERTIES SOVERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_VERSION}")
	set_target_properties(exodus PROPERTIES SOVERSION "24.07")

#
# Create timebank.txt from timebank.h
# ###################################
#
	add_custom_command(
	    OUTPUT timebank.txt
	    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ./timebank.h.to_txt.sh
	    DEPENDS timebank.h
	    COMMENT "Updating timebank.txt if necessary"
	)

# Custom target to make this command part of the build system
#
	add_custom_target(
	    update_timebank_txt
	    ALL
	    DEPENDS timebank.txt
	)

# Optionally, if you want this to run every time you build:
#
	add_dependencies(exodus update_timebank_txt)


#
# Install libexodus.so and exovar
# ####################
#
# CMAKE_INSTALL_PREFIX defaults to /usr/local/

	install(
		TARGETS exodus
		DESTINATION lib
	)

	if (EXO_MODULE)
		install(
			TARGETS std exovar exoprog
			DESTINATION lib
		)
	endif()

##
## Install modules pcm?
## ###############
##
#	install(
#		FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/exodus.dir/vartyp.pcm
#		DESTINATION lib
#	)


# Install headers - non module
# ###############
#
	if (NOT EXO_MODULE)
		install(
			FILES
				varb.h
				var.h
				vartyp.h
				varerr.h
				varb_friends.h
				dim.h
				job.h
				job_manager.h
				task_manager.h
				rex.h
			DESTINATION include/exodus
		)
	endif()

#
# Install headers
# ###############
#
	install(
		FILES

#			varb.h
#			var.h
#			vartyp.h
#			varerr.h
#			varb_friends.h
#			dim.h
#			rex.h

			vardefs.h
			exodus.h
			range.h
			exoimpl.h
			exomacros.h
			exoenv.h
			threadsafequeue.h
			exofuncs.h
			format.h
			#mvimpl.h mvipc.h mvconfig.h mvlink.h
			program.h
			library.h
			dict.h
			common.h
			exoprog.h
			exocallable.h
			job.h
			job_manager.h
			task_manager.h
			threadpool.h
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
