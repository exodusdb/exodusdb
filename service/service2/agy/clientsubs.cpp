#include <exodus/library.h>
libraryinit()

#include <xrefsubs.h>
#include <trim2.h>
#include <generalsubs.h>
#include <validcode2.h>
#include <validcode3.h>
#include <authorised.h>
#include <locking.h>
#include <generalsubs2.h>
#include <quote2.h>
#include <safeselect.h>
#include <flushindex.h>
#include <createjob.h>
#include <btreeextract.h>
#include <singular.h>

#include <gen.h>
#include <agy.h>
#include <win.h>

#include <window.hpp>

var locklist;
var clientcode;
var xx;
var vn;
var msg;
var reply;
var oldbrandvn;
var brandn2;
var v69;
var v70;
var v71;
var newbrandcodes;
var temp;
var brandcodex;
var adid;
var wsmsg;

function main(in mode) {
	//c agy
	//jbase

	//to create all brands, jobs, accountindex for clients
	//WINDOWSTUB GENERAL.SUBS CALLSUBS,CLIENTS,CLIENT.SUBS,POSTWRITE

	//see brand multivalues in postwrite and val.brand
	var brandfns = "2" _VM_ "3" _VM_ "19" _VM_ "31" _VM_ "33" _VM_ "65" _VM_ "77" _VM_ "78";
	var nbrandfns = brandfns.count(VM) + 1;

	if (mode == "PERP") {

		if (win.curramvgroup == 1) {

			if (win.amvaction == 2 or win.amvaction == 3) {
				win.valid = 0;
				win.amvaction = 0;
				return 0;
			}

		}

	} else if (mode == "BUILDCLIENTGROUPMEMBERS") {

		win.datafile = "CLIENTS";
		win.srcfile = agy.clients;
		//group1
		call xrefsubs("BUILD", 16, win.datafile, win.srcfile, 29, locklist);
		//group2
		call xrefsubs("BUILD", 55, win.datafile, win.srcfile, 56, locklist);

	} else if (mode == "FIXCLIENT29IN19") {
		agy.clients.select();
nextclient:
		if (readnext(clientcode)) {
			//locked group clients will be skipped but unlikely to be esp. at startup
			if (lockrecord("CLIENTS", agy.clients, clientcode, xx, 9)) {
				var client;
				if (client.read(agy.clients, clientcode)) {
					var origclient = client;
					var brandaccs = client.a(19);
					if (brandaccs) {
						var nbrandaccs = brandaccs.count(VM) + 1;

						for (var brandaccn = 1; brandaccn <= nbrandaccs; ++brandaccn) {
							var brandacc = brandaccs.a(1, brandaccn);
							//proper account numbers have sm character in them
							if (not(brandacc.index(SVM, 1))) {
								client.r(19, brandaccn, "");
								if (not(client.a(29).locateby(brandacc, "AL", vn))) {
									client.inserter(29, vn, brandacc);
								}
							}
						};//brandaccn;

						client.r(19, trim2(client.a(19), VM, "B"));
						client.r(29, trim2(client.a(29), VM));

						if (client ne origclient) {
							//print ' ':clientcode:
							client.write(agy.clients, clientcode);
						}

					}
				}
				call unlockrecord("CLIENTS", agy.clients, clientcode);
			}
			goto nextclient;
		}

	//called from agencyproxy for VAL.BRAND.NEW (entering new documents)
	} else if (mode == "VAL.BRAND") {

		if (win.is.length() and not win.is) {
			if (win.isorig.length() == 0) {
				msg = DQ ^ (win.is ^ DQ) ^ " ZERO BRAND CODE IS NOT ALLOWED";
				return invalid(msg);
			}
		}

		//0 is equal to 00 and 000 !
		if (win.is == win.isorig and win.is.length() == win.isorig.length()) {
			return 0;
		}

		call generalsubs("VAL.ALPHANUMERIC");

		if (win.isorig.length()) {

			var brandcode = win.isorig;
			gosub chkbrandused( brandcodex);
			if (not win.valid) {
				return 0;
			}

		}
		//end else

		//check not belong to another client
		var brand;
		if (brand.read(agy.brands, win.is)) {
			if (brand.a(1) ne ID) {

				var qq = "Brand code " ^ (DQ ^ (win.is ^ DQ)) ^ " is already used";
				qq ^= " for||" ^ brand.a(2) ^ "|" ^ brand.a(3);

				//prevent moving unauthorised brands
				if (not(validcode2(brand.a(10), brand.a(1, 1), win.is, agy.brands, msg))) {
					msg = qq ^ FM ^ FM ^ msg;
					return invalid(msg);
				}
				//also in brands dict authorised
				if (not(validcode3(brand.a(13, 1), "", "", agy.vehicles, msg))) {
					msg = qq ^ FM ^ FM ^ msg;
					return invalid(msg);
				}

				//q:=' for||':brand<2>:'|':brand<3>

				//invalid if user not authorised to move brands
				if (not(authorised("BRAND CHANGE CLIENT", msg))) {
					msg = qq ^ FM ^ FM ^ msg;
					win.is = win.isorig;
					return invalid(msg);
				}

				var otherclient;
				if (otherclient.read(agy.clients, brand.a(1))) {
					if (otherclient.a(35)) {
						if (not authorised("CLIENT CREATE STOPPED", msg)) {
							msg = "";
							//if otherclient<35> then
							qq.r(-1, FM ^ otherclient.a(1) ^ " is currently \"stopped\" so you cannot move its brands");
							qq.r(-1, "You could unstop the client if appropriate.");
							qq.r(-1, DQ ^ (otherclient.a(35) ^ DQ));
							msg = qq ^ FM ^ FM ^ msg;
							return invalid(msg);
						}
					}
				}

				qq ^= "||Do you wish to MOVE that brand to this client ?";
				qq ^= "|GENERALLY IT IS NOT A GOOD IDEA TO DO THIS!";
				qq ^= "||Warning! It may take some time to save this client";
				qq ^= " if there are very many historical ads for this brand";
				//indicate OK but pass conflicting brand for user to decide
				if (not(not SYSTEM.a(33))) {
					USER3 = "OK " ^ qq;
					//valid=0
					//return the copied brand in data
					USER1 = brand;
					return 0;
				}

				//invalid if user choosed not to move the brand
				if (not(decide(qq, "Yes" _VM_ "No", reply))) {
					reply = "";
				}
				if (reply ne 1) {
					win.valid = 0;
					win.is = win.isorig;
					return 0;
				}

				//copy brand name, product type over
				//but not account number or executive because they could be defaulted from the client
				//and also should be reviewed perhaps
				RECORD.r(3, win.mvx, brand.a(2));
				RECORD.r(33, win.mvx, brand.a(12));

			}
		}

		//move brand within client
		if (RECORD.a(2).locateusing(win.is, VM, oldbrandvn)) {
			//@record<2,mv>=is
			//@record<3,mv>=@record<3,oldbrandvn>
			//@record<19,mv>=@record<19,oldbrandvn>
			//@record=delete(@record,2,oldbrandvn,0)
			//@record=delete(@record,3,oldbrandvn,0)
			//@record=delete(@record,19,oldbrandvn,0)
			for (var fnn = 1; fnn <= nbrandfns; ++fnn) {
				var fn = brandfns.a(1, fnn);
				RECORD.r(fn, win.mvx, RECORD.a(fn, oldbrandvn));
				RECORD.eraser(fn, oldbrandvn);
			};//fnn;

			win.valid = 0;
			win.displayaction = 5;
			win.reset = 4;
			return 0;
		}

		//end

		var tt = win.is;
		tt.converter(LOWERCASE ^ UPPERCASE ^ "0123456789", "");
		if (tt ne "") {
			msg = "PLEASE USE ONLY LETTERS AND NUMBERS FOR BRAND CODES";
			return invalid(msg);
		}
		if (win.is.length() > 5 or win.is.index(" ", 1)) {
			msg = "THE BRAND CODE MAY BE UP TO|5 CHARACTERS WITH NO SPACES";
			return invalid(msg);
		}

	} else if (mode == "VAL.NAME") {
		if (win.winext > win.wi and win.is == "") {
			msg = "NAME IS REQUIRED";
			return invalid(msg);
		}

	} else if (mode == "PREWRITE") {

		gosub security(mode);
		if (not win.valid) {
			return 0;
		}

		//check no deleted brands are in use
		gosub checkdeletedbrands( mode);
		if (not win.valid) {
			return 0;
		}

		//lock old and new group clients
		//group1
		call xrefsubs("PREWRITE", 16, "CLIENTS", win.srcfile, 29, locklist);
		if (not win.valid) {
			return 0;
		}
		//group2
		call xrefsubs("PREWRITE", 55, "CLIENTS", win.srcfile, 56, locklist);
		if (not win.valid) {
			return 0;
		}

		//remove any brands from other clients

		var brandcodes = RECORD.a(2);
		var nbrands = brandcodes.count(VM) + (brandcodes ne "");

		//lock all other clients
		var otherclientcodes = "";
		var otherbrandcodes = "";
		for (var brandn = 1; brandn <= nbrands; ++brandn) {
			var otherbrandcode = brandcodes.a(1, brandn);
			var brand;
			if (brand.read(agy.brands, otherbrandcode)) {
				var otherclientcode = brand.a(1, 1);
				if (otherclientcode ne ID) {

					if (not(locking("LOCK", "CLIENTS", otherclientcode, "", locklist, 3, msg))) {
						gosub unlockall();
						msg = DQ ^ (otherclientcode ^ DQ) ^ " client is in use elsewhere|Brand cannot be transferred right now||" ^ msg;
						return invalid(msg);
					}

					otherclientcodes.r(1, -1, otherclientcode);
					otherbrandcodes.r(1, -1, otherbrandcode);
				}
			}
		};//brandn;

		//remove the brand from other clients
		for (var clientn = 1; clientn <= 9999; ++clientn) {
			clientcode = otherclientcodes.a(1, clientn);
		///BREAK;
		if (not clientcode) break;;
			var brandcode = otherbrandcodes.a(1, clientn);
			var client;
			if (client.read(agy.clients, clientcode)) {
				if (client.a(2).locateusing(brandcode, VM, brandn2)) {
					for (var fnn = 1; fnn <= nbrandfns; ++fnn) {
						var fn = brandfns.a(1, fnn);
						client.eraser(fn, brandn2);
					};//fnn;
					client.write(agy.clients, clientcode);
				}
			}
		};//clientn;

		//update old and new group clients
		//group1
		call xrefsubs("POSTWRITE", 16, "CLIENTS", win.srcfile, 29, locklist);
		//group2
		call xrefsubs("POSTWRITE", 55, "CLIENTS", win.srcfile, 56, locklist);

		//unlock all other clients
		gosub unlockall();

		//update version log
		call generalsubs2(mode);

		//move ads to new client (if fails then can run CREATEADS)
		//should really be done in postwrite in case fails
		if (otherbrandcodes) {
			call pushselect(0, v69, v70, v71);
			otherbrandcodes.swapper(VM, "*" ^ VM);
			otherbrandcodes ^= "*";
			call safeselect("SELECT ADS WITH BRAND_AND_DATE STARTING " ^ quote2(otherbrandcodes) ^ " (S)");
			if (LISTACTIVE) {
				//client code, client group code, client group2 code
				//TODO this assumes the common case of client in its own group
				//BUT should really set GROUP_CODE and GROUP2_CODE separately in case not
				perform("CLEARFIELD ADS CLIENT_CODE CLIENT_GROUP_CODE2 CLIENT_GROUP2_CODE2/" ^ ID);
				call flushindex("ADS");
			}
			call popselect(0, v69, v70, v71);
		}

	} else if (mode == "POSTWRITE") {

		//write brand name to brands file
		//sjb 99/3/10 and brand levels
		var clientname = RECORD.a(1);
		var oldclientname = win.orec.a(1);
		var brandcodes = RECORD.a(2);
		var nbrands = brandcodes.count(VM) + (brandcodes ne "");
		var brandlevels = "";
		var client = RECORD;
		//clientcompanycode=field(client<14,1,1>,',',2)
		var clientcompanycode = calculate("COMPANY_CODE");
		var clientexecutivecode = client.a(30);
		var clientcontact = client.a(64);
		//clientmarketcode=client<32,1>
		var clientmarketcode = client.a(32);
		var clientgroupcode = client.a(16);
		if (not clientgroupcode) {
			clientgroupcode = ID;
		}
		var clientgroup2code = client.a(55);
		if (not clientgroup2code) {
			clientgroup2code = ID;
		}

		for (var brandn = 1; brandn <= nbrands; ++brandn) {

			var brandcompanycode0 = client.a(19, brandn, 1).field(",", 2);

			var brandcompanycode = brandcompanycode0;
			if (brandcompanycode == "") {
				brandcompanycode = clientcompanycode;
			}

			var brandexecutivecode = client.a(31, brandn);
			if (brandexecutivecode == "") {
				brandexecutivecode = clientexecutivecode;
			}

			var brandcontact = client.a(65, brandn);
			if (brandcontact.a(1, 1, 1) == "") {
				brandcontact.r(1, 1, 1, clientcontact.a(1, 1, 1));
			}
			if (brandcontact.a(1, 1, 2) == "") {
				var tt = clientcontact.a(1, 1, 2);
				if (tt) {
					brandcontact.r(1, 1, 2, tt);
				}
			}

			var productcategorycode = client.a(33, brandn);

			var brandcode = brandcodes.a(1, brandn);
			if (brandcode ne "") {
				var brandname = RECORD.a(3, brandn);
				//build the brand code heirarchy
				var leveln = brandname.length() - (brandname.trimf()).length() + 1;
				//cut off any lower levels
				if (leveln > 1) {
					brandlevels = brandlevels.field(VM, 1, leveln - 1);
				}else{
					brandlevels = "";
				}
				//fill in the lower levels at least to level 3
				for (var ii = 1; ii <= 3; ++ii) {
					if (brandlevels.a(1, ii) == "") {
						brandlevels.r(1, ii, brandcode);
					}
				};//ii;

				var oldbrand;
				if (not(oldbrand.read(agy.brands, brandcode))) {
					oldbrand = "";
				}
				var brand = oldbrand;

				//done in prewrite now to lock all other clients first
				//remove brand from other client if present there already
				//read brand from brands,brandcode then
				//otherclientcodes=oldbrand<1>
				//locate @id in otherclientcodes<1> setting otherclientn then
				// otherclientcodes=delete(otherclientcodes,1,otherclientn,0)
				// end
				//if otherclientcodes<>'' and otherclientcodes ne @id then
				// read otherclient from clients,otherclientcodes then
				// locate brandcode in otherclient<2> setting vn then
				// otherclient=delete(otherclient,2,vn,0)
				// otherclient=delete(otherclient,3,vn,0)
				// otherclient=delete(otherclient,19,vn,0)
				// write otherclient on clients,otherclientcodes
				// end
				// end
				// end

				brand.r(1, ID);
				brand.r(2, brandname.trimf());
				brand.r(3, clientname);
				brand.r(8, brandlevels);
				brand.r(10, brandcompanycode);
				brand.r(11, brandexecutivecode);
				brand.r(12, productcategorycode);
				brand.r(13, clientmarketcode);
				brand.r(14, clientgroupcode);
				brand.r(15, clientgroup2code);
				brand.r(16, brandcontact);

				//brand<17>=trim(convert(',',vm,client<77,brandn>))
				//brand<18>=trim(convert(',',vm,client<78,brandn>))

				var tt = client.a(77, brandn);
				tt.converter(",", VM);
				brand.r(17, tt.trim());

				tt = client.a(78, brandn);
				tt.converter(",", VM);
				brand.r(18, tt.trim());

				//WRITEV @ID ON BRANDS,brandcode,1
				//WRITEV trimf(BRAND.NAME) ON BRANDS,brandcode,2
				//writev client.name on brands,brandcode,3

				if (brand ne oldbrand) {
					brand.write(agy.brands, brandcode);
				}

				//create jobs for timesheets
				if (brandcompanycode0) {

					//jobno=brandcode:'-':brandcompanycode0
					var jobno = brandcompanycode0.xlate("COMPANIES", 28, "X");
					if (not jobno) {
						jobno = brandcompanycode0;
					}
					jobno ^= "-" ^ brandcode;

					var job;
					if (not(job.read(agy.jobs, jobno))) {
						//TODO enable update of "brand" jobs with new names etc
						//if 1 then

						job = "";
						//job<1>=field(date() 'D2/E','/',2)+0:'/':field(date() 'D2/E','/',3)
						tt = (var().date()).oconv("D2/E").field("/", 2) + 0;
						tt ^= "/";
						tt ^= (var().date()).oconv("D2/E").field("/", 3);
						job.r(1, tt);
						job.r(2, brandcode);
						if ((brand.a(2).ucase()).index(brand.a(3).ucase(), 1) or (brand.a(3).ucase()).index(brand.a(2).ucase(), 1)) {
							job.r(9, brand.a(2));
						}else{
							job.r(9, brand.a(3) ^ ", " ^ brand.a(2));
						}
						job.r(12, clientmarketcode.a(1, 1));
						job.r(14, brandcompanycode0);
						job.r(28, brandcompanycode0);

							/*;
							//in unlikely case that someone is creating "timesheet client job" then
							//this will fail here resulting in missing "timesheet client job"
							//however cannot hang or cancel transaction in this POSTWRITE code
							if lockrecord('JOBS',jobs,jobno,recordx,9) then;

								//in general.subs2 and client.subs,postwrite
								version.fn=29;
								status.fn=7;
								firstversion='A';
								defaultstatus='0';

								job<status.fn>=defaultstatus;
								job<version.fn+1>=@username;
								job<version.fn+2>=date():'.':time() 'R(0)#5';
								job<version.fn+3>=trim(@station);
								job<version.fn+4>=firstversion;
								job<version.fn+5>=defaultstatus;

								write job on jobs,jobno;

								call unlockrecord('JOBS',jobs,jobno);

								end;
							*/
						call createjob(job, jobno);

					}
				}

			}

		};//brandn;

		newbrandcodes = RECORD.a(2);
		gosub deletebrands();
		call flushindex("CLIENTS");
		call flushindex("BRANDS");

		//gosub getvalidbrands

		//very similar code in LISTCOLLECTIONS and CLIENT.SUBS,POSTWRITE
		var clientindexfile;
		if (clientindexfile.open("!CLIENTS", "")) {
			var acnos = RECORD.a(14) ^ VM ^ RECORD.a(19);
			var naccs = acnos.count(VM) + 1;
			for (var accn = 1; accn <= naccs; ++accn) {
				var acno = acnos.a(accn, 1, 2);
				if (acno) {
					var clientindex;
					if (not(clientindex.read(clientindexfile, "INVERTED*ACNO*" ^ acno))) {
						clientindex = "";
					}
					if (not(clientindex.a(1).locateusing(ID, VM, xx))) {
						clientindex.r(1, -1, ID);
						clientindex.write(clientindexfile, "INVERTED*ACNO*" ^ acno);
					}
				}
			};//accn;
		}

	} else if (mode == "PREDELETE") {

		gosub security(mode);
		if (not win.valid) {
			return 0;
		}

		RECORD = "";
		gosub checkdeletedbrands( mode);
		if (not win.valid) {
			return 0;
		}

		//lock and remove from group client
		//group1
		call xrefsubs("DELETE", 16, "CLIENTS", win.srcfile, 29, locklist);
		//group2
		call xrefsubs("DELETE", 55, "CLIENTS", win.srcfile, 56, locklist);

		gosub unlockall();

		//update version log
		call generalsubs2(mode);

	} else if (mode == "POSTDELETE") {
		newbrandcodes = "";
		gosub deletebrands();
		call flushindex("CLIENTS");
		call flushindex("BRANDS");

		//call client.subs('GETVALIDBRANDS')

	} else if (mode == "POSTINIT") {
		gosub security(mode);
		if (not win.valid) {
			return 0;
		}

	} else if (mode == "POSTREAD") {

		//option to read previous versions
		call generalsubs2(mode);
		if (not win.valid) {
			return 0;
		}

		msg = "";
		if (RECORD) {
			if (not(validcode2(calculate("COMPANY_CODE"), ID.field("~", 1), "", agy.brands, msg))) {
unlockclient:
				xx = unlockrecord(win.datafile, win.srcfile, ID);
				//msg='SORRY YOU ARE NOT AUTHORISED|TO ACCESS THIS CLIENT'
				gosub invalid(msg);
				win.reset = 5;
				return 0;
			}
			//xx=@record<30,1>
			if (not(validcode3(RECORD.a(32, 1), "", "", agy.brands, msg))) {
				goto unlockclient;
			}
		}

		//optionally prevent update of stopped clients
		if (win.wlocked) {
			//tt=ucase(@record<1>:@record<35>)
			//if index(tt,'(STOP)',1) or index(tt,'<STOP>',1) then
			if (RECORD.a(35) or (RECORD.a(1)).index("(STOP)", 1) or (RECORD.a(1)).index("<STOP>", 1)) {
				//CLIENT CREATE STOPPED prevents people from create new documents
				//schedule/orders etc. for to the stopped clients
				//if security('CLIENT ACCESS STOPPED',msg,'CLIENT CREATE STOPPED') else
				if (not(authorised("CLIENT UPDATE TERMS", msg))) {
					win.wlocked = 0;
					//explain to the user why cannot update
					win.reset = -1;
					call mssg("This client is stopped and you are not authorised to update it" ^ FM ^ FM ^ msg);
				}
			}
		}

		//now lock is on client completely
		//if validcode('BRAND %ANY%',@record<2>,msg) else goto unlockclient

		gosub security(mode);
		if (not win.valid) {
			return 0;
		}

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " - unknown mode skipped in CLIENT.SUBS";
		return invalid(msg);
	}
//L3434:
	return 0;

}

