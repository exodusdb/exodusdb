#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <shell2.h>
#include <singular.h>
#include <sysmsg.h>

#include <system_common.h>

#include <sys_common.h>
#include <win_common.h>

#include <window.hpp>

	var filename;
var key;
var msg;
var file;
var errors;
var lengthx;  //num
var osfile;
var temposfilename83;
var nimported;	//num
var fileptr;	//num
var buff;
var linenox;  //num
var eof;	  //num
var line;
var csv;
var ptr;  //num
//var ncols;
var dictfile;
var xx;
var cell;
var temp;
var nquotes;  //num
var recordx;
var allowduplicate;
var op;
var op2;
var wspos;
var wsmsg;

function main(in mode) {
	//doesnt seem to be any limit on number of files per folder
	//in ntfs filing system at least under win 2003 server
	//test wrote 1,000,000 files containing the filenumber 1,2,3 etc in about 5 mins
	//and the files could be randomly read and written at excellent speed

	//determine upload directory
	var uploadroot = SYSTEM.f(49);
	//if uploadroot='' then uploadroot='..\images\'
	//if uploadroot[-1,1] ne '\' then uploadroot:='\'
	var virtualroot = "../images/";
	virtualroot.converter("/", OSSLASH);

	if (mode eq "POSTUPLOAD") {

		filename		   = request_.f(3);
		key				   = request_.f(4);
		var targetfilename = request_.f(5);
		var newstatus	   = request_.f(6);

		//lock it
		gosub lockfile();
		if (msg) {
			return invalid(msg);
		}

		var rec;
		if (not(rec.read(file, key))) {
			msg = "upload.subs cannot read " ^ filename ^ " " ^ key;
postuploadfail:
			gosub unlockfile();
			call  sysmsg(msg);
			return invalid(msg);
		}

		var dictids		 = "VERSION^STATUS^USERNAME^DATETIME^STATION"_var;
		var fns			 = "";
		var dictfilename = "DICT." ^ filename;
		//		for (const var ii : range(1, 99)) {
		//			var dictid = dictids.field("*", ii);
		//
		//			///BREAK;
		//			if (not dictid)
		//				break;
		for (var dictid : dictids) {
			var fn = (dictid ^ "_ARCHIVED").xlate(dictfilename, 2, "X");
			if (not(fn) or not(fn.isnum())) {
				msg = (dictid ^ "_ARCHIVED").quote() ^ " is missing from " ^ dictfilename ^ " in upload.subs";
				goto postuploadfail;
			}
			fns ^= fn ^ _FM;
		}  //ii;
		fns.popper();

		var ii2			   = rec.f(fns.f(1)).fcount(_VM);
		rec(fns.f(1), ii2) = targetfilename;
		rec(fns.f(2), ii2) = newstatus;
		rec(fns.f(3), ii2) = USERNAME;
		rec(fns.f(4), ii2) = date() ^ "." ^ time().oconv("R(0)#5");
		rec(fns.f(5), ii2) = STATION;

		rec.write(file, key);

		gosub unlockfile();

	} else if (mode.field(".", 1) eq "MAKEUPLOADPATH") {

		if (not(authorised("UPLOAD CREATE", msg, ""))) {
			return invalid(msg);
		}

		//additional option to check file and key is not locked
		filename			= request_.f(3);
		key					= request_.f(4);
		var ensurenotlocked = request_.f(5);
		if (ensurenotlocked eq "undefined") {
			ensurenotlocked = "";
		}
		if (filename and ensurenotlocked) {
			gosub lockfile();
			if (msg) {
				//if we cant lock then why should be unlock?
				//gosub unlockfile
				return invalid(msg);
			}
			gosub unlockfile();
		}

		var uploadpath = mode.field(".", 2, 9999).lcase();
		if (uploadpath eq "") {
			return 0;
		}
		var tt = "\\";
		uploadpath.converter(tt, OSSLASH);

		if (uploadpath.cut(2).contains("..")) {
			msg = uploadpath.quote() ^ " ..  is not allowed";
			gosub unlockfile();
			return invalid(msg);
		}

		//cannot have * characters in keys represented in dos filenames
		//fixed in UI
		//convert '\/:*?<>|' to '--------' in uploadpath
		//convert '"' to "'" in uploadpath

		//if doesnt terminate with a \ then assume that the last bit is filename
		if (not uploadpath.ends(OSSLASH)) {
			uploadpath = uploadpath.field(OSSLASH, 1, uploadpath.count(OSSLASH)) ^ OSSLASH;
		}

		//ok if any files found
		//initdir uploadroot:uploadpath:'*.*'
		//if dirlist() then return
		if (oslistf(uploadroot ^ uploadpath ^ "*.*")) {
			return 0;
		}

		//otherwise make the folders in parent order
		let nsubfolders = uploadpath.count(OSSLASH);
		for (const var subfoldern : range(1, nsubfolders)) {
			var subfolder = uploadpath.field(OSSLASH, 1, subfoldern);
			//call subdirs(uploadroot:subfolder:char(0),result)
			//if result else
			var fullsubfolder = (uploadroot ^ subfolder).lcase();
			//remove doubled up separators - \\ on winddows or // on linux
			fullsubfolder.replacer(OSSLASH_ OSSLASH_, OSSLASH);
			//osopen fullsubfolder to xx else if status() ne 2 then
			//cannot use dos 8 characters since keys can be > 8 characters
			call shell2("dir " ^ fullsubfolder, errors);
			if (errors) {

				//call shell('md ':uploadroot:subfolder)
				//call mkdir(fullsubfolder:char(0),xx)
				//call mkdir2(fullsubfolder)
				//cannot use dos 8 characters since keys can be > 8 characters
				//actually mkdir2 calls md so it is the same thing
				call shell2("mkdir " ^ fullsubfolder, errors);

				if (not(VOLUMES)) {
					call shell2("chmod a+rwx -R " ^ fullsubfolder);
				}

				//check that folder was created
				//call subdirs(fullsubfolder:char(0),result)
				//if result else
				//osopen fullsubfolder to xx else if status() ne 2 then
				if (errors) {
					if (USERNAME eq "EXODUS") {
						msg = fullsubfolder;
					} else {
						msg = "(uploadroot)" ^ subfolder;
					}
					msg ^= " upload folder cannot be created";
					msg(-1) = errors;
					return invalid(msg);
				}
			}
		}  //subfoldern;

	} else if (mode.field(".", 1) eq "VERIFYUPLOAD") {

		//similar in VERIFY AND DELETEUPLOAD
		var uploadpath = mode.field(".", 2, 9999).lcase();
		//fixed in ui now
		//convert '\/:*?<>|' to '--------' in uploadpath
		//convert '"' to "'" in uploadpath

		//initdir uploadroot:uploadpath
		//if dirlist() else
		var tt = (shell2("dir " ^ ((uploadroot ^ uploadpath).quote()) ^ " /b", errors)).ucase();
		if (tt eq "" or tt.contains("FILE NOT FOUND")) {
			msg = "Error: Nothing uploaded in " ^ uploadroot ^ uploadpath;
			return invalid(msg);
		}

	} else if (mode.field(".", 1) eq "OPENUPLOAD") {

		//MODE eg = "OPENUPLOAD.test\upload\jobs\j1003\7.Selection_206.png"

		data_ = virtualroot;
		//data<2>=filenames

		//var virtualfilebase = mode.field(".", 2, 9999).lcase(); //done below
		var virtualfilebase = mode.field(".", 2, 9999);

		// Convert \ and / to OSSLASH
		virtualfilebase.converter("\\", "/");
		virtualfilebase.converter("/", OSSLASH);
		//fixed in UI now
		//convert '\/:*?<>|' to '--------' in virtualfilebase
		//convert '"' to "'" in virtualfilebase

		if (virtualfilebase.contains(OSSLASH)) {
			//uploadpath=field(virtualfilebase,OSSLASH,1,count(virtualfilebase,OSSLASH))
		} else {
			//uploadpath='*'
			//virtualfilebase='*'
		}

		//lcase upload path (not filenames which must retain case)
		var filename = virtualfilebase.field2(OSSLASH, -1, 1);
		//virtualfilebase.lcaser().paster(-len(filename), filename);
		//virtualfilebase.lcaser().pasterall(-len(filename), filename);
		virtualfilebase.lcaser();
		//virtualfilebase.cutter(-len(filename)) ^= filename;
		virtualfilebase.cutter(-len(filename));
		virtualfilebase ^= filename;

		//search for extensions
		//may be only one extension now
		var dirpatt = uploadroot ^ virtualfilebase;

		//if dirpatt[-1,1]<>'.' then dirpatt:='.'
		dirpatt ^= "*";

		if (dirpatt.cut(2).contains("..")) {
			msg = dirpatt.quote() ^ " .. is not allowed";
			return invalid(msg);
		}

		//why does % come encoded as %25?
		dirpatt.replacer("%25", "%");

		if (request_.f(3) eq "NEW") {
			if (oslistf(dirpatt)) {
				var cmd = "rm " ^ (dirpatt.quote());
				osshell(cmd);
			}
			return 0;
		}

		var uploadfilenames = oslistf(dirpatt);

		//check one or more files exist
		//if uploadfilenames else
		// msg='UPLOAD_NOT_FOUND'
		// goto invalid
		// end

		//select extensions if many .jpg .gif .mpg etc.
		//insert the missing virtual path
		if (uploadfilenames) {

			let nuploads = uploadfilenames.fcount(_FM);
			var ndeep	 = virtualfilebase.fcount(OSSLASH);
			for (const var uploadn : range(1, nuploads)) {
				var uploadfilename		 = uploadfilenames.f(uploadn);
				uploadfilename			 = virtualfilebase.fieldstore(OSSLASH, ndeep, 1, uploadfilename);
				uploadfilenames(uploadn) = uploadfilename;
			}  //uploadn;
			uploadfilenames.converter(_FM, _VM);
		}

		data_(2) = uploadfilenames;

		//no longer required since updated in PLAN.SUBS POSTREAD
		//since VIEW/OPENUPLOAD is no longer being called automatically after upload
		//hack to register material uploads after uploading (relies on view after)
		//only materials upload directly into images folder
		//hack since a media file should not be referred to in GBP upload.subs
		//if index(virtualfilebase,'/',1) else
		// open 'MATERIALS' to materials then
		//  materialid=virtualfilebase
		//  read material from materials,materialid then
		//   if material<16> ne uploadfilenames then
		//    material<16>=uploadfilenames
		//    *write without lock - any user updating it save will be blocked - but unlikely
		//    write material on materials,materialid
		//    end
		//   end
		//  end
		// end

	} else if (mode.field(".", 1) eq "DELETEUPLOAD") {

		if (not(authorised("UPLOAD DELETE", msg, ""))) {
			return invalid(msg);
		}

		//MODE eg = "DELETEUPLOAD.test\upload\jobs\j1003\7.Selection_206.png"

		//similar in VERIFY AND DELETEUPLOAD
		var uploadpath = mode.field(".", 2, 9999).lcase();
		//fixed in UI now
		//convert '\/:*?<>|' to '--------' in uploadpath
		//convert '"' to "'" in uploadpath

		//initdir uploadroot:uploadpath
		//if dirlist() else
		var tt = (shell2("dir " ^ ((uploadroot ^ uploadpath).quote()) ^ " /b", errors)).ucase();
		if (tt eq "" or tt.contains("FILE NOT FOUND")) {
			msg = "Error: Nothing uploaded in " ^ uploadroot ^ uploadpath;
			return invalid(msg);
		}

		tt = uploadroot ^ uploadpath;
		if (tt.cut(2).contains("..")) {
			msg = tt.quote() ^ " .. is not allowed";
			return invalid(msg);
		}

		var cmd = "rm " ^ (tt.quote());
		osshell(cmd);

		if (oslistf(uploadroot ^ uploadpath)) {
			msg = (uploadroot ^ uploadpath ^ " file cannot be deleted").quote();
			return invalid(msg);
		}

	} else if (mode eq "IMPORT") {
		var uploadpath	  = RECORD.f(1);
		var startatrown	  = RECORD.f(2);
		var headertype	  = RECORD.f(3);
		lengthx			  = RECORD.f(4);
		filename		  = RECORD.f(5);
		var dictfilename  = RECORD.f(6);
		var dictcolprefix = RECORD.f(7).ucase();
		var keydictid	  = RECORD.f(8);
		var keyfunction	  = RECORD.f(9);
		//reserve first 10 for non-imported additional info
		var fieldoffset		 = RECORD.f(10);
		var importcode		 = RECORD.f(11);
		var linenofn		 = RECORD.f(12);
		var importfilenamefn = RECORD.f(13);
		var importcodefn	 = RECORD.f(14);
		var datewords		 = RECORD.f(15);
		var timewords		 = RECORD.f(16);
		var validating		 = RECORD.f(17);
		osfile				 = "";

		if (uploadpath.cut(2).contains("..")) {
			msg = uploadpath.quote() ^ " .. is not allowed";
			return invalid(msg);
		}

		if (not dictcolprefix) {
			dictcolprefix = "IMPORT";
		}
		if (not dictfilename) {
			dictfilename = filename;
		}
		if (dictfilename.first(4) ne "DICT") {
			dictfilename.prefixer("DICT.");
		}
		//if keydictid else keydictid='@ID'
		if (not lengthx) {
			lengthx = 4096;
		}
		if (not fieldoffset) {
			fieldoffset = 10;
		}
		file = "";

		if (not importcode) {
			importcode = (field2(uploadpath, OSSLASH, -1)).ucase();
		}
		if (importcode.contains(".")) {
			var tt = field2(uploadpath, ".", -1);
			importcode.cutter(-tt.len() - 1);
		}
		importcode.converter(" .", "--");
		RECORD(11) = importcode;

		//if dirlist2(uploadroot:uploadpath) then
		// msg=quote(uploadroot:uploadpath:' file cannot be found')
		// goto invalid
		// end

		temposfilename83 = (SYSTEM.f(24) ^ var(1000000).rnd()).first(8) ^ "._IM";

		msg = (uploadroot ^ uploadpath).quote() ^ "\r\n";
		msg =
			"Uploaded file cannot be found/copied"
			"\r\n";
		msg ^= temposfilename83.quote() ^
			   " file cannot be opened"
			   ")";

		nimported = "";

		//from here on gosub cleanup to clean up temporary file

		var	 cmd = "cp " ^ ((uploadroot ^ uploadpath).quote()) ^ " " ^ temposfilename83;
		call shell2(cmd, errors);
		if (errors) {
			printl(errors);
			gosub invalid(msg);
			gosub cleanup();
			return 0;
		}

		if (not(osfile.osopen(temposfilename83))) {
			gosub invalid(msg);
			gosub cleanup();
			return 0;
		}

		fileptr = 0;
		buff	= "";
		linenox = 0;
		//cols os "name vm start vm len" etc one col per fm
		var cols  = "";
		nimported = 0;

		while (true) {

nextline:
			/////////
			gosub getline();

			if (eof)
				break;

			if (not(line and linenox ge startatrown)) {
				goto nextline;
			}

			//determine cols from first col heading
			if (not(cols)) {

				csv = line.index("\",\"");
				if (csv) {

					gosub parseline(line);
					line.converter(_VM, _FM);
					cols = line;

				} else {
					var offset = 1;
					while (true) {
						var tt = line.index("  ");

						if (not tt)
							break;

						cols(-1) = line.first(tt - 1) ^ VM ^ offset;
						for (ptr = tt; ptr <= 999999; ++ptr) {

							if (line[ptr + 1] ne " ")
								break;

						}  //ptr;
						cols ^= VM ^ ptr;
						offset += ptr;
						line.cutter(ptr);
					}  //loop;
				}

				goto nextline;
			}

			let ncols = cols.fcount(_FM);
			//first record, open files and create dictionary
			if (file eq "") {

				if (not(file.open(filename, ""))) {
					call mssg(lasterror());
					win.valid = 0;
					gosub cleanup();
					return 0;
				}
				if (not(dictfile.open(dictfilename, ""))) {
					call mssg(lasterror());
					win.valid = 0;
					gosub cleanup();
					return 0;
				}

				//create dictionary
				//grec='BY NUMID'
				//grec='ID-SUPP'
				//MUST be pure dictids
				var grec = "";

				if (linenofn) {
					grec ^= "LINE_NO ";
					var dictrec = "F" _FM ^ linenofn ^ _FM "Line No." ^ str(FM, 6) ^ "R" _FM ^ "10";
					dictrec.write(dictfile, "LINE_NO");
				}

				for (const var coln : range(1, ncols)) {
					var dictrec = "F";
					dictrec(2)	= coln + fieldoffset;
					dictrec(3)	= capitalise(cols.f(coln, 1));
					dictrec(10) = "10";
					var dictid	= dictcolprefix ^ "_" ^ cols.f(coln, 1).convert(" ", "_").ucase();

					var CONV = "";
					var just = "L";
					var nn	 = dictid.fcount("_");
					for (const var ii : range(1, nn)) {
						var word = dictid.field("_", ii);
						if (datewords.locate(word, xx)) {
							CONV = "[DATE,4*]";
							just = "R";
						} else {
							if (timewords.locate(word, xx)) {
								CONV = "[TIME2,48MTS]";
								just = "R";
							}
						}
					}  //ii;
					cols(coln, 4) = CONV;
					dictrec(7)	  = CONV;
					dictrec(9)	  = just;

					grec ^= dictid ^ " ";
					dictrec.write(dictfile, dictid);
				}  //coln;
				grec.popper();
				("G" _FM _FM ^ grec).write(dictfile, dictcolprefix);
				("G" _FM _FM ^ grec).write(dictfile, "@CRT");

				if (keyfunction) {
					if (not keydictid) {
						keydictid = dictcolprefix ^ "_TEMPKEY";
					}
					var tt = "S" _FM _FM ^ keydictid;
					tt(8)  = keyfunction;
					tt(9)  = "R";
					tt(10) = 10;

					tt.write(dictfile, keydictid);
				}
			}

			if (csv) {
				gosub parseline(line);
			}

			var rec = "";
			for (const var coln : range(1, ncols)) {
				var col = cols.f(coln);
				if (csv) {
					cell = line.f(1, coln);
				} else {
					cell = line.b(col.f(1, 2), col.f(1, 3)).trimlast();
				}
				if (cell.len()) {
					if (col.f(1, 4)) {
						var cell0 = cell;
						var CONV  = col.f(1, 4);
						if (CONV.contains("TIME")) {
							//if no : in time then assume is already seconds
							if (cell.contains(":")) {
								cell = iconv(cell, CONV);
							}
						} else {
							cell = iconv(cell, CONV);
						}
						if (not(cell.len())) {
							call mssg(cell0.quote() ^ " cannot be converted in line " ^ linenox ^ " col " ^ coln);
							//indicate strange but leave workable date/time
							cell = "00";
						}
					}
				}
				rec(coln + fieldoffset) = cell;
			}  //coln;

			key = importcode ^ "*" ^ linenox;
			if (importfilenamefn) {
				rec(importfilenamefn) = uploadpath;
			}
			if (importcodefn) {
				rec(importcodefn) = importcode;
			}
			if (linenofn) {
				rec(linenofn) = linenox;
			}
			//TODO once (on record one) should really check that any
			//columns defined in the keydict as {} fields EXIST in the dictfile
			if (keydictid) {
				key = calculate(keydictid, dictfile, key, rec, 0);
			}

			nimported += 1;

			if (not validating) {
				rec.write(file, key);
			}

		}  //loop;

		gosub cleanup();

	} else {
		msg = mode.quote() ^ " invalid mode in upload.subs";
		return invalid(msg);
	}

	return 0;
}

