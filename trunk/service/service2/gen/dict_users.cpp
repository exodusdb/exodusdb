#include <exodus/library.h>

libraryinit(user_and_dept_name)
function main() {
	return RECORD.a(1)^" - "^RECORD.a(5);
}
libraryexit(user_and_dept_name)


libraryinit(user_and_dept_name2)
function main() {
	return "222="^RECORD.a(1)^" - "^RECORD.a(5);
}
libraryexit(user_and_dept_name2)
