#if EXO_MODULE
	import std;
#else
#	include <iostream>
#endif

#include <exodus/program.h>
programinit()

// Note: man pages are generated with a Unicode FOUR-PER-EM SPACE for fixed layout.
// man pages comments.replacer("\u22c5", "\u2005"); // "⋅" Unicode operator point operator -> " " FOUR-PER-EM SPACE
// comments.replacer("␣", "\u2005");
// Source code comments contain "\u22c5+"; "⋅" Unicode operator point operator and "␣" space indicater
// man_page_white_space_code_point = "\u2005"; // " " FOUR-PER-EM SPACE

// for T2H (txt to html), H2M (html to manpage) and HTMLTIDY
#include "htmllib2.h"

// Editor syntax highlighting example.
// various func/var comments exodus/c++ literals exovars jumps  allcaps
// purple  white    cyan     green      yellow   brown   pink     red
func dummy_function() {
#define dummydef
	/* A comment */
	// Another comment
	var dummy;
	if (not open("DEFINITIONS" to DEFINITIONS))
		abort(lasterror());
	TRACE(R"__(\\)__")
	return 0;
}

let syntax =

R"__(gendoc osfilename ... [codefile_dir] [{OPTIONS}]

osfilename: e.g. ~/exodus/exodus/libexodus/exodus/{var.h,dim.h}

codefile_dir: Optional output of testing_var_h.cpp

Options:
	m = man page       ext .1
	h = html (default) ext .htm
	w = wiki           ext .wiki
	o - Output to original dir
	V - Verbose
)__";

bool wiki = OPTIONS.contains("w");
bool man = OPTIONS.contains("m");
bool html = not (wiki or man);
let doc_ext = man ? "1" : (wiki ? "wiki" : "htm");

bool output_to_orig_dir = OPTIONS.contains("o");

let verbose = OPTIONS.count("V");

var tableno = 0;

// Default to current dir for example code output dir
var codefile_dir = "";
var ncodematches = 0;

var all_code_matches = "";
var all_code_matches_ptr = 1;
let code_match_delim1 = "\n//``````\n";
let code_match_delim2 = "``````";
// For style switcher
let style_switcher_class = "light";

func main() {

	if (not COMMAND.count(FM)) {
		abort(syntax);
	}

	// Skip if htmllib2 not installed or loadable yet
	try {
		call htmllib2("T2H", STATUS, "");
	} catch (...) {
		loglasterror();
		stop("gendoc: htmllib2 not installed yet. " ^ lasterror());
	}

	if (html) {

		// Pass 1 to collect and syntax highlight c++ code examples
		main2(true);

		// pygmentize cpp syntax highlighting
		{

			// A named lambda since it needs to be called twice
			auto converter = [this](in cpp_in) -> var {

				var html_out;

				// requires plugin install. cd ~/exodus/pygment && ./install.sh
				let plugin_lexer = "exoduscpp";

				// We dont want html header nor css style sheet.
				// For full output in a complete html file including css
				// pygmentize -l exoduscpp -f html -O full,style=colorful -o test.html test.cpp -v"
				let oscmd = "pygmentize -l " ^ plugin_lexer ^ " -f html";

				// osprocess
				var errors, exit_status;
				if (not var::osprocess(
					oscmd,
					/*in*/ cpp_in,
					/*out*/ html_out, errors, exit_status)
				)
					abort(lasterror() ^ " " ^ exit_status ^ " " ^ errors);

				// Remove the first two tags. <div class="highlight"><pre>
				html_out.cutter(html_out.indexn(">", 2));

				// Remove a redundent span
				if (html_out.starts("<span></span>"))
					html_out.cutter(13);

				// Remove the last two tags. </pre></div>
				html_out.fieldstorer("<", -2, -2, "");
				html_out.popper();

				return html_out;
			};

			// 0. How pygmentize wraps a \n char
			//15:15:41 root@de1:~/exodus/pygment# pygmentize -l exodus_cpp -f html
			//<div class="highlight"><pre><span></span>
			//</pre></div>
			//15:15:49 root@de1:~/exodus/pygment# pygmentize -l exodus_cpp -f html | xxd
			//00000000: 3c64 6976 2063 6c61 7373 3d22 6869 6768  <div class="high
			//00000010: 6c69 6768 7422 3e3c 7072 653e 3c73 7061  light"><pre><spa
			//00000020: 6e3e 3c2f 7370 616e 3e0a 3c2f 7072 653e  n></span>.</pre>
			//00000030: 3c2f 6469 763e 0a</div>.

			// 1. Convert all the code matches to html
			let converted_code_matches = converter(all_code_matches);

			// 2. Work out how pygmentize converts our code match separator
			//so we can restore our original unconverted code match separator
			let converted_code_match_delim1 = converter(code_match_delim1);

			// All code matches becomes the html converted version
			all_code_matches = converted_code_matches.replace(converted_code_match_delim1, code_match_delim2);

		}
		{
			// Get css_cpp_style. Not required.
			// We already hard coded our heavily customised pygmentize style.
			if (false) {
				// How to get a fresh copy of pygmentize default html style
				var errors, exit_status;
				if (not var::osprocess("pygmentize -S default -f html", "", css_cpp_style, errors, exit_status))
					abort(lasterror());
				css_cpp_style.prefixer("<style>\n");
				css_cpp_style.appender("</style>\n");
			}
		}
	}

	// Pass 2
	main2(false);

	stop();
}

