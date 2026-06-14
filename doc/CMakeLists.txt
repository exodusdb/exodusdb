#
# Config
# ######
#
	cmake_minimum_required(VERSION 3.10)
	project(exodus_doc)

	message("----------------------")
	message("-- CMAKE EXODUS DOC --")
	message("----------------------")

#
# Documentation generation (and side-effect test file generation)
# ################################################################
#
# This subproject is responsible for generating:
#   - exodus/var.1   (man page)
#   - exodus/var.htm (HTML documentation)
#   - test/testing_var.h.cpp  (comprehensive test exercising the public API, auto-generated from header comments)
#
# It depends on the gendoc executable (built by the cli subproject).
# The generation uses the *built* gendoc binary, which is the mechanism that
# avoids the historical circular dependencies between the CLI, the generated
# test file, and the test subproject.
#
# The "incestuous" relationship is preserved: building the documentation target
# ensures that gendoc has been built and then runs it against the current headers
# to refresh both the docs and the testing_var.h.cpp test file.
#
# testing_var.h.cpp is still committed in the source tree (as a seed so that
# the test subproject can be built without first running the documentation target).
# The test/CMakeLists.txt auto-discovers it via GLOB_RECURSE on test_*.cpp
# (with a special-case build rule for historical reasons).
#
# To (re)generate documentation and the derived test file:
#     ninja documentation
# or
#     make documentation
#
# This target is not added to ALL by default, so normal builds do not
# automatically overwrite source files.

# We expect the following variables to have been set by the parent:
#   BIN_CLI_DIR        - binary directory of the cli subproject (where gendoc lives)
#   SRC_EXODUS_DIR     - source directory of the exodus/ subproject (for headers + output of .1/.htm)
#   GENDOC_INPUT_HEADERS - list of headers passed to gendoc (for proper dependency tracking)

if(NOT DEFINED BIN_CLI_DIR)
    message(FATAL_ERROR "BIN_CLI_DIR must be set by parent before add_subdirectory(doc)")
endif()
if(NOT DEFINED SRC_EXODUS_DIR)
    message(FATAL_ERROR "SRC_EXODUS_DIR must be set by parent before add_subdirectory(doc)")
endif()

set(SET_MY_ENV ${CMAKE_COMMAND} -E env EXO_LIBDIR=${BIN_CLI_DIR})

# Outputs produced by gendoc.sh (still generated into the source tree for Option A)
set(GENERATED_VAR1   ${SRC_EXODUS_DIR}/var.1)
set(GENERATED_VARHTM ${SRC_EXODUS_DIR}/var.htm)
set(GENERATED_TEST   ${CMAKE_SOURCE_DIR}/test/testing_var.h.cpp)

add_custom_command(
    OUTPUT ${GENERATED_VAR1} ${GENERATED_VARHTM} ${GENERATED_TEST}
    DEPENDS gendoc ${GENDOC_INPUT_HEADERS}
    COMMAND ${SET_MY_ENV} bash -c "${CMAKE_SOURCE_DIR}/cli/gendoc.sh ${BIN_CLI_DIR}/ ${SRC_EXODUS_DIR}"
    COMMENT "Running gendoc to generate man page (var.1), HTML docs (var.htm), and testing_var.h.cpp from headers"
    VERBATIM
)

add_custom_target(documentation
    DEPENDS ${GENERATED_VAR1} ${GENERATED_VARHTM} ${GENERATED_TEST}
)

# Install the generated man page.
# (var.htm is treated as a committed documentation source artifact.)
install(FILES ${GENERATED_VAR1}
    DESTINATION share/man/man1
)
