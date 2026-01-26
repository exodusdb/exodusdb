#!/bin/bash
set -euxo pipefail
PS4='+ [clang ${SECONDS}s] '
:
: =================================================
: 'Install clang from llvm and precompile std.cppm'
:
: 'Syntax is ./install_clang.sh [CLANG_VER] [CPPSTD]'
: =================================================
:
	CLANG_VER=${1:-22}
	CPPSTD=${2:-c++26}
:
: Config for libc++, /usr/local and std module
:
	STDLIB=libc++
	INSTALL_PREFIX=/usr/local
#	CLANG_FLAGS=-std=c++26 -stdlib=libc++
	STD_CPPM_FILE="/usr/lib/llvm-$CLANG_VER/share/libc++/v1/std.cppm"
:
: Validate
: --------
:
	if [[ ! $CPPSTD =~ \+\+ ]]; then
		echo "CPPSTD must be like c++20, c++2c, gnu++23 etc."
		exit 1
	fi
#:
#: 0. Upgrade
#:
#	sudo apt update -q
#	sudo apt upgrade -y
:
: 1. llvm.sh - Download and run the automatic installer script for version $CLANG_VER
:
	curl -LOs https://apt.llvm.org/llvm.sh
	chmod +x llvm.sh
	sudo ./llvm.sh $CLANG_VER
:
: 2. Install all the important and useful clang packages
:
	sudo apt -y install clang-$CLANG_VER lld-$CLANG_VER libc++-$CLANG_VER-dev libc++abi-$CLANG_VER-dev clang-tools-$CLANG_VER
:
: Optional but very useful extras
:
:	sudo apt install clangd-$CLANG_VER libomp-$CLANG_VER-dev
	sudo apt install clang-format-$CLANG_VER clang-tidy-$CLANG_VER
:
: Configure standard command line access c++ and clang++ for c++, cc and clang for c
:
: 1. First make sure the alternatives are registered.
:
	sudo update-alternatives --install /usr/bin/clang   clang   /usr/bin/clang-$CLANG_VER   80
	sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-$CLANG_VER 80
:
: Optional: also register the generic c++ / cc names. Very useful
:
	sudo update-alternatives --install /usr/bin/c++  c++  /usr/bin/clang++-$CLANG_VER 80
	sudo update-alternatives --install /usr/bin/cc   cc   /usr/bin/clang-$CLANG_VER   80
	sudo update-alternatives --install /usr/bin/lldb lldb /usr/bin/lldb-$CLANG_VER   80
:
: 2. Then just set the alternatives.
:
	sudo update-alternatives --set clang++ /usr/bin/clang++-$CLANG_VER    # boost bootstrap uses this
	sudo update-alternatives --set c++     /usr/bin/clang++-$CLANG_VER    # ← most build tools use this
:
	sudo update-alternatives --set clang   /usr/bin/clang-$CLANG_VER
	sudo update-alternatives --set cc      /usr/bin/clang-$CLANG_VER
	sudo update-alternatives --set lldb    /usr/bin/lldb-$CLANG_VER
#:
#: 'Precompile std.pcm to allow import std;'
#: =======================================
#:
#: 1. Locate std.cppm. path can vary a little.
#:
#: Common locations in 2025–2026:
#: /usr/lib/llvm-$CLANG_VER/share/libc++/v1/std.cppm
#: /usr/share/libc++/v1/std.cppm
#:
#:
#: 2. Precompile the std module. BMI = .pcm file
#:
#	clang++-$CLANG_VER \
#		-std=$CPPSTD \
#		-stdlib=$STDLIB \
#		-Wno-reserved-identifier \
#		-Wno-reserved-module-identifier \
#		--precompile \
#		-o std.pcm \
#		$STD_CPPM_FILE
#
#	mv std.pcm $INSTALL_PREFIX/lib
#	echo $CPPSTD > $INSTALL_PREFIX/lib/cppstd
#
#:
#: Create a cpp file to test import std;
#:
#	TESTFILE=/tmp/test_clang_$CLANG_VER
#	cat <<-'EOF' > $TESTFILE.cpp
#		import std;
#		#include <version>
#		int main() {
#		#if __cpp_lib_print
#			std::println("std::println: Hello modules from Clang-{} c++{} + libc++! π ≈ {:.6}", __clang_major__, __cplusplus/100%100, std::numbers::pi);
#		#else
#			std::printf("std::printf: Hello modules from Clang-%d c++%ld + libc++! π ≈ %.6f\n", __clang_major__, __cplusplus/100%100, std::numbers::pi);
#		#endif
#			std::printf("std::string SSO capacity is %ld\n", std::string().capacity());
#			return 0;
#		}
#	EOF
#:
#: Compile it
#:
#	clang++-$CLANG_VER \
#		-std=$CPPSTD \
#		-stdlib=$STDLIB \
#		-fprebuilt-module-path=$INSTALL_PREFIX/lib \
#		-o $TESTFILE \
#		$TESTFILE.cpp
#:
#: Run it
#:
#	$TESTFILE
#:
#: Verify output
#:
#	$TESTFILE | grep "Clang-$CLANG_VER"
#	rm $TESTFILE.cpp $TESTFILE
:
: Clean up
:
	rm llvm.sh
:
: =======================================
: $0 $* Completed
: =======================================
