#include <exodus/library.h>
libraryinit()

#include <otherusers.h>
#include <shutdownothers.h>

#include <sys_common.h>

var interactive;
var mode;
var dataset;
var voc;
var xx;
var otherusersx;
var nn;
var tt;
var options;
var t2;
var t3;
var reply;//num
var result;
var msg;
var datasetcode;

function main(in mode0, in dataset0) {
	//c sys
	//global all

	interactive = not(SYSTEM.a(33));
	if (mode0.unassigned()) {
		mode = "";
	} else {
		mode = mode0;
	}
	if (dataset0.unassigned()) {
		dataset = "";
	} else {
		dataset = dataset0;
	}
	if (mode eq "") {
		mode = "LOCK";
	}

	if (dataset eq "CURRENTDATASET") {
		dataset = SYSTEM.a(17);
		}

	if (not(voc.open("VOC", ""))) {
		call mssg("CANNOT OPEN VOC FILE FOR LOCKING");
		return 0;
	}

	//unlocking
	//////////

	if (mode eq "UNLOCK") {
		if (not dataset) {
			voc.unlock( "BACKUP*");
		}
		//why again?
		xx = unlockrecord("", voc, "BACKUP*" ^ dataset);
		goto success;
	}

	//locking
	////////

	//check no other users
checkotherusers:
	otherusersx = otherusers(dataset);
	if (otherusersx) {
		nn = otherusersx.a(1);
		if (not interactive) {
			return 0;
		}
		otherusersx = otherusersx.a(2);
		otherusersx.converter(VM, " ");

		//t=if dataset then ' of dataset ':dataset else ''
		if (dataset) {
			tt = " of dataset " ^ dataset;
		} else {
			tt = "";
		}

		//if decide('!There ':(if n=1 then 'is ' else 'are '):n:' other workstation':(if n>1 then 's' else ''):'|using EXODUS ':dataset:' at the moment.||All other users':t:' must exit EXODUS|before you can continue.||':otherusersx:'|','Retry|Cancel',reply) else reply=0
		options = "Retry|Cancel";
		if (dataset) {
			options ^= "|Attempt to close the " ^ dataset ^ " processes and retry";
		}
		//if decide('!There ':(if n=1 then 'is ' else 'are '):n:' other workstation':(if n>1 then 's' else ''):'|using EXODUS ':dataset:' at the moment.||All other users':t:' must exit EXODUS|before you can continue.||':otherusersx:'|',options,reply) else reply=0
		if (nn eq 1) {
			t2 = "is ";
		} else {
			t2 = "are ";
		}
		if (nn gt 1) {
			t3 = "s";
		} else {
			t3 = "";
		}
		if (not(decide("!There " ^ t2 ^ nn ^ " other workstation" ^ t3 ^ "|using EXODUS " ^ dataset ^ " at the moment.||All other users" ^ tt ^ " must exit EXODUS|before you can continue.||" ^ otherusersx ^ "|", options, reply))) {
			reply = 0;
		}
		if (reply eq 3) {
			call shutdownothers("", dataset, result);
			reply = 1;
		}
		if (reply ne 1) {
			return 0;
		}
		goto checkotherusers;
	}

lockit:
	msg = "YOU CANNOT USE THE SYSTEM AT THE MOMENT";
	msg.r(-1, "BECAUSE SOMEBODY ELSE IS PERFORMING A PROCESS");
	msg.r(-1, "THAT REQUIRES EXCLUSIVE USE OF THE SYSTEM");

	if (dataset) {

		//temporary exclusive lock
		if (not(lockrecord("", voc, "BACKUP*"))) {
			goto invalidx;
		}

		if (not(lockrecord("", voc, "BACKUP*" ^ dataset))) {

			xx = unlockrecord("", voc, "BACKUP*");

			//msg2=msg
			if (dataset) {
				msg.r(-1, dataset.quote() ^ "- SYSTEM IS LOCKED");
			}
invalidx:
			options = "Retry|Cancel";
			if (dataset) {
				options ^= "|Attempt to close the " ^ dataset ^ " processes and retry";
			}
			if (not(decide(msg, options, reply))) {
				reply = 0;
			}
			if (reply eq 3) {
				call shutdownothers("", datasetcode, result);
				reply = 1;
			}
			if (reply ne 1) {
				return 0;
			}
			goto lockit;
		}

		//release exclusive lock
		xx = unlockrecord("", voc, "BACKUP*");

	} else {

		if (not(lockrecord("", voc, "BACKUP*"))) {
			goto invalidx;
		}

	}

////////
success:
////////
	return 1;
}

libraryexit()