subroutine deletebrands() {
	var brandcodes = win.orec.a(2);
	var nbrands = brandcodes.count(VM) + (brandcodes ne "");
	for (var brandn = 1; brandn <= nbrands; ++brandn) {
		var brandcode = brandcodes.a(1, brandn);
		if (not(newbrandcodes.a(1).locateusing(brandcode, VM, temp))) {
			agy.brands.deleterecord(brandcode);
			
		}
	};//brandn;

	return;

}

subroutine checkdeletedbrands(in mode) {
	//checkdeletedbrands(in mode)

	//check no deleted brands are still in use
	var brandcodesx = win.orec.a(2);
	var nbrandsx = brandcodesx.count(VM) + (brandcodesx ne "");
	for (var brandnx = 1; brandnx <= nbrandsx; ++brandnx) {
		brandcodex = brandcodesx.a(1, brandnx);
		if (mode.index("DELETE", 1)) {
			gosub chkbrandused( brandcodex);
		}else{
			if (not(RECORD.a(2).locateusing(brandcodex, VM, xx))) {
				gosub chkbrandused( brandcodex);
			}
		}
	};//brandnx;

	return;

}

subroutine chkbrandused(in brandcodex) {
	//chkbrandused(in brandcodex)

	//always ok to delete new brands not yet saved
	var brand;
	if (not(brand.read(agy.brands, brandcodex))) {
		return;
	}

	//dont check new brand codes or codes moved from another client
	if (brand.a(1, 1) ne ID) {
		return;
	}

	//check brand not used in any schedules, plans or jobs
	if (not brandcodex) {
		return;
	}
	//check brand not used in any schedules
	var tt = "BRAND_CODE" ^ VM ^ brandcodex ^ FM;
	var hits = "";
	var dict;
	if (dict.open("dict_SCHEDULES")) {
		call btreeextract(tt, "SCHEDULES", dict, hits);
		if (hits) {
			msg = DQ ^ (brandcodex ^ DQ) ^ "  brand cannot be deleted or changed|because it is in use on the following schedules:";
			hits.converter(VM, " ");
			msg.r(-1, hits.field(" ", 1, 5));
			gosub invalid(msg);
			return;
		}
	}

	//check brand not used in any plans
	if (dict.open("dict_PLANS")) {
		call btreeextract(tt, "PLANS", dict, hits);
		if (hits) {
			msg = DQ ^ (brandcodex ^ DQ) ^ "  brand cannot be deleted or changed|because it is in use on the following plans:";
			hits.converter(VM, " ");
			msg.r(-1, hits.field(" ", 1, 5));
			gosub invalid(msg);
			return;
		}
	}

	//check brand not used in any ads
	//this could be case if brand code entered per materials on schedule
	if (agy.ads.open("ADS", "")) {
		call pushselect(0, v69, v70, v71);
		call safeselect("SELECT 10 ADS with BRAND_AND_DATE STARTING " ^ (DQ ^ (brandcodex ^ "*" ^ DQ)) ^ " (S)");
		if (not readnext(adid)) {
			adid = "";
		}
		hits = LISTACTIVE;
		call popselect(0, v69, v70, v71);
		if (adid) {
			msg = DQ ^ (brandcodex ^ DQ) ^ "  brand cannot be deleted or changed|because it is in use on at least the following schedule:";
			msg.r(-1, adid.xlate("ADS", 1, "C"));
			gosub invalid(msg);
			return;
		}
	}

	//check brand not used in any jobs
	if (dict.open("dict_JOBS")) {
		call btreeextract(tt, "JOBS", dict, hits);
		if (hits) {
			msg = DQ ^ (brandcodex ^ DQ) ^ "  brand cannot be deleted or changed|because it is in use on the following jobs:";
			hits.converter(VM, " ");
			msg.r(-1, hits.field(" ", 1, 5));
			gosub invalid(msg);
			return;
		}
	}

	return;

}

subroutine unlockall() {
	call locking("UNLOCKALL", "", "", "", locklist, 0, xx);
	return;

}


libraryexit()