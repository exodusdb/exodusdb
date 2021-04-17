#!/bin/bash
set -eux

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
	git clone https://github.com/ulfjack/ryu.git

: build and test
: ==============
:
	cd ryu
	bazel build ryu/ryu
	bazel test ryu/ryu
	bazel build ryu/ryu_parse
	bazel test ryu/ryu_parse

: install
: =======
:
	cp bazel-bin/ryu/libryu.{a,so} /usr/local/lib
	cp bazel-bin/ryu/libryu_parse.{a,so} /usr/local/lib
	mkdir -p /usr/local/include/ryu
	cp ryu/{ryu.h,ryu_parse.h} /usr/local/include/ryu
	ldconfig

