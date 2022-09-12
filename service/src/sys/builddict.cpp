#include <exodus/program.h>
programinit()

//#include <sys_common.h>

var blocktype;
var blockid;
var blockline;
var block;
var ans;
var xx;
var macron;
var paramn;//num
var lineparam;
var wordn;
var fieldn;//num

var macros;
var nmacros;
var dictids;
var doall;
var filename;
var dictfilename;
var nlns;
var doc;
var splitline;
var ln;

function main() {
	//#include <system_common.h>

	filename = SENTENCE.field(" ", 2);
	dictfilename = SENTENCE.field(" ", 3);

	if (not(filename and dictfilename)) {
		call mssg("Syntax is builddict sourcefilename targetdictfilename" ^ FM ^ "e.g. builddict DOS dict.ads");
		stop();
	}

	var file;
	if (not(file.open(filename, ""))) {
		call fsmsg();
		stop();
	}
	if (not(DICT.open(dictfilename))) {
		call fsmsg();
		stop();
	}

	var key = dictfilename ^ ".txt";
	if (not(doc.read(file, key))) {
		call mssg(key.quote() ^ " is missing from " ^ filename);
		stop();
	}
	doc.converter("\n",FM);
	doc.converter("\t"," ");
	nlns = doc.count(FM) + 1;

	//note the dictids are oswritten out to 'BDICT'
	//you are advised to LIST filename with those bdicts to test

	ln = 1;
	macros = "";
	nmacros = "";
	dictids = "";
	doall = "";

	while (true) {
		gosub getblock();
		///BREAK;
		if (not blocktype) break;

		if (blocktype eq "macro") {
			gosub definemacro();

		} else if (blocktype eq "dict") {
			gosub writedict();

		} else if (blocktype eq "comment") {
			//ignore

		} else {
			call mssg(blocktype.quote() ^ " unknown blocktype");
			stop();
		}

	}//loop;

	stop();

	return "";
}

subroutine definemacro() {
	nmacros += 1;
	macros(1, nmacros) = blockid;
	splitline = blockline.trim();
	splitline.converter(" ", VM);
	macros(2, nmacros) = lower(splitline.field(VM, 4, 999999));
	macros(3, nmacros) = lower(lower(block));
	return;
}

subroutine writedict() {

	block.lowerer().trimmerb(VM);

	if (not block.index("function main()"))
		block ^= VM ^ "\treturn ANS;";

	// blockid may be followed by |1|7 for example
	// to generate 7 suffixed dictid ending _1 to _7
	// The various dict titles will be multivalues
	// found in the base dict record which has no suffix.
	var suffix_from = blockid.field("|", 2);
	var suffix_upto = blockid.field("|", 3);

	var basedictrec;
	for (var suffixno : range(suffix_from, suffix_upto)) {

		var dictid = blockid.field("|",1);

		// Suffix can be 0
		if (suffixno) {

			if (suffixno eq 1) {
				var osdatfilename = "dat/" ^ dictfilename ^ "/" ^ dictid;

				var dictrec;
				if (not(dictrec.read(DICT, dictid))) {
					call mssg(dictid.quote() ^ " is missing from DICT." ^ dictfilename);
					stop();
				}
				if (not(dictrec.osread(osdatfilename))) {
					call mssg(osdatfilename.quote() ^ " is missing");
					stop();
				}
				dictrec = dictrec.iconv("TX");

				basedictrec = dictrec;
			}

			dictid ^= "_" ^ suffixno;

		}

		var osdatfilename = "dat/" ^ dictfilename ^ "/" ^ dictid;

		var dictrec;
		if (not(dictrec.read(DICT, dictid))) {
			call mssg(dictid.quote() ^ " is missing from DICT." ^ dictfilename);
			stop();
		}
		if (not(dictrec.osread(osdatfilename))) {
			call mssg(osdatfilename.quote() ^ " is missing");
			stop();
		}
		dictrec = dictrec.iconv("TX");

		dictids ^= " " ^ dictid;
		//var(dictids).oswrite("bdict");

		var f8 = dictrec.f(8).trimb(VM);
		dictrec(8) = f8;
		var origdictrec = dictrec;

		// Update the source code
		dictrec(8) = block;
		if (suffixno)
			dictrec.swapper("{||}", suffixno);

		// Get the relevent column title
		if (suffixno)
			dictrec(3) = basedictrec.f(3, suffixno);

		if (dictrec ne origdictrec) {
			printl(var().chr(12), dictid);
			printl();
			printl("---------- OLD ", dictid, " ----------");
			printl(origdictrec.f(8).swap(VM, "\n"));
			printl();
			if (doall) {
				ans = "Y";
			} else {
	//			print("press any key:");
	//			osflush();
	//			xx.inputn(1);
	//			if (var("Qq").index(xx)) {
	//				stop();
	//			}
				printl("---------- NEW ", dictid, " ----------");
				printl(dictrec.f(8).swap(VM, "\n"));
				print("Update (y=yes, q=quit, a=all, n=no)");
				osflush();
				ans.inputn(1);
				if (var("Qq").index(ans)) {
					printl();
					stop();
				}
				if (var("Aa").index(ans)) {
					ans = "Y";
					doall = "Y";
				}
			}
			if (ans eq "Y" or ans eq "y") {
				//dictrec.write(DICT, dictid);
				oswrite(oconv(dictrec,"TX") on osdatfilename);
			}
		}

	}

	return;
}

