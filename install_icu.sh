#!/usr/bin/env bash
set -euxo pipefail
PS4='+ [icu ${SECONDS}s] '
:
: Download, build and install icu from github with clang/libc++
: =============================================================
:
: './install_icu.sh [ICU_VER]'
:
	DEFAULT_ICU_VER=78.2
:	etc. - See https://github.com/unicode-org/icu/releases
:
: Config
: ------
:
	ICU_VER=${1:-$DEFAULT_ICU_VER}
	INSTALL_PREFIX=/usr/local/icu-libcxx-${ICU_VER}

	TARBALL="icu4c-${ICU_VER}-sources.tgz"
	URL="https://github.com/unicode-org/icu/releases/download/release-${ICU_VER}/${TARBALL}"
:
: Configure with clang + libc++
:
#	export CC=clang
#	export CXX=clang++
	export CC=cc
	export CXX=c++
	export CFLAGS="-O2 -Wall -fPIC"
	export CXXFLAGS="-O2 -Wall -fPIC"
	export CXXFLAGS="$CXXFLAGS -nostdinc++ -isystem/usr/include/c++/v1"
	export LDFLAGS="-stdlib=libc++ -lc++abi -fuse-ld=lld"
#	export LIBS="-lc++abi"   # sometimes needed instead of/in addition to LDFLAGS
	CLANG_MAJOR=$(cc --version 2>&1 | grep -oP 'clang version \K\d+')

:
: Install dependencies
: --------------------
:
	sudo apt install -y make wget tar curl gpg
:
: Download + verify
: -----------------
:
	curl -LOs "${URL}"
:
: Verify
: ------
:
	curl -LOs "${URL}.asc"
	curl -LOs https://raw.githubusercontent.com/unicode-org/icu/main/KEYS
	gpg --import KEYS
	gpg --verify "${TARBALL}.asc" "${TARBALL}"
:
: Extract official tarball creates top-level icu/
: ===============================================
:
	RELEASE_DIR=icu-release-${ICU_VER}
	rm $RELEASE_DIR -rf
	mkdir --parents $RELEASE_DIR
	tar xzf "${TARBALL}" -C $RELEASE_DIR --strip-components=1
	pushd $RELEASE_DIR/source

	./runConfigureICU Linux \
	  --prefix=${INSTALL_PREFIX} \
	  --enable-static \
	  --disable-shared \
	  --disable-extras \
	  --disable-samples \
	  --disable-tests \
	  --disable-strict
:
: Build
: -----
:
	make clean
	make -j$(nproc)

:
: Install
: -------
:
	sudo make install

:
: Cleanup
: -------
:
	popd
	rm $RELEASE_DIR -rf
	rm $TARBALL
	rm $TARBALL.asc
	rm KEYS

: ====================================================================
: ICU ${ICU_VER} with libc++ installed to $INSTALL_PREFIX
: ====================================================================
