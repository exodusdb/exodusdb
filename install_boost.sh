#!/bin/bash
set -euxo pipefail
PS4='+ [boost ${SECONDS}s] '
:
: =============================================================================
: 'Build Boost with Clang + libc++ on Ubuntu LTS'
: 'Output: libs especially regex & locale without libstdc++.so dependency'
: 'Default is Boost 1.90.0, requires and discovers latest /usr/local/icu-libcxx-NN.N'
:
: './install_boost.sh [BOOST_VER] # default is 1.90.0'
:
: =============================================================================
	BOOST_VER=${1:-1.90.0}
:
: Config
:
	INSTALL_PREFIX="/usr/local"
:
	ICU_PREFIX=$(ls $INSTALL_PREFIX/icu-libcxx-* -d|sort --version-sort|tail -n1)
:
: From github
:
	# boost-1.90.0
	BOOST_DIR="boost-${BOOST_VER}"
	# boost-1.90.0-b2-nodocs.tar.gz
	BOOST_TAR="${BOOST_DIR}-b2-nodocs.tar.gz"
	BOOST_URL="https://github.com/boostorg/boost/releases/download/boost-${BOOST_VER}/${BOOST_TAR}"
#:
#: From archives.boost.io
#:
#	BOOST_DIR="boost_${BOOST_VER//./_}"
#	BOOST_TAR="${BOOST_DIR}.tar.bz2"
#	# https://archives.boost.io/release/1.90.0/source/boost_1_90_0.tar.gz
#	BOOST_URL="https://archives.boost.io/release/${BOOST_VER}/source/${BOOST_TAR}"
:
:  Colors for output
:
	RED='\033[0;31m'
	GREEN='\033[0;32m'
	YELLOW='\033[1;33m'
	NC='\033[0m' # No Color
:
: clang++ and clang are required
: ==============================
:
	if ! which clang++ || ! which clang ; then
		CLANG_VER=`c++ --version | head -n1|cut -d'.' -f 1|grep -Po '\d+'`
		sudo update-alternatives --install /usr/bin/clang   clang   /usr/bin/clang-$CLANG_VER   80
		sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-$CLANG_VER 80
	fi
:
: STEP 0: Install dependencies
: ============================
:
	sudo apt install -y \
		curl \
		tar \
		xz-utils \
		pkg-config \
		libbz2-dev \
		zlib1g-dev \
		bzip2 \
		make

:
: STEP 1: Download and extract boost
: ==================================
:
: Download
:
	curl -LOs "${BOOST_URL}"
:
: Verify checksum
:
	curl -LOs "${BOOST_URL}.txt"
	EXPECTED_CHECKSUM=$(awk '{ print $1 }' "$BOOST_TAR.txt")
#	sha256sum boost-1.90.0-b2-nodocs.tar.gz
	CURRENT_CHECKSUM=$(sha256sum "$BOOST_TAR" | awk '{ print $1 }')
	[[ "$CURRENT_CHECKSUM" = "$EXPECTED_CHECKSUM" ]]
	rm $BOOST_TAR.txt
:
: Unpack
:
	rm -rf "${BOOST_DIR}"
#	tar -xjf "${BOOST_TAR}"
	tar -xzf "${BOOST_TAR}"
	rm "${BOOST_TAR}"
:
: STEP 2: Build and install
: =========================
:
	pushd "${BOOST_DIR}"
:
: Clean previous build artifacts
: ------------------------------
	rm -rf bin.v2 || true
:
: Bootstrapping Boost with clang
: ------------------------------
:
	./bootstrap.sh --with-toolset=clang
:
: Configure user-config.jam for libc++
: ------------------------------------
	(set +x && echo 'using clang : : clang++ : <cxxflags>"-stdlib=libc++" <linkflags>"-stdlib=libc++" ;' > tools/build/src/user-config.jam)
:
: Build selected libraries
: ------------------------
	(set +x && echo -e "${YELLOW}Building Selected Boost libraries...${NC}")
:
: IF ANY TESTS FAIL THAT SHOULDNT e.g. has_icu - important for boost_locale, see
: boost_1_90_0/bin.v2/config.log for info
:
: Add -ldl linker flag to allow use linking to custom icu on older OS e.g. 20.04
:
	sudo ./b2 -j$(nproc) \
		"-sICU_PATH=${ICU_PREFIX}" \
		toolset=clang \
		cxxflags="-stdlib=libc++ -I${ICU_PREFIX}/include" \
		linkflags="-stdlib=libc++ -L${ICU_PREFIX}/lib -ldl" \
		threading=multi \
		link=shared,static \
		boost.locale.icu=on \
		boost.locale.iconv=off \
		--with-date_time \
		--with-system \
		--with-regex \
		--with-thread \
		--with-locale \
		--with-fiber \
		--with-context \
		--durations \
		install --prefix="${INSTALL_PREFIX}" | grep -vP "^(include/boost|common.copy)"

	popd
	ldconfig
:
: STEP 3: Verify
: ==============
:
	(set +x && echo -e "\n${YELLOW}→ Verification:${NC}")

	(set +x && echo -e "\nICU libs:")
	ls -l "${ICU_PREFIX}/lib/" | grep icu

	(set +x && echo -e "\nChecking for libstdc++ dependency (should be empty!):")
#	for LIBBOOST_FILE in /usr/local/lib/libboost*.so; do
	for BOOST_LIBNAME in atomic chrono date_time locale regex thread fiber context; do
		LIBBOOST_FILE=/usr/local/lib/libboost_${BOOST_LIBNAME}.so
		test -x ${LIBBOOST_FILE}
		if ldd ${LIBBOOST_FILE} | grep -i stdc++; then
			(set +x && echo -e "${RED}→ libstdc++ found in ${LIBBOOST_FILE} (bad!)${NC}")
			exit 1
		else
			(set +x && echo -e "${GREEN}→ No libstdc++ found in ${LIBBOOST_FILE} (good!)${NC}")
		fi
	done

	(set +x && echo -e "${GREEN}Build completed successfully!${NC}")
:
: Cleanup
:
	rm ${BOOST_DIR} -rf
:
: ===========================================================
: Boost ${BOOST_VER} installed successfully to ${INSTALL_PREFIX}
: You can now build your boost project with -stdlib=libc++ flags.
: Recommended CMake additions:
:   add_compile_options\(-stdlib=libc++\)
:   add_link_options\(-stdlib=libc++\)
: Enjoy your clean libc++ Boost stack\! 🚀
: ============================================================
