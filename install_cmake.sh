#!/usr/bin/env bash
set -euxo pipefail
PS4='+ [cmake ${SECONDS}s] '
:
: "=== Install cmake from github binary or source"
: ===============================================
:
: 'Syntax is ./install_cmake.sh binary [RELEASE]'
: 'Syntax is ./install_cmake.sh source [BRANCH_OR_TAG_OR_HASH]'
:
: 'RELEASE:'
: '  4.2.1'
: '  3.31.8'
: '  etc.'
:
: 'BRANCH_OR_TAG_OR_HASH :'
: '  release  : (default) Latest release -> currently 4.2.1'
: '  master   :           Latest dev     -> currently 4.2.20260120-g89d5a82'
: '  v4.2.1   : Available tags - see github repo for info.'
: '  g89d5a82 : Any available hash'
: '  etc.'
:
: 'Installs /usr/local/bin/cmake (overides apt and snap cmake)'
:
	MODE=${1:-binary}
	RELEASE=${2:-4.2.1}
	BRANCH_OR_TAG_OR_HASH=${2:-release}
:
: 0. Configuration
:
	GIT_REPO_URL=https://github.com/Kitware/CMake
	INSTALL_PREFIX="/usr/local"
	TEMP_DIR="/tmp/cmake-exodusdb"
	CLEANUP_TEMPDIR=true
	export CXXFLAGS="-Wno-unused-variable -Wno-deprecated-declarations -Wno-switch -Wno-deprecated-literal-operator"
:
: 1. Install dependencies
:
#	sudo apt update -qq
#	sudo apt install -y --no-install-recommends \
#	    git cmake build-essential cmake-build  # cmake-build here is only for bootstrapping cmake if needed
:
	if [[ $MODE == binary ]]; then
		rm -rf cmake-$RELEASE-linux-x86_64.sh
		wget --no-verbose $GIT_REPO_URL/releases/download/v$RELEASE/cmake-$RELEASE-linux-x86_64.sh
		sudo sh cmake-$RELEASE-linux-x86_64.sh --prefix=$INSTALL_PREFIX --exclude-subdir --skip-license
		rm cmake-$RELEASE-linux-x86_64.sh
	else

: 2. Get repo
:
		rm "$TEMP_DIR" -rf
		git clone --depth 1 --single-branch ${BRANCH_OR_TAG_OR_HASH:+-b $BRANCH_OR_TAG_OR_HASH} "$GIT_REPO_URL" "$TEMP_DIR"
		cd "$TEMP_DIR"
:
: 3. Prepare
:
		apt-get install -y libssl-dev
		./bootstrap --parallel=$(nproc)
:
: 4. Make
:
		make -j$(nproc)
:
: 5. Install
:
		make install

	fi
:
: 6. Cleanup
:
	cd ..
	$CLEANUP_TEMPDIR && rm -rf "$TEMP_DIR"
:
: 7. Complete
:
	cmake --version
: ================================================================
: "New cmake is now in: $INSTALL_PREFIX/bin"
: "You can test it in your CMake project with: cmake -G Ninja ..."
: ================================================================
