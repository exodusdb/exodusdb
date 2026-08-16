#include <exodus/library.h>
#include <srv_common.h>

// performed by generalproxy mode "ABOUT"
// Help → About: library build info, web module dates, bin/lib file ranges
libraryinit()

#include <service_common.h>

var text;
var is_exodus;	 // num

func main() {

	is_exodus = USERNAME == "EXODUS";

	text = "Copyright NEOSYS / EXODUS. All rights reserved.";

	// ── Exodus library (git stamp baked into libexodus at cmake time) ──
	// version() returns Local/Remote branch timestamps + commit URLs (see var::version)
	gosub section("EXODUS library");
	var ver = version().convert("\n", FM);
	if (is_exodus)
		text ^= FM ^ ver;
	else
		// non-EXODUS: first line only (Local: branch date hash)
		text ^= FM ^ ver.f(1);

	// Build environment available to every Exodus program
	gosub section("Build");
	text ^= FM ^ "OS:       " ^ _OS_NAME ^ " " ^ _OS_VERSION;
	text ^= FM ^ "Platform: " ^ PLATFORM ^ "  " ^ _COMPILER ^ " " ^ var(_COMPILER_VERSION) ^ "  c++" ^ var(_CPP_STANDARD);

	// Process file date (listen binary via EXECPATH)
	var execinfo = EXECPATH.osfile();
	if (execinfo) {
		var line = fmtdatetime(execinfo) ^ "  process";
		if (is_exodus)
			line ^= "  " ^ EXECPATH;
		text ^= FM ^ line;
	}

	// ── Web modules under service/www/3 ──
	// osinfo(6) refreshes each dir mtime from its contents, then we report that date
	var www3 = findpath("exodus/service/www/3", var("../www/3") ^ FM ^ "../../www/3" ^ FM ^ "www/3");
	gosub section("Web modules");
	if (www3) {
		if (is_exodus)
			text ^= "  (" ^ www3 ^ ")";
		gosub listwebmodules(www3);
	} else {
		text ^= FM ^ "(not found)";
	}

	// ── File date ranges in $EXO_HOME/bin and lib ──
	var exo_home = osgetenv("EXO_HOME");
	if (not exo_home)
		exo_home = osgetenv("HOME");
	gosub section("Service");
	if (exo_home) {
		if (is_exodus)
			text ^= "  (" ^ exo_home ^ ")";
		gosub filerange(exo_home ^ OSSLASH ^ "bin", "bin");
		gosub filerange(exo_home ^ OSSLASH ^ "lib", "lib");
	} else {
		text ^= FM ^ "(EXO_HOME/HOME not set)";
	}

	// Prefer data_ so client displayresponsedata shows the body cleanly
	// (exoui_note converts FM to newlines on the browser)
	data_ = text;
	call note(text);

	return 0;
}

// Blank line then section title. Use a space-only blank line so the web
// confirm popup (HTML) still shows a gap — empty <p/> / stacked <br> can collapse.
subr section(in title) {
	text ^= FM ^ " " ^ FM ^ title;
	return;
}

// Format osfile/osdir/osinfo fields 2+3 as "yyyy/mm/dd hh:mm:ss" (date first for alignment)
func fmtdatetime(in info) {
	if (not info)
		return "";
	// f(2)=date (pick dayno), f(3)=time (seconds)
	var d = info.f(2);
	var t = info.f(3);
	if (not d)
		return "";
	// zero-pad month and day
	var s = d.oconv("DY") ^ "/" ^ ("00" ^ d.oconv("DM")).last(2) ^ "/" ^ ("00" ^ d.oconv("DD")).last(2);
	if (t != "")
		s ^= " " ^ t.oconv("MTS");
	return s;
}

// Home-relative path, else first relative candidate that is a directory
func findpath(in under_home, in relatives_fm) {

	var home = osgetenv("HOME");
	if (home) {
		var p = home ^ OSSLASH ^ under_home;
		p.converter("/", OSSLASH);
		if (p.osdir())
			return p;
	}

	var n = relatives_fm.fcount(FM);
	for (var i = 1; i <= n; ++i) {
		var p = relatives_fm.f(i);
		p.converter("/", OSSLASH);
		if (p.osdir())
			return p;
	}
	return "";
}

// One line per web module dir: "yyyy/mm/dd hh:mm:ss  name"
// osinfo(6) updates dir mtime from contents and returns latest content date
subr listwebmodules(in dirpath) {

	var names = oslistd(dirpath);
	if (not names) {
		text ^= FM ^ "(empty or unreadable)";
		return;
	}
	names = names.sort();

	var nn = names.fcount(FM);
	for (var i = 1; i <= nn; ++i) {
		var name = names.f(i);
		if (name == "." or name == "..")
			continue;

		var path = dirpath ^ OSSLASH ^ name;
		if (not path.osdir())
			continue;

		// mode 6: recursive content mtime + update dir stamps so the figure is trustworthy
		var info = path.osinfo(6);
		if (not info)
			info = path.osdir();

		text ^= FM ^ fmtdatetime(info) ^ "  " ^ name;
	}
	return;
}

// Min..max file mtime among top-level files in a directory
subr filerange(in dirpath, in label) {

	var path = dirpath;
	path.converter("/", OSSLASH);
	if (not path.osdir()) {
		text ^= FM ^ "(no " ^ label ^ ")";
		return;
	}

	var names = oslistf(path);
	if (not names) {
		text ^= FM ^ "(empty " ^ label ^ ")";
		return;
	}

	var mindt = "";
	var maxdt = "";
	var mininfo = "";
	var maxinfo = "";

	var nn = names.fcount(FM);
	for (var i = 1; i <= nn; ++i) {
		var name = names.f(i);
		if (name == "." or name == "..")
			continue;
		var fpath = path ^ OSSLASH ^ name;
		var info  = fpath.osfile();
		if (not info)
			continue;
		// comparable key: date * 1e5 + time (time is seconds of day)
		var key = info.f(2) ^ "." ^ info.f(3).oconv("R(0)#5");
		if (mindt == "" or key < mindt) {
			mindt	= key;
			mininfo = info;
		}
		if (maxdt == "" or key > maxdt) {
			maxdt	= key;
			maxinfo = info;
		}
	}

	if (not mininfo) {
		text ^= FM ^ "(no files in " ^ label ^ ")";
		return;
	}

	// date first: "from - to  label"
	text ^= FM ^ fmtdatetime(mininfo) ^ " - " ^ fmtdatetime(maxinfo) ^ "  " ^ label;
	return;
}

}; // libraryexit()
