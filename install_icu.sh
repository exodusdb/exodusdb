#!/usr/bin/env bash
set -euxo pipefail

:
: Prerequisites - run once
:
	sudo apt install -y make wget tar

	ICU_VER="78.2"
	TARBALL="icu4c-${ICU_VER}-sources.tgz"
	URL="https://github.com/unicode-org/icu/releases/download/release-${ICU_VER}/${TARBALL}"

	EXPECTED_SHA256="3e99687b5c435d4b209630e2d2ebb79906c984685e78635078b672e03c89df35"

	INSTALL_PREFIX=/usr/local/icu-libcxx-${ICU_VER}

:
: Download + verify
: =================
:
	wget -O "${TARBALL}" "${URL}"
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
	export CC=clang
	export CXX=clang++
#	export CFLAGS="-O2 -Wall -fPIC"
	export CXXFLAGS="-stdlib=libc++ -O2 -Wall -fPIC"
	export LDFLAGS="-stdlib=libc++ -lc++abi"

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