func main2(in pass1) {

	if (verbose eq 1)
		logputl("\n---------- Pass ", pass1 ? 1 : 2, " ----------");

	var docfilename = ostempfile();

	// Start a temporary file for the example code
	//////////////////////////////////////////////

	var all_code_cleanups = "";
	var first_osfilenameonly = "";

	// Use static to only generate it on the first pass
	static let codefile = ostempfile();
//	std::ofstream codefile(codefile.toString());
	osclose(codefile);
//	osremove(codefile);
	if (not oswrite("" on codefile))
		abort(lasterror());
	if (osfile(codefile).f(1))
		abort(osfile(codefile));
	codefile << "/" "/ Generated by cli " << COMMAND.f(1) << "\n";
	codefile << "\n";
	codefile <<
R"__(#include <cassert>
#if EXO_MODULE
//	import std;
#	include <vector>
#	include <iomanip>
#else
#	include <vector>
#	include <iomanip>
#endif
#include <exodus/program.h>
//#if EXO_FORMAT
#	define println printl
#	define print printx
//#endif
programinit()

// For co_run example
func add(in a, in b) {
	set_async_result(a + b);
	return "";
}

func main() {

	// Options to skip testing
	{
		let msg = "Test passed. Without commencing because ";

		if (osgetenv("EXO_NODATA")) stop(msg ^ "EXO_NODATA was set.");

		if (not connect())  stop(msg ^ "No default db connection.");

		if (not open("xo_clients") or not reccount("xo_clients"))
		stop(msg ^ "xo_clients file is missing or empty.");
	}

	// Clean up before starting
	gosub cleanup();
)__";

	var all_html_contents = "";

///////////
// nextfile
///////////

	let nfiles = COMMAND.fcount(FM) - 1;
	dim file_texts(nfiles);
	file_texts = "";
	var filen = 0;

	for (let osfilename : COMMAND.remove(1)) {

		filen++;

		// output dir for examples .cpp
		if (osdir(osfilename)) {
			codefile_dir = osfilename;
			continue;
		}

		dim src;
		if (not src.osread(osfilename)) {
			loglasterror();
			continue;
		}
//		src.converter("\n\r", _FM);
		let osfilenameonly = osfilename.field(OSSLASH, -1);

		var osfile_doc_title = "";

		// code is output using the name of the first file
		if (not first_osfilenameonly)
			first_osfilenameonly = osfilenameonly;

		// Start the code examples for this osfile
//		var file_header = osfilenameonly;
		////////////////
		// Code examples - from var.h
		////////////////
		codefile << "\n////////////////";
		codefile << "\n/" "/ Code examples " << osfilenameonly;
		codefile << "\n////////////////";
		codefile << "\n";

		var file_contents = "";

		var new_objs = "";
		var defined_objs = "";
		var all_func_sigs = "";
		tableno = 0;

		var class_name = osfilenameonly.field(".", 1);

		// Start a file for the documentation
		/////////////////////////////////////

//		std::ofstream docfile(docfilename.toString());
		var docfile = docfilename;

		auto skipping = true;
		auto in_table = false;

		var contiguous_comments = "";

		var default_objname = "var";

///////////
// nextline
///////////

		// Indexing instead of fieldwise so we can look back from any line
		for (let lineno : range(1, src.rows())) {

			// Alias srcline into the src dim array
			var& srcline = src[lineno];

			// tabs become spaces
////			srcline.replacer("\t", " ");
//			srcline.replacer("\t", "");
////			srcline.trimmer();
//			srcline.trimmerboth();
			// needed to catch invisible things like //\t//
			srcline.replacer("\t", " ");
			srcline.trimmerboth();

			// Skip all preprocessor lines
			if (srcline.starts("#"))
				continue; // nextline

			if (srcline == "private:") {
				// Ignore all comments in private
				// (unless turned on by a table title below)
				skipping = true;
				continue; // nextline
			}

			if(srcline == "public:")
				// Inspect everything in public
				skipping = false;

			if (srcline.starts("[[noreturn") and srcline.ends("]]"))
				// Skip noreturn
				continue; // nextline

			if (srcline.starts("[[deprecated"))
				// Skip deprecated flags
				continue; // nextline

			if (srcline.starts("class ")) {
				// Always capture class info
				srcline.replacer("PUBLIC ", "");
				// Find class xxx : or { or final
				let class_match = srcline.match(R"__(class\s+([a-zA-Z0-9_]+)\s*[:{f])__");
				if (class_match) {
					class_name = class_match.f(1, 2);
					default_objname = class_name;
//					if (class_name == "ExoProgram")
					if (class_name.listed("ExoProgram,JobManager,TaskManager"))
						default_objname = "";
				}
				continue; // nextline
			}

			if (srcline.starts("template"))
				continue; // nextline

			// osfile title is like // gendoc: xxxxxxxxxxxxxx
			if (srcline.starts("/" "/ gendoc:")) {
				osfile_doc_title = srcline.field(":", 2, 999);
				continue; // nextline
			}

			////////////////////////////
			// Start a section and table
			////////////////////////////
			// Lines like "///... some text:"
			const static rex doc_rex(R"__(^(/{3,})\s+(.*):$)__");
			var table_title = srcline.match(doc_rex);
			if (table_title) {

				skipping = false;

				// Close any prior table
				if (in_table) {
					if (man) {}
					else if (wiki) {
						docfile << "|}" << std::endl;
					} else {
						docfile << "</table>" << std::endl;
					}
					in_table = false;
				}

				// Empty table title results in closing table and skipping following functions
				// until another table title is found.
				if (not trim(table_title.f(1, 3)))
					continue; // nextline

				in_table = true;

				let short_title = table_title.f(1, 3).tcase().replace("Mv", "MV").replace("Os", "OS").replace("Db", "DB");

				let level = table_title.f(1, 2).len();
				if (man) {

					// Section header
					docfile << ".SH " << ++tableno << ". " << short_title.ucase() << std::endl;
					docfile << ".SH" << std::endl;
					docfile << std::endl;
				}
				else if (wiki) {

					let wiki_title = str("=", level);
					docfile << wiki_title << " " << short_title << " " << wiki_title << std::endl;
					docfile << "" << std::endl;
					docfile << "{|class=\"wikitable\"" << std::endl;
					docfile << "!Use!!Function!!Description" << std::endl;

				} else {

					var table_id = short_title.convert(" /", "__");

					let table_entry = "<li><a href=\"#" ^ table_id ^ "\">" ^ short_title ^ "</a></li>\n";
					file_contents ^= table_entry;

					docfile << "<h" << level << " id=\"" << table_id << "\">" << short_title << "</h" << level << ">" << std::endl;
					docfile << "" << std::endl;
					docfile << "<table class=\"wikitable\">" << std::endl;
					docfile << "<tr> <th>Use</th> <th>Function</th> <th>Description</th> </tr>" << std::endl;
				}
				continue; // nextline

			} // table title

			if (skipping or not in_table)
				// Skipping by default.
				// private: turns skip off.
				// public: and table titles turn skip off
				continue; // nextline

			//////////////////////////////////////
			// function detector/rejector (tricky)
			//////////////////////////////////////
			// Continue to next source line on rejection

			var funcx_prefix;
			struct Funcx {var is_static; var no_discard; var is_ctor; var prefix = "";} funcx_mut;
			{
				//	static int getprecision() const;
				let origsrcline = srcline;
				// TODO replacer to return a bool that it did something?
				// but a returned bool could easily be accidentally used as a var.
				// Use a thread_local global variable flag?
				srcline.replacer(R"__(\bstatic\s+)__"_rex, "");
				funcx_mut.is_static = srcline ne origsrcline;

				// Not documenting ND or not since mostly ND
				funcx_mut.no_discard = srcline.starts("ND ");
				if (funcx_mut.no_discard)
					srcline.cutter(3);

				// funcx_prefix starts life as the first word
				// trim leading friend and const/constexpr etc.
				static const rex rex1 = rex(R"__(^((friend)|(const[^\s]*))\s)__", "i");
				srcline.replacer(rex1, "");
				srcline.trimmerfirst();
				funcx_prefix = srcline.field(" ", 1);

				// Suppress lines that are not comments or function declarations
				if (not funcx_prefix.starts("/") and not srcline.match(R"__(^[`\sa-zA-Z0-9_:&]+\()__")) {
					var remainder = srcline.field(" ", 2, 99999).trimfirst();
					if (not remainder.starts("operator"))
						funcx_prefix = "";
				}
				var definitely_func = false;
//				if (funcx_prefix == "auto" and srcline.contains("->")) {
				if (srcline.field(" ", 1) == "auto" and srcline.contains("->")) {
					funcx_prefix = srcline.field("->", 2).field("{", 1).field(";", 1).trim();
					srcline = srcline.field("->", 1).trimlast();
					definitely_func = true;
				}

				if (verbose eq 2)
					TRACE("000 " ^ funcx_prefix ^ " ... " ^ srcline)

				funcx_mut.is_ctor = funcx_prefix.starts("dim(", "var(");
				if (funcx_mut.is_ctor) {
				}

				// std::array<var, N> unpack(SV delim = _FM) const {
//				else if ((funcx_prefix.starts("std::array") or srcline.starts("std::array")) and srcline.ends("{")) {
//				else if (funcx_prefix.starts("std::array") or srcline.starts("std::array")) {
//					funcx_prefix = "auto [v1, v2, ...] =";
//				}

				else if (funcx_prefix == "var" or funcx_prefix == "int")
					// Include lines starting var
					// Warning: lines like "var xxx = 123;" are NOT function declarations
					funcx_prefix ^= "=";

				else if (funcx_prefix == "std::string") funcx_prefix = "var="; // i/oconv private

				else if (funcx_prefix.starts("var_")) funcx_prefix = "var="; // i/oconv private

				else if (funcx_prefix == "RETVAR") funcx_prefix = "var="; // from var_base
				else if (funcx_prefix == "RETVARREF") funcx_prefix = "var="; // from var_base

				// const var&
				else if (funcx_prefix == "CVR")  funcx_prefix = "chainable";// only for output/logput/errput etc.
				else if (funcx_prefix == "CBR") funcx_prefix = "chainable"; // from var_base

				// var& (various types)
				else if (funcx_prefix == "io")   funcx_prefix = "chainable";// none should return this
				else if (funcx_prefix == "IO")   funcx_prefix = "cmd"; // ditto. (now IO=void for mutator -er)
				else if (funcx_prefix == "VARREF") funcx_prefix = "VARREF"; // exo::var& from var_base
				else if (funcx_prefix == "out")  funcx_prefix = "chainable";// only input and getlocale

				// bool
				else if (funcx_prefix == "bool") funcx_prefix = "if";  // many return true/false

				// void
				else if (funcx_prefix == "void") funcx_prefix = "cmd"; // many return nothing

				// dim
				else if (funcx_prefix == "dim") funcx_prefix = "dim=";

				// Job3
				else if (funcx_prefix == "Job") funcx_prefix = "(Job)";

				// int (rare)
				else if (funcx_prefix == "int") {funcx_prefix = "int";} // setprecision/getprecision

				else if (definitely_func) {
				}
				else {

					if (funcx_prefix == "/" "/") {

						// Collect possible function preamble comments
						// Exactly // (two slashes, not more)
						contiguous_comments ^= srcline.substr(4) ^ FM_;

					} else {

						if (verbose == 2)
							TRACE(srcline)

						// Throw away non-function preamble comments
						////////////////////////////////////////////

						// From thrown away comments, get the default obj name for all following functions
						// except where overridden in specific function comments.
						// Only match "obj is xxx|yyy|zzz()" at the start of the first line of comment.
						let new_default_objname = contiguous_comments.match(R"__(^\s*obj is ([a-zA-Z_][a-zA-Z0-9_()|]+))__").f(1, 2);

						if (new_default_objname) {
							new_objs ^= new_default_objname ^ FM;
							default_objname = new_default_objname;
						}

						// Throw away collected comments if not contiguous
						contiguous_comments = "";
					}
					continue; // nextline
				}

				// Skip [[deprecated]] and [[undocumented]] functions
				const static rex undocumented(R"__((deprecated|undocumented))__", "i");
				if (contiguous_comments.match(undocumented))
					continue; // nextline

			} // function detector/rejector
			// Get a constant copy of the originals.
			// No way to mark something const after declaration in c++.
			const Funcx funcx_const = funcx_mut;

			///////////////////////////////
			// Found a function declaration
			///////////////////////////////

			if (verbose eq 1)
				TRACE("111 " ^ srcline)

			// Consume the comments in case we skip the function
			var comments = move(contiguous_comments);

			if (not funcx_const.is_ctor) {
				//std::array<var, N> unpack(SV delim = _FM) const {
				let sep = srcline.starts("std::array<") ? ">" : " ";
				srcline = srcline.field(sep, 2, 9999).trimfirst();
			}

//			var comments = srcline.field("{", 1).field(";", 2, 9999).trimmerfirst("/ ");
//			var comments = srcline.field("/" "/", 2, 99999).trimfirst();
			let trailing_comment = srcline.field("/" "/", 2, 99999);
			srcline = srcline.field("/" "/", 1).field("{", 1).trimlast();
			let funcname = srcline.field("(", 1);

			// Use trailing comment overerides leading comments
			if (trailing_comment)
				comments = trailing_comment;
			else
				comments.popper();

			// Extract and remove any "obj is xxx|yyy|zzz()" lines in comments for specific functions
			// comments is FM separated at this point (dont forget that \s matches FM chars)
			let objmatch = comments.match(R"__([ \t]*obj is ([a-zA-Z_][a-zA-Z0-9_()|]+)[^\n)__" _FM "]*");
			if (objmatch) {
				comments.replacer(FM ^ objmatch.f(1, 1), "");
				new_objs ^= objmatch.f(1, 2).convert("|()", FM) ^ FM_;
			}
			let objname = objmatch.f(1, 2);

			// Add a backtick if an odd numbers is present indicating forgot to close c++ code
			if (comments.count("`") % 2) {
				errputl("---");
				errputl(comments.convert(FM, NL));
//				if (comments.count("`") > 1 ) {
					abort("Error: Odd number of backticks in " ^ funcname.quote());
//				}
//				logputl("Warning: Missing closing backtick in " ^ funcname.quote());
//				comments ^= "`";
			}

			comments.converter(_FM, _NL);

			// Convert Exodus comment text (pseudo markdown) to html first
			if (not pass1)
//				comments = t2hl(comments.convert(_FM, _NL));
				comments = htmllib2("T2H", STATUS, comments.convert(_FM, _NL));

			//////////////////////
			// Code blocks handler
			//////////////////////

			// Format backticked source code fragments
			if (comments.contains("`")) {

				// regex between pairs of backticks
				const static rex backquoted_rex {R"__(`([^`]*)`)__"};

				var codematches = comments.match(backquoted_rex);

				// Replace space formatting with single space
				codematches.replacer("\u22c5+"_rex, " "); // "⋅" Unicode operator point operator -> space

////////////////
// nextcodematch
////////////////
				for (var codematch : codematches) {
					codematch = codematch.f(1, 2);

					// Just accumulate code matches in pass1
					if (pass1) {
						// First line indented 1 char by the the back tick
						if (not codematch.starts(" "))
							codematch.replacer("\n ", "\n");
						all_code_matches ^= codematch ^ code_match_delim1;
						continue;
					}

					// ... becomes proper code
					let aborting = " abort(\"" ^ funcname ^ ": \" ^ lasterror());";
					codematch.replacer(") ... ok", ") {/" "*ok*" "/} else " ^ aborting);
					codematch.replacer(") ... true", ") {/" "*true*" "/} else " ^ aborting);
					codematch.replacer(" ...", aborting);
					codematch.replacer("\n", "\n\t\t");
					codematch.replacer("\n\t\t\n", "\n");

					codematch.replacer("\t ", "\t");

					// Remove "// Cleanup" lines to be put in heading of
					const static rex cleanup_pattern {R"__(^[^\n]*// Cleanup[^\n]*)__"};
					var cleanups = codematch.match(cleanup_pattern);
					if (cleanups) {
						for (let cleanup : cleanups) {
							all_code_cleanups ^= cleanup ^ "\n";
						}
						codematch.replacer(cleanup_pattern, "");
					}

					// Convert some comments to assertions
					// USE ␣ to indicate spaces. They will be converted to spaces in any assert.
					// Warning: Doesnt handle double quotes inside double quote)?

					// IMPLICT ASSERTS
					// e.g. '   let|var v1 = xxxxxxx ; // "X" // zzzz' where zzzz can be a literal number 9999, "xxx", true or false
					//  '1112222222 3344444444444 55556666666'
//					const static rex rex3 = rex(R"__(^(\s*)(let|var)?\s+([a-z0-9A-Z_]+) = ([^\n]*?);[ \t]*//\s+(true|false|["0-9.-][^\n]*?)([ \t]//[^\n]*?)?$)__", "m");
					const static rex rex3 = rex(R"__(^(\s*)(let|var)\s+([a-z0-9A-Z_]+) = ([^\n]*?);[ \t]*//\s+(true|false|["0-9.-][^\n]*?)([ \t]//[^\n]*?)?$)__", "m");

//					codematch.replacer(rex3, "\t$1$2 $3$4; assert($3 == $5);$6");
					codematch.replacer(rex3, "\t\t$2 $3 = $4; assert($3.errputl() == $5);$6");

					// e.g. '   v1 = xxxxxxx ; // "X" // zzzz' where zzzz can be a literal number 9999, "xxx", true or false
					//  '1112222222 3344444444444 55556666666'
					const static rex rex3b = rex(R"__(^(\s*)()([a-z0-9A-Z_]+) = ([^\n]*?);[ \t]*//\s+(true|false|["0-9.-][^\n]*?)([ \t]//[^\n]*?)?$)__", "m");
					codematch.replacer(rex3b, "\t\t$2$3 = $4; assert($3.errputl() == $5);$6");

					// ARROW ASSERTS
					//if (not field.readf(file, key, fieldno)) abort("readf" ": " ^ lasterror()); // field -> "G"
					// e.g. '  // v1 -> "f1^X^f3"_var // v2 -> "a b c"'
					//1   2233333333333333
					// First only because it messes up if not repeated separately.
					// Why does it need multiple replaces? (possibly because group 3 overlaps with group 1)
					const static rex rex5 = rex(R"__(([^/])//\s*([a-zA-Z0-9_.()]+)\s*->\s*([^/\n]*))__", "f");
					codematch.replacer(rex5, "$1 assert($2.errputl() == $3);");
					codematch.replacer(rex5, "$1 assert($2.errputl() == $3);");
					codematch.replacer(rex5, "$1 assert($2.errputl() == $3);");
					codematch.replacer(rex5, "$1 assert($2.errputl() == $3);");
					codematch.replacer(rex5, "$1 assert($2.errputl() == $3);");

					// Fix " -> unassigned"
					codematch.replacer(".errputl() == unassigned", ".unassigned()");

//					// MUTATOR ASSERTS
//					// e.g. 'v1.xxx xxx xxx // zzzz // xxxxxx'
//					// e.g. 'v1.ucaser(); // "ABC"'
					const static rex rex4 = rex(R"__(^(\s*)([a-z0-9A-Z_]+)\.([^/\n]*)//\s+([^/\n]*)([^\n]*))__");
					codematch.replacer(rex4, "$1$2.$3;assert($2 == $4);$5");

					// Replace hacked spaces ␣ with real spaces " "
					codematch.textconverter("␣", " ");

					// Wrap in {}
					codematch.prefixer("\n\t{\n\t\t");
					codematch ^= "\n\t}\n";

					// Exclude format from tests in old version of Ubuntu
					bool uses_format = codematch.match(R"__((print|println|format)\()__");
					if (uses_format) {
						codefile << "#if EXO_FORMAT\n";
					}

					// Lead with function name
					codematch.prefixer("\n\tprintl(" ^ srcline.quote() ^ ");");

					// Global definitions;
					for (var objname : new_objs) {
						objname.converter("()", "");
						if (not objname or objname == "var")
							continue; // nextcodematch

						if (not locate(objname, defined_objs)) {
							defined_objs ^= objname ^ VM;
							// Declare a var if "obj is"
							//codematch.prefixer("\n\tvar " ^ objname ^ ";\n");
						}
					}
					new_objs = "";

					// Move rhs asserts onto separate lines
//					codematch.replacer(rex(R"__(;\s*assert([^\n;]*))__"), ";\n\t\tassert\$1\n\t");

					// Tidy up
					codematch.replacer(rex(R"__(\n;)__"), ";\n");
					codematch.replacer(rex(R"__(\n(\n[\t ]*assert))__"), "$1");
					codematch.replacer(rex(R"__([\t ]*\n)__"), "\n");

					ncodematches++;
					codefile << codematch;

					if (uses_format) {
						codefile << "#endif\n";
					}
				} // next codematch

				// We only need to collect code matches in pass1
				if (pass1)
					continue; // nextline

				//////////////////
				// FORMAT COMMENTS
				//////////////////

				// Remove one leading space (should only be from code)
				comments.replacer("\n ", "\n");
				//oswrite(comments on "x");

				// tag backticked code as c++
				if (man) {

					// Man page codes
					// .RS Right shift
					// .nf No format
					// .RE REturn (left shift)
					// .fi formatting
					// Prefix by a section title "Example:"
//					comments.replacer(backquoted_rex, "\nExample:\n.RS\n.nf\n$1\n.fi\n.RE\n");
					// Let h2ml do the job
//					comments.replacer(backquoted_rex, "\n<b>Example:</b><code>\n$1\n</code>\n");
					// html2 cant do the job unless html chars are escaped and for man output pygmenter has not been called to do the job
					// therefore do the html escaping here
					comments.replacer(
						backquoted_rex,
						[](in code_match) {
							let group1 =
								code_match(1, 2)
								// html escapes
								.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
								// man page escapes (handled by h2m in final stage)
								//.replace(_BSL, _BSL _BSL).replace(".", _BSL "&.")
							;
							return "\n<p><b>Example:</b><code>\n" ^ group1 ^ "\n</code></p>\n";
						}
					);
				}
				else if (wiki)

					// wiki
					comments.replacer(backquoted_rex, "<syntaxhighlight lang=\"c++\">\n$1</syntaxhighlight>");

				else {

					// html

//					// Javascript instead of cpp because it gives better highlighting for exodus c++. See above too.
//					// (All function calls are highlighted)
//					comments.replacer(backquoted_rex, "\n<pre><code class='hljs-ncdecl language-javascript'>$1</code></pre>\n");
//					comments.replacer(backquoted_rex, "\n<pre><code class='language-cpp'>$1</code></pre>\n");
//					comments.replacer(backquoted_rex, "\n<textarea class=code-block>$1</textarea>\n");

					// REPLACE CODE WITH PASS1 CONVERTED_CODE

					// Use a lambda that consumes all_code_matches sequentially using a pointer into all_code_matches
					comments.replacer(
						backquoted_rex,
						[this](in /*codeblock*/) {
//TRACE(all_code_matches_ptr)
							let end_charn = all_code_matches.index(code_match_delim2, all_code_matches_ptr);
							let code_match_len = end_charn ? end_charn - all_code_matches_ptr : all_code_matches.len();
							let code_match = all_code_matches.substr(all_code_matches_ptr, code_match_len);
							all_code_matches_ptr += code_match_len + code_match_delim2.len();
//TRACE(codeblock)
//TRACE(all_code_matches_ptr)
//TRACE(end_charn)
//TRACE(code_match_len)
//TRACE(code_match)
//input();
//							return "<div class=\"highlight " ^ style_switcher_class ^ "\"><pre>" ^ code_match ^ "</pre></div>";
							return "<pre>" ^ code_match ^ "</pre>";
						}

					); // replace code with syntax hightlighted html code

				}
				// Ordinary spaces for aligning code in html and wiki
				if (not man)
					comments.replacer("\u22c5", " "); // "⋅" Unicode operator point operator -> " "

			} // code blocks handler

/////////////////////////
// probably function line
/////////////////////////

			// Can still bail out anywhere down to lineinit below

			// After using backticks to delimit c++ code.
			comments.replacer("{backtick}", "`");

			if (man) {

				// man page formatting function by function

				// hide dot/space formatting
				comments.replacer("\u22c5", "\u2005"); // "⋅" Unicode operator point operator -> " " FOUR-PER-EM SPACE
				comments.replacer("␣", "\u2005");

//				comments = h2ml(comments);
				comments = htmllib2("H2M", STATUS, comments);

			} else if (wiki) {

				// wiki
				comments.replacer(_FM, "\n\n");
			}

			// Clean up function args
			// Can reject the function and continue to next line
			var line2 = srcline.field(";", 1);
			{

				if (verbose == 1) {
					TRACE("aaa " ^ line2)
				}

				// "ARGS&... appendable" -> "appendable, ..."
				line2.replacer(R"__(\b[a-zA-Z][a-zA-Z0-9]*\s*&{1,2}\s*\.\.\.\s*\b([a-zA-Z][a-zA-Z0-9]*))__"_rex, "$1, ...");

				line2.replacer("F&& ","");// async
				line2.replacer("\\bstd::size_t\\b", "");
				line2.replacer("\\bconst&&\\b", "");
				line2.replacer("\\bconst&\\b", "");
				line2.replacer("\\bconst\\b"_rex, "");
				line2.replacer("\\bin\\b"_rex, "");
				line2.replacer("\\bSV\\b"_rex, "");
				line2.replacer("(int)", "(INT)"); // preserve operator++(int) for later analysis
				line2.replacer("\\bint\\b"_rex, "");
				line2.replacer("\\bint\\b"_rex, "");
				line2.replacer("\\bchar\\*"_rex, "");
				line2.replacer("\\bbool "_rex, "");
				line2.replacer("\\bbool "_rex, "");
				line2.replacer("\\bstd::size_t "_rex, "");
				line2.replacer("\\brex& "_rex, "");
	//			line2.replacer("\\bdim& "_rex, ""); // needed to create this text below "dim d1 = d2; // Copy"
				line2.replacer("\\bCVR "_rex, "");
				line2.replacer("\\bCBR "_rex, "");
				line2.replacer("\\bVBR "_rex, "");
				line2.replacer("\\bVARREF "_rex, "io");
				line2.replacer("\\bCONSTEXPR "_rex, "");
				line2.replacer("= _", "= ");
				line2.trimmer();
				line2.replacer("( ", "(");
				line2.replacer(" )", ")");

	//			line2.replacer("dim& ", "dim ");
	//			line2.replacer("dim& ", "");
				line2.replacer("noexcept", "");

				// SKIP temporaries and deleted functions
				if (line2.ends("&&") || line2.ends("= delete"))
					continue; // nextline

				// Skip copy and move assignment
				// void operator=(const dim& rhs) &;
				// void operator=(dim&& rhs) & noexcept {
	//			if (line2.starts("operator=(const " ^ class_name ^ "&"))
	//				continue;
	//			if (line2.starts("operator=(" ^ class_name ^ "&&"))
	//				continue;
				if (line2.match(R"__(\boperator\s*=\s*\(\s*(const)?\s*[a-zA-Z0-9_]+\s*&)__"))
					continue; // nextline

	//			// operator=(v1)
	//			// dim d1 = v1;
	//			if (line2.match(R"__(\boperator\s*=)__")) {
	//				line2.replacer(R"__(\boperator=\(([a-zA-Z0-9_]+)\))__"_rex, class_name ^ " " ^ class_name.first() ^ "1 = $1;");
	//			}

				if (line2.ends("&")) {
					line2.popper();
					line2.trimmerlast();
				}

				if (line2.ends("REF")) {
					line2.cutter(-3);
					line2.trimmerlast();
				}

			} // end of clean up function arguments

			// SKIP if no comments and already output
			if (not comments and locate(line2, all_func_sigs))
				continue; // nextline

			all_func_sigs ^= line2 ^ VM;

			if (verbose == 1) {
				TRACE("bbb " ^ line2)
			}

			// Remove inline multi-line quotes.
			// We defined them as inline documentation in the function definition.
			// std::array<var, N> unpack/*<N>*/(SV delim /*= _FM*/) const {
			line2.replacer("/" "*", "");
			line2.replacer("*" "/", "");

			// i/oconv_MD(const char* conversion) -> i/oconv("MD")
			var func_decl0 = line2.replace(R"__(([io])conv_([A-Z]+)\([a-zA-Z0-0_*]*\))__"_rex, "$1conv(\"$2\")");

			//var  exoprog_date(in type, in input0, in ndecs0, out output);
			func_decl0 = func_decl0.replace(R"__(exoprog_date\(.*)__"_rex, "iconv|oconv(var, \"[DATE]\")");

			//var  exoprog_number(in type, in input0, in ndecs0, out output);
			func_decl0 = func_decl0.replace(R"__(exoprog_number\(.*)__"_rex, "iconv|oconv(var, \"[NUMBER]\")");

			// Work out func_decl and adjust funcx_prefix
			// iomanip - reject the function and skip to nextline
			var func_decl;
			{
				if (not funcx_const.is_ctor) {

					func_decl0.replacer("dim& ", "");

					if (line2.match(R"__(\boperator\s*=)__")) {

						// "operator=(v1)" -> "dim d1 = v1;"
						func_decl = func_decl0.replace(R"__(\boperator\s*=\s*\(\s*([a-zA-Z0-9_]+)\s*\))__"_rex, class_name ^ " " ^ class_name.first() ^ "1 = $1;");
					}

					else if (line2.match(R"__(\boperator\s*<<\s*\()__")) {

						// operator<<(auto& value)
						// operator<<(std::ostream& (*manip)(std::ostream&))
						if (line2.contains("manip"))
	//						func_decl = "osfile << std::setw(n) << std::endl; etc.";
							continue; // nextline
						else
							func_decl = "osfile << anything << std::endl;";
						funcx_prefix = "";
					}
					else if (line2.match(R"__(\boperator\s*\[\]\s*\()__")) {

						// "operator[](rowno)"
						// "operator[](rowno, colno)"
						// -> "var v1 = d1[rown];   d1[rown] = v1;"
						// -> "var v1 = d1[rown, coln];   d1[rown, coln] = v1;"
						func_decl = func_decl0.replace(
							R"__(\boperator\s*\[\]\s*\(\s*([^\)]+)\))__"_rex,
							 "var v1 = " ^ class_name.first() ^ "1[$1];   " ^ class_name.first() ^ "1[$1] = v1;"
						);
						if (html or wiki)
							func_decl.replacer("   ", "<br>");
						// Use on left and right hand side
						funcx_prefix = "";
						}

					else if (line2.contains("operator\"\"_")) {
						// operator""_var(cstr, std::size_t size);
						// ->
						// var v1 = ""_var
						func_decl = func_decl0.replace(
							R"__(\boperator""_([a-zA-Z0-9_])([a-zA-Z0-9_]+).*)__"_rex,
	//						"$1$2 ${1}1 = \"\"_$1$2"
							"\"\"_$1$2"
						);

					}
					else if (line2.match(R"__(\boperator\s*\(\)\s*\()__")) {
//						var matched = line2.match(R"__(\boperator\s*\(\)\s*\()__");
//TRACE(matched)  // "operator()("
						//operator()(fieldno, valueno = 0, subvalueno = 0);
						func_decl = func_decl0.replace(" = 0", "").replace(
							R"__(\boperator\s*\(\)\s*\(\s*([^\)]+)\))__"_rex,
							class_name.first() ^ "2($1);   " ^ class_name.first() ^ "1($1) = v2"
						);
					}

					else if (line2.match(R"__(\boperator\s*[\+\-\*\/\%\!\^]\s*\(.*)__")) {
						var matched = line2.match(R"__(\boperator\s*[\+\-\*\/\%\!\^]\s*\(.*)__");
//TRACE(matched)
//TRACE(matched)  // "operator+(var)"
//TRACE(matched)  // "operator-(var)"
//TRACE(matched)  // "operator*(var)"
//TRACE(matched)  // "operator/(var)"
//TRACE(matched)  // "operator%(var)"

//TRACE(matched)  // "operator+()"
//TRACE(matched)  // "operator-()"
//TRACE(matched)  // "operator!()"

						if (matched.contains("()")) {
							//operator+()    -> +v2
							func_decl = func_decl0.replace(
								R"__(\boperator\s*([\+\-\*\/\%\!\^])\s*\(.*)__"_rex,
								"$1 " ^ class_name.first() ^ "1"
							);
						} else {
							//operator+(var) -> v2 + v3
							func_decl = func_decl0.replace(
								R"__(\boperator\s*([\+\-\*\/\%\^])\s*\(.*)__"_rex,
								class_name.first() ^ "2 $1 " ^ class_name.first() ^ "3"
							);
						}
					}

					else if (line2.match(R"__(\boperator\s*[\+\-\*\/\%\^]=\s*\(.*)__")) {
						//operator+=(var)
						// v1 += v2
						func_decl = func_decl0.replace(
							R"__(\boperator\s*([\+\-\*\/\%\^]=)\s*\(.*)__"_rex,
							class_name.first() ^ "1 $1 " ^ class_name.first() ^ "2"
						);

						// Prevent prefixing with "var v1 = "
						funcx_prefix = "";
					}

					else if (line2.match(R"__(\boperator\s*[\+\-]{2,2}\s*\(INT\))__")) {
						//operator++(int);
						// v1 ++
						func_decl = func_decl0.replace(
							R"__(\boperator\s*([\+\-]{2,2})\s*\(.*)__"_rex,
							class_name.first() ^ "1 $1"
						);

						// Prevent prefixing with "var v1 = "
						funcx_prefix = "";
					}

					else if (line2.match(R"__(\boperator\s*[\+\-]{2,2}\s*\(\))__")) {
						//operator++();
						// ++ v1
						func_decl = func_decl0.replace(
							R"__(\boperator\s*([\+\-]{2,2})\s*\(.*)__"_rex,
							"$1 " ^ class_name.first() ^ "1"
						);

						// Prevent prefixing with "var v1 = "
						funcx_prefix = "";
					}

	//				else if (funcx_prefix == "int=") {
	//					func_decl = func_decl0;
	//				}
					else if (funcx_prefix.starts("auto")) {
						// std::array<var N> unpack etc.
						func_decl = funcx_prefix ^ " " ^ func_decl0;
	//					TRACE(funcx_prefix) // "auto [a, b, ...] ="
	//					TRACE(srcline)  // "unpack(SV delim = _FM) const"
	//					TRACE(func_decl)// "auto [a, b, ...] = unpack(delim = FM)"
					}

					else {
						// xxxxxxxxx -> var.xxxxxxxxx
						if (default_objname) {
							func_decl = (objname ?: default_objname) ^ "." ^ func_decl0;
						} else
							func_decl = func_decl0;

						if (funcx_const.is_static) {
							// var(). -> var::
							func_decl.replacer("().", "::");
							// var(). -> var::
							func_decl.replacer("var.", "var::");
						}
					}

				} // not is_ctor

				else {
					// ctor
	//TRACE(func_decl0)
	//TRACE: func_decl0 = "dim() = default"
	//TRACE: func_decl0 = "dim(dim& sourcedim)"
	//TRACE: func_decl0 = "dim(dim&& sourcedim)"
	//TRACE: func_decl0 = "dim(nrows, ncols = 1)"
	//TRACE: func_decl0 = "dim(std::initializer_list<T> list)"
					// dim(xxx) -> dim d(xxx);
					// dim() -> dim d;
					var args = func_decl0.field("(", 2, 999999).field(")", 1);
					var arg1 = args.field(" ", 1);
					let class_letter = func_decl0.first(1);

					// Default ctor
					if (arg1 == "") {
						// dim() -> dim d1;
						func_decl = class_name ^ " " ^ func_decl0.first(1) ^ "1;";
						funcx_prefix = "";
					}
					// Copy ctor
					else if (arg1 == (class_name ^ "&")) {
						// dim(dim&) -> dim d1 = d2;
						funcx_prefix = "";
						func_decl = class_name ^ " " ^ class_letter ^ "1 = " ^ class_letter ^ "2; /" "/ Copy";
					}
					// Move ctor
					else if (arg1 == (class_name ^ "&&")) {
						// dim(dim&&) -> dim d1 = dim();
						funcx_prefix = "";
						func_decl = class_name ^ " " ^ class_letter ^ "1 = " ^ class_name ^ "(); /" "/ Move";
					}
					else if (arg1.starts("std::initializer_list")) {
						// -> dim d1 = {"a", "b", "c", "d" ...};
						funcx_prefix = "";
						func_decl = class_name ^ " " ^ class_letter ^ R"__(1 = {"a", "b", "c" ...};)__" " /" "/ Initializer list";
					}
					// Other ctors
					else {
						// dim(nrows, ncols = 1) -> "dim d1(nrows, ncols = 1);
						func_decl = class_name ^ " " ^ class_letter ^ "1(" ^ args ^ ");";
						funcx_prefix = "";
					}

				} // ctor

				if (funcx_prefix == "cmd" or funcx_prefix == "cmd2")
					funcx_prefix = "";

			} // func_decl and adjust funcx_prefix

//////////////
// outlineinit
//////////////

			// No going back from here because we start outputting to docfile

			if (funcx_prefix.starts("std::array")) {
				// std::array<var, N>
				funcx_prefix = "auto [v1, v2, ...] = ";
			}
			else if (funcx_prefix.starts("std::generator")) {
				// std::generator<AsyncResult&>
				// std::generator<ExoEnv&>
				funcx_prefix = "range-based-for";
			}

			if (man) {

				var func_decl2 = func_decl;

				let match1 = funcx_prefix.match("([a-zA-Z_0-9]+)=").f(1, 2);

				if (funcx_prefix == "if")
					func_decl2 = "if (" ^ func_decl ^ ") ...";

				else if (match1) {
					//var= int=
//					func_decl2 = "var v1 = " ^ func_decl ^ ";";
//					func_decl2 = "int i1 = " ^ func_decl ^ ";";
					func_decl2 = match1 ^ " " ^ match1.first() ^ "1 = " ^ func_decl ^ ";";
				}
				else {
					func_decl2 = funcx_prefix;
					if (not funcx_prefix.empty())
						 func_decl2 ^= " ";
					func_decl2 ^= func_decl;
				}
//				else if (funcx_prefix == "var=")
//					func_decl2 = "var v1 = " ^ func_decl ^ ";";

//if (srcline.contains("unpack")) {
//TRACE(srcline)
//TRACE(funcx_prefix)
//TRACE(func_decl)
//TRACE(func_decl2)
//}
//TRACE(srcline)  // "unpack(SV delim = _FM) const"
//TRACE(funcx_prefix) // "auto [a, b, ...] ="
//TRACE(func_decl)// "strvar.unpack(delim = FM)"
//TRACE(func_decl2)   // "strvar.unpack(delim = FM)"

				// Subsection header
				docfile << ".SS" << std::endl;
				// Put on a separate line to avoid suppression of double quote chars
				docfile << func_decl2 << std::endl;

////				docfile << ".sp" << std::endl; // ignored
//				// Hack to defeat man's stubborn insistence on suppressing blank lines between section title and section text
//				docfile << ".nf" << std::endl;
////				docfile << " " << std::endl; // ignored
////				docfile << ".sp" << std::endl; // ignored
////				docfile << "\u000A" << std::endl; // ignored
////				docfile << "\u2003" << std::endl; //U+2003 EM SPACE
//				docfile << "\u2005" << std::endl; //U+2005 Four-Per-Em Space
////				docfile << ".fi" << std::endl;
//				docfile << ".fi";
////TRACE(comments)
////input();
				docfile << comments << std::endl;
				docfile << "" << std::endl;
			}

			else if (wiki) {
				docfile << "|-" << std::endl;
				docfile << "|" << funcx_prefix << "||" << func_decl << "||" << comments << std::endl;
			}
			else {
				// html

				// https links in comments are already converted to <a> tags in htmllib2("T2H") above.

				let encoded_funcx_prefix = funcx_prefix.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
				let funcx_prefix_wrapped = funcx_prefix ? ("<p><code>" ^ encoded_funcx_prefix ^ "</code></p>") : "";
//				func_decl.replacer("&", "&amp;");
//				func_decl.replacer("<", "&lt;");
//				func_decl.replacer(">", "&gt;");

				// template args
				static rex template_arg_rex = R"__(<([A-Z]+)>)__"_rex;
				func_decl.replacer(template_arg_rex, "&lt;$1&gt;");

//				<p>┌─────────┬─────────────┬───────────────────────────┬─────────────────────────┬───────────────────────────┐</p>
//				<p>│ Command │ Mechanism   │ Execution                 │ Use Case                │ Environment               │</p>
//				<p>├─────────┼─────────────┼───────────────────────────┼─────────────────────────┼───────────────────────────┤</p>
				if (comments.contains("┌─"))
					call htmllib2("BOX2TABLE", comments);

				// Finally its a three column row
				docfile
					<< "<tr>"
					<< "<td style = \"white-space:nowrap\">"  << funcx_prefix_wrapped << "</td>"
					<< "<td><p><code>" << func_decl<< "</code></p></td>"
					<< "<td>"  << comments << "</td>"
					<< "</tr>" << std::endl;
			}

		} // srcline

/////////////
// osfileexit
/////////////

		//////
		// doc
		//////
		{
			var doc_body;
			if (not osread(doc_body from docfilename)) {
				abort(osfilenameonly ^ " missing doc section header? " ^ lasterror());
			}

			if (not osremove(docfilename))
				loglasterror();

			// Close prior table
			if (in_table) {
				if (man) {}
				else if (wiki)
					doc_body ^= "|}:" "\n";
				else
					doc_body ^= "</table>" "\n";
			}

			// Create the final doc text

			var doc_text = "";

			// 1. css and contents

			if (man) {}

			else if (wiki) {}

			else if (html) {

				let osfile_id = osfilenameonly.field(".");
				let osfile_title = osfile_doc_title ?: osfilenameonly.field(".");
				all_html_contents ^= "<li><a href=\"#" ^ osfile_id ^ "\">" ^ osfile_title ^ "</a></li>\n";

				// Contents
				doc_text ^= "<div class=\"toc\">\n";

				doc_text ^= "<h4 id=\"" ^ osfile_id ^ "\">Contents:</h4>\n";

				doc_text ^= "<ol>\n";
				doc_text ^= file_contents;
				doc_text ^= "\n";
				doc_text ^= "</ol>\n";

				doc_text ^= "</div>\n";

			}

			// 2. header

			if (man) {
				doc_text ^= ".TH " ^ osfilenameonly;
//				doc_text ^= "\n";
//				doc_text ^= file_header;
				doc_text ^= "\n\n";
//				doc_text ^= ".PD 0 \\\" Turn off blank lines in .TP\n";


			}
			else {
//				doc_text ^= file_header;
				doc_text ^= "\n\n";

//				// Remove all p before and after div
//				doc_body.replacer("<p>\n<div", "<div");
//				doc_body.replacer("</div>\n</p>", "</div>");
//				doc_body.replacer("<p></p>", "");
			}

			// 3. body

			doc_text ^= doc_body;
			doc_text ^= "\n";

			if (not output_to_orig_dir) {

				if (man)
					outputl(doc_text);
				else
					// Save it for output after all_html_contents in this program's exit
					file_texts[filen] = doc_text.move();
			}
			else {
				// Change .h to .1, .wiki. .htm etc.
				var doc_osfilename = osfilename.fieldstore(".", -1, 1, doc_ext);

				// Update doc if needed
				var old_doc_text;
				if (not osread(old_doc_text from doc_osfilename))
					old_doc_text = "";
				if (doc_text ne old_doc_text) {
					if (not oswrite(doc_text on doc_osfilename))
						abort(lasterror());
					outputl("gendoc: Installing: ", doc_osfilename);
				} else
					outputl("gendoc: Up-to-date: ", doc_osfilename);
			}

		} // doc

	} // osfilename

///////
// exit
///////

	if (pass1)
		return 0;

	// Prefix multifile contents
	if (html and all_html_contents) {

		var doc_text = "";
		let page_title = "";
		doc_text ^=
			"<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n"
			"<title>" ^ page_title ^ "</title>"
		;

		// css and script
		doc_text ^= css;
		doc_text ^= "\n";

		// css for cpp syntax
		doc_text ^= css_cpp_style;

//		doc_text ^= style_switcher_head;

		doc_text ^=
			"</head>\n"
			"<body class=\"" ^ style_switcher_class ^ "\">\n"
		;

//		doc_text ^= style_switcher_dropdown;

		if (all_html_contents) {

			doc_text ^= "<div class=\"toc\">\n";

			doc_text ^= "<h3>Sections:</h3>\n";

			doc_text ^= "<ol>\n";
			doc_text ^= all_html_contents;
			doc_text ^= "\n";
			doc_text ^= "</ol>\n";

			doc_text ^= "</div>\n";
		}

		// Append each file's html
		for (in file_text : file_texts)
			doc_text ^= file_text;

		// Close everything
		doc_text ^=
			"</body>\n"
			"</html>"
		;

//		// Tidy the text if possible
//		if (osshell("which tidy 1> /dev/null")) {
//			var doc_out;
//			var err_out;
//			var exit_status;
////			var cmd = "tidy -i -w 0";
//			var cmd = xmllint --format --html input.html > output.html
//			// Ignore warnings (status 1), and errors (status 2) as long as output > input
////oswrite(doc_text on "x");
//			if (not osprocess(cmd, doc_text, doc_out, err_out, exit_status) and exit_status > 1 and len(doc_out) < len(doc_text))
//				logputl(err_out);
//			else
//				doc_text = move(doc_out);
////oswrite(doc_text on "y");
//		}
//

		// Properly indent the html (body only)
		let errcode = htmllib2("HTMLTIDY", DATA, doc_text);
		if (not errcode or len(DATA) > len(doc_text))
			doc_text = move(DATA);

		// Output the html header and multi-file section contents
		printl(doc_text);

	} // html

	/////////////////////
	// handle code output
	/////////////////////
	if (not ncodematches) {
		if (not osremove(codefile)) {loglasterror();}
	} else {

		// Finalise the code output
		codefile << "\n////////";
		codefile << "\n// exit:";
		codefile << "\n////////";
		codefile << "\n";
		codefile << "\n\tgosub cleanup();" << std::endl;
		codefile << "\n\tprintl(\"Test passed.\");\n";
		codefile << "\n\treturn 0;\n}";
		codefile << "\nsubroutine cleanup() {" << std::endl;
		codefile << "\n\t" "/" "/Note that all '// Cleanup' lines in the original source code must use literals and not variables" << std::endl;
		codefile << "\t" "/" "/otherwise the generated code will VNA on cleanup" << std::endl;
		codefile << all_code_cleanups;
		codefile << "};";
		codefile << "\n\n}; /" "/ programexit()" << std::endl;

		// Move the temp code examples cpp file into destination
		////////////////////////////////////////////////////////

		codefile.close();

		if (codefile_dir) {

			var newcode = "";
			if (not osread(newcode from codefile))
				abort(lasterror());

			let code_filename = codefile_dir ^ "/" ^ "testing_'orig'.cpp"_var.replace("'orig'", first_osfilenameonly);
			var oldcode = "";
			if (not osread(oldcode from code_filename)) {
				oldcode = "";
			}

			// Only update if the text has changed
			logput("gendoc: ");
			if (newcode != oldcode) {
				if (not oscopy(codefile to code_filename))
					abort(lasterror());
				logput("Installing:");
			} else
				logput("Up-to-date:");
			logputl(" ", code_filename);
		}

	} // code

	return 0;

} // main2

