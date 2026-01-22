	cmake_minimum_required(VERSION 3.16)
	project(exodus)

	message("----------------------")
	message("-- CMAKE EXODUS CLI --")
	message("----------------------")

# Respect parent options/variables
	cmake_policy(SET CMP0015 NEW)

	include_directories(${SRC_EXODUS_DIR}/.. ${FMT_INCLUDE} ../fmt/include)
	link_directories(${EXODUS_DIR})

	if(TARGET exovar)
		link_libraries(exovar)
	endif()
	if(TARGET std)
		link_libraries(std)
	endif()

# Common compile options for everything
	add_compile_options(
		"-fvisibility-inlines-hidden"
		"-fno-omit-frame-pointer"
		"-ffunction-sections"
		"-fdata-sections"
	)

# ───────────────────────────────────────────────
# 1. CREATE EXECUTABLES + INSTALL
# ───────────────────────────────────────────────

	set(EXECUTABLES
		compile
		list
		dict2sql
		edic
		configexodus
		copyfile
		copyrec
		listfiles
		listdict
		createfile
		createindex
		dbcreate
		dbgrep
		dblist
		dbcopy
		dbcomp
		dbdelete
		dbattach
		delete
		clearfile
		deletefile
		deleteindex
		edir
		ddview
		listindex
		testsort
		syncdat
		cpp2dat
		convsyntax
		fixdeprecated
		var2let
		redefine
		fire
		sselect
		gendoc
		exodus-cli
	)

	foreach(exe IN LISTS EXECUTABLES)
		string(REGEX REPLACE "^([^-]+).*" "\\1" source_file_name "${exe}")
		add_executable(${exe} ${source_file_name}.cpp)
		target_link_libraries(${exe} PRIVATE exodus var stdc++fs pthread)
		install(TARGETS ${exe} DESTINATION bin)
	endforeach()

# ───────────────────────────────────────────────
# 2. CREATE LIBRARIES + INSTALL
# ───────────────────────────────────────────────

	set(LIBRARIES
		compile2
		htmllib2
		nlist
		dict_xo_clients
		testlib
	)

	foreach(lib IN LISTS LIBRARIES)
		add_library(${lib} SHARED ${lib}.cpp)

		target_link_libraries(${lib} PRIVATE exodus)
#		if(TARGET std)
#			target_link_libraries(${lib} PRIVATE std)
#		endif()

		install(TARGETS ${lib} DESTINATION lib)
	endforeach()

# ───────────────────────────────────────────────
# 3. CUSTOMISATIONS AFTER LOOPS
# ───────────────────────────────────────────────

# Per-target customisations

# Compile options
	target_compile_options(nlist PUBLIC "-O2")

# Extra sources
	target_sources(gendoc PRIVATE
		${SRC_VAR_DIR}/var.h
	)

# Output name rename
	set_target_properties(exodus-cli PROPERTIES OUTPUT_NAME exodus)

# Rebuild gendoc when headers change
	file(GLOB gendoc_files "${SRC_EXODUS_DIR}/*.h")
	add_custom_target(gendoc_depends SOURCES ${gendoc_files})
	add_dependencies(gendoc htmllib2 gendoc_depends)

# ───────────────────────────────────────────────
# 4. REFRESH SHARED LIBRARY CACHE
# ───────────────────────────────────────────────

	install(CODE "
		execute_process(
			COMMAND ldconfig
			OUTPUT_VARIABLE OUTVAR
			ERROR_VARIABLE  ERRVAR
			RESULT_VARIABLE RESVAR
		)
		message(\"ldconfig <\${OUTVAR}>\\n<\${ERRVAR}>\\n<\${RESVAR}>\")
	")

# ───────────────────────────────────────────────
# 5. INSTALL HEADERS
# ───────────────────────────────────────────────

	install(FILES
		nlist.h
		DESTINATION include/exodus
	)

# ───────────────────────────────────────────────
# 6. MAN PAGES & generated files
# ───────────────────────────────────────────────

# Post-build step: generate man page etc.
	set(SET_MY_ENV ${CMAKE_COMMAND} -E env EXO_LIBDIR=${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(TARGET gendoc POST_BUILD
					   COMMAND ${SET_MY_ENV} bash -c "${CMAKE_CURRENT_SOURCE_DIR}/gendoc.sh ${CMAKE_CURRENT_BINARY_DIR}/ ${SRC_EXODUS_DIR}"
					   COMMENT "Running gendoc post-build to generate man page & related files"
					   VERBATIM
	)

	install(FILES ${SRC_EXODUS_DIR}/var.1
		DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
	)

# ───────────────────────────────────────────────
# 7. CLEANUP local shadowing files
# ───────────────────────────────────────────────

	install(CODE "
		execute_process(
			COMMAND rm -f \$ENV{HOME}/lib/libdict_xo_clients.so
						  \$ENV{HOME}/lib/libhtmllib2.so
						  \$ENV{HOME}/lib/libnlist.so
						  \$ENV{HOME}/inc/htmllib2.h
						  \$ENV{HOME}/inc/nlist.h
		)
	")
