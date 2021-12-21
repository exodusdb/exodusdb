#!/bin/bash
set -eux

: ======================================
: Floating point to_chars implementation
: ======================================
:
: 'No longer required in g++11 in Ubuntu 22.04 since it is built-in'
:
: 'Both Clangs libc++ and GCCs libstdc++ now support floating-point to_chars() powered by Ryu.'
: 'Clangs implementation was ported by @mordante from MSVC, which in turn was derived from'
: 'Ulfs repo here, and is available in Clang trunk (should be part of Clang 14).'
: 'GCCs implementation shipped in GCC 11, see their release notes.'
: 'Example on Compiler Explorer: https://godbolt.org/z/f8Ms36qxc Note that you must enable -std=c++17 for this feature.'
:

: install basel build system
: ==========================
:
	#https://docs.bazel.build/versions/master/install-ubuntu.html
	sudo apt install -y apt-transport-https curl gnupg
	curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > bazel.gpg
	sudo mv bazel.gpg /etc/apt/trusted.gpg.d/
	echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
	sudo apt update && sudo apt -y install bazel

: get ryu
: =======
:
	test -d ryu || git clone https://github.com/ulfjack/ryu.git

: build and test
: ==============
:
	cd ryu
	bazel build ryu/ryu || true
	bazel test ryu/ryu || true
	bazel build ryu/ryu_parse || true
	bazel test ryu/ryu_parse || true

: install
: =======
:
	cp bazel-bin/ryu/libryu.{a,so} /usr/local/lib
	cp bazel-bin/ryu/libryu_parse.{a,so} /usr/local/lib
	mkdir -p /usr/local/include/ryu
	cp ryu/{ryu.h,ryu_parse.h} /usr/local/include/ryu
	ldconfig