//////
// css
//////

var css =
R"__(
<style>

:root {
--bs-font-monospace: SFMono-Regular,Menlo,Monaco,Consolas,"Liberation Mono","Courier New",monospace;
}
	body {
		margin: 10px;
	font-size: 1rem;
	font-weight: 400;
		font-family: system-ui,-apple-system,"Segoe UI",Roboto,"Helvetica Neue","Noto Sans","Liberation Sans",Arial,sans-serif,"Apple Color Emoji","Segoe UI Emoji","Segoe UI Symbol","Noto Color Emoji";
		line-height: 1.5;
	}

	pre {
		margin: 0px;
		white-space: pre;
		overflow-x: auto;
		max-width: none;
		width: auto;
	}

	table, th, td {
		border: 1px solid #a2a9b1;
		border-collapse: collapse;
	}

	th {
		background-color: #eaecf0;
		font-weight: bold;
		text-align: left;
		padding: 8px;
	}

	td {
		padding: 0 0.5em 0 0.7em;
		vertical-align: top;
	}

	.toc, .toccolours {
		border: 1px solid #a2a9b1;
		background-color: #f8f9fa;
		padding: 5px;
		font-size: 95%;
	}

	.toc {
		display: table;
		padding: 7px;
	}

	.wikitable, .box_drawing {
		background-color: #f8f9fa;
		color: #202122;
		margin: 1em 0;
		border: 1px solid #a2a9b1;
		border-collapse: collapse;
	}

	.box_drawing tr:nth-child(2) {
		background-color: #eaecf0;
		font-weight: bold;
	}

	p {
		max-width: 50ch;
		margin: 0.4em 0 0.5em 0;
	}

	/* paras starting em/em get indented*/
	p:has(> em:first-child) {
		padding-left: 68px;
		border-top: 1px dashed grey;
		/*border-bottom: 1px dashed grey;*/
	}

	/* first line of description is the title.*/
	/* Exclude p with anything in it like em/em */
	td:nth-child(3) p:first-child:not(:has(*)) {
		font-weight: 500;
	}

	/* We only use em/em for leading words (arguments) */
	em {
	display: inline-block;
	min-width: 64px; /* Fill the undone indent approximately */
	margin-left: -68px; /* Undo indent for the box */
	}

	/* Style em like code*/
	code, em {
		color:inherit; /* ignore styling from mediawiki/bootstrap etc.*/
		font-family: var(--bs-font-monospace);
		font-style: normal;
		/*font-size: smaller;*/
		font-weight: normal;
	}

	img {
		border: 0;
		vertical-align: middle
	}

	hr {
		height: 1px;
		background-color: #a2a9b1;
		border: 0;
		margin: 0.2em 0
	}

	h1, h2, h3, h4, h5, h6 {
		color: #000;
		margin: 0;
		padding-top: 0.5em;
		padding-bottom: 0.17em;
		overflow: hidden
	}

	h1, h2 {
		margin-bottom: 0.6em;
		border-bottom: 1px solid #a2a9b1
	}

	h3, h4, h5 {
		margin-bottom: 0.3em
	}

	h1 {
		font-size: 188%;
		font-weight: normal
	}

	h2 {
		font-size: 150%;
		font-weight: normal
	}

	h3 {
		font-size: 128%
	}

	h4 {
		font-size: 116%
	}

	h5 {
		font-size: 108%
	}

	h6 {
		font-size: 100%
	}

	p {
		margin: 0.4em 0 0.5em 0
	}

	p img {
		margin: 0
	}

	ul {
		margin: 0.3em 0 0 1.6em;
		padding: 0
	}

	ol {
		list-style-image: none
	}

	li {
		margin-bottom: 0.1em
	}

	dl {
		display: grid;
		grid-template-columns: minmax(auto, max-content) 1fr; /* Left: expands to fit content, Right: remaining space */
		/*gap: 10px;*/ /* Space between rows and columns */
		overflow: hidden; /* Clears floats for the dl container */
		/*align-items: baseline;*/ /* Align grid items by baseline */
	align-items: stretch; /* Change from baseline to stretch to ensure equal heights */
max-width: 50ch;
	}

	dt, dd {
		margin: 0;
		box-sizing: border-box;
		border-bottom: 1px dashed grey;
		line-height: 1.5; /* Consistent line height */
		vertical-align: baseline; /* Ensure text aligns on baseline */
	}

	dt {
		grid-column: 1; /* Left column */
		padding: 0 5px 0 5px;
//		text-align: center;
		font-weight: bold;
		font-family: var(--bs-font-monospace);
		white-space: pre-wrap;
	/*	font-size: 95%;*
	/*	padding-top: 0.15em;*/ /* Nudge dt text baseline down to match dd */
	}

	dd {
		grid-column: 2; /* Right column */
		margin-left:1em;
	}


	.gendoc_function {
		color: #800;
		font-weight: 700;
	}

