#include <exodus/library.h>
libraryinit()

#include <shell2.h>
#include <sysmsg.h>

var printopts;
var pdfcmd;

function main(io osfilename, in printopts0, out errors) {

	// osfilename in  ../data/xxxxxxxx.htm
	// osfilename out ../data/xxxxxxxx.pdf (if successful)

	// Prefer chromium now instead of wkhtmltopdf which may not be available after 24.04

	/* Installation of chromium for Ubuntu
		Ubuntu 20.04, 22.04, 24.04 and probably other versions
		sudo snap install chromium
	*/

	/* Installation of wkhtmltopdf cmd for Ubuntu;

		See exodus/srvice/install_all.sh for exact script

		// doesnt work well on 18.04 or 20.04
		sudo apt-get install wkhtmltopdf;

		// THIS WORKS

		https://wkhtmltopdf.org/downloads.html;

		# 18.04 bionic
		wget https://github.com/wkhtmltopdf/packaging/releases/download/0.12.6-1/wkhtmltox_0.12.6-1.bionic_amd64.deb;
		dpkg -i wkhtmltox_0.12.6-1.bionic_amd64.deb;
		apt-get --fix-broken install;

		# 20.04 focal
		wget https://github.com/wkhtmltopdf/packaging/releases/download/0.12.6-1/wkhtmltox_0.12.6-1.focal_amd64.deb;
		dpkg -i wkhtmltox_0.12.6-1.focal_amd64.deb;
		apt-get --fix-broken install;

//		# 22.04 jammy
//		https://wkhtmltopdf.org/;
//		wget https://downloads.wkhtmltopdf.org/0.12/0.12.5/wkhtmltox_0.12.5-1.bionic_amd64.deb;
//		dpkg -i wkhtmltox_0.12.5-1.bionic_amd64.deb;
//		apt-get --fix-broken install;

		# 22.04 jammy
		# 24.04 noble use older version
		wget https://github.com/wkhtmltopdf/packaging/releases/download/0.12.6.1-2/wkhtmltox_0.12.6.1-2.jammy_amd64.deb
		dpkg -i wkhtmltox_0.12.6.1-2.jammy_amd64.deb;
		apt-get --fix-broken install;

		// test
		/usr/local/bin/wkhtmltopdf http://google.com google.pdf;

		*/

	errors = "";

	// X means dont convert
//	if (printopts0.unassigned()) {
//		printopts = "";
//	} else {
//		printopts = printopts0;
//	}
	printopts = printopts0.or_default("");
	if (printopts.contains("X")) {
		return 0;
	}

	// only convert .htm files
//	if (osfilename.lcase().last(4) != ".htm") {
	if (not osfilename.lcase().ends(".htm")) {
		return 0;
	}

	// generate an output filename
	var pdffilename = osfilename;
	pdffilename.paster(-3, 3, "pdf");

//	if (VOLUMES) {
//		let exe = oscwd().contains(":") ? ".exe" : "";
//		pdfcmd	= "html2pdf";
//		if (exe) {
//			pdfcmd = "c:\\windows\\" ^ pdfcmd ^ exe;
//		}
//	} else {

	// Test is duplicated in convpdf.cpp and htmllib2.cpp
	pdfcmd = "chromium";
	if (osshell("which " ^ pdfcmd ^ " > /dev/null")) {

		// Required confirmation to run unsafe if user is root
		pdfcmd ^= " --no-sandbox";

		// Run in the background
		pdfcmd ^= " --headless";

		// Only required for older versions of chromium when headless
		pdfcmd ^= " --disable-gpu";

		// Output file
		pdfcmd ^= " --print-to-pdf=" ^ pdffilename;

		// Input file
		pdfcmd ^= " " ^ osfilename;

		// By default, file:// URIs cannot read other file:// URIs. This is an override for developers who need the old behavior for testing. ↪
		//pdfcmd ^= " --allow-file-access-from-files";

		// Orientation and header footer are created in htmllib2
		//pdfcmd ^= " --no-pdf-header-footer";
		//pdfcmd ^= " --use-system-default-printer";

	} else {

		pdfcmd = "/usr/local/bin/wkhtmltopdf";
		if (not pdfcmd.osfile()) {
			pdfcmd = "/usr/bin/wkhtmltopdf";
		}
//	}

		// quit without conversion if conversion program doesnt exist
		if (not pdfcmd.osfile()) {
			errors = "convpdf: chromium/wkhtmltopdf converters cannot be found.|EXODUS support has been emailed.";
			call sysmsg(errors, "html2pdf");
			return 0;
		}

		// if index(printopts,'F',1) then
		pdfcmd ^= " --footer-left [page]/[topage]";
		// end

		if (printopts.contains("L")) {
			pdfcmd ^= " -O landscape";
		}

		pdfcmd ^= " --print-media-type";
		// required on 18.04 to access local images
		pdfcmd ^= " --enable-local-file-access";

		pdfcmd ^= " -q " ^ osfilename ^ " " ^ pdffilename;

	}

	printx("convhtml2pdf :");
	call shell2(pdfcmd, errors);

	if (errors) {
		errors = "html2pdf: " ^ errors;
		printl(pdfcmd);
		printl(errors);
	} else {
		osfilename = pdffilename;
	}

	return 0;
}

libraryexit()
