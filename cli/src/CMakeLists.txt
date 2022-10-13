#
# CONFIG
# ######
#
	cmake_minimum_required(VERSION 3.1)
	project(exodus)

#
# INIT
# ####
#
	message("")
	message("--EXODUS CLI CMAKE INIT --")

#use CMakeList.txt options in parent directory

#
# Allow relative link directories
# ###############################
#
	cmake_policy(SET CMP0015 NEW)
	include_directories(../../exodus/libexodus)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}../../exodus/libexodus/exodus)

#
# Determine C++ version - C++17/18 for Ubuntu 20+
# ###############################################
#
	execute_process(
	        COMMAND         grep "Ubuntu 2" /etc/lsb-release
	        OUTPUT_VARIABLE OUTVAR
	        ERROR_VARIABLE  ERRVAR
	        RESULT_VARIABLE RESVAR
	)
	if(RESVAR EQUAL "0")
	        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a")
	else()
	        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	endif()

#
# 1. COMPILE
# ##########
#
	#
	# DONT FORGET TO ADD TO ALL STAGES (1, 2 AND 3)
	#

	#NB add any NEW programs here and in TWO MORE sections further down
	#target_link_libraries(
	#install(

	# nlist -> bin/list
	add_executable(list list.cpp)
#	# cmake - handling executable and library with same name
#	# https://gist.github.com/jlgerber/eafc4ee2b9954e27dd2bb009496b1b03
#	#set_target_properties(nlist PROPERTIES OUTPUT_NAME list)
#	set_target_properties(nlist PROPERTIES RUNTIME_OUTPUT_NAME list)

	#largest first
	add_executable(compile compile.cpp)
	add_executable(olist olist.cpp)
	add_executable(dict2sql dict2sql.cpp)
	add_executable(edic edic.cpp)
	add_executable(configexodus configexodus.cpp)
	add_executable(copyfile copyfile.cpp)
	add_executable(copyrec copyrec.cpp)

	add_executable(listfiles listfiles.cpp)
	add_executable(listdict listdict.cpp)
	add_executable(createfile createfile.cpp)
	add_executable(createindex createindex.cpp)

	add_executable(dbcreate dbcreate.cpp)
	add_executable(dbgrep dbgrep.cpp)
	add_executable(dblist dblist.cpp)
	add_executable(dbcopy dbcopy.cpp)
	add_executable(dbcomp dbcomp.cpp)
	add_executable(dbdelete dbdelete.cpp)
	add_executable(dbattach dbattach.cpp)

	add_executable(delete delete.cpp)
	add_executable(clearfile clearfile.cpp)
	add_executable(deletefile deletefile.cpp)
	add_executable(deleteindex deleteindex.cpp)
	add_executable(edir edir.cpp)
	add_executable(listindex listindex.cpp)
	add_executable(testsort testsort.cpp)
	add_executable(sync_dat sync_dat.cpp)
	add_executable(convsyntax convsyntax.cpp)
	add_executable(redefine redefine.cpp)

	#name is name only ... input and output files are exodus
	add_executable(exodus-cli exodus.cpp)
	set_target_properties(exodus-cli PROPERTIES OUTPUT_NAME exodus)

	# lib #
	add_library(htmllib2 SHARED htmllib2.cpp)
	add_library(nlist SHARED nlist.cpp)
	add_library(dict_xo_clients SHARED dict_xo_clients.cpp)
	#add_library(number SHARED number.cpp)

