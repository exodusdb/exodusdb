#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <shell2.h>


var printopts;

function main(in printopts0, out errors) {
	//c sys in,out

	//gets filename from system<2> and updated system<2> if successful

	var pdfcmd = "c:\\windows\\html2pdf.exe";

	var htmlfilename = SYSTEM.a(2);

	errors = "";

	//X means dont convert
	if (printopts0.unassigned()) {
		printopts = "";
	}else{
		printopts = printopts0;
	}
	if (printopts.index("X", 1)) {
		return 0;
	}

	//only convert .HTM files
	if (htmlfilename.ucase().substr(-4,4) ne ".HTM") {
		return 0;
	}

	//quit without conversion if conversion program doesnt exist
	if (not(pdfcmd.osfile())) {
		errors = "html2pdf is not installed.|NEOSYS support has been emailed.";
		call sysmsg(errors, "html2pdf");
		return 0;
	}

	//generate an output filename
	var pdffilename = htmlfilename;
	pdffilename.splicer(-3, 3, "pdf");

	//if index(printopts,'F',1) then
	pdfcmd ^= " --footer-left [page]/[topage]";
	// end

	if (printopts.index("L", 1)) {
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
