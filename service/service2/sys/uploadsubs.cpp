#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <authorised.h>
#include <shell2.h>
#include <singular.h>

#include <gen.h>
#include <win.h>

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
var fileoffset;//num
var anycr;//num
var LINENO;//num
var eof;//num
var line;
var ptr;//num
var ncols;
var dictfile;
var xx;
var recordx;
var allowduplicate;
var wsmsg;

function main(in mode) {
	//c sys
	//doesnt seem to be any limit on number of files per folder
	//in ntfs filing system at least under win 2003 server
	//test wrote 1,000,000 files containing the filenumber 1,2,3 etc in about 5 mins
	//and the files could be randomly read and written at excellent speed

	//global ptr,ii

	//determine upload directory
	var uploadroot = SYSTEM.a(49);
	//if uploadroot='' then uploadroot='..\images\'
	//if uploadroot[-1,1] ne '\' then uploadroot:='\'
	var virtualroot = "../images/";

	if (mode == "POSTUPLOAD") {

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
		if (not dictid) break;;
			var fn = (dictid ^ "_ARCHIVED").xlate(dictfilename, 2, "X");
			if (not fn or not fn.isnum()) {
				msg = DQ ^ (dictid ^ "_ARCHIVED" ^ DQ) ^ " is missing from " ^ dictfilename ^ " in upload.subs";
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

	} else if (mode.field(".", 1) == "MAKEUPLOADPATH") {

		if (not(authorised("UPLOAD CREATE", msg, ""))) {
			return invalid(msg);
		}

		//additional option to check file and key is not locked
		filename = USER0.a(3);
		key = USER0.a(4);
		var ensurenotlocked = USER0.a(5);
		if (ensurenotlocked == "undefined") {
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

		var uploadpath = mode.field(".", 2, 9999);
		if (uploadpath == "") {
			return 0;
		}

		if (uploadpath.substr(3,99999).index("..", 1)) {
			msg = DQ ^ (uploadpath ^ DQ) ^ " ..  is not allowed";
			gosub unlockfile();
			return invalid(msg);
		}

		//cannot have * characters in keys represented in dos filenames
		//fixed in UI
		//convert '\/:*?<>|' to '--------' in uploadpath
		//convert '"' to "'" in uploadpath

		//if doesnt terminate with a \ then assume that the last bit is filename
		if (uploadpath[-1] ne "\\") {
			uploadpath = uploadpath.field("\\", 1, uploadpath.count("\\")) ^ "\\";
		}

		//ok if any files found
		//initdir uploadroot:uploadpath:'*.*'
		//if dirlist() then return
		if (oslistf(uploadroot ^ uploadpath ^ "*.*")) {
			return 0;
		}

		//otherwise make the folders in parent order
		var nsubfolders = uploadpath.count("\\");
		for (var subfoldern = 1; subfoldern <= nsubfolders; ++subfoldern) {
			var subfolder = uploadpath.field("\\", 1, subfoldern);
			//call subdirs(uploadroot:subfolder:\00\,result)
			//if result else
			var fullsubfolder = uploadroot ^ subfolder;
			fullsubfolder.swapper("\\\\", "\\");
			//osopen fullsubfolder to xx else if status() ne 2 then
			//cannot use dos 8 characters since keys can be > 8 characters
			call shell2("dir " ^ fullsubfolder, errors);
			if (errors) {

				//call shell('md ':uploadroot:subfolder)
				//call mkdir(fullsubfolder:char(0),xx)
				//call mkdir2(fullsubfolder)
				//cannot use dos 8 characters since keys can be > 8 characters
				//actually mkdir2 calls md so it is the same thing
				call shell2("md " ^ fullsubfolder, errors);

				//check that folder was created
				//call subdirs(fullsubfolder:\00\,result)
				//if result else
				//osopen fullsubfolder to xx else if status() ne 2 then
				if (errors) {
					if (USERNAME == "NEOSYS") {
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

	} else if (mode.field(".", 1) == "VERIFYUPLOAD") {

		//similar in VERIFY AND DELETEUPLOAD
		var uploadpath = mode.field(".", 2, 9999);
		//fixed in ui now
		//convert '\/:*?<>|' to '--------' in uploadpath
		//convert '"' to "'" in uploadpath

		//initdir uploadroot:uploadpath
		//if dirlist() else
		var tt = (shell2("dir " ^ (DQ ^ (uploadroot ^ uploadpath ^ DQ)) ^ " /b", errors)).ucase();
		if ((tt == "") or tt.index("FILE NOT FOUND", 1)) {
			msg = "Error: Nothing uploaded in " ^ uploadroot ^ uploadpath;
			return invalid(msg);
		}

	} else if (mode.field(".", 1) == "OPENUPLOAD") {

		USER1 = virtualroot;
		//data<2>=filenames

		var virtualfilebase = mode.field(".", 2, 9999);
		//fixed in UI now
		//convert '\/:*?<>|' to '--------' in virtualfilebase
		//convert '"' to "'" in virtualfilebase

		if (virtualfilebase.index("\\", 1)) {
			//uploadpath=field(virtualfilebase,'\',1,count(virtualfilebase,'\'))
		}else{
			//uploadpath='*'
			//virtualfilebase='*'
		}

		//search for extensions
		//may be only one extension now
		var dirpatt = uploadroot ^ virtualfilebase;
		if (dirpatt[-1] ne ".") {
			dirpatt ^= ".";
		}
		dirpatt ^= "*";

		if (dirpatt.substr(3,99999).index("..", 1)) {
			msg = DQ ^ (dirpatt ^ DQ) ^ " .. is not allowed";
			return invalid(msg);
		}

		//why does % come encoded as %25?
		dirpatt.swapper("%25", "%");

		if (USER0.a(3) == "NEW") {
			var cmd = "DEL " ^ (DQ ^ (dirpatt ^ DQ));
			osshell(cmd);
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

			var ndeep = virtualfilebase.count("\\") + 1;
			for (var uploadn = 1; uploadn <= nuploads; ++uploadn) {
				var uploadfilename = uploadfilenames.a(uploadn);
				uploadfilename = virtualfilebase.fieldstore("\\", ndeep, 1, uploadfilename);
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
		// materialid=virtualfilebase
		// read material from materials,materialid then
		// if material<16> ne uploadfilenames then
		// material<16>=uploadfilenames
		// *write without lock - any user updating it save will be blocked - but unlikely
		// write material on materials,materialid
		// end
		// end
		// end
		// end

	} else if (mode.field(".", 1) == "DELETEUPLOAD") {

		if (not(authorised("UPLOAD DELETE", msg, ""))) {
			return invalid(msg);
		}

		//similar in VERIFY AND DELETEUPLOAD
		var uploadpath = mode.field(".", 2, 9999);
		//fixed in UI now
		//convert '\/:*?<>|' to '--------' in uploadpath
		//convert '"' to "'" in uploadpath

		//initdir uploadroot:uploadpath
		//if dirlist() else
		var tt = (shell2("dir " ^ (DQ ^ (uploadroot ^ uploadpath ^ DQ)) ^ " /b", errors)).ucase();
		if ((tt == "") or tt.index("FILE NOT FOUND", 1)) {
			msg = "Error: Nothing uploaded in " ^ uploadroot ^ uploadpath;
			return invalid(msg);
		}

		tt = uploadroot ^ uploadpath;
		if (tt.substr(3,99999).index("..", 1)) {
			msg = DQ ^ (tt ^ DQ) ^ " .. is not allowed";
			return invalid(msg);
		}

		var cmd = "DEL " ^ (DQ ^ (tt ^ DQ));
		osshell(cmd);

		if (oslistf(uploadroot ^ uploadpath)) {
			msg = DQ ^ (uploadroot ^ uploadpath ^ " file cannot be deleted" ^ DQ);
			return invalid(msg);
		}

	} else if (mode == "IMPORT") {
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

		if (uploadpath.substr(3,99999).index("..", 1)) {
			msg = DQ ^ (uploadpath ^ DQ) ^ " .. is not allowed";
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
			importcode = (field2(uploadpath, "\\", -1)).ucase();
		}
		if (importcode.index(".", 1)) {
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

		msg = DQ ^ (uploadroot ^ uploadpath ^ DQ) ^ "\r\n";
		msg = "Uploaded file cannot be found/copied" "\r\n";
		msg ^= DQ ^ (temposfilename83 ^ DQ) ^ " file cannot be opened" ")";

		nimported = "";

		//from here on gosub cleanup to clean up temporary file

		var cmd = "COPY " ^ (DQ ^ (uploadroot ^ uploadpath ^ DQ)) ^ " " ^ temposfilename83;
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

		fileoffset = 0;
		anycr = 0;
		LINENO = 0;
		//cols os "name vm start vm len" etc one col per fm
		var cols = "";
		nimported = 0;

		while (true) {
			gosub getline();
		///BREAK;
		if (not(not eof)) break;;
			if (line and (LINENO >= startatrown)) {

				//determine cols from first col heading
				if (not cols) {
					var offset = 1;
					while (true) {
						var tt = line.index("  ", 1);
					///BREAK;
					if (not tt) break;;
						cols.r(-1, line.substr(1,tt - 1) ^ VM ^ offset);
						for (ptr = tt; ptr <= 999999; ++ptr) {
						///BREAK;
						if (line[ptr + 1] ne " ") break;;
						};//ptr;
						cols ^= VM ^ ptr;
						offset += ptr;
						line.splicer(1, ptr, "");
					}//loop;
					ncols = cols.count(FM) + 1;
	//oswrite cols on 'cols'

				}else{

					//first record, open files and create dictionary
					if (file == "") {

						if (not(file.open(filename, ""))) {
							call fsmsg();
							win.valid = 0;
							gosub cleanup();
							return 0;
						}
						var dictfile;
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
								if (datewords.locateusing(word, VM, xx)) {
									CONV = "[DATE,4*]";
									just = "R";
								}else{
									if (timewords.locateusing(word, VM, xx)) {
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
									if (CONV.index("TIME", 1)) {
										//if no : in time then assume is already seconds
										if (cell.index(":", 1)) {
											cell = cell.iconv(CONV);
										}
									}else{
										cell = cell.iconv(CONV);
									}
									if (not(cell.length())) {
										call mssg(DQ ^ (cell0 ^ DQ) ^ " cannot be converted in line " ^ LINENO ^ " col " ^ coln);
										//indicate strange but leave workable date/time
										cell = "00";
									}
								}
							}
							rec.r(coln + fieldoffset, cell);
						};//coln;

						key = importcode ^ "*" ^ LINENO;
						if (importfilenamefn) {
							rec.r(importfilenamefn, uploadpath);
						}
						if (importcodefn) {
							rec.r(importcodefn, importcode);
						}
						if (linenofn) {
							rec.r(linenofn, LINENO);
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
		msg = DQ ^ (mode ^ DQ) ^ " invalid mode in upload.subs";
		return invalid(msg);
	}
//L3020:
	return 0;

}

subroutine cleanup() {
	osfile.osclose();
	temposfilename83.osdelete();
	ANS = nimported;
	return;

}

subroutine getline() {
	call osbread(line, osfile,  fileoffset, lengthx);

	if (not(line.length())) {
		eof = 1;
		return;
	}

	//ignore lf (should only be after cr)
	if (anycr) {
		if (line[1] == "\n") {
			line.splicer(1, 1, "");
			fileoffset += 1;
		}
	}

	var tt = line.index("\r", 1);
	if (tt) {
		anycr = 1;
	}else{
		tt = line.index("\n", 1);
		//assume if no cr/lf then we are at the end of the file
		//TODO should really increase the lengthx
		if (not tt) {
			tt = line.length();
		}
	}

	fileoffset += tt;
	line.splicer(tt, 999999, "");

	LINENO += 1;
	eof = 0;
	return;

}

subroutine lockfile() {
	msg = "";
	if (not(file.open(filename, ""))) {
		msg = DQ ^ (filename ^ DQ) ^ " file cannot be opened";
		return;
	}

	var waitsecs = 3;
	if (not(lockrecord(filename, file, key, recordx, waitsecs, allowduplicate))) {
		gosub unlockfile();
		msg = "Cannot upload at the moment because";
		msg.r(-1, filename ^ " " ^ (DQ ^ (key ^ DQ)) ^ " is being updated by ");
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
