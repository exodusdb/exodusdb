#!/bin/bash
set -euxo pipefail

	pygmentize -l exoduscpp -f html -o test_act.htm test.cpp || exit 1
	diff test_exp.htm test_act.htm && echo Test passed.