subroutine getblock() {
	blocktype = "";
	blockid = "";
	blockline = "";
	block = "";
	for (; ln <= nlns; ++ln) {
		var line = doc.f(ln);
		var word1 = line.field(" ", 1);

		if (word1 eq "#define" or word1 eq "#comment") {

			if (blocktype) {
				block.splicer(1, 1, "");
				while (true) {
					///BREAK;
					if (not(block[-1] eq FM)) break;
					block.splicer(-1, 1, "");
				}//loop;
				return;
			}

			if (word1 eq "#comment") {
				blockline = "#define comment";
			} else {
				blockline = line.trim();
			}
			blocktype = blockline.field(" ", 2);
			blockid = blockline.field(" ", 3);

			//cater for macro on the define dict line
			if (blocktype eq "dict") {
				var tt = blockline.field(" ", 4, 999999);
				if (tt) {
					line = "#include " ^ tt;
					goto addline;
				}
			}

		} else {

addline:
			splitline = line.trim();
			splitline.converter(" ", VM);
			word1 = splitline.f(1, 1);
			if (word1 eq "#include") {
				var macroid = splitline.f(1, 2);
				var lineparams = splitline.field(VM, 3, 999999);
				if (macros.f(1).locate(macroid, macron)) {
					var macroline = raise(macros.f(2, macron));
					var macrotext = raise(raise(macros.f(3, macron)));

					var nmlns = macrotext.count(FM) + 1;
					for (var mln = 1; mln <= nmlns; ++mln) {
						var mline0 = macrotext.f(mln).convert("\t"," ");
						var mline = mline0.trim(" ");
						var mword1 = mline.field(" ", 1);
						if (mword1 eq "#ifdef" or mword1 eq "#ifndef") {
							var mword2 = mline.field(" ", 2);
							if (macroline.locate(mword2, paramn)) {
								lineparam = lineparams.f(1, paramn);
								//dot means not defined while we are using spaces to separate params
								if (lineparam eq ".") {
									lineparam = "";
								}
							} else {
								lineparam = "";
							}
							if (lineparam.len()) {
								if (mword1 eq "#ifndef") {
deletemline:
									macrotext.remover(mln);
									mln -= 1;
									goto nextmln;
								}
insertmline:
								//remove #ifdef xxx
								mline0.converter(" " ^ VM, VM ^ " ");
								if (mline0.locate(mword1, wordn)) {
									mline0.remover(1, wordn);
								}
								if (mline0.locate(mword2, wordn)) {
									mline0.remover(1, wordn);
								}
								mline0.converter(" " ^ VM, VM ^ " ");
								mline0 = str("\t", len(mline0) - len(mline0.trimf())) ^ mline0.trimf();
								macrotext(mln) = mline0;

							} else {
								if (mword1 eq "#ifdef") {
									goto deletemline;
								}
								goto insertmline;
							}

						}
nextmln:;
					} //mln;

					//substitute all macro parameters
					var nparams = macroline.count(VM) + 1;
					for (paramn = 1; paramn <= nparams; ++paramn) {
						var old = macroline.f(1, paramn);
						var newx = lineparams.f(1, paramn);
						macrotext.swapper(old, newx);
					} //paramn;

					//speed up fieldwise {} for F fields. avoid calling a dict subroutine
					var allfields = "";
					var fields = "";
//					var findfieldn = 0;
//					while (true) {
					for (var findpos = 1;; findpos++) {

//						findfieldn += 1;
						//findpos = macrotext.index2("{", findpos);
						findpos = macrotext.index("{", findpos);
						///BREAK;
						if (not findpos)
							break;

						var fieldname = macrotext.substr(findpos + 1, "}", COL2);

						// skip c++ brackets { } and only convert old calculated fields like {XXXXXXXXX}
						if (not fieldname.match("^[A-Za-z_]\\w*$"))
							continue;

						if (not(allfields.locate(fieldname, fieldn))) {
							allfields(1, -1) = fieldname;
							var tempdict;
							if (tempdict.read(DICT, fieldname)) {
								if (tempdict.f(1) eq "F") {
									fields(1, -1) = fieldname;
									fields(2, -1) = tempdict.f(2);
									fields(3, -1) = lower(tempdict.f(3));
								}
							} else {
								call mssg(fieldname.quote() ^ " field is missing from DICT " ^ DICT);
							}
						}
					}//loop;

					var equates = "\t/" "/GENERATED BY " ^ SENTENCE.quote();
					equates ^= FM ^ "\t/" "/DO NOT EDIT MANUALLY";
					if (fields) {
						var nfields = fields.f(1).count(VM) + 1;
						for (fieldn = 1; fieldn <= nfields; ++fieldn) {
							var fieldname = fields.f(1, fieldn);
							macrotext.swapper("{" ^ fieldname ^ "}", fieldname);
							//swap '{':fieldname:'}' with '@record<':fields<2,fieldn>:'>' in macrotext
							equates(-1) = "\t#define " ^ fieldname ^ "\tRECORD.f(" ^ fields.f(2, fieldn) ^ ")";
						} //fieldn;
						macrotext.splicer(1, 0, equates ^ FM);
					}

					macrotext.transfer(line);

				} else {
					//call mssg(macroid.quote() ^ " macro is undefined in line " ^ ln ^ FM ^ doc.f(ln));
					//stop();
				}

			}

			var indents = len(line)-len(trimf(line));

			block ^= FM ^ str("\t",indents) ^ trimf(line);

		}

//nextln:;
	} //ln;

	return;
}

programexit()
