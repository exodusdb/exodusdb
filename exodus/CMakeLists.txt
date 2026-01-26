#
# Config
# ######
#
	cmake_minimum_required(VERSION 3.16)
	project(libexodus)

	message("---------------------")
	message("-- CMAKE LIBEXODUS --")
	message("---------------------")

# Force our custom ICU location
set(ICU_ROOT "/usr/local/icu-libcxx-78.2")

# Optional: also add to prefix path for safety (helps if FindICU falls back to config mode)
list(APPEND CMAKE_PREFIX_PATH "${ICU_ROOT}")

find_package(ICU REQUIRED COMPONENTS uc i18n data)

#
# BOOST FIBER OPTION -DEXO_BOOST_FIBER=ON/OFF or enabled by default
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

	if(POLICY CMP0167)
		cmake_policy(SET CMP0167 OLD)
	endif()
	set(Boost_NO_WARN_NEW_VERSIONS ON)
	set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "Disable Boost's own CMake config mode")

    if (EXO_BOOST_FIBER)
#		find_package(Boost REQUIRED COMPONENTS date_time regex thread locale chrono atomic fiber context)
		find_package(Boost REQUIRED COMPONENTS fiber context)
	else()
#		find_package(Boost REQUIRED COMPONENTS date_time regex thread locale chrono atomic)
	endif()

	message(" Boost_LIBRARIES=${Boost_LIBRARIES}")
	message(" Boost_INCLUDEDIR=${Boost_INCLUDE_DIR}")
	message(" Boost_VERSION=${Boost_VERSION}")

	include_directories(${BOOST_INCLUDE_DIR})

# ??
############
#
	include_directories(. ..)

##
## fmt
## ###
##

	include_directories(${FMT_INCLUDE})

#
# exodus own include
# ##################
#
	include_directories(exodus)

#
# module exoprog
# ##############
#
	if (EXO_MODULE)
		add_library(exoprog)
		add_dependencies(exoprog std_module)
#		target_include_directories(exoprog PRIVATE ${PostgreSQL_INCLUDE_DIRS} ${BOOST_INCLUDE_DIR})
		target_include_directories(exoprog PRIVATE ${BOOST_INCLUDE_DIR})
#		target_include_directories(exoprog PRIVATE ${FMT_INCLUDE})
		#target_include_directories(exoprog PRIVATE exodus/include)
		#target_compile_features(exoprog PUBLIC cxx_std_23)
		target_sources(exoprog
			PUBLIC
			FILE_SET modules_public
			TYPE CXX_MODULES
			FILES
				exoprog.cppm
		)
#		if (EXO_MODULE EQUAL 2)
#			target_link_libraries(exoprog std)
#		endif()
		target_link_libraries(exoprog exovar)
		if (EXO_FORMAT EQUAL 2)
			target_link_libraries(exoprog fmt)
		endif()
		if (EXO_FORMAT EQUAL 3)
#			target_link_libraries(exoprog ${FMT_LIBRARIES})
#			target_link_libraries(exoprog std::fmt)
#			target_link_libraries(exoprog fmt/libfmt.a)
		endif()
	endif()
#
# libexodus source
##################
#
	add_library(exodus SHARED
		# Big ones first for efficient parallel compilation
		exoprog.cpp
		exoenv.cpp
		exofuncs.cpp
		exocallable.cpp
		varformatter.h

		task_manager.cpp

		job.cpp
		job_manager.cpp
		threadsafequeue.cpp
		threadpool.cpp
		result_range.cpp

		term_cursor.cpp
	)
	add_dependencies(exodus std_module)

#
# link libs
# #########
#
	# Boost and Postgresql
	target_link_libraries(exodus
		var
		Boost::fiber
		Boost::context
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

#
# readline
##########
#
	target_link_libraries(exodus
			readline
	)

#
# Request .so versioning e.g. /usr/local/lib/libexodus.so -> libexodus.so.XX.YY
# ######################
#
	#set_target_properties(exodus PROPERTIES SOVERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_VERSION}")
	set_target_properties(exodus PROPERTIES SOVERSION "${EXODUS_RELEASE}")

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
			TARGETS exoprog
			DESTINATION lib
		)
	endif()

#	if (EXO_MODULE EQUAL 2)
#		install(
#			TARGETS std
#			DESTINATION lib
#		)
#	endif()

# Install headers - non module
# ###############
#
	if (NOT EXO_MODULE)
		install(
			FILES

				exoprog.h
				exocallable.h
				exofuncs.h
				exoenv.h

				job.h
				job_manager.h
				task_manager.h
				result_range.h
				threadpool.h
				threadsafequeue.h

			DESTINATION include/exodus
		)
	endif()

#
#
# Install headers
# ###############
#
	install(
		FILES

			exodus.h
			exomacros.h
			exoimpl.h
			varformatter.h

			program.h
			library.h
			dict.h
			common.h

			ioconv_custom.h
			printtx.hpp
			htmllib2.h
		DESTINATION include/exodus
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
