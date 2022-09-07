#!/bin/bash
set -euo pipefail

	FILESPEC=${1:-*.cpp}

	./cpplint.py $FILESPEC |&
	egrep -v '(whitespace|whitespace/line_length|whitespace/blank_line|whitespace/tab|whitespace/comments|brace on one side)' |
	grep -v alt_tokens |
	grep -v Copyright |
	grep -v readability/todo |
	grep -v 'Done proc' |
	grep -v readability/fn_size |
	grep -v 'already included'
