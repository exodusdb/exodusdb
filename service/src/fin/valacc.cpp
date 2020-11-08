#include <exodus/library.h>
libraryinit()

#include <btreeextract.h>
#include <giveway.h>
#include <authorised.h>

#include <fin_common.h>
#include <gen_common.h>

var mode;
var validaccs;
var temp;
var plussign;
var accn;//num
var accno;
var posn;
var xx;
var vn;
var reply;//num
var ok;//num
var acc;
var buff;
var ledgername;
var accno2;
var msg0;
var positive;

function main(in mode0, io is, io isorig, in validaccs0, io msg, io acctype) {
	//c fin in,io,io,in,io,io
	//global accn

	#include <common.h>
	#include <general_common.h>

	if (mode0.unassigned()) {
		mode = "";
	}else{
		mode = mode0;
	}
	if (acctype.unassigned()) {
		acctype = "";
	}
	if (isorig.unassigned()) {
		isorig = "";
	}
	if (validaccs0.unassigned()) {
		validaccs = "";
	}else{
		validaccs = validaccs0;
	}

	//returns true if is a valid account
	//otherwise returns false with error message in MSG

	var autogetmany = mode == "AUTOGETMANY";
	if (autogetmany) {
		mode = "";
	}

	var many = mode == "MANY";
	if (many) {
		mode = "";
	}
	//WARNING TODO: check ternary op following;
	var ss = many ? "(s)" : "";

	var remotecall = SYSTEM.a(33);
	var interactive = mode ne "";
	msg = "";

	//ZZZ if part two has'nt changed but part one has then update part one

	is.ucaser();
	is = is.a(1, 1, 1);
	if (is == "") {
		return 1;
	}

	//check company exists
	var iscompanycode = is.field(",", 2);
	if (iscompanycode ne "") {
		if (not(temp.read(gen.companies, iscompanycode))) {
			msg = iscompanycode.quote() ^ " Company does not exist";
			return 0;
		}
	}

	//990702 fail if accounts system not installed
	//open 'ACCOUNTS' to x else
	// *call note('!NEOSYS ACCOUNTING MODULE IS NOT INSTALLED')
	// return 0
	// end

chkacc:
	var isaccno = is.field(",", 1);

	//check that the account exists
	if (fin.account.read(fin.accounts, "." ^ isaccno)) {
		is = isaccno;
		if (iscompanycode ne "") {
			is ^= "," ^ iscompanycode;
		}
		is.r(1, 1, 2, fin.account.a(10));
	}else{

		//prevent leading '.'
		if (isaccno[1] == ".") {
			goto badacc;
		}

		if (fin.account.read(fin.accounts, isaccno)) {

			//dot account missing? this could be skipped but best to block and force fix
			if (fin.account.a(10) == isaccno) {
				msg = "internal error \"." ^ isaccno ^ "\" record is missing";
				return 0;
			}

			//if interactive and not(remotecall) then
			//no message if batch validation
			if ((interactive and not(remotecall)) and mode ne 1) {
				//print char(7):
				call note("NOTE: " ^ isaccno ^ " HAS BEEN CHANGED TO " ^ fin.account.a(10));
			}
			is = fin.account.a(10);
			if (iscompanycode ne "") {
				is ^= "," ^ iscompanycode;
			}
			is.r(1, 1, 2, isaccno);
		}else{

			//return invalid if not found and not interactive
			if (interactive) {
badacc:
				msg = isaccno.quote() ^ " - Account does not exist";
				return 0;
			}

			isaccno.ucaser();

			//try to use indexes
			if (listindexes("ACCOUNTS", "UPPERCASE_NAME.XREF")) {
				var acclist = "";
				var storedict = DICT;
				if (DICT.open("dict_ACCOUNTS")) {
					plussign = isaccno[-1] == "+";
					if (plussign) {
						isaccno.splicer(-1, 1, "");
					}
					//temp=trim(is.accno):' '
					//swap ' ' with ']':vm in temp
					//temp[-1,1]=fm

					temp = isaccno;
					temp.converter("][\\&", " ");
					temp.trimmer();
					temp.swapper(" ", "]&");
					temp ^= "]";

					temp = "UPPERCASE_NAME.XREF" ^ VM ^ temp ^ FM;

					call btreeextract(temp, "ACCOUNTS", DICT, acclist);

					DICT = storedict;
				}

				if (acclist) {

					//build list
					acclist.converter(VM, FM);
					var acclist2 = "";
					for (accn = 1; accn <= 9999; ++accn) {
						accno = acclist.a(accn);
					//while accno and len(acclist2)<30000
						///BREAK;
						if (not(accno and (acclist2.length() < maxstrsize / 2))) break;
						if (not interactive) {
							if (giveway()) {
								{}
							}
						}
						if (plussign) {
							if (accno.substr(1,isaccno.length()) ne isaccno) {
								goto nextaccno;
							}
						}
						if (accno[1] ne ".") {
							var tt = acclist2;
							tt.converter(FM, VM);
							if (not(tt.locateby("AL",accno,posn))) {

								//skip wrong ledgers
								if (var("CHART,STOCK,JOB").locateusing(",",validaccs.substr(1,5),xx)) {
									if (not(fin.account.read(fin.accounts, accno))) {
										fin.account = "";
									}
									if (validaccs.substr(1,5) == "CHART") {
										if (not(validaccs.substr(7,9999).locateusing(FM,fin.account.a(2),vn))) {
											accno = "";
										}
									}else{
										if (not(fin.account.a(23).field(";", 2) == validaccs.substr(1,5))) {
											accno = "";
										}
									}
								}else{
									if (acctype) {
										if (not(fin.account.read(fin.accounts, accno))) {
											fin.account = "";
										}
									}
								}

								//skip wrong account type (open item/balance forward)
								if (acctype) {
									if (acctype == "-") {
										if (fin.account.a(6)) {
											accno = "";
										}
									}else{
										if (fin.account.a(8) ne acctype) {
											if (not((fin.account.a(8) == "") and (acctype == "O"))) {
												accno = "";
												}
											}
										}
									}

								if (accno ne "") {
									acclist2.inserter(posn, accno);
								}

							}
						}
nextaccno:;
					};//accn;

					if (acclist2 == "") {
						goto accname;
					}

					acclist2.transfer(acclist);
					var naccs = acclist.count(FM) + 1;

					//popup select
					if (naccs == 1) {
						accno = acclist;

					}else{
						var params = "1:35:::Account";
						params ^= "\\" "10:12:::A/c No.";
						params ^= "\\" "2:10:::Ledger";
						params ^= "\\" "5:10:::Company";

						if (autogetmany) {
							if (not(decide("!You have selected " ^ naccs ^ " accounts,|do you want all of them ?", "", reply))) {
								reply = 2;
							}
							if (reply == 1) {
								is = acclist;
								return 1;
							}
						}

						var yy = CRTHIGH / 2 - acclist.count(FM) / 2 - 4;
						if (yy < 0) {
							yy = 0;
						}

						if (not(remotecall)) {
							//print char(7):
							//ACCNO=POP.UP(0,Y,ACCOUNTS,acclist,PARAMS,'T',many,'Which account':s:' do you want ?','','','1','K')
							accno = "";
						}else{
							naccs = acclist.count(FM) + 1;
							acclist2 = "";
							for (accn = 1; accn <= naccs; ++accn) {
								if (fin.account.reado(fin.accounts, acclist.a(accn))) {
									accno = fin.account.a(10);
									gosub checkaccountauthorised(msg);
									if (ok) {
										acclist2 ^= FM ^ accno ^ VM ^ fin.account.a(1) ^ VM ^ fin.account.a(2) ^ VM ^ fin.account.a(5);
									}
								}
							//while len(acclist2)<32000
								///BREAK;
								if (not(acclist2.length() < maxstrsize / 2)) break;
							};//accn;
							acclist2.splicer(1, 1, "");

							//trim off unresolved accounts after limit
							acclist = acclist2.field(FM, 1, accn);

							msg = "/ACCOUNTLIST/" ^ acclist;
							accno = "";
						}
						if (accno == "") {
							return 0;
						}
						if (many) {
							//n=count(accno,fm)+1
							//for i=1 to n
							// reado acc from accounts,accno<i> then accno<i>=acc<10>
							// next i
							accno.converter(FM, TM);
							is = SVM ^ accno;
							return 1;
						}
					}

					//get the account
					if (not(acc.reado(fin.accounts, accno))) {
	//missingacc:
						//print char(7):
						call mssg(accno.quote() ^ " account is missing|(try rebuilding indexes)");
						return 0;
					}
					is = acc.a(10);

					if (naccs == 1) {
						//print char(7):
						if (not remotecall) {
							if (not(decide(acc.a(1) ^ " (" ^ is ^ ")|" ^ acc.a(2) ^ "||" "IS THIS THE ONE YOU WANT ?", "", reply))) {
								reply = 2;
							}
							if (reply == 2) {
								return 0;
							}
						}
					}

					goto chkacc;

				}
			}

accname:
			//either bad account number or account name
			if (not(remotecall)) {
				if (not(decide("!" ^ (is.field(",", 1).quote()) ^ " is not an account number.||Is it the account name|or part of the name ?", "", reply))) {
					reply = 2;
				}
			}else{
				reply = 2;
				msg = is.field(",", 1).quote() ^ " - Account number does not exist";
				return 0;
			}

			if (reply == 1) {

				call note("Searching for " ^ isaccno ^ "|(Please wait or press [Esc] to exit)", "UB", buff, "");
				select(fin.accounts);
				var rejectedaccnos = "";

nextacc:
				if (esctoexit()) {
					call mssg("", "DB", buff, "");
					return "";
				}

				if (readnext(accno)) {
					temp.input("", -1);
					if (temp) {
						if (temp == var().chr(27)) {
							call mssg("", "DB", buff, "");
							return "";
						}
						DATA ^= temp;
					}
					if (acc.read(fin.accounts, accno)) {
						temp = acc.a(1);
						temp.ucaser();
						//PRINT TEMP
						if (temp.index(isaccno)) {
							if (not(ledgername.readv(fin.ledgers, acc.a(2), 1))) {
								ledgername = "";
							}
							if (accno[1] == ".") {
								accno2 = accno.substr(2,99);
							}else{
								accno2 = acc.a(10);
							}
							if (not(rejectedaccnos.a(1).locate(accno2,temp))) {
								//print char(7):
								if (not(decide(acc.a(1) ^ " (" ^ accno2 ^ ")|" ^ ledgername ^ "||" "IS THIS THE ONE YOU WANT ?", "", reply))) {
									call mssg("", "DB", buff, "");
									return 0;
								}
								if (reply == 1) {
									call mssg("", "DB", buff, "");
									is = accno2;
									goto chkacc;
								}else{
									rejectedaccnos ^= VM ^ accno2;
								}
							}
						}
					}
					goto nextacc;
				}
				call mssg("", "DB", buff, "");
				msg = isaccno.quote() ^ "|" "No accounts can be found.|";
				//print char(7):
				call mssg(msg);
			}
			return "";
		}
	}

	//check company code if entered
	if (iscompanycode ne "") {

		//check account matches company
		if (fin.account.a(5) and fin.account.a(5) ne iscompanycode) {
			msg = fin.account.a(1) ^ "|belongs to company " ^ (fin.account.a(5).quote());
			return 0;
		}

	}

	//check if valid account/ledger
	if (validaccs == "*") {
		//valid accs "*" allows all accounts, including job accounts
	} else if (((validaccs == "STOCK") or (validaccs == "JOB")) or (validaccs == "")) {
		//valid accs "" means not job account
		if (not(fin.account.a(23).field(";", 2) == validaccs)) {
			//WARNING TODO: check ternary op following;
			var tt = validaccs ? validaccs : "VALID";
			msg = is.a(1, 1, 1).quote() ^ " - is not a " ^ tt ^ " account";
			return 0;
		}
	} else if (validaccs.substr(1,5) == "CHART") {
		if (not(validaccs.substr(7,9999).locateusing(FM,fin.account.a(2),vn))) {
			msg = is.a(1, 1, 1).quote() ^ " - Account is in the wrong ledger." ^ FM ^ "(See Journal Setup)";
			return 0;
		}
	} else {
		if (not(validaccs.locateusing(FM,is.a(1, 1, 1).field(",", 1),vn))) {
			if (not(validaccs.locateusing(FM,is.a(1, 1, 2).field(",", 1),vn))) {
				msg = is.a(1, 1, 1).quote() ^ " - Account is not allowed here." ^ FM ^ "(See Journal Setup)";
				return 0;
			}
		}
	}

	//account is set
	accno = is.a(1, 1, 1).field(",", 1);
	gosub checkaccountauthorised(msg);

	return ok;
}

subroutine checkaccountauthorised(io msg) {

	ok = 0;

	//positive account access trumps ledger restrictions
	if (not(authorised("#ACCOUNT ACCESS " ^ (accno.quote()), msg, ""))) {

		var ledgercode = fin.account.a(2);
		if (authorised("LEDGER ACCESS", msg0, "")) {
			positive = "";
		}else{
			positive = "#";
		}
		if (not(authorised(positive ^ "LEDGER ACCESS " ^ (ledgercode.quote()), msg, ""))) {
			//msg='Sorry A/c: ':account<1>:'||':msg0:' nor Ledger Access ':quote(ledgercode)
			msg.r(-1, "nor ACCOUNT ACCESS " ^ (accno.quote()));
			return;
		}

	}

	msg = "";
	ok = 1;
	return;
}

libraryexit()
