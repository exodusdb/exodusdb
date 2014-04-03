#include <exodus/library.h>
libraryinit()

#include <giveway.h>
#include <authorised.h>
#include <singular.h>
#include <materialsubs.h>

#include <gen.h>
#include <agy.h>
#include <win.h>

#include <window.hpp>//after win

var msg;
var imagelocation;
var materialno;
var brandcode;
var imagefilen;//num
var matn;//num
var xx;
var file;
var filename;
var docname;
var chainn;
var matbrandcode;
var doc;
var reply;//num
var wsmsg;

function main(in mode) {
	//jbase
	var interactive = not SYSTEM.a(33);

	win.valid = 1;

	if (mode == "DELETEMATERIAL") {

		if (not(authorised("MATERIAL DELETE", msg, ""))) {
			return invalid(msg);
		}
		gosub getimagelocation();

		var materialcode = PSEUDO.a(1);

		if (not(oslist(imagelocation ^ materialcode))) {
			msg = DQ ^ (imagelocation ^ materialcode ^ " file cannot be found" ^ DQ);
			return invalid(msg);
		}

		var cmd = "DEL " ^ (DQ ^ (imagelocation ^ materialcode ^ DQ));
		osshell(cmd);

		if (oslist(imagelocation ^ materialcode)) {
			msg = DQ ^ (imagelocation ^ materialcode ^ " file cannot be deleted" ^ DQ);
			return invalid(msg);
		}

	} else if (mode.field("-", 1) == "VIEWMATERIAL") {

		gosub getimagelocation();

		materialno = mode.field("-", 2);
		if (not materialno) {
			materialno = ID;
		}
		if (not materialno) {
			msg = "Choose a material first";
			return invalid(msg);
		}

		brandcode = mode.field("-", 3);

		//get first material in chain
		call materialsubs("GETMATERIALCHAIN-" ^ materialno ^ "-" ^ brandcode);
		materialno = field2(ANS.a(1), VM, -1);
		ANS = "";
		if (not win.valid) {
			return 0;
		}

		//search for extensions
		var imagefilename = imagelocation ^ materialno;
		var imagefilenames = oslist(imagefilename ^ ".*");

		//check one or more files exist
		if (not imagefilenames) {
			if (interactive) {
				msg = "The image for material " ^ materialno ^ " is not available.";
				msg.r(-1, "|(Searched for " ^ (DQ ^ (imagefilename ^ ".jpg .gif .mpg etc." ^ DQ)) ^ ")|");
			}else{
				msg = "IMAGE_NOT_FOUND " ^ materialno;
			}
			return invalid(msg);
		}

		//select extensions if many .jpg .gif .mpg etc.
		//and build the imagefile
		var nimages = imagefilenames.count(FM) + 1;
		if (nimages > 1 and interactive) {
			if (interactive) {
				if (not(decide("", imagefilenames ^ "", imagefilen))) {
					return 0;
				}
			}
		}else{
			imagefilen = 1;
		}

		if (interactive) {
			imagefilename = imagelocation ^ imagefilenames.a(imagefilen);
			perform("OPEN " ^ imagefilename);
		}else{
			for (var imagen = 1; imagen <= nimages; ++imagen) {
				imagefilenames.r(imagen, "..\\images\\" ^ imagefilenames.a(imagen));
			};//imagen;
			USER1 = imagefilenames;
			USER1.converter(FM, "\r");
		}

	} else if (mode == "GETSCHEDULEMATERIALS") {
		var schid = PSEUDO.a(1);
		var schedule;
		if (not(schedule.read(agy.schedules, schid))) {
			msg = DQ ^ (schid ^ DQ) ^ " Schedule does not exist";
			return invalid(msg);
		}
		var materialdata = schedule.a(184);
		materialdata.r(2, schedule.a(154));
		materialdata.r(3, schedule.a(155));
		materialdata.r(4, schedule.a(11));
		materialdata.r(5, schedule.a(197));
		materialdata.r(6, schedule.a(198));
		materialdata.r(7, schedule.a(199));

		//get any new material codes from the material file
		var matletters = materialdata.a(1);
		var nmats = matletters.count(VM) + 1;
		for (matn = 1; matn <= nmats; ++matn) {

			var matid = materialdata.a(2, matn);
			if (not matid) {
				matid = schid ^ "." ^ matletters.a(1, matn);
			}
			var material;
			if (material.read(agy.materials, matid)) {
				if (material.a(1)) {
					materialdata.r(7, matn, material.a(1));
				}
			}

		};//matn;

		//add any materials in date grid not in the list
		var dategrid = schedule.a(22);
		//smdates
		dategrid.converter(" " _SM_ _VM_ "1234567890", "");

		for (var ii = 1; ii <= dategrid.length(); ++ii) {
			var mdn;
			var materialletter=dategrid[ii];
			if (not(materialdata.locate(materialletter, mdn, 1))) {
				materialdata.r(1, -1, materialletter);
			}
		};//ii;

		ANS = materialdata;

	} else if (mode.field("-", 1) == "GETMATERIALCHAIN") {

		materialno = mode.field("-", 2);
		var origmaterialno = materialno;
		brandcode = mode.field("-", 3);

		var materialnos = "";
		var schids = "";
		var materialdescs = "";
		var materialdesc = "";

		if (win.templatex == "PLANS") {

			var plans;
			if (not(plans.open("PLANS", ""))) {
				msg = "PLANS file cannot be opened";
				return invalid(msg);
			}

			file = plans;
			filename = "PLANS";
			docname = "plan";

		}else{
			file = agy.schedules;
			filename = "SCHEDULES";
			docname = "schedule";
		}

		while (true) {

			if (not interactive) {
				if (not giveway()) {
					{}
				}
			}

			var schid = materialno.field(".", 1);
			var letter = materialno.field(".", 2);

			//check not in self referencing chain of materials
			//and save the chain of materialnos
			if (schid) {
				if (materialnos.locateusing(materialno, VM, chainn)) {
					msg = "Material " ^ (DQ ^ (materialno ^ DQ)) ^ " refers back to|the same material on plan/schedule " ^ (DQ ^ (schids.a(1, chainn) ^ DQ));
					return invalid(msg);
				}else{

					if (letter == "") {

						//check the material file
						var material;
						if (not(material.read(agy.materials, schid))) {
							msg = DQ ^ (materialno ^ DQ) ^ " is not in the materials file";
							return invalid(msg);
						}

						//check the material brand
						matbrandcode = material.a(3);
						gosub confirmbrand();
						if (not win.valid) {
							return 0;
						}

						materialdesc = material.a(2);

					}

					materialnos.r(1, chainn, materialno);
					schids.r(1, chainn, schid);
					materialdescs.r(1, chainn, materialdesc);
				}

			}

		///BREAK;
		if (schid == "" or letter == "") break;;

			if ((win.templatex == "PLANS" or win.templatex == "SCHEDULES") and schid == ID) {
				doc = RECORD;
			}else{
				var doc;
				if (not(doc.read(file, schid))) {
					if (interactive) {
						msg = DQ ^ (schid ^ DQ) ^ " is not in the " ^ filename ^ " file.";
						return invalid(msg);
					}
					//allow upload image before schedule is saved
					doc = "";
				}
			}

			//locate the material in the prior schedule
			if (not(doc.locate(letter, matn, 184))) {

				if (interactive) {
					var lettersources = "X" ^ doc.a(184) ^ doc.a(22) ^ doc.field(FM, 171, 13);
					if (not(lettersources.index(letter, 1))) {
						msg = "Material " ^ (DQ ^ (letter ^ DQ)) ^ " is not used in " ^ docname ^ " " ^ (DQ ^ (schid ^ DQ));
						return invalid(msg);
					}
				}
			}

			//check brand agrees
			matbrandcode = doc.a(11);
			gosub confirmbrand();
			if (not win.valid) {
				return 0;
			}

			//get the prior material if any
			materialno = doc.a(154, matn);
			materialdesc = doc.a(155, matn);
			materialdescs.r(1, chainn, materialdesc);

		}//loop;

		ANS = materialnos ^ FM ^ schids ^ FM ^ materialdescs;

	} else if (mode == "PREDELETE") {
		gosub security(mode);

	} else if (mode == "POSTDELETE" or mode == "POSTWRITE") {
		//call flushindex("MATERIALS");

	} else if (mode == "POSTINIT") {
		gosub security(mode);

	} else if (mode == "POSTREAD") {
		gosub security(mode);

	} else if (1) {
		msg = DQ ^ (mode ^ DQ) ^ "INVALID MODE IN MATERIAL.SUBS";
	}

	return 0;

}

