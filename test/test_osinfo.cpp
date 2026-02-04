#include <exodus/program.h>
programinit()

// Test osinfo (6)'s ability to recursively update dir mod times and return recursive dir size

// Currently only tests osinfo recursive modes. See test_os for normal usage.

// Can be tested manually on / e.g. compile 'osinfo("/", 8)'

// recursive modes
// mode 4 = just return max date and recursive size without updating os dir times
// mode 6 = mode 4 plus update fs dir times where necessary
// mode 8 = undocumented osinfo mode 8 which

let origin = R"(
drwxr-xr-x 1 root root  8 2026-02-02 06:30:26.363399901 +0000 a1
drwxr-xr-x 1 root root 16 2025-03-01 19:00:03.123456789 +0000 a1/b1
drwxr-xr-x 1 root root  4 2025-02-01 19:00:02.123456789 +0000 a1/b1/c1
drwxr-xr-x 1 root root 12 2025-01-01 19:00:01.123456789 +0000 a1/b1/c1/d1
-rw-r--r-- 1 root root 10 2025-01-01 20:00:01.123456789 +0000 a1/b1/c1/d1/f1
-rw-r--r-- 1 root root 10 2025-01-01 20:00:00.123456789 +0000 a1/b1/c1/d1/f2
-rw-r--r-- 1 root root 10 2025-01-01 20:00:03.123456789 +0000 a1/b1/c1/d1/f3
-rw-r--r-- 1 root root 10 2025-01-01 20:00:12.123456789 +0000 a1/b1/f1
-rw-r--r-- 1 root root 10 2025-01-01 20:00:10.123456789 +0000 a1/b1/f2
-rw-r--r-- 1 root root 10 2025-01-01 20:00:11.123456789 +0000 a1/b1/f3
drwxr-xr-x 1 root root 16 2024-01-01 00:00:00.123456789 +0000 a1/b2
drwxr-xr-x 1 root root  4 2024-01-01 00:00:00.123456789 +0000 a1/b2/c2
drwxr-xr-x 1 root root 20 2026-02-02 06:34:26.388400104 +0000 a1/b2/c2/d2
-rw-r--r-- 1 root root 10 2024-01-02 00:00:00.123456789 +0000 a1/b2/c2/d2/g1
-rw-r--r-- 1 root root 10 2024-01-01 00:00:00.123456789 +0000 a1/b2/c2/d2/g2
-rw-r--r-- 1 root root 10 2024-01-04 00:00:00.123456789 +0000 a1/b2/c2/d2/g3
lrwxrwxrwx 1 root root  7 2026-02-02 06:34:26.386400088 +0000 a1/b2/c2/d2/l1 -> nowhere
lrwxrwxrwx 1 root root 17 2026-02-02 06:34:26.388400104 +0000 a1/b2/c2/d2/l2 -> ../../../b1/c1/d1
-rw-r--r-- 1 root root 10 2024-01-12 00:00:00.123456789 +0000 a1/b2/g1
-rw-r--r-- 1 root root 10 2024-01-10 00:00:00.123456789 +0000 a1/b2/g2
-rw-r--r-- 1 root root 10 2024-01-11 00:00:00.123456789 +0000 a1/b2/g3
)"_var.cut(1);

let expect = R"(
drwxr-xr-x 1 root root  8 2026-02-02 06:34:26.388400104 +0000 a1
drwxr-xr-x 1 root root 16 2025-03-01 19:00:03.123456789 +0000 a1/b1
drwxr-xr-x 1 root root  4 2025-02-01 19:00:02.123456789 +0000 a1/b1/c1
drwxr-xr-x 1 root root 12 2025-01-01 20:00:03.123456789 +0000 a1/b1/c1/d1
-rw-r--r-- 1 root root 10 2025-01-01 20:00:01.123456789 +0000 a1/b1/c1/d1/f1
-rw-r--r-- 1 root root 10 2025-01-01 20:00:00.123456789 +0000 a1/b1/c1/d1/f2
-rw-r--r-- 1 root root 10 2025-01-01 20:00:03.123456789 +0000 a1/b1/c1/d1/f3
-rw-r--r-- 1 root root 10 2025-01-01 20:00:12.123456789 +0000 a1/b1/f1
-rw-r--r-- 1 root root 10 2025-01-01 20:00:10.123456789 +0000 a1/b1/f2
-rw-r--r-- 1 root root 10 2025-01-01 20:00:11.123456789 +0000 a1/b1/f3
drwxr-xr-x 1 root root 16 2026-02-02 06:34:26.388400104 +0000 a1/b2
drwxr-xr-x 1 root root  4 2026-02-02 06:34:26.388400104 +0000 a1/b2/c2
drwxr-xr-x 1 root root 20 2026-02-02 06:34:26.388400104 +0000 a1/b2/c2/d2
-rw-r--r-- 1 root root 10 2024-01-02 00:00:00.123456789 +0000 a1/b2/c2/d2/g1
-rw-r--r-- 1 root root 10 2024-01-01 00:00:00.123456789 +0000 a1/b2/c2/d2/g2
-rw-r--r-- 1 root root 10 2024-01-04 00:00:00.123456789 +0000 a1/b2/c2/d2/g3
lrwxrwxrwx 1 root root  7 2026-02-02 06:34:26.386400088 +0000 a1/b2/c2/d2/l1 -> nowhere
lrwxrwxrwx 1 root root 17 2026-02-02 06:34:26.388400104 +0000 a1/b2/c2/d2/l2 -> ../../../b1/c1/d1
-rw-r--r-- 1 root root 10 2024-01-12 00:00:00.123456789 +0000 a1/b2/g1
-rw-r--r-- 1 root root 10 2024-01-10 00:00:00.123456789 +0000 a1/b2/g2
-rw-r--r-- 1 root root 10 2024-01-11 00:00:00.123456789 +0000 a1/b2/g3
)"_var.cut(1);

