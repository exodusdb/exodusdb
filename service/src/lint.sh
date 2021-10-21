#!/bin/bash
set -euo pipefail

	./cpplint.py ???/*.cpp |&
	egrep -v '(whitespace|whitespace/line_length|whitespace/blank_line|whitespace/tab|whitespace/comments)' |
	grep -v alt_tokens |
	grep -v Copyright |
	grep -v readability/todo |
	grep -v 'Done proc' |
	grep -v readability/fn_size |
	grep -v 'already included'
