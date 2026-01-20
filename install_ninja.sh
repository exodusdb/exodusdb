#!/usr/bin/env bash
set -euxo pipefail
PS4='+ [ninja ${SECONDS}s] '
:
: "=== Download, build and install ninja from github"
: =================================================
:
: 'Syntax is ./install_ninja.sh [BRANCH_TAG_HASH]'
:
: 'BRANCH_TAG_HASH :'
:
: 'release : (default) Latest release -> 1.13.2 currently'
: 'master  :           Latest dev     -> 1.14.0 currently'
: 'v1.12.0 : Available tags - see github repo for info.'
: '3441b63 : Any available hash'
:
: 'Installs /usr/local/bin/ninja (overides apt and snap ninja)'
:
	BRANCH_TAG_HASH=${1:-release}
:
: 0. Configuration
:
	GIT_REPO_URL=https://github.com/ninja-build/ninja
	INSTALL_DIR="/usr/local/bin"
	NINJA_BIN="ninja"
	BUILD_DIR="build-cmake"
	CLONE_DIR="$HOME/ninja-exodusdb"
	CLONE_DIR="/tmp/ninja-exodusdb"
	RM_SRC_AFTER=true
:
: 1. Install dependencies
:
#	sudo apt update -qq
#	sudo apt install -y --no-install-recommends \
#	    git cmake build-essential ninja-build  # ninja-build here is only for bootstrapping cmake if needed
:
: 2. Get repo
:
    rm "$CLONE_DIR" -rf
    git clone --depth 1 --single-branch ${BRANCH_TAG_HASH:+-b $BRANCH_TAG_HASH} "$GIT_REPO_URL" "$CLONE_DIR"
    cd "$CLONE_DIR"
:
: 3. Make
:
	cmake -S . -B "$BUILD_DIR" \
	    -DCMAKE_BUILD_TYPE=Release \
	    -DBUILD_TESTING=OFF \
	    -DCMAKE_INSTALL_PREFIX=/usr/local   # not used for binary install, but good practice

	cmake --build "$BUILD_DIR" --config Release --parallel "$(nproc)"
:
: 4. Verify
:
	if [ ! -f "$BUILD_DIR/$NINJA_BIN" ]; then
		echo "Error: Ninja binary not found in $BUILD_DIR/$NINJA_BIN"
	    exit 1
	fi
:
: 5. Install
:
	sudo cp -f "$BUILD_DIR/$NINJA_BIN" "$INSTALL_DIR/$NINJA_BIN"
	sudo chmod 755 "$INSTALL_DIR/$NINJA_BIN"
:
: 6. Cleanup
:
	cd ..
	$RM_SRC_AFTER && rm -rf "$CLONE_DIR"
:
: 7. Complete
:
	"$INSTALL_DIR/$NINJA_BIN" --version
	ninja --version
:
: "New ninja is now in: $INSTALL_DIR/$NINJA_BIN"
: "You can test it in your CMake project with: cmake -G Ninja ..."
