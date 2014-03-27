#include <exodus/library.h>
libraryinit()

#include <btreeextract.h>
#include <giveway.h>
#include <authorised.h>

#include <fin.h>
#include <gen.h>

var temp;
var plussign;
var accno;
var posn;
var xx;
var vn;
var reply;//num
var ok;//num
var buff;
var accno2;
var msg0;
var positive;
var acctype;

function main(io mode, io is, io isorig, in validaccs, io msg, in acctype0) {
	//y2k
	//returns true if is a valid account
	//otherwise returns false with error message in MSG
	if (acctype0.unassigned()) {
		acctype = "";
	} else {
		acctype = acctype0;
	}

	//defeat compiler warning unused
	if (false) print(isorig);

	var autogetmany = mode == "AUTOGETMANY";
	if (autogetmany) {
		mode = "";
		}

	var many = mode == "MANY";
	if (many) {
		mode = "";
	}

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
		var temp;
		if (not(temp.read(gen.companies, iscompanycode))) {
			msg = DQ ^ (iscompanycode ^ DQ) ^ " Company does not exist";
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
			if (interactive and not remotecall and mode ne 1) {
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
				msg = DQ ^ (isaccno ^ DQ) ^ " - Account does not exist";
				return 0;
			}

			isaccno.ucaser();

			//try to use indexes
			//if (indices2("ACCOUNTS", "UPPERCASE_NAME.XREF")) {
			if (true) {
				var acclist = "";
				var storedict = DICT;
				if (DICT.open("DICT", "ACCOUNTS")) {
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
					for (var accn = 1; accn <= 9999; ++accn) {
						accno = acclist.a(accn);

						///BREAK;
						if (not(accno and acclist2.length() < 30000))
							break;;

						if (not interactive) {
							if (giveway()) {
								{}
							}
						}
						if (plussign) {
							if (accno.substr(1, isaccno.length()) ne isaccno) {
								goto nextaccno;
							}
						}
						if (accno[1] ne ".") {
							if (not(acclist2.locateby(accno, "AL", posn))) {

								//skip wrong ledgers
								if (var("CHART" _VM_ "STOCK" _VM_ "JOB").locate(validaccs.substr(1, 5), xx, 1 )) {
									if (not(fin.account.read(fin.accounts, accno))) {
										fin.account = "";
									}
									if (validaccs.substr(1, 5) == "CHART") {
										if (not(validaccs.substr(7, 9999).locate(fin.account.a(2), vn))) {
											accno = "";
										}
									}else{
										if (not(fin.account.a(23).field(";", 2) == validaccs.substr(1, 5))) {
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
											if (not(fin.account.a(8) == "" and acctype == "O")) {
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
					}

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

						//var yy = _CRTHIGH / 2 - acclist.count(FM) / 2 - 4;
						//if (yy < 0) {
						//	yy = 0;
						//}

						if (not remotecall) {
							//print char(7):
							//accno = pop_up(0, yy, fin.accounts, acclist, params, "T", many, "Which account" ^ ss ^ " do you want ?", "", "", "1", "K");
						}else{
							naccs = acclist.count(FM) + 1;
							acclist2 = "";
							var accn;
							for (accn = 1; accn <= naccs; ++accn) {
								if (fin.account.read(fin.accounts, acclist.a(accn))) {
									accno = fin.account.a(10);
									gosub checkaccountauthorised(msg);
									if (ok) {
										acclist2 ^= FM ^ accno ^ VM ^ fin.account.a(1) ^ VM ^ fin.account.a(2) ^ VM ^ fin.account.a(5);
									}
								}

								///BREAK;
								if (not(acclist2.length() < 32000))
									break;;

							};//accn;
							acclist2.splicer(1, 1, "");

							//trim off unresolved accounts after 32000
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
							// read acc from accounts,accno<i> then accno<i>=acc<10>
							// next i
							accno.converter(FM, TM);
							is = SVM ^ accno;
							return 1;
						}
					}

					//get the account
					var acc;
					if (not(acc.read(fin.accounts, accno))) {
//missingacc:
						//print char(7):
						call mssg(DQ ^ (accno ^ DQ) ^ " account is missing|(try rebuilding indexes)");
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
			if (not remotecall) {
				if (not(decide("!" ^ (DQ ^ (is.field(",", 1) ^ DQ)) ^ " is not an account number.||Is it the account name|or part of the name ?", "", reply))) {
					reply = 2;
				}
			}else{
				reply = 2;
				msg = DQ ^ (is.field(",", 1) ^ DQ) ^ " - Account number does not exist";
				return 0;
			}

			if (reply == 1) {

				call note2("Searching for " ^ isaccno ^ "|(Please wait or press [Esc] to exit)", "UB", buff, "");
				fin.accounts.select();
				var rejectedaccnos = "";

nextacc:
				if (mv.esctoexit()) {
					call msg2("", "DB", buff, "");
					return "";
				}

				if (readnext(accno)) {
					temp.input(-1, 0);
					if (temp) {
						if (temp == var().chr(27)) {
							call msg2("", "DB", buff, "");
							return "";
						}
						mv.DATA ^= temp;
					}
					var acc;
					if (acc.read(fin.accounts, accno)) {
						temp = acc.a(1).ucase();
						//PRINT TEMP
						if (temp.index(isaccno, 1)) {
							var ledgername;
							if (not(ledgername.readv(fin.ledgers, acc.a(2), 1))) {
								ledgername = "";
							}
							if (accno[1] == ".") {
								accno2 = accno.substr(2, 99);
							}else{
								accno2 = acc.a(10);
							}
							if (not(rejectedaccnos.locate(accno2, temp, 1))) {
								//print char(7):
								if (not(decide(acc.a(1) ^ " (" ^ accno2 ^ ")|" ^ ledgername ^ "||" "IS THIS THE ONE YOU WANT ?", "", reply))) {
									call msg2("", "DB", buff, "");
									return 0;
								}
								if (reply == 1) {
									call msg2("", "DB", buff, "");
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
				call msg2("", "DB", buff, "");
				msg = DQ ^ (isaccno ^ DQ) ^ "|" "No accounts can be found.|";
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
			msg = fin.account.a(1) ^ "|belongs to company " ^ (DQ ^ (fin.account.a(5) ^ DQ));
			return 0;
		}

	}

	//check if valid account/ledger
	if (validaccs == "*") {
		//valid accs "*" allows all accounts, including job accounts

	} else if (validaccs == "STOCK" or validaccs == "JOB" or validaccs == "") {
		//valid accs "" means not job account

		if (not(fin.account.a(23).field(";", 2) == validaccs)) {
			var tt = validaccs ? (validaccs) : "VALID";
			msg = DQ ^ (is.a(1, 1, 1) ^ DQ) ^ " - is not a " ^ tt ^ " account";
			return 0;
		}

	} else if (validaccs.substr(1, 5) == "CHART") {
		if (not(validaccs.substr(7, 9999).locate(fin.account.a(2), vn))) {
			msg = DQ ^ (is.a(1, 1, 1) ^ DQ) ^ " - Account is in the wrong ledger." ^ FM ^ "(See Journal Setup)";
			return 0;
		}

	} else {
		if (not(validaccs.locate(is.a(1, 1, 1).field(",", 1), vn))) {
			if (not(validaccs.locate(is.a(1, 1, 2).field(",", 1), vn))) {
				msg = DQ ^ (is.a(1, 1, 1) ^ DQ) ^ " - Account is not allowed here." ^ FM ^ "(See Journal Setup)";
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
	if (not(authorised("#ACCOUNT ACCESS " ^ accno.quote(), msg, ""))) {

		var ledgercode = fin.account.a(2);
		if (authorised("LEDGER ACCESS", msg0, "")) {
			positive = "";
		}else{
			positive = "#";
		}
		if (not(authorised(positive ^ "LEDGER ACCESS " ^ (DQ ^ (ledgercode ^ DQ)), msg, ""))) {
			//msg='Sorry A/c: ':account<1>:'||':msg0:' nor Ledger Access ':quote(ledgercode)
			msg.r(-1, "nor ACCOUNT ACCESS " ^ (DQ ^ (accno ^ DQ)));
			return;
		}

	}

	msg = "";
	ok = 1;
	return;

}


libraryexit()
