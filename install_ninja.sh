#!/usr/bin/env bash
set -euxo pipefail
:
: "=== DL Build and install Ninja from exodusdb fork"
: =================================================
:
: Configuration
:
	REPO_URL="https://github.com/exodusdb/ninja.git"
	INSTALL_DIR="/usr/local/bin"
	NINJA_BIN="ninja"
	BUILD_DIR="build-cmake"
	CLONE_DIR="$HOME/ninja-exodusdb"   # or /tmp/ninja-exodusdb if you prefer temporary
#:
#: 1. Install build dependencies (if missing)
#: "→ Installing dependencies (sudo may ask for password)"
#:
#	sudo apt update -qq
#	sudo apt install -y --no-install-recommends \
#	    git cmake build-essential ninja-build  # ninja-build here is only for bootstrapping cmake if needed
:
: 2. Clone or update the repository
:
	if [ -d "$CLONE_DIR" ]; then
:
: "→ Updating existing clone in $CLONE_DIR"
:
	    cd "$CLONE_DIR"
	    git fetch --quiet origin
	    git reset --hard origin/master   # or replace 'master' with the branch you want
	    git clean -fdx -q || true
	else
:
: "→ Cloning fresh copy to $CLONE_DIR"
:
	    git clone --depth 1 "$REPO_URL" "$CLONE_DIR"
	    cd "$CLONE_DIR"
	fi

:
: "→ Configuring and building Ninja with CMake"
:	rm -rf "$BUILD_DIR"
	cmake -S . -B "$BUILD_DIR" \
	    -DCMAKE_BUILD_TYPE=Release \
	    -DBUILD_TESTING=OFF \
	    -DCMAKE_INSTALL_PREFIX=/usr/local   # not used for binary install, but good practice

	cmake --build "$BUILD_DIR" --config Release --parallel "$(nproc)"
:
: 4. Verify the built binary
:
	if [ ! -f "$BUILD_DIR/$NINJA_BIN" ]; then
		echo "Error: Ninja binary not found in $BUILD_DIR/$NINJA_BIN"
	    exit 1
	fi
:
: "→ Installing ninja to $INSTALL_DIR (sudo required)"
:
	sudo cp -f "$BUILD_DIR/$NINJA_BIN" "$INSTALL_DIR/$NINJA_BIN"
	sudo chmod 755 "$INSTALL_DIR/$NINJA_BIN"
:
: "→ Cleaning up"
:
	cd ..
	rm -rf "$CLONE_DIR"
:
: "=== Installation complete ==="
:
	"$INSTALL_DIR/$NINJA_BIN" --version
	ninja --version
:
: "New ninja is now in: $INSTALL_DIR/$NINJA_BIN"
: "You can test it in your CMake project with: cmake -G Ninja ..."