subroutine cleanup() {
	osfile.osclose();
	temposfilename83.osremove();
	ANS = nimported;
	return;
}

subroutine getline() {

	if (buff.len() lt lengthx) {

addbuff:
		////////
		call osbread(temp, osfile, fileptr, lengthx);
		temp.converter("\n\f", "\r\r");
		buff ^= temp;

		//get more if no line ending
		if (temp.len() and not(buff.contains("\r"))) {
			goto addbuff;
		}

		if (not(buff.len())) {
			eof	 = 1;
			line = "";
			return;
		}
	}

	//skip empty lines
	//	while (true) {
	//		///BREAK;
	//		if (not(buff.starts("\r"))) break;
	//		buff.cutter(1);
	//	}//loop;
	buff.trimmerfirst("\r");
	if (not(buff.len())) {
		goto addbuff;
	}

	line = buff.field("\r", 1);
	buff.cutter(line.len() + 1);

	nquotes = line.count(DQ);
	while (true) {
		///BREAK;
		if (not(((nquotes / 2).floor() * 2 ne nquotes) and buff.len()))
			break;
		var line2 = buff.field("\r", 1);
		nquotes += line2.count(DQ);
		buff.cutter(line2.len() + 1);
		line ^= "\n";
		line ^= line2;
	}  //loop;

	linenox += 1;
	eof = 0;

	return;
}

