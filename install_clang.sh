#!/bin/bash
set -euxo pipefail
:
: 'Install clang from llvm and precompile std.cppm'
: ===============================================
:
	CLANG_VER=${1:-22}
:
: 0. Upgrade
:
	sudo apt update
	sudo apt upgrade -y
:
: 1. Download and run the automatic installer script for version $CLANG_VER
:
	wget https://apt.llvm.org/llvm.sh
	chmod +x llvm.sh
	sudo ./llvm.sh $CLANG_VER
:
: 2. Install the important packages. Including libc++ for modules
:
	sudo apt -y install clang-$CLANG_VER lld-$CLANG_VER libc++-$CLANG_VER-dev libc++abi-$CLANG_VER-dev clang-tools-$CLANG_VER
:
: Optional but very useful extras
:
:	sudo apt install clangd-$CLANG_VER libomp-$CLANG_VER-dev
:	sudo apt install clang-format-$CLANG_VER clang-tidy-$CLANG_VER
:
: 'Configure standard command line access'
: c++ and clang++ for c++, cc and clang for c
: ===========================================
:
: 1. First make sure the alternatives are registered.
:
	sudo update-alternatives --install /usr/bin/clang   clang   /usr/bin/clang-$CLANG_VER   80
	sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-$CLANG_VER 80
:
: Optional: also register the generic c++ / cc names. Very useful
:
	sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-$CLANG_VER 80
	sudo update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-$CLANG_VER   80
:
: 2. Then just set them. This is the non-interactive part
:
	sudo update-alternatives --set clang++ /usr/bin/clang++-$CLANG_VER    # boost bootstrap uses this
	sudo update-alternatives --set c++     /usr/bin/clang++-$CLANG_VER    # ← most build tools use this
:
	sudo update-alternatives --set clang   /usr/bin/clang-$CLANG_VER
	sudo update-alternatives --set cc      /usr/bin/clang-$CLANG_VER
:
: 'Precompile std.pcm to allow import std;'
: =======================================
:
: 1. Locate std.cppm. path can vary a little.
:
: Common locations in 2025–2026:
: /usr/lib/llvm-$CLANG_VER/share/libc++/v1/std.cppm
: /usr/share/libc++/v1/std.cppm
:
	STD_CPPM_FILE="/usr/lib/llvm-$CLANG_VER/share/libc++/v1/std.cppm"
:
: 2. Precompile the std module. BMI = .pcm file
:
	clang++-$CLANG_VER -std=c++26 -stdlib=libc++ \
		-Wno-reserved-identifier -Wno-reserved-module-identifier \
		--precompile -o std.pcm "$STD_CPPM_FILE"
	mv std.pcm /usr/local/lib

#main.cpp
#import std;
#
#int main() {
#    std::print("Hello modules from Clang " __clang_major__ " + libc++! π ≈ {:.6}\n", std::numbers::pi);
#}
#	# 3. Compile & link your program
#	clang++-$CLANG_VER -std=c++23 -stdlib=libc++ \
#	  -fmodule-file=std=std.pcm \
#	  std.pcm main.cpp -o myprogram
#
#	# Run
#./myprogram
