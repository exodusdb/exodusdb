#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <shell2.h>

var printopts;
var pdfcmd;

function main(in printopts0, out errors) {
	//c sys in,out

	//gets filename from system<2> and updates system<2> if successful

		/* install for ubuntu 18.04;
		https://wkhtmltopdf.org/;
		wget https://downloads.wkhtmltopdf.org/0.12/0.12.5/wkhtmltox_0.12.5-1.bionic_amd64.deb;
		dpkg -i wkhtmltox_0.12.5-1.bionic_amd64.deb;
		apt --fix-broken install;
		wkhtmltopdf http://google.com google.pdf;
		*/

	var htmlfilename = SYSTEM.a(2);

	errors = "";

	//X means dont convert
	if (printopts0.unassigned()) {
		printopts = "";
	}else{
		printopts = printopts0;
	}
	if (printopts.index("X")) {
		return 0;
	}

	//only convert .htm files
	if (htmlfilename.lcase().substr(-4,4) ne ".htm") {
		return 0;
	}

	if (VOLUMES) {
		//WARNING TODO: check ternary op following;
		var exe = oscwd().index(":") ? ".exe" : "";
		pdfcmd = "html2pdf";
		if (exe) {
			pdfcmd = "c:\\windows\\" ^ pdfcmd ^ exe;
		}
	}else{
		pdfcmd = "/usr/local/bin/wkhtmltopdf";
	}

	//quit without conversion if conversion program doesnt exist
	if (not(pdfcmd.osfile())) {
		errors = "html2pdf is not installed.|EXODUS support has been emailed.";
		call sysmsg(errors, "html2pdf");
		return 0;
	}

	//generate an output filename
	var pdffilename = htmlfilename;
	pdffilename.splicer(-3, 3, "pdf");

	//if index(printopts,'F',1) then
	pdfcmd ^= " --footer-left [page]/[topage]";
	// end

	if (printopts.index("L")) {
		pdfcmd ^= " -O landscape";
	}

	pdfcmd ^= " --print-media-type";

	pdfcmd ^= " -q " ^ htmlfilename ^ " " ^ pdffilename;

	print("convhtml2pdf :");
	call shell2(pdfcmd, errors);

	if (errors) {
		errors = "html2pdf: " ^ errors;
	}else{
		SYSTEM.r(2, pdffilename);
	}

	return 0;
}

libraryexit()