function main() {
	printl("test_osinfo says 'Hello World!'");

	// Temporary working dir
	let tempdirname = "t_osinfo";
	if ((osdir(tempdirname) and not osrmdir(tempdirname, true)) or not osmkdir(tempdirname) or not oscwd(tempdirname))
		abort(lasterror());

	// Save the origin and expected data
	if (not oswrite(origin on "t_origin.txt")) abort(lasterror());
	if (not oswrite(expect on "t_expect.txt")) abort(lasterror());
	gosub cleancols("t_origin.txt");
	gosub cleancols("t_expect.txt");

	// multiple spaces are just column separators
	var raw = origin.trim();
	raw.trimmer();
	raw.converter("\n ", FM ^ VM);
	for (let line : raw) {
		if (not line)
			continue;
		//TRACE(line)
		let filename = line.f(1, 9);
		if (line.starts("d")) {
			if (not osmkdir(filename)) {
				lasterror().errputl();
				abort(lasterror());
			}
		}
		else if (line.starts("l")) {
			if (not osshell("ln --symbolic " ^ line.f(1, 11) ^ " " ^ filename))
				abort(lasterror());
		}
		else {
			if (not oswrite("0123456789" on filename))
				abort(lasterror());
		}
	}
	// set times in reverse order
	for (let line : raw.reverse()) {
		if (not line)
			continue;
		let filename = line.f(1, 9);
		let cmd = "touch   " ^ filename ^ " -m --no-dereference --date='" ^ line.f(1, 6) ^ " " ^ line.f(1, 7) ^ "'";
//		let cmd = "touch   " ^ filename ^ " -m --date='" ^ line.f(1, 6) ^ " " ^ line.f(1, 7) ^ "'";
		if (not osshell(cmd))
			abort(lasterror());
	}

	// lambda to generate tree info
	auto write_tree_info = [this](in tagname) {
		let cmd = "ls -ld --full-time `find a1`";
		var txt;
		if (not txt.osshellread(cmd))
			abort(lasterror());
		if (not oswrite(txt on "t_" ^ tagname ^ ".txt"))
			abort(lasterror());
		gosub cleancols("t_" ^ tagname ^ ".txt");

	};

	// Verify successful creation of initial dir tree
	write_tree_info("before");
	if (not osshell("diff t_origin.txt t_before.txt"))
		abort(lasterror());

	// Check osinfo (2) doesnt return latest mod time nor total size
	{
		logputl();
		TRACE(osinfo("a1", 2))
		if (osinfo("a1", 2) ne "^21218^23426"_var)
			abort("osinfo 2 not as expected (expected \"^21218^23426\")");
	}

	// Check osinfo (4) does return latest mod time and total size without updating
	{
		var osinfo4 = osinfo("a1", 4);
		TRACE(osinfo4)
		if (osinfo4 ne "120^21218^23666"_var)
			abort("osinfo(4) not as expected (expected \"120^21218^23666\")");
	}

	// Call undoc test mode 8 and verify it returns the latest mod time and total size
	logputl();
	var osinfo8 = osinfo("a1", 8);
	TRACE(osinfo8)
	if (not osinfo8 eq "120^21218^23666"_var)
		abort("osinfo(8) test mode passed but result not as expected (expected \"120^21218^23666\"");

	//////////////////////////////////////////
	// Update all the dir times using osinfo 6
	//////////////////////////////////////////
	logputl();
	var osinfo6 = osinfo("a1", 6);
	TRACE(osinfo6)
	if (osinfo6 ne "120^21218^23666"_var)
		abort("osinfo(6) not as expected (expected \"120^21218^23666\")");

	// Verify expected result
	write_tree_info("result");
	if (not osshell("diff t_expect.txt t_result.txt"))
		abort(lasterror());

	// Check osinfo (2) now returns latest mod time but not total size
	{
		logputl();
		var osinfo2 = osinfo("a1", 2);
		TRACE(osinfo2)
		if (osinfo2 ne "^21218^23666"_var)
			abort("osinfo(2) (after osinfo(6) not as expected (expected \"^21218^23666\")");
	}

	// Check osinfo (4) still returns latest mod time and total size
	{
		logputl();
		var osinfo4 = osinfo("a1", 4);
		TRACE(osinfo4)
		if (osinfo4 ne "120^21218^23666"_var)
			abort("osinfo(4) (after osinfo(6) not as expected (expected \"120^21218^23666\")");
	}

	printl("Test passed.");

	return 0;
}

subr cleancols(in txtfilename) {
	// Remove the first 5 fragile columns except the first letter of the line
	// drwxr-xr-x 1 root root  8 2026-02-02 06:30:26.363399901 +0000 a1
	if (not osshell(R"(sed -E -i 's/^(\S)(\S+\s+)(\S+\s+)(\S+\s+)(\S+\s+)(\S+\s+)/\1 /' )" ^ txtfilename))
		abort(lasterror());
}

}; // programexit()