subroutine lockfile() {
	msg = "";
	if (not(file.open(filename, ""))) {
		msg = filename.quote() ^ " file cannot be opened";
		return;
	}

	var waitsecs = 3;
	if (not(lockrecord(filename, file, key, recordx, waitsecs, allowduplicate))) {
		gosub unlockfile();
		msg			 = "Cannot upload at the moment because";
		msg(-1)		 = filename ^ " " ^ (key.quote()) ^ " is being updated by ";
		var lockuser = (filename ^ "*" ^ key).xlate("LOCKS", 4, "X");
		if (lockuser) {
			msg ^= lockuser;
		} else {
			msg ^= "someone else";
		}
	}

	return;
}

subroutine unlockfile() {
	call unlockrecord(filename, file, key);
	return;
}

subroutine parseline(io line) {

	//preserve escaped quotes
	line.replacer(_BS _DQ, "&quote;");

	//preserve commas inside quotes
	nquotes = line.count(DQ);
	for (var quoten = 2; quoten <= nquotes; quoten += 2) {
		var tt = line.field(DQ, quoten);
		if (tt.contains(",")) {
			tt.replacer(",", "&comma;");
			line = line.fieldstore(DQ, quoten, 1, tt);
		}
	}  //quoten;

	line.converter(",", _VM);

	line.replacer(DQ, "");
	line.replacer("&quote;", DQ);
	line.replacer("&comma;", ",");

	return;
}

libraryexit()
