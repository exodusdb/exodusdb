#!/bin/bash
set -euxo pipefail
PS4='+ [boost ${SECONDS}s] '
:
:  =============================================================================
:  'Complete script: Build ICU + Boost with Clang + libc++ on Ubuntu LTS'
:  'Goal: Clean Boost libraries (especially regex & locale) without libstdc++.so dependency'
:
:  'Current as of January 2026'
:  'ICU 78.2 (latest stable), Boost 1.90.0'
:  =============================================================================


:  ==================== CONFIGURATION ====================

#	ICU_VERSION="78.2"
#	ICU_TAR="icu4c-78_2-src.tgz"
#	ICU_URL="https://github.com/unicode-org/icu/releases/download/release-78-2/${ICU_TAR}"
#	ICU_PREFIX="/usr/local/icu-libcxx-${ICU_VERSION}"

	ICU_VERSION="78.2"
	TAG="release-${ICU_VERSION}"
	ICU_TAR="${TAG}.tar.gz"
	ICU_URL="https://github.com/unicode-org/icu/archive/${TAG}.tar.gz"
	ICU_PREFIX="/usr/local/icu-libcxx-${ICU_VERSION}"

	BOOST_VERSION="1.90.0"
	BOOST_DIR="boost_1_90_0"
	BOOST_TAR="boost_1_90_0.tar.bz2"
	BOOST_URL="https://archives.boost.io/release/${BOOST_VERSION}/source/${BOOST_TAR}"
	BOOST_INSTALL_PREFIX="/usr/local"

	apt install -y bzip2 make
:
: Number of cores for parallel build
:
	JOBS=$(nproc)
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
	if ! which clang++ || !which clang ; then
		CLANG_VER=`c++ --version | head -n1|cut -d'.' -f 1|grep -Po '\d+'`
		sudo update-alternatives --install /usr/bin/clang   clang   /usr/bin/clang-$CLANG_VER   80
		sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-$CLANG_VER 80
	fi
:
: Starting ICU + Boost libc++ build process...
: ============================================
:
: STEP 1: Install dependencies
: ============================
: → Installing build dependencies...
:
	sudo apt install -y \
		wget \
		tar \
		xz-utils \
		pkg-config \
		libbz2-dev \
		zlib1g-dev \

#		build-essential
:
: STEP 2: Build Boost with custom ICU
: ===================================
: → Downloading and building Boost ${BOOST_VERSION}...
:
#	cd ~

	if [ ! -f "${BOOST_TAR}" ]; then
		wget --no-verbose --timestamping "${BOOST_URL}"
	fi

	if [ ! -d "${BOOST_DIR}" ]; then
		tar -xjf "${BOOST_TAR}"
	fi

	cd "${BOOST_DIR}"
:
: Clean previous build artifacts ?
: --------------------------------
	rm -rf bin.v2 || true
:
: Bootstrapping Boost with clang
: ------------------------------
#	test -x ./b2 ||
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
	sudo ./b2 -j${JOBS} \
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
		install --prefix="${BOOST_INSTALL_PREFIX}" | grep -vP "^(include/boost|common.copy)"

	ldconfig
:
: Boost ${BOOST_VERSION} installed successfully to ${BOOST_INSTALL_PREFIX}
:
:  ==================== VERIFICATION ====================

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
: You can now rebuild your project with -stdlib=libc++ flags.
: Recommended CMake additions:
:   add_compile_options\(-stdlib=libc++\)
:   add_link_options\(-stdlib=libc++\)
:
: Enjoy your clean libc++ Boost stack\! 🚀