#
# 2. LINK
# #######
#
	#dl is needed for dynamic link library code

	# Largest first
	target_link_libraries(compile exodus stdc++fs pthread ${LIBS})

	target_link_libraries(list exodus stdc++fs ${LIBS})
	target_link_libraries(olist exodus stdc++fs ${LIBS})
	target_link_libraries(dict2sql exodus stdc++fs ${LIBS})
	target_link_libraries(edic exodus stdc++fs ${LIBS})
	target_link_libraries(configexodus exodus stdc++fs ${LIBS})
	target_link_libraries(copyfile exodus stdc++fs ${LIBS})
	target_link_libraries(copyrec exodus stdc++fs ${LIBS})

	target_link_libraries(createfile exodus stdc++fs ${LIBS})
	target_link_libraries(createindex exodus stdc++fs ${LIBS})

	target_link_libraries(dbcreate exodus stdc++fs ${LIBS})
	target_link_libraries(dblist exodus stdc++fs ${LIBS})
	target_link_libraries(dbgrep exodus stdc++fs ${LIBS})
	target_link_libraries(dbcopy exodus stdc++fs ${LIBS})
	target_link_libraries(dbcomp exodus stdc++fs ${LIBS})
	target_link_libraries(dbdelete exodus stdc++fs ${LIBS})
	target_link_libraries(dbattach exodus stdc++fs ${LIBS})

	target_link_libraries(delete exodus stdc++fs ${LIBS})
	target_link_libraries(clearfile exodus stdc++fs ${LIBS})
	target_link_libraries(deletefile exodus stdc++fs ${LIBS})
	target_link_libraries(deleteindex exodus stdc++fs ${LIBS})
	target_link_libraries(edir exodus stdc++fs ${LIBS})
	#target_link_libraries(number exodus stdc++fs ${LIBS})
	target_link_libraries(exodus-cli exodus stdc++fs ${LIBS})
	target_link_libraries(listfiles exodus stdc++fs ${LIBS})
	target_link_libraries(listdict exodus stdc++fs ${LIBS})
	target_link_libraries(listindex exodus stdc++fs ${LIBS})
	target_link_libraries(testsort exodus stdc++fs ${LIBS})
	target_link_libraries(sync_dat exodus stdc++fs ${LIBS})
	target_link_libraries(convsyntax exodus stdc++fs ${LIBS})
	target_link_libraries(redefine exodus stdc++fs ${LIBS})

#
# 3. INSTALL BINs
# ###############
#
	install(
		TARGETS
			compile
			createfile
			createindex

			dbcreate
			dblist
			dbgrep
			dbcopy
			dbcomp
			dbdelete
			dbattach

			delete
			clearfile
			copyfile
			copyrec
			deletefile
			deleteindex
			edir
			edic
			dict2sql
			exodus-cli
			list
			olist
			listfiles
			listdict
			listindex
			testsort
			configexodus
			sync_dat
			convsyntax
			redefine

		DESTINATION
			bin
	)

#
# 4. INSTALL LIBs
# ###############
#
	## CMAKE_PREFIX=/usr/local/
	#install(TARGETS dict_xo_clients DESTINATION lib)
	#push it into /usr/lib where it can be found by dlopen
	#install(TARGETS dict_xo_clients number DESTINATION /usr/lib)
	install(
		TARGETS
			dict_xo_clients
			htmllib2
			nlist
		DESTINATION
			lib
	)

	# In libexodus and cli now that both install into lib
	install(
		CODE
		"
			execute_process(
				COMMAND
					ldconfig
				OUTPUT_VARIABLE
					OUTVAR
				ERROR_VARIABLE
					ERRVAR
				RESULT_VARIABLE
					RESVAR
			)
	        message(
				ldconfig <\${OUTVAR}>\n
	            <\${ERRVAR}>\n
	            <\${RESVAR}>\n
	        )
		"
	)

# 5. INSTALL INCLUDES
# ###################
#
	install(
		FILES
			# Moved to libexodus/exodus so that can be included when building cli
			# Also widely used service programs
			# printtx.hpp
			# htmllib2.h

			# For any programs that prefer to call nlist instead of perform/execute it
			nlist.h

		DESTINATION
			include/exodus
	)

# 6. CLEAN OUT ANY EDIC/COMPILE libs in ~/lib THAT WILL SHADOW THE REAL FILE IN /usr/local/lib
#############################################
	install(
		CODE "
			execute_process(
				COMMAND
					rm -f $ENV{HOME}/lib/libdict_xo_clients.so $ENV{HOME}/lib/libhtmllib2.so $ENV{HOME}/lib/libnlist.so
			)
		"
    )


#
# Exit
# ####
#
	message("--EXODUS CLI CMAKE EXIT --")
