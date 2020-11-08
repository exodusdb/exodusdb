#include <exodus/library.h>
libraryinit()

#include <generalsubs2.h>
#include <authorised.h>
#include <singular.h>
#include <safeselect.h>
#include <sortarray.h>

#include <gen_common.h>
#include <win_common.h>

#include <window.hpp>

var msg;
var temp;//num
var op;
var xx;
var zero;//num
var op2;
var wspos;
var wsmsg;

function main(in mode) {
	//c gen

	#include <general_common.h>

	win.valid = 1;

	if (mode == "PERP") {
		//COMMON /EXCHANGE/ EXCHANGE.DATE
		#define exchangedate RECUR0
		RECUR0 = var().date();
	} else if (mode == "VAL.CODE") {
		if (win.is.length() > 3) {
			msg = win.is.quote() ^ " - maximum of 3 characters";
			return invalid(msg);
		}

		if (var("1234567890-., ").index(win.is[1])) {
			msg = "THE CURRENCY CODE MUST START WITH AN ALPHABETIC CHARACTER";
			return invalid(msg);
		}

	} else if (mode == "VAL.NDECS") {

		//prevent leading zeroes
		if (win.is.length()) {
			win.is += 0;
		}

		if (not(win.isorig.length())) {
			return 0;
		}
		temp = win.orec.a(3);
		if (not temp) {
			temp = 0;
		}
		if (win.is < temp) {
			msg = "NOTE: IF YOU REDUCE THE NUMBER OF DECIMALS AND YOU|HAVE ALREADY MADE ENTRIES WITH THE GREATER NUMBER OF DECIMALS| YOU WILL GET ROUNDING ERRORS ON SOME REPORTS";
			call note(msg);
		}

	} else if (mode == "VAL.EXCHANGE.RATE") {
		if (win.is.substr(1,2) == "1/") {
			var reqconv = win.is.substr(3,99);
			if (not(reqconv.isnum() and reqconv)) {
				win.is = "";
				return 0;
			}
			var accuracy = 8;
			win.is = (1 / reqconv).oconv("MD" ^ accuracy ^ "0P");
			win.is = trim(win.is, "0", "B");
		}
		win.isorig = "XXX";

	} else if ((mode == "POSTINIT") or (mode == "POSTREAD")) {

		//option to read old versions
		call generalsubs2(mode);
		if (not(win.valid)) {
			return 0;
		}

		op = "CURRENCY";
		gosub security2(mode, op);
		if (not(win.valid)) {
			return 0;
		}

		//optionally prevent update of stopped currencies
		if (win.wlocked) {
			var tt = ((RECORD.a(1) ^ RECORD.a(25)).ucase()).convert("<>", "()");
			if (RECORD.a(25) or tt.index("(STOP)")) {
				if (not(authorised("CURRENCY ACCESS STOPPED", msg))) {
					win.wlocked = 0;
					//explain to the user why cannot update
					win.reset = -1;
					call mssg("This currency is stopped and you are not authorised to update it");
				}
			}
		}

		//if mode='POSTREAD' and wlocked then
		if ((mode == "POSTREAD") and (RECORD.a(12) == "")) {
			MV = 0;
			RECORD.r(12, calculate("CONVERSION2"));
		}

	} else if (mode == "PREDELETE") {

		op = "CURRENCY";
		gosub security2(mode, op);
		if (not(win.valid)) {
			return 0;
		}

		if (USERNAME ne "NEOSYS") {
			msg = "Deleting the base currency is not allowed";
			msg.r(-1, "Except by NEOSYS technical staff");
			return invalid(msg);
		}

		var filenamesx = "CLIENTS*INV_CURR_CODE";
		filenamesx ^= ",CLIENTS*CONV_CURR_CODE";
		filenamesx ^= ",MARKETS,CHARTS,SUPPLIERS,VEHICLES";
		filenamesx ^= ",COMPANIES*MAIN_CURRENCY";
		filenamesx ^= ",COMPANIES*PAYMENT_CURRENCY_CODES";
		var nfiles = filenamesx.count(",") + 1;
		for (var filen = 1; filen <= nfiles; ++filen) {

			//prevent deletion if any records
			var filename = filenamesx.field(",", filen);
			var fieldname = filename.field("*", 2);
			if (fieldname) {
				filename = filename.field("*", 1);
			}else{
				fieldname = "CURRENCY_CODE";
			}
			if (xx.open(filename, "")) {
				var cmd = "SELECT 1 " ^ filename ^ " WITH " ^ fieldname ^ " " ^ (ID.quote()) ^ " (S)";
				printl(cmd);
				perform(cmd);
				if (LISTACTIVE) {
					if (not(readnext(temp))) {
						temp = "";
					}
					clearselect();
					var keyfield = singular(filename).lcase();
					if (keyfield == "job") {
						keyfield ^= " no.";
					}else{
						keyfield ^= " code";
					}
					msg = "This currency has been used for " ^ keyfield ^ " " ^ (temp.quote()) ^ " and maybe others.|It cannot be deleted unless they are deleted first.";
					return invalid(msg);
				}
			}

		};//filen;

		if (USER4) {
			msg = "";
			return invalid(msg);
		}

		ANS = "";
		win.valid = 1;

		//update version log
		call generalsubs2(mode);

	} else if (mode == "PREWRITE") {

		//check not changing decimal places of base currency while data exists
		//to stop the ill-informed from messing up
		var basecurr = gen.company.a(3);
		if (((USERNAME ne "NEOSYS" and (ID == basecurr)) and win.orec.a(3) ne "") and RECORD.a(3) ne win.orec.a(3)) {
			msg = "Changing the number of decimals";
			msg.r(-1, "of the base currency is not allowed");
			msg.r(-1, "Except by NEOSYS technical staff");
			return invalid(msg);
		}

		//check exchange rate is 1 for base currency
		if (ID == basecurr) {
			if (RECORD.a(5) ne 1) {
				msg = ID.quote() ^ " is base currency so its exchange rate must be 1";
				return invalid(msg);
			}
		}

		//prevent zero exchange rate
		if (not(RECORD.a(5).locate("0",zero))) {
			if (not(RECORD.a(12).locate("0",zero))) {
				zero = 0;
			}
		}
		if (zero) {
			msg = "Exchange rate cannot be 0 in line " ^ zero;
			return invalid(msg);
		}

		//check reducing currency decimals versus balances file if available
		//TODO check versus other files if no accounts
		if (win.orec.a(3) ne "") {

			//generally prevent even senior users from changing the number of decimals
			//since can cause havoc that is very difficult to recover from
			//eg after increasing to 3 decimal places it almost impossible to
			//revert to 2 decimal places with lots of errors especially in finance
			if (RECORD.a(3) ne win.orec.a(3)) {
				if (not(authorised("CURRENCY UPDATE DECIMALS", msg, "NEOSYS"))) {
					return invalid(msg);
				}
			}

			//if really reducing then check a load of files
			//for the presence of the currency and disallow if found
			if (RECORD.a(3) < win.orec.a(3)) {

				printl("Searching for currency ", ID, " preventing decimal place reduction");

				//check balances for any existing currency
				var filename = "ACCOUNTS BALANCES";
				var cmd = "SELECT 1 BALANCES WITH CURRENCY_CODE " ^ (ID.quote());
				cmd ^= " OR WITH OTHER_CURR_CODES " ^ (ID.quote());
				printl(cmd);
				call safeselect(cmd);
				if (LISTACTIVE) {
baddecimals:
					clearselect();
					msg = ID.quote() ^ " currency is used in " ^ capitalise(filename);
					msg.r(-1, FM ^ "Reducing the number of decimals of");
					msg.r(-1, "a currency in use is not allowed");
					msg.r(-1, "in order to prevent rounding errors");
					return invalid(msg);
				}

				if (FILES(0).locateusing(FM,"SCHEDULES",xx)) {

					//check schedules for any existing currency
					filename = "MEDIA SCHEDULES";
					cmd = "SELECT 1 SCHEDULES WITH CURRENCY_CODE " ^ (ID.quote());
					cmd ^= " OR WITH VEHICLE_CURR_CODE " ^ (ID.quote());
					printl(cmd);
					call safeselect(cmd);
					if (LISTACTIVE) {
						goto baddecimals;
					}

					//check plans for any existing currency
					filename = "MEDIA PLANS";
					cmd = "SELECT 1 PLANS WITH CURRENCY_CODE " ^ (ID.quote());
					cmd ^= " OR WITH VEHICLE_CURR_CODE " ^ (ID.quote());
					printl(cmd);
					call safeselect(cmd);
					if (LISTACTIVE) {
						goto baddecimals;
					}

				}

				if (FILES(0).locateusing(FM,"JOBS",xx)) {

					//check production orders for any existing currency
					filename = "PRODUCTION ORDERS";
					cmd = "SELECT 1 PRODUCTION_ORDERS WITH CURR_CODE " ^ (ID.quote());
					printl(cmd);
					call safeselect(cmd);
					if (LISTACTIVE) {
						goto baddecimals;
					}

					//check production estimates for any existing currency
					filename = "PRODUCTION ESTIMATES";
					cmd = "SELECT 1 PRODUCTION_INVOICES WITH CURR_CODE " ^ (ID.quote());
					printl(cmd);
					call safeselect(cmd);
					if (LISTACTIVE) {
						goto baddecimals;
					}

				}

			}
		}

		//sort exchange rates into descending date order
		call sortarray(RECORD, 4 ^ VM ^ 5 ^ VM ^ 12, "DR");

		//@record<12>=''

		//update version log
		call generalsubs2(mode);

	} else if (mode == "POSTWRITE") {

		//clear the currency code buffer
		gen.accounting.r(4, "");
		gen.accounting.r(5, "");

	} else if (mode == "POSTDELETE") {

	//CASE MODE='EXPORT'
	// GOSUB EXPORT
	//
	//CASE MODE='IMPORT'
	// GOSUB IMPORT
	//
	//case mode='REORDER'
	// gosub reorder

	} else {
		msg = mode.quote() ^ " UNRECOGNISED MODE IN CURRENCY.SUBS";
		call note(msg);
	}

	return 0;

	/*;
	///////
	EXPORT:
	///////
		if security('currency file export',msg,'') else return invalid(msg);
		gosub selectdiskette;
		if diskette else return 0;

		read currcodes from currencies,'%RECORDS%' else;
			select currencies;
			currcodes='';
	nextcurr:
			readnext currcode then;
				currcodes<-1>=currcode;
				goto nextcurr;
				end;
			end;
		ncurrs=count(currcodes,@fm)+1;

		if currcodes else;
			msg='Please set up some currencies first';
			return invalid(msg);
			end;

		//confirm
		if decide('OK to start exporting ?|':ncurrs:' currencies found','',reply) else return 0;
		if reply ne 1 then return 0;

		//delete existing records ?
		initdir diskette:'\*.cur';
		osfilenames=dirlist();
		if osfilenames then;
			if decide('Do you want to delete the existing|currency records on the diskette ?','',reply) else return 0;
			if reply=1 then;
				call note2('Deleting,||Press Esc to interrupt','UB',buff,'');
				for filen=1 to 9999;
					filename=osfilenames<filen>;
					if esc.to.exit() then;
						call mssg('','DB',buff,'');
						return 0;
						end;
				while filename;
					print @aw<30>:@(38,@crthigh/2):filen:'. ':field(filename,'/',2)[1,'.'];
					osdelete diskette:'\':filename;
					next;
				call mssg('','DB',buff,'');
				end;
			end;

		//copy the currencies to the diskette
		open 'DOS' to dos else;
			call fsmsg();
			return 0;
			end;
		call note2('Exporting,||Press Esc to interrupt','UB',buff,'');

		basecurr=company<3>;

		for currn=1 to ncurrs;
		until esc.to.exit();
			currcode=currcodes<currn>;
			print @aw<30>:@(38,@crthigh/2):currn:'. ':currcode;
			read rec from currencies,currcode then;
				rec<7>=basecurr;
				write rec on dos,diskette:'\':currcode:'.cur';
				end;
			next currn;
		call mssg('','DB',buff,'');

		msg=ncurrs:' records exported.';
		gosub note;

		return 0;

	///////
	import:
	///////

		//ensure we get latest base currency in case changed
		call init.company(curr.company);

		if security('currency file import',msg,'') else return invalid(msg);
		gosub selectdiskette;
		if diskette else return 0;
		initdir diskette:'\*.cur';
		osfilenames=dirlist();
		if osfilenames else;
			msg='THERE ARE NO CURRENCY RECORDS ON THAT DISKETTE|(ARE YOU USING THE RIGHT DISKETTE ?)';
			gosub invalid;
			goto import;
			end;

		osfilenames[1,0]=diskette:'\';
		if osfilenames[-1,1]=@fm then osfilenames[-1,1]='';
		swap @fm with @fm:diskette:'\' in osfilenames;
		question='Which currency(ies) do you want to import ?';
		onemany=1;
		//osfilenames=POP.UP(0,0,'@DOS',osfilenames,'2:40:L::Description ':STATUS:'\3:9:L:':@DATE.FORMAT:':Date\4:8:L:MTH:Time\1:10:L::Created by','T',ONEMANY,question,'','','','K')
		if osfilenames else return 0;

		ncurrs=count(osfilenames,@fm)+1;

		//confirm
		if decide('OK to start importing ?|':ncurrs:' currencies found','',reply) else return 0;
		if reply ne 1 then return 0;

		//copy the records to the currencies file
		open 'DOS' to dos else;
			call fsmsg();
			return 0;
			end;
		call note2('Importing,||Press Esc to interrupt','UB',buff,'');
		import.base.rec='';
		basecurr=company<3>;
		for currn=1 to ncurrs;
			print @aw<30>:@(38,@crthigh/2):currn:'.';
			osfilenamex=osfilenames<currn>;
			read rec from dos,osfilenamex then;
				currcode=field(osfilenamex,'\',2)[1,'.'];

				//convert exchange rates to this companies base currency
				import.base=rec<7>;

				//if same currency or either is not specified then leave rates as they are
				begin case;
				case import.base=basecurr or import.base='' or basecurr='';

				//if it is our own currency then set the exchange rate to 1
				case currcode=basecurr;
					rec<4>=1;
					rec<5>=1;

				//if they are different currencies
				case 1;

					//get the own base import record (to get the back rates)
					if import.base.rec else;
						osread import.base.rec from diskette:'\':basecurr:'.cur' else;
							msg=quote(basecurr):' - currency missing from diskette|The currency rates need to be converted to our base currency.';
							return invalid(msg);
							end;
						end;

					//if it is the base currency of the exporter then copy dates
					// (rates will be inverted "1/" lower down)
					if currcode=import.base then;
						rec<4>=import.base.rec<4>;
						n=count(rec<4>,vm)+(rec<4>#'');
						for i=1 to n;
							rec<5,i>=1;
							next;
						end;

					//convert the rates from import base to own base
					nrates=count(rec<4>,vm)+(rec<4>#'');
					for raten=1 to nrates;
						DATEX=rec<4,raten>;
						rate=rec<5,raten>;

						//find the back rate from the imported own base record
						if raten=1 then;
							//ensure the latest rate is used for the latest rate
							backrate=import.base.rec<5,1>;
						end else;
							locate DATEX in import.base.rec<4> by 'DR' setting temp then;
								backrate=import.base.rec<5,temp>;
							end else;
								backrate='';
								end;
							end;
						if backrate else;
							backrate=import.base.rec<5,temp-1>;
							end;

						//do the convertion
						if backrate and num(backrate) then;
							newrate=((rate/backrate) 'MD80P')+0;
							rec<5,raten>=newrate;
							end;

						next raten;

					end case;

				write rec on currencies,currcode;
				end;
		until esc.to.exit();
			next currn;
		call mssg('','DB',buff,'');

		valid=1;
		msg=ncurrs:' records imported.';
		gosub note;
		return 0;

	selectdiskette:
	///////////////
		diskette=decide('Please insert diskette now.||Which diskette are you using ?|(Press Esc to cancel)','A:|B:|C:|D:',reply);
		if diskette else return 0;
		declare function drvready;
		if drvready(diskette='B:') else;
			print char(7):
			call msg('THERE IS A PROBLEM WITH DRIVE ':DISKETTE:'|HAVE YOU INSERTED THE DISKETTE CORRECTLY ?','','','');
			goto selectdiskette;
			end;
		return 0;

	////////
	REORDER:
	////////
		READ RECORDS FROM CURRENCIES,'%RECORDS%' ELSE;
			msg='Please install quickdex on the CURRENCIES file first';
			return invalid(msg);
			end;
		PARAMS='2:50:L::Currency\1:5:L::Code';

		if count(records,fm) else;
			msg='THERE IS ONLY ONE CURRENCY - YOU CANNOT REORDER';
			return invalid(msg);
			end;

		for currencyn=1 to 9999;
			currencycode=records<currencyn>;
		while currencycode;
			read temp from currencies,currencycode else currency='';
			records<currencyn,2>=temp<1>;
			next currencyn;

		QUESTION='Please reorder the currencies.  (Press F9 when finished)|';
		question<-1>='INSTRUCTIONS: 1) Select a currency by pressing "Enter"        ';
		question<-1>='              2) Move the currency by pressing Up/Down        ';
		question<-1>='              3) Deselect the currency by pressing Enter again';
		newrecords=POP.UP(0,0,'',RECORDS,PARAMS,'R','S':@FM:2,question,'','','','');
		if newrecords else return 0;

		for currencyn=1 to 9999;
			currencycode=newrecords<currencyn>;

		while currencycode;

			newrecords<currencyn>=newrecords<currencyn,1>;
			next currencyn;

		CALL RTP57(3, '', CURRENCIES<1,2>, '%RECORDS%', '', newrecords, results);
		RETURN;
	*/

}

libraryexit()
