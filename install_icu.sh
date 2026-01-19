#!/usr/bin/env bash
set -euxo pipefail
PS4='+ [icu ${SECONDS}s] '
:
: Prerequisites - run once
:
	sudo apt install -y make wget tar

	ICU_VER="78.2"
	TARBALL="icu4c-${ICU_VER}-sources.tgz"
	URL="https://github.com/unicode-org/icu/releases/download/release-${ICU_VER}/${TARBALL}"

	EXPECTED_SHA256="3e99687b5c435d4b209630e2d2ebb79906c984685e78635078b672e03c89df35"
#:
#: Ensure clang tools are available for libc++
#:
	INSTALL_PREFIX=/usr/local/icu-libcxx-${ICU_VER}
	CLANG_MAJOR=$(cc --version 2>&1 | grep -oP '^Ubuntu clang version \K\d+')
#	sudo apt -y install lld-$CLANG_MAJOR libc++-$CLANG_MAJOR-dev libc++abi-$CLANG_MAJOR-dev clang-tools-$CLANG_MAJOR

:
: Download + verify
: =================
:
	wget --no-verbose --timestamping "${URL}"
	ACTUAL=$(sha256sum "${TARBALL}" | cut -d' ' -f1)
	[[ "$ACTUAL" != "$EXPECTED_SHA256" ]] && { echo "Hash mismatch!"; exit 1; }
: "Hash OK"

:
: Extract official tarball creates top-level icu/
: ===============================================
:
	RELEASE_DIR=icu-release-${ICU_VER}
	rm $RELEASE_DIR -rf
	mkdir --parents $RELEASE_DIR
	tar xzf "${TARBALL}" -C $RELEASE_DIR --strip-components=1
	cd $RELEASE_DIR/source

:
: Configure with clang + libc++
: =============================
:
#	export CC=clang
#	export CXX=clang++
	export CC=cc
	export CXX=c++
	export CFLAGS="-O2 -Wall -fPIC"
#	export CXXFLAGS="-stdlib=libc++ -O2 -Wall -fPIC"
	export CXXFLAGS="-O2 -Wall -fPIC"
	export LDFLAGS="-stdlib=libc++ -lc++abi"
#	export LIBS="-lc++abi"   # sometimes needed instead of/in addition to LDFLAGS

:
: Optional but recommended: help find libc++ headers if needed
:
	export CXXFLAGS="$CXXFLAGS -nostdinc++ -isystem/usr/include/c++/v1"

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
: =====
:
	make clean
	make -j$(nproc)

:
: Install
: =======
:
	sudo make install

:
: ICU 78.2 with libc++ installed to $INSTALL_PREFIX
: ====================================================================