</style>
)__";

var css_cpp_style =

R"__(<style>
pre { line-height: 125%; }
td.linenos .normal { color: inherit; background-color: transparent; padding-left: 5px; padding-right: 5px; }
span.linenos { color: inherit; background-color: transparent; padding-left: 5px; padding-right: 5px; }
td.linenos .special { color: #000000; background-color: #ffffc0; padding-left: 5px; padding-right: 5px; }
span.linenos.special { color: #000000; background-color: #ffffc0; padding-left: 5px; padding-right: 5px; }



/*	0. .hll/.err
	1. Comments
	2. Keywords
	3. Operator
	4. Literals
	5. Name
	6. Generic
	7. Text.
*/

/* Highlight and Error */
:root {
  --highlight-bg-color: #ffffcc;
  --error-border-color: #FF0000;
  --highlight-font-weight: normal;
  --highlight-font-style: normal;
  --master-bold: bold; /*var(--master-bold)*/
  --master-italic: normal; /*var(--master-italic)*/
}
.hll { background-color: var(--highlight-bg-color); font-weight: var(--highlight-font-weight); font-style: var(--highlight-font-style); }
.err { border: 1px solid var(--error-border-color); font-weight: var(--highlight-font-weight); font-style: var(--highlight-font-style); } /* Error */

/* Comments */
:root {
  --comment-main-color: #3D7B7B;
  --comment-preproc-color: #9C6500;
  --comment-font-weight: var(--master-bold);
  --comment-font-style: var(--master-italic);
}
.c   { color: var(--comment-main-color); font-weight: var(--comment-font-weight); font-style: var(--comment-font-style); } /* Comment */
.ch  { color: var(--comment-main-color); font-weight: var(--comment-font-weight); font-style: var(--comment-font-style); } /* Comment.Hashbang */
.cm  { color: var(--comment-main-color); font-weight: var(--comment-font-weight); font-style: var(--comment-font-style); } /* Comment.Multiline */
.cp  { color: var(--comment-preproc-color); font-weight: var(--comment-font-weight); font-style: var(--comment-font-style); } /* Comment.Preproc */
.cpf { color: var(--comment-main-color); font-weight: var(--comment-font-weight); font-style: var(--comment-font-style); } /* Comment.PreprocFile */
.c1  { color: var(--comment-main-color); font-weight: var(--comment-font-weight); font-style: var(--comment-font-style); } /* Comment.Single */
.cs  { color: var(--comment-main-color); font-weight: var(--comment-font-weight); font-style: var(--comment-font-style); } /* Comment.Special */

/* Keywords */
:root {
  --keyword-main-color: #008000; /*green*/
  --keyword-type-color: #B00040; /*reddish*/
  --keyword-flow-color: #FF5722; /*orange*/
  --keyword-cnst-color: #FF0000; /*red*/
  --keyword-font-weight: var(--master-bold);
  --keyword-font-style: normal;
}
.k   { color: var(--keyword-main-color); font-weight: var(--keyword-font-weight); font-style: var(--keyword-font-style); } /* Keyword */
.kc  { color: var(--keyword-cnst-color); font-weight: var(--keyword-font-weight); font-style: var(--keyword-font-style); } /* Keyword.Constant */

/* used in exodus_cpp.py EXTRA_DECLARATIONS 'func', 'subr', 'function', 'subroutine' */
.kd  { color: var(--keyword-main-color); font-weight: var(--keyword-font-weight); font-style: var(--keyword-font-style); } /*Keyword.Declaration*/

.kn  { color: var(--keyword-main-color); font-weight: var(--keyword-font-weight); font-style: var(--keyword-font-style); } /* Keyword.Namespace */
.kp  { color: var(--keyword-main-color); font-weight: var(--keyword-font-weight); font-style: var(--keyword-font-style); } /* Keyword.Pseudo */

/* used in exodus_cpp.py EXTRA_FLOW_CONTROL 'stop', 'abort', 'abortall', 'call', 'gosub' */
.kr  { color: var(--keyword-flow-color); font-weight: var(--keyword-font-weight); font-style: var(--keyword-font-style); } /* Keyword.Reserved */

/* used in exodus_cpp.py EXTRA_TYPES 'dim', 'var', '_var', 'rex', '_rex', 'let', 'in', 'out', 'io', 'qqqqqqq'
									'libraryinit', 'libraryexit',
									 'commoninit', 'commonexit',
									'dictinit', 'dictexit'*/
.kt  { color: var(--keyword-type-color); font-weight: var(--keyword-font-weight); font-style: var(--keyword-font-style); } /* Keyword.Type */

/* Operators */
:root {
  --operator-main-color: #666666;
  --operator-word-color: #AA22FF;
  --operator-font-weight: var(--master-bold);
  --operator-font-style: normal;
}
.o   { color: var(--operator-main-color); font-weight: var(--operator-font-weight); font-style: var(--operator-font-style); } /* Operator */
.ow  { color: var(--operator-word-color); font-weight: var(--operator-font-weight); font-style: var(--operator-font-style); } /* Operator.Word */

/* Literals */
:root {
  --literal-color: #0000FF; /* Unified color (using original string main color as default) */
  --literal-font-weight: var(--master-bold);/* Unified weight (defaulting to normal) */
  --literal-font-style: var(--master-italic);   /* Unified style (defaulting to normal) */
}
.m   { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.Number */
.mb  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.Number.Bin */
.mf  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.Number.Float */
.mh  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.Number.Hex */
.mi  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.Number.Integer */
.mo  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.Number.Oct */
.il  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.Number.Integer.Long */
.nb  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Name.Builtin (Really Literal.Number.Boolean?) */
.s   { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String */
.sa  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Affix */
.sb  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Backtick */
.sc  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Char */
.dl  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Delimiter */
.sd  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Doc */
.s2  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Double */
.se  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Escape */
.sh  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Heredoc */
.si  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Interpol */
.sx  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Other */
.sr  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Regex */
.s1  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Single */
.ss  { color: var(--literal-color); font-weight: var(--literal-font-weight); font-style: var(--literal-font-style); } /* Literal.String.Symbol */

/* Unidentified Names - Perhaps the most important part of a program strangely unhighlighted by pygmentize default style.*/
:root {
  --unidentified-color: #000000;
  --unidentified-font-weight: var(--master-bold);
  --unidentified-font-style: normal;
}
.n   { color: var(--unidentified-color); font-weight: var(--unidentified-font-weight); font-style: var(--unidentified-font-style); } /* Unidentified */

/* Names */
:root {
  --name-attribute-color: #687822;
  --name-builtin-color: #008000;
  --name-class-color: #B00040; /* same as .kt keyword type color to be like types*/
  --name-constant-color: #880000;
  --name-decorator-color: #AA22FF;
  --name-entity-color: #717171;
  --name-exception-color: #CB3F38;
  --name-label-color: #767600;
  --name-tag-color: #008000;
  --name-variable-color: #19177C;
  --name-font-weight: var(--master-bold);
  --name-font-style: normal;
}
.na  { color: var(--name-attribute-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Attribute */
.nc  { color: var(--name-class-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Class */
.no  { color: var(--name-constant-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Constant */
.nd  { color: var(--name-decorator-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Decorator */
.ni  { color: var(--name-entity-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Entity */
.ne  { color: var(--name-exception-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Exception */

/* used in exodus_cpp.py EXTRAC_FUNCTIONS massive list*/
.nf  { color: var(--name-class-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Function */

.nl  { color: var(--name-label-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Label */
.nn  { color: var(--name-class-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Namespace */
.nt  { color: var(--name-tag-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Tag */
.nv  { color: var(--name-variable-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Variable */
.bp  { color: var(--name-builtin-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Builtin.Pseudo */
.fm  { color: var(--name-class-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Function.Magic */
.vc  { color: var(--name-variable-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Variable.Class */
.vg  { color: var(--name-variable-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Variable.Global */
.vi  { color: var(--name-variable-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Variable.Instance */
.vm  { color: var(--name-variable-color); font-weight: var(--name-font-weight); font-style: var(--name-font-style); } /* Name.Variable.Magic */

/* Generic */
:root {
  --generic-deleted-color: #A00000;
  --generic-error-color: #E40000;
  --generic-heading-color: #000080;
  --generic-inserted-color: #008400;
  --generic-output-color: #717171;
  --generic-subheading-color: #800080;
  --generic-traceback-color: #0044DD;
  --generic-font-weight: var(--master-bold);
  --generic-font-style: italic;
}
.gd  { color: var(--generic-deleted-color); font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Deleted */
.ge  { font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Emph */
.ges { font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.EmphStrong */
.gr  { color: var(--generic-error-color); font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Error */
.gh  { color: var(--generic-heading-color); font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Heading */
.gi  { color: var(--generic-inserted-color); font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Inserted */
.go  { color: var(--generic-output-color); font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Output */
.gp  { color: var(--generic-heading-color); font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Prompt */
.gs  { font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Strong */
.gu  { color: var(--generic-subheading-color); font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Subheading */
.gt  { color: var(--generic-traceback-color); font-weight: var(--generic-font-weight); font-style: var(--generic-font-style); } /* Generic.Traceback */

/* Whitespace */
:root {
  --whitespace-color: #bbbbbb;
  --text-font-weight: normal;
  --text-font-style: normal;
}
.w   { color: var(--whitespace-color); font-weight: var(--text-font-weight); font-style: var(--text-font-style); } /* Text.Whitespace */










</style>
)__";

let style_switcher_head = R"__(
<style>
		#style-select {
position: fixed; /* Keeps it in place while scrolling */
top: 10px;  /* Distance from the top */
right: 10px;/* Distance from the right */
z-index: 1000;  /* Ensures it stays on top of other content */
		}

.highlight { 
padding: 15px; 
border: 1px solid #ddd; 
border-radius: 4px; 
}
.highlight pre {
margin: 0;
}
body.light {
background: #f8f8f8;
color: #333;
}
body.dark {
background: #1e1e1e;
color: #ddd;
}
/* Default light and dark fallbacks */
.highlight.light {
background: #ffffff;
border-color: #ccc;
}
.highlight.dark {
background: #2d2d2d;
border-color: #555;
}
</style>
<link id="pygments-style" rel="stylesheet" href="">
)__";

let style_switcher_dropdown = R"__(
	<select id="style-select">
<option value="default" data-theme="light">default</option>
<option value="autumn" data-theme="light">autumn</option>
<option value="borland" data-theme="light">borland</option>
<option value="bw" data-theme="light">bw</option>
<option value="colorful" data-theme="light">colorful</option>
<option value="emacs" data-theme="light">emacs</option>
<option value="friendly" data-theme="light">friendly</option>
<option value="manni" data-theme="light">manni</option>
<option value="murphy" data-theme="light">murphy</option>
<option value="pastie" data-theme="light">pastie</option>
<option value="perldoc" data-theme="light">perldoc</option>
<option value="solarized-light" data-theme="light">solarized-light</option>
<option value="tango" data-theme="light">tango</option>
<option value="vs" data-theme="light">vs</option>

<option value="fruity" data-theme="dark">fruity</option>
<option value="monokai" data-theme="dark">monokai</option>
<option value="native" data-theme="dark">native</option>
<option value="solarized-dark" data-theme="dark">solarized-dark</option>
<option value="vim" data-theme="dark">vim</option>
</select>
)__";

let style_switcher_foot = R"__(
<script>
function setCookie(name, value, days) {
	const date = new Date();
	date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
	const expires = "expires=" + date.toUTCString();
	document.cookie = name + "=" + value + ";" + expires + ";path=/";
}

function getCookie(name) {
	const nameEQ = name + "=";
	const ca = document.cookie.split(';');
	for(let i = 0; i < ca.length; i++) {
		let c = ca[i];
		while (c.charAt(0) == ' ') c = c.substring(1);
		if (c.indexOf(nameEQ) == 0)
			return c.substring(nameEQ.length);
	}
	return null;
}

function loadStyle(styleName, theme) {
	const styleLink = document.getElementById('pygments-style');
	styleLink.href = `https://cdn.jsdelivr.net/gh/richleland/pygments-css@master/${styleName}.css`;
	document.body.className = theme;
	document.querySelector('.highlight').className = `highlight ${theme} ${styleName}`;
	setCookie('selectedStyle', styleName, 365);
	setCookie('selectedTheme', theme, 365);
}

const select = document.getElementById('style-select');
const savedStyle = getCookie('selectedStyle');
const savedTheme = getCookie('selectedTheme');

if (savedStyle && savedTheme) {
	select.value = savedStyle;
	loadStyle(savedStyle, savedTheme);
} else {
	loadStyle('default', 'light');
}

select.addEventListener(
	'change',
	function(e) {
		const selectedOption = e.target.selectedOptions[0];
		const theme = selectedOption.getAttribute('data-theme');
		loadStyle(e.target.value, theme);
	}
);
</script>
)__";

}; // programexit()
