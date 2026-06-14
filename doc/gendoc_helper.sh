#!/bin/bash
set -euxo pipefail

# gendoc_helper.sh
#
# Single source of truth for the two gendoc invocations that generate:
#   - var.1 (man page) + testing_var.h.cpp (the header-derived test file)
#   - var.htm (HTML documentation)
#
# This script is called by:
#   - cli/CMakeLists.txt (POST_BUILD on the gendoc target — provides the
#     "automatic refresh when you build the CLI tools" behaviour)
#   - doc/CMakeLists.txt (the explicit 'documentation' custom target)
#
# It receives the authoritative header list from CMake (GENDOC_INPUT_HEADERS)
# so there is no duplication of the list between build system and script.
#
# For manual/ad-hoc runs from a source tree, use cli/gendoc.sh instead
# (it has a fallback header list).

	GENDOC_BIN_DIR=${1:?GENDOC_BIN_DIR required}
	EXODUS_SRC_DIR=${2:?EXODUS_SRC_DIR required}
	TEST_DIR=${3:?TEST_DIR required}
	shift 3

	HEADER_FILES=("$@")

	EXE="${GENDOC_BIN_DIR}/gendoc"

: 'First call: with test dir, generates man page + the test_*.cpp file (testing_var.h.cpp)'
	"${EXE}" "${TEST_DIR}" "${HEADER_FILES[@]}" {m} > "${EXODUS_SRC_DIR}/var.1"

: 'Second call: generates the HTML docs'

	"${EXE}" "${HEADER_FILES[@]}" {h} > "${EXODUS_SRC_DIR}/var.htm"
