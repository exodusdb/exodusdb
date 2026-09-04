#include <exodus/library.h>
libraryinit()

#include <shell2.h>
#include <sysmsg.h>

var printopts;
var pdfcmd;

func main(io osfilename, in printopts0, out errors) {

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
	printopts = printopts0.or_default("");
	if (printopts.contains("X")) {
		return 0;
	}

	// only convert .htm files
	if (not osfilename.lcase().ends(".htm")) {
		return 0;
	}

	// Check htm doesnt contain exo delimiters else
	// remove them otherwise they show as char □
	{
		var htm;
		if (not htm.osread(osfilename)) {
			abort(lasterror());
		}
		if (htm.match(("("^ RM ^"|"^ FM ^"|"^ VM ^"|"^ SM ^"|"^ TM ^"|"^ STM ^")"))) {
			// remove ~ from new filename else file will be deleted after an hour
			let badhtm_filename = osfilename.convert("~", "") ^ ".bad-with-delms";
			if (not osfilename.oscopy(badhtm_filename)) {
				abort(lasterror());
			}
			htm.converter(_ALL_FMS, "");
			if (not htm.oswrite(osfilename)) {
				abort(lasterror());
			}
			var msg = "WARNING: convpdf removed exo delimiters before converting htm file\n";
			msg    ^= "Original htm:" ^ badhtm_filename ^ "\n";
			msg    ^= "Backtrace added to help identify culprit program\n\n";
			sysmsg(msg ^ backtrace(), "Delimiters removed from htm before convert to pdf");
		}
	}

	// generate an output filename
	var pdffilename = osfilename;
	pdffilename.paster(-3, 3, "pdf");

	// Pre-flight check that chromium can embed url srced images in htm
	gosub check_imgurl(osfilename);

	// Test is duplicated in convpdf.cpp and htmllib2.cpp
	// chromium might be aliased to google-chrome
	pdfcmd = "chromium";
	if (osshell("which " ^ pdfcmd ^ " > /dev/null")) {

		// Required confirmation to run unsafe if user is root
		pdfcmd ^= " --no-sandbox";

		// Run in the background
		pdfcmd ^= " --headless";

		// Only required for older versions of chromium when headless
		pdfcmd ^= " --disable-gpu";

		// Various for good luck
		pdfcmd ^= "\
			--disable-setuid-sandbox \
			--disable-dev-shm-usage \
			--disable-background-timer-throttling \
			--disable-backgrounding-occluded-windows \
			--disable-renderer-backgrounding \
			--no-first-run \
			--user-data-dir=/tmp/chrome-headless \
		";

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
	pdfcmd = "DBUS_SESSION_BUS_ADDRESS=disabled: " ^ pdfcmd;
	call shell2(pdfcmd, errors);

	if (errors and not osfile(pdffilename).f(1)) {
		errors = pdfcmd ^ "\nxxx\n" ^ errors;
		printl(pdfcmd);
		printl(errors);
	} else {
		errors = "";
		osfilename = pdffilename;
	}

	return 0;
}

subroutine check_imgurl(in htmfilename) {

	// Warn devs if <img> that use url as src will fail in PDF conversion due to connection issues
	// If network hairpinning on router is not set up or the host's /etc/hosts is redirecting traffic elsewhere
	// then chromium will fail to retrieve it and the pdf will silently be missing them

	// TODO check that relative and absolute file paths can be read by chromium.
	// if chromium installed with snap, relative paths sometimes cannot be read by chromium!

	// Check/get any <img> with src using domain path
	var htm_contents;
	if (not htm_contents.osread(htmfilename)) {
		sysmsg("convpdf.cpp says: " ^ lasterror());
		abort("System error has occurred || Please contact support");
	}
	var all_imgsrcs = htm_contents.match(R"(<img .+? src=\"https?[^ ]+\.(png|PNG|jpeg|JPEG|jpg|JPG|gif|GIF|svg|SVG|webp|WEBP)\")"_rex);
	if (not all_imgsrcs) {
		return;
	}

	// Extract just the img url and for speed,
	// assume all url use the same domain and test only one
	var img_url = all_imgsrcs.f(0, 1);
	img_url = img_url.match("src.+$").replace("(src=|\")"_rex, "");
	// No option in chromium to say check if you can access this file url
	// so simulate what chromium would do using wget/curl
	var http_code = "";
	if (osshell("which curl")) {
		http_code = osshellread("curl -s -o /dev/null -w \"%{http_code}\" " ^ img_url);
	} else {
		http_code = osshellread("wget --server-response --spider " ^ img_url ^ " 2>&1 | awk '/^  HTTP/{print $2}' |tail -n1");
	}

	// Success, chromium *should* be able to embed url/img
	if (http_code == "200") {
		return;
	}

	// Save htm for investigation in work dir
	let htmfile_copy = htmfilename.replace(".htm", ".broken_url");
	if (not htmfilename.oscopy(htmfile_copy)) {
		loglasterror();
	}

	// Warn dev
	var errmsg	= "Warning from convpdf: Failed retrieve image from " ^ img_url.quote();
	errmsg     ^= " || This image will be missing from PDF documents/reports.";
	errmsg     ^= " || HTTP return code: " ^ http_code.quote();
	errmsg     ^= " || Original htm file " ^ htmfile_copy.quote();
	errmsg     ^= " || This may be a network hairpinning issue. Check host's /etc/hosts or router settings";
	sysmsg(errmsg);

	return;
} // end of check_img_access()

}; // libraryexit()
