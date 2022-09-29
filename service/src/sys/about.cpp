#include <exodus/library.h>
libraryinit()

#include <system_common.h>

#include <sys_common.h>

var interactive;
var l10;
var l20;
var l30;
var colsep;
var text;
var file;
var nrecs;//num
var mod;
var dir;
var versiondata;
var ver;

function main() {

	interactive = false; //not(SYSTEM.f(33));

	if (interactive) {
		l10 = "L#10";
		l20 = "L#20";
		l30 = "L#30";
		colsep = " ";
	} else {
		l10 = "";
		l20 = "";
		l30 = "";
		colsep = "   ";
	}
	text = "Copyright NEOSYS Software. All rights reserved.";
	text ^= FM ^ FM ^ oconv("Module", l20) ^ colsep ^ oconv("Build date", l30);
	//text:='Year 2000' l10

	file = "SCHEDULES";
	gosub getnrecs();
	if (nrecs) {
		mod = "MEDIA";
		dir = "ADAGENCY";
		gosub getver();
	}

	file = "JOBS";
	gosub getnrecs();
	if (nrecs) {
		mod = "JOB";
		dir = "ADAGENCY";
		gosub getver();
	}

	file = "BALANCES";
	gosub getnrecs();
	if (nrecs) {
		mod = "ACCOUNTS";
		dir = "ACCOUNTS";
		gosub getver();
	}

	mod = "GENERAL";
	dir = "GENERAL";
	gosub getver();

	if (interactive) {
		text ^= FM;
	} else {
		text.replacer(FM, "\r\n");
	}

	call note(text);

	stop();

	return 0;

	return "";
}

subroutine getver() {
	versiondata = "/version.dat";
	versiondata.converter("/", OSSLASH);
	//osread ver from dir:'\version.dat' else return;*ver='Not installed'
	if (not(ver.osread(dir ^ versiondata))) {
		return;
	}

	text ^= FM ^ oconv(mod, l20) ^ colsep ^ oconv(ver, l30);
	//text:='Tested OK' l10

	return;
}

subroutine getnrecs() {
	nrecs = "";
	if (not(file.open(file, ""))) {
		return;
	}
	nrecs = file.reccount();
	if (not nrecs) {
		select(file);
		if (readnext(ID)) {
			nrecs = 1;
		}
		clearselect();
	}
	return;
}

libraryexit()
