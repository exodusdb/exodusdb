#include <exodus/library.h>
libraryinit()

#include <shell2.h>
#include <sysmsg.h>

	var printopts;
var pdfcmd;

function main(io osfilename, in printopts0, out errors) {

	//osfilename in  ../data/xxxxxxxx.htm
	//osfilename out ../data/xxxxxxxx.pdf (if successful)

	/* Installation of wkhtmltopdf cmd for Ubuntu;

		//doesnt work well on 18.04 or 20.04
		sudo apt-get install wkhtmltopdf;

		//THIS WORKS

		https://wkhtmltopdf.org/downloads.html;

		# 18.04 bionic
		wget https://github.com/wkhtmltopdf/packaging/releases/download/0.12.6-1/wkhtmltox_0.12.6-1.bionic_amd64.deb;
		dpkg -i wkhtmltox_0.12.6-1.bionic_amd64.deb;
		apt --fix-broken install;

		# 20.04 focal
		wget https://github.com/wkhtmltopdf/packaging/releases/download/0.12.6-1/wkhtmltox_0.12.6-1.focal_amd64.deb;
		dpkg -i wkhtmltox_0.12.6-1.focal_amd64.deb;
		apt --fix-broken install;

		https://wkhtmltopdf.org/;
		wget https://downloads.wkhtmltopdf.org/0.12/0.12.5/wkhtmltox_0.12.5-1.bionic_amd64.deb;
		dpkg -i wkhtmltox_0.12.5-1.bionic_amd64.deb;
		apt --fix-broken install;

		//test
		/usr/local/bin/wkhtmltopdf http://google.com google.pdf;

		*/

	errors = "";

	//X means dont convert
	if (printopts0.unassigned()) {
		printopts = "";
	} else {
		printopts = printopts0;
	}
	if (printopts.contains("X")) {
		return 0;
	}

	//only convert .htm files
	if (osfilename.lcase().last(4) ne ".htm") {
		return 0;
	}

	if (VOLUMES) {
		var exe = oscwd().contains(":") ? ".exe" : "";
		pdfcmd	= "html2pdf";
		if (exe) {
			pdfcmd = "c:\\windows\\" ^ pdfcmd ^ exe;
		}
	} else {
		pdfcmd = "/usr/local/bin/wkhtmltopdf";
		if (not(pdfcmd.osfile())) {
			pdfcmd = "/usr/bin/wkhtmltopdf";
		}
	}

	//quit without conversion if conversion program doesnt exist
	if (not(pdfcmd.osfile())) {
		errors = "html2pdf is not installed.|EXODUS support has been emailed.";
		call sysmsg(errors, "html2pdf");
		return 0;
	}

	//generate an output filename
	var pdffilename = osfilename;
	pdffilename.paster(-3, 3, "pdf");

	//if index(printopts,'F',1) then
	pdfcmd ^= " --footer-left [page]/[topage]";
	// end

	if (printopts.contains("L")) {
		pdfcmd ^= " -O landscape";
	}

	pdfcmd ^= " --print-media-type";
	//required on 18.04 to access local images
	pdfcmd ^= " --enable-local-file-access";

	pdfcmd ^= " -q " ^ osfilename ^ " " ^ pdffilename;

	print("convhtml2pdf :");
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
