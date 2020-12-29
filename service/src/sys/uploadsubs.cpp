#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <authorised.h>
#include <shell2.h>
#include <singular.h>

#include <gen_common.h>
#include <win_common.h>

#include <window.hpp>

var filename;
var key;
var msg;
var file;
var ii;//num
var errors;
var lengthx;//num
var osfile;
var temposfilename83;
var nimported;//num
var fileptr;//num
var buff;
var linenox;//num
var eof;//num
var line;
var ptr;//num
var ncols;
var dictfile;
var xx;
var temp;
var recordx;
var allowduplicate;
var op;
var op2;
var wspos;
var wsmsg;

function main(in mode) {
	//c sys
	//doesnt seem to be any limit on number of files per folder
	//in ntfs filing system at least under win 2003 server
	//test wrote 1,000,000 files containing the filenumber 1,2,3 etc in about 5 mins
	//and the files could be randomly read and written at excellent speed

	#include <general_common.h>
	//global ptr,ii

	#define request USER0
	#define data USER1

	//determine upload directory
	var uploadroot = SYSTEM.a(49);
	//if uploadroot='' then uploadroot='..\images\'
	//if uploadroot[-1,1] ne '\' then uploadroot:='\'
	var virtualroot = "../images/";
	virtualroot.converter("/", OSSLASH);

	if (mode eq "POSTUPLOAD") {

		filename = USER0.a(3);
		key = USER0.a(4);
		var targetfilename = USER0.a(5);
		var newstatus = USER0.a(6);

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
			call sysmsg(msg);
			return invalid(msg);
		}

		var dictids = "VERSION*STATUS*USERNAME*DATETIME*STATION";
		var fns = "";
		var dictfilename = "DICT." ^ filename;
		for (ii = 1; ii <= 99; ++ii) {
			var dictid = dictids.field("*", ii);

			///BREAK;
			if (not dictid) break;
			var fn = (dictid ^ "_ARCHIVED").xlate(dictfilename, 2, "X");
			if (not(fn) or not(fn.isnum())) {
				msg = (dictid ^ "_ARCHIVED").quote() ^ " is missing from " ^ dictfilename ^ " in upload.subs";
				goto postuploadfail;
			}
			fns.r(ii, fn);
		};//ii;

		ii = rec.a(fns.a(1)).count(VM) + (rec.a(fns.a(1)) ne "");
		rec.r(fns.a(1), ii, targetfilename);
		rec.r(fns.a(2), ii, newstatus);
		rec.r(fns.a(3), ii, USERNAME);
		rec.r(fns.a(4), ii, var().date() ^ "." ^ var().time().oconv("R(0)#5"));
		rec.r(fns.a(5), ii, STATION);

		rec.write(file, key);

		gosub unlockfile();

	} else if (mode.field(".", 1) eq "MAKEUPLOADPATH") {

		if (not(authorised("UPLOAD CREATE", msg, ""))) {
			return invalid(msg);
		}

		//additional option to check file and key is not locked
		filename = USER0.a(3);
		key = USER0.a(4);
		var ensurenotlocked = USER0.a(5);
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

		if (uploadpath.substr(3,99999).index("..")) {
			msg = uploadpath.quote() ^ " ..  is not allowed";
			gosub unlockfile();
			return invalid(msg);
		}

		//cannot have * characters in keys represented in dos filenames
		//fixed in UI
		//convert '\/:*?<>|' to '--------' in uploadpath
		//convert '"' to "'" in uploadpath

		//if doesnt terminate with a \ then assume that the last bit is filename
		if (uploadpath[-1] ne OSSLASH) {
			uploadpath = uploadpath.field(OSSLASH, 1, uploadpath.count(OSSLASH)) ^ OSSLASH;
		}

		//ok if any files found
		//initdir uploadroot:uploadpath:'*.*'
		//if dirlist() then return
		if (oslistf(uploadroot ^ uploadpath ^ "*.*")) {
			return 0;
		}

		//otherwise make the folders in parent order
		var nsubfolders = uploadpath.count(OSSLASH);
		for (var subfoldern = 1; subfoldern <= nsubfolders; ++subfoldern) {
			var subfolder = uploadpath.field(OSSLASH, 1, subfoldern);
			//call subdirs(uploadroot:subfolder:char(0),result)
			//if result else
			var fullsubfolder = (uploadroot ^ subfolder).lcase();
			//remove doubled up separators - \\ on winddows or // on linux
			fullsubfolder.swapper(OSSLASH OSSLASH, OSSLASH);
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
					}else{
						msg = "(uploadroot)" ^ subfolder;
					}
					msg ^= " upload folder cannot be created";
					msg.r(-1, errors);
					return invalid(msg);
				}

			}
		};//subfoldern;

	} else if (mode.field(".", 1) eq "VERIFYUPLOAD") {

		//similar in VERIFY AND DELETEUPLOAD
		var uploadpath = mode.field(".", 2, 9999).lcase();
		//fixed in ui now
		//convert '\/:*?<>|' to '--------' in uploadpath
		//convert '"' to "'" in uploadpath

		//initdir uploadroot:uploadpath
		//if dirlist() else
		var tt = (shell2("dir " ^ ((uploadroot ^ uploadpath).quote()) ^ " /b", errors)).ucase();
		if (tt eq "" or tt.index("FILE NOT FOUND")) {
			msg = "Error: Nothing uploaded in " ^ uploadroot ^ uploadpath;
			return invalid(msg);
		}

	} else if (mode.field(".", 1) eq "OPENUPLOAD") {

		USER1 = virtualroot;
		//data<2>=filenames

		var virtualfilebase = mode.field(".", 2, 9999).lcase();
		virtualfilebase.converter("\\", "/");
		virtualfilebase.converter("/", OSSLASH);
		//fixed in UI now
		//convert '\/:*?<>|' to '--------' in virtualfilebase
		//convert '"' to "'" in virtualfilebase

		if (virtualfilebase.index(OSSLASH)) {
			//uploadpath=field(virtualfilebase,OSSLASH,1,count(virtualfilebase,OSSLASH))
		}else{
			//uploadpath='*'
			//virtualfilebase='*'
		}

		//search for extensions
		//may be only one extension now
		var dirpatt = uploadroot ^ virtualfilebase;

		//if dirpatt[-1,1]<>'.' then dirpatt:='.'
		dirpatt ^= "*";

		if (dirpatt.substr(3,99999).index("..")) {
			msg = dirpatt.quote() ^ " .. is not allowed";
			return invalid(msg);
		}

		//why does % come encoded as %25?
		dirpatt.swapper("%25", "%");

		if (USER0.a(3) eq "NEW") {
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
			var nuploads = uploadfilenames.count(FM) + 1;

			var ndeep = virtualfilebase.count(OSSLASH) + 1;
			for (var uploadn = 1; uploadn <= nuploads; ++uploadn) {
				var uploadfilename = uploadfilenames.a(uploadn);
				uploadfilename = virtualfilebase.fieldstore(OSSLASH, ndeep, 1, uploadfilename);
				uploadfilenames.r(uploadn, uploadfilename);
			};//uploadn;
			uploadfilenames.converter(FM, VM);

		}

		USER1.r(2, uploadfilenames);

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

		//similar in VERIFY AND DELETEUPLOAD
		var uploadpath = mode.field(".", 2, 9999).lcase();
		//fixed in UI now
		//convert '\/:*?<>|' to '--------' in uploadpath
		//convert '"' to "'" in uploadpath

		//initdir uploadroot:uploadpath
		//if dirlist() else
		var tt = (shell2("dir " ^ ((uploadroot ^ uploadpath).quote()) ^ " /b", errors)).ucase();
		if (tt eq "" or tt.index("FILE NOT FOUND")) {
			msg = "Error: Nothing uploaded in " ^ uploadroot ^ uploadpath;
			return invalid(msg);
		}

		tt = uploadroot ^ uploadpath;
		if (tt.substr(3,99999).index("..")) {
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
		var uploadpath = RECORD.a(1);
		var startatrown = RECORD.a(2);
		var headertype = RECORD.a(3);
		lengthx = RECORD.a(4);
		filename = RECORD.a(5);
		var dictfilename = RECORD.a(6);
		var dictcolprefix = RECORD.a(7);
		var keydictid = RECORD.a(8);
		var keyfunction = RECORD.a(9);
		//reserve first 10 for non-imported additional info
		var fieldoffset = RECORD.a(10);
		var importcode = RECORD.a(11);
		var linenofn = RECORD.a(12);
		var importfilenamefn = RECORD.a(13);
		var importcodefn = RECORD.a(14);
		var datewords = RECORD.a(15);
		var timewords = RECORD.a(16);
		var validating = RECORD.a(17);
		osfile = "";

		if (uploadpath.substr(3,99999).index("..")) {
			msg = uploadpath.quote() ^ " .. is not allowed";
			return invalid(msg);
		}

		if (not dictcolprefix) {
			dictcolprefix = "IMPORT";
		}
		if (not dictfilename) {
			dictfilename = filename;
		}
		if (dictfilename.substr(1,4) ne "DICT") {
			dictfilename.splicer(1, 0, "DICT.");
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
		if (importcode.index(".")) {
			var tt = field2(uploadpath, ".", -1);
			importcode.splicer(-tt.length() - 1, 999, "");
		}
		importcode.converter(" .", "--");
		RECORD.r(11, importcode);

		//if dirlist2(uploadroot:uploadpath) then
		// msg=quote(uploadroot:uploadpath:' file cannot be found')
		// goto invalid
		// end

		temposfilename83 = (SYSTEM.a(24) ^ var(1000000).rnd()).substr(1,8) ^ ".$IM";

		msg = (uploadroot ^ uploadpath).quote() ^ "\r\n";
		msg = "Uploaded file cannot be found/copied" "\r\n";
		msg ^= temposfilename83.quote() ^ " file cannot be opened" ")";

		nimported = "";

		//from here on gosub cleanup to clean up temporary file

		var cmd = "cp " ^ ((uploadroot ^ uploadpath).quote()) ^ " " ^ temposfilename83;
		call shell2(cmd, errors);
		if (errors) {
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
		buff = "";
		linenox = 0;
		//cols os "name vm start vm len" etc one col per fm
		var cols = "";
		nimported = 0;

		while (true) {
			gosub getline();
			///BREAK;
			if (not(not(eof))) break;
			if (line and linenox ge startatrown) {

				//determine cols from first col heading
				if (not(cols)) {
					var offset = 1;
					while (true) {
						var tt = line.index("  ");
						///BREAK;
						if (not tt) break;
						cols.r(-1, line.substr(1,tt - 1) ^ VM ^ offset);
						for (ptr = tt; ptr <= 999999; ++ptr) {
							///BREAK;
							if (line[ptr + 1] ne " ") break;
						};//ptr;
						cols ^= VM ^ ptr;
						offset += ptr;
						line.splicer(1, ptr, "");
					}//loop;
					ncols = cols.count(FM) + 1;
	//oswrite cols on 'cols'

				}else{

					//first record, open files and create dictionary
					if (file eq "") {

						if (not(file.open(filename, ""))) {
							call fsmsg();
							win.valid = 0;
							gosub cleanup();
							return 0;
						}
						if (not(dictfile.open(dictfilename, ""))) {
							call fsmsg();
							win.valid = 0;
							gosub cleanup();
							return 0;
						}

						//create dictionary
						//grec='BY NUMID'
						//grec='ID-SUPP'
						//MUST be pure dictids
						var grec = "";
						for (var coln = 1; coln <= ncols; ++coln) {
							var dictrec = "F";
							dictrec.r(2, coln + fieldoffset);
							dictrec.r(3, capitalise(cols.a(coln, 1)));
							dictrec.r(10, "10");
							var dictid = dictcolprefix ^ "_" ^ cols.a(coln, 1).convert(" ", "_").ucase();

							var CONV = "";
							var just = "L";
							var nn = dictid.count("_") + 1;
							for (ii = 1; ii <= nn; ++ii) {
								var word = dictid.field("_", ii);
								if (datewords.locate(word,xx)) {
									CONV = "[DATE,4*]";
									just = "R";
								}else{
									if (timewords.locate(word,xx)) {
										CONV = "[TIME2,48MTS]";
										just = "R";
									}
								}
							};//ii;
							cols.r(coln, 4, CONV);
							dictrec.r(7, CONV);
							dictrec.r(9, just);

							grec ^= " " ^ dictid;
							dictrec.write(dictfile, dictid);
						};//coln;
						("G" ^ FM ^ FM ^ grec).write(dictfile, dictcolprefix);

						if (keyfunction) {
							if (not keydictid) {
								keydictid = dictcolprefix ^ "_TEMPKEY";
							}
							var tt = "S" ^ FM ^ FM ^ keydictid;
							tt.r(8, keyfunction);
							tt.r(9, "R");
							tt.r(10, 10);

							tt.write(dictfile, keydictid);
						}

					}else{

						var rec = "";
						for (var coln = 1; coln <= ncols; ++coln) {
							var col = cols.a(coln);
							var cell = line.substr(col.a(1, 2),col.a(1, 3)).trimb();
							if (cell.length()) {
								if (col.a(1, 4)) {
									var cell0 = cell;
									var CONV = col.a(1, 4);
									if (CONV.index("TIME")) {
									//if no : in time then assume is already seconds
										if (cell.index(":")) {
											cell = iconv(cell, CONV);
										}
									}else{
										cell = iconv(cell, CONV);
									}
									if (not(cell.length())) {
										call mssg(cell0.quote() ^ " cannot be converted in line " ^ linenox ^ " col " ^ coln);
									//indicate strange but leave workable date/time
										cell = "00";
									}
								}
							}
							rec.r(coln + fieldoffset, cell);
						};//coln;

						key = importcode ^ "*" ^ linenox;
						if (importfilenamefn) {
							rec.r(importfilenamefn, uploadpath);
						}
						if (importcodefn) {
							rec.r(importcodefn, importcode);
						}
						if (linenofn) {
							rec.r(linenofn, linenox);
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
					}

				}
			}

		}//loop;

		gosub cleanup();

	} else {
		msg = mode.quote() ^ " invalid mode in upload.subs";
		return invalid(msg);
	}

	return 0;
}

subroutine cleanup() {
	osfile.osclose();
	temposfilename83.osdelete();
	ANS = nimported;
	return;
}

subroutine getline() {

	if (buff.length() lt lengthx) {

addbuff:
////////
		call osbread(temp, osfile,  fileptr, lengthx);
		temp.converter("\n\f", "\r\r");
		buff ^= temp;

		//get more if no line ending
		if (temp.length() and not(buff.index("\r"))) {
			goto addbuff;
		}

		if (not(buff.length())) {
			eof = 1;
			line = "";
			return;
		}

	}

	//skip empty lines
	while (true) {
		///BREAK;
		if (not(buff[1] eq "\r")) break;
		buff.splicer(1, 1, "");
	}//loop;
	if (not(buff.length())) {
		goto addbuff;
	}

	line = buff.field("\r", 1);
	buff.splicer(1, line.length() + 1, "");

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
		msg = "Cannot upload at the moment because";
		msg.r(-1, filename ^ " " ^ (key.quote()) ^ " is being updated by ");
		var lockuser = (filename ^ "*" ^ key).xlate("LOCKS", 4, "X");
		if (lockuser) {
			msg ^= lockuser;
		}else{
			msg ^= "someone else";
		}
	}

	return;
}

subroutine unlockfile() {
	call unlockrecord(filename, file, key);
	return;
}

libraryexit()
