#!/bin/bash
set -euxo pipefail

:
: Extract function headers from timebank.h
: ----------------------------------------
#:
#: e.g.
#:
#: '  else if (++acno && arg1 == "void dim::write(in filevar, in key) const") {}'
#:
# appending || exit 0 to avoid breaking build after ninja clean but why?
	grep -P -v "'^\s*//'" timebank.h|grep -P 'arg1 == "(.*)' -o timebank.h|cut -d'"' -f2-| cut -d'{' -f1| sed 's/ $//g'| sed 's/)$//g' | sed 's/"$//g' | nl -v 500 -s' ' -w3 > timebank.txt.new || exit 0

:
: Update timebank.txt if changed
: ------------------------------
#:
#: e.g.
#:
#: '504 void dim::write(in filevar, in key) const'
#:
	diff timebank.txt.new timebank.txt || ( cp -v timebank.txt.new timebank.txt && echo Updated )

	rm timebank.txt.new -f
#:
#: Finished $0 $*
#: ------------------------