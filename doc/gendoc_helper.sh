#!/bin/bash
set -euxo pipefail
PS4='+ [gendoc_helper.sh:$LINENO ${SECONDS}s] '
: $0 "$@"

# gendoc_helper.sh
#
# Single source of truth for the two gendoc generation commands.
# This eliminates the long duplicated bash -c strings from the CMakeLists.txt files.
#
# Called from:
#   - cli/CMakeLists.txt as a POST_BUILD side-effect on the gendoc target
#     (so normal builds of the CLI tools refresh the man page + the test file).
#   - doc/CMakeLists.txt for the explicit 'documentation' target (for the HTML docs).
#
# The header list is passed from CMake (GENDOC_INPUT_HEADERS) as arguments,
# so there is no duplication of the list.
#
# The first three arguments are always the bin dir, src dir, and test dir
# (use empty string for test dir when doing only the {h} html generation).

GENDOC_BIN_DIR=${1:?}
EXODUS_SRC_DIR=${2:?}
TEST_DIR=${3}

EXE="${GENDOC_BIN_DIR}/gendoc"

shift 3
HEADER_FILES=("$@")

if [ -n "${TEST_DIR}" ]; then
	: '{m} mode (man page + generate testing_var.h.cpp from code examples in headers)'
	"${EXE}" "${TEST_DIR}" "${HEADER_FILES[@]}" {m} > "${EXODUS_SRC_DIR}/var.1"
else
	: '{h} mode (HTML docs)'
	"${EXE}" "${HEADER_FILES[@]}" {h} > "${EXODUS_SRC_DIR}/var.htm"
fi
