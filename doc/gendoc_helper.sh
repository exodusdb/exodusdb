#!/bin/bash
set -euxo pipefail

# Helper to run the two gendoc generations.
# This keeps the actual commands in one place, avoiding duplication in CMakeLists.txt files.
# Called from cli/CMakeLists.txt (as POST_BUILD side-effect on gendoc) and doc/CMakeLists.txt.

GENDOC_BIN_DIR=${1:?GENDOC_BIN_DIR required}
EXODUS_SRC_DIR=${2:?EXODUS_SRC_DIR required}
TEST_DIR=${3:?TEST_DIR required}
shift 3

HEADER_FILES=("$@")

EXE="${GENDOC_BIN_DIR}/gendoc"

# First call: with test dir, generates man page + the test_*.cpp file (testing_var.h.cpp)
"${EXE}" "${TEST_DIR}" "${HEADER_FILES[@]}" {m} > "${EXODUS_SRC_DIR}/var.1"

# Second call: generates the HTML docs
"${EXE}" "${HEADER_FILES[@]}" {h} > "${EXODUS_SRC_DIR}/var.htm"
