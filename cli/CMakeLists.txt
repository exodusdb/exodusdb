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
		add_dependencies(${exe} std_module)
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

		add_dependencies(${lib} std_module)

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

# Compile options
	target_compile_options(compile PUBLIC -DEXODUS_RELEASE=${EXODUS_RELEASE} -DEXODUS_PATCH=${EXODUS_PATCH})

# Extra sources + full header dependencies for gendoc
# (gendoc reads these headers at runtime to extract documentation and examples,
#  so we must make the gendoc target depend on them for correct incremental builds)
	target_sources(gendoc PRIVATE ${GENDOC_INPUT_HEADERS})

# Output name rename
	set_target_properties(exodus-cli PROPERTIES OUTPUT_NAME exodus)

# Rebuild gendoc when any of its input headers change
	add_custom_target(gendoc_depends SOURCES ${GENDOC_INPUT_HEADERS})
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
# 6. Documentation & generated test file
# ───────────────────────────────────────────────
#
# The actual generation commands live in doc/gendoc_helper.sh
# (single source of truth — no duplication of the gendoc invocations).
#
# A lightweight POST_BUILD on gendoc triggers the helper so that
# normal builds of the CLI tools automatically refresh the docs
# (var.1 / var.htm) and the header-derived test file (testing_var.h.cpp).
# This preserves the useful coupling the project has historically relied on.
#
# See ../doc/CMakeLists.txt for the explicit 'documentation' custom target
# (for "ninja documentation" or making it a dependency of other things).

	add_custom_command(TARGET gendoc POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E env EXO_LIBDIR=${CMAKE_CURRENT_BINARY_DIR}
		        ${CMAKE_SOURCE_DIR}/doc/gendoc_helper.sh ${CMAKE_CURRENT_BINARY_DIR} ${SRC_EXODUS_DIR} ${CMAKE_SOURCE_DIR}/test ${GENDOC_INPUT_HEADERS}
		COMMENT "Running gendoc post-build to generate man page, html docs and testing_var.h.cpp"
		VERBATIM
	)

# The install for var.1 has been moved to doc/CMakeLists.txt
# (to keep documentation artifacts with the doc subproject).


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