subroutine confirmbrand() {
	/*
	if (brandcode == "" or matbrandcode == brandcode) {
		return;
	}
	if (not(decide2("WARNING:|" ^ capitalise(docname) ^ " " ^ (DQ ^ (ID ^ DQ)) ^ " is for brand " ^ (DQ ^ (brandcode ^ DQ)) ^ "|but material " ^ (DQ ^ (materialno ^ DQ)) ^ " was for brand " ^ (DQ ^ (matbrandcode ^ DQ)), "OK" _VM_ "Cancel", reply, 2))) {
		reply = 0;
	}
	if (reply ne 1) {
		win.valid = 0;
	}
	*/
	win.valid = 0;
	return;

}

subroutine getimagelocation() {

	//determine image directory
	imagelocation = agy.agp.a(54);
	if (imagelocation == "") {
		imagelocation = "..\\IMAGES\\";
	}
	if (imagelocation[-1] ne "\\") {
		imagelocation ^= "\\";
	}
	if (imagelocation.substr(1, 2) == "..") {
		var parentdir = var().osdir();
		parentdir.splicer(-1, 1, "");
		//t=field2(parentdir,'\',-1)
		//parentdir[-len(t)-1,len(t)+1]=''
		parentdir = parentdir.field("\\", 1, parentdir.count("\\"));
		imagelocation.splicer(1, 2, parentdir);
	}

	//check image directory exists
	if (not var().oslistf(imagelocation, "*.*")) {
		msg = "Before you can view material images|the images must be scanned and|saved in the directory " ^ imagelocation ^ "|as files like .jpg .gif .mpg etc.";
		call mssg(msg);

		//make the directory
		var temp = imagelocation;
		temp.splicer(-1, 1, "");
		//suspend 'md ':temp
		//call mkdir(temp:char(0),xx)
		temp.osmkdir();

		return;

	}

	return;

}


libraryexit()
