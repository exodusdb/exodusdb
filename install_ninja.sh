#!/usr/bin/env bash
set -euxo pipefail
PS4='+ [ninja ${SECONDS}s] '
:
: "=== Install ninja from github binary or source"
: ===============================================
:
: 'Syntax is ./install_ninja.sh binary [RELEASE]'
: 'Syntax is ./install_ninja.sh source [BRANCH_OR_TAG_OR_HASH]'
:
: 'RELEASE:'
: '  1.13.2'
: '  3.31.8'
: '  etc.'
:
: 'BRANCH_OR_TAG_OR_HASH :'
: '  release  : (default) Latest release -> currently 1.13.2'
: '  master   :           Latest dev     -> currently 1.14.0'
: '  v1.13.2   : Available tags - see github repo for info.'
: '  g89d5a82 : Any available hash'
: '  etc.'
:
: 'Installs /usr/local/bin/ninja (overides apt and snap ninja)'
:
	MODE=${1:-binary}
	RELEASE=${2:-1.13.2}
	BRANCH_OR_TAG_OR_HASH=${2:-release}
:
: 0. Configuration
:
	GIT_REPO_URL=https://github.com/ninja-build/ninja
	INSTALL_PREFIX="/usr/local"
	TEMP_DIR="/tmp/ninja-exodusdb"
	CLEANUP_TEMPDIR=true
	export CXXFLAGS="-Wno-unused-variable -Wno-deprecated-declarations -Wno-switch -Wno-deprecated-literal-operator"
:
: 1. Install dependencies
:
#	sudo apt update -qq
#	sudo apt install -y --no-install-recommends \
#	    git ninja build-essential ninja-build  # ninja-build here is only for bootstrapping ninja if needed
	apt-get install -y unzip
:
	if [[ $MODE == binary ]]; then
		rm -rf ninja-linux.zip
		wget --no-verbose $GIT_REPO_URL/releases/download/v$RELEASE/ninja-linux.zip
		unzip -o ninja-linux.zip -d $INSTALL_PREFIX/bin
		rm ninja-linux.zip
	else
:
: 2. Get repo
:
		rm "$TEMP_DIR" -rf
		git clone --depth 1 --single-branch ${BRANCH_OR_TAG_OR_HASH:+-b $BRANCH_OR_TAG_OR_HASH} "$GIT_REPO_URL" "$TEMP_DIR"
		cd "$TEMP_DIR"
:
: 3. Prepare
:
		cmake -S . -B build \
		    -DCMAKE_BUILD_TYPE=Release \
		    -DBUILD_TESTING=OFF \
		    -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX   # not used for binary install, but good practice
:
: 4. Make
:
		cmake --build build --config Release --parallel "$(nproc)"
:
: 5. Install
:
		sudo cp -f "build/ninja" "$INSTALL_PREFIX/bin/ninja"
		sudo chmod 755 "$INSTALL_PREFIX/bin/ninja"
:
	fi
:
: 6. Cleanup
:
	cd ..
	$CLEANUP_TEMPDIR && rm -rf "$TEMP_DIR"
:
: 7. Complete
:
	ninja --version
: ================================================================
: "New ninja is now in: $INSTALL_PREFIX/bin"
: "You can test it in your CMake project with: ninja -G Ninja ..."
: ================================================================
