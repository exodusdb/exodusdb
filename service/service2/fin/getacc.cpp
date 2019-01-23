#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <pop_up.h>
#include <initcompany.h>
#include <valacc.h>
#include <safeselect.h>

#include <fin.h>
#include <gen.h>

var reqcompanies;
dim chart;
var ledgerorbalances;
var xx;
var doctype;
var suffix;
var temp;
var lastvoucherno;//num
var lastbatchno;//num
var firstbatchno;//num
var msg;
var many;//num
var accstk;
var accstk2;
var reply;//num
var onemany;//num
var ss;
var newlevel;//num
var buffer;

function main(io mode, io validaccs, io reqcompanies2, io acctype, io reqledgertype, io getacc2, io inpaccnos) {
	//c fin io,io,io,io,io,io,io
	//y2k
	//mode is accounts,LEDGERS,LEDGER or blank which means ACCOUNT
	//valid.accs is comma or fm list of account numbers or ledgers if starts CHART
	// for specific mode ACCOUNT it is the default
	//company optionally limits which companies (mv list)
	//acctype optionally limits which account types (mv list)

	var interactive = not SYSTEM.a(33);

	if (validaccs.unassigned()) {
		validaccs = "";
	}
	if (validaccs.unassigned()) {
		validaccs = "";
	}
	if (reqledgertype.unassigned()) {
		reqledgertype = "";
	}
	if (reqcompanies2.unassigned()) {
		reqcompanies2 = "";
	}
	if (acctype.unassigned()) {
		acctype = "";
	}

	//req.companies=if req.companies2 then req.companies2 else curr.company
	//req.companies=if req.companies2 then req.companies2 else curr.company.codes
	if (reqcompanies2) {
		reqcompanies = reqcompanies2;
		if (reqcompanies == "*") {
			reqcompanies = "";
		}
	}else{
		if (mode.index("VOUCHER")) {
			reqcompanies = fin.currcompany;
		}else{
			reqcompanies = fin.currcompanycodes;
		}
	}

	if ((validaccs == "STOCK") or (validaccs == "*")) {
		reqledgertype = validaccs;
		validaccs = "";
	}

	chart.redim(50);
	chart="";
	gosub get.acctype.desc();

	ANS = "";
	var ledgercode = "";
	validaccs.converter(",", FM);
	var valid = 1;

	if (mode.substr(1,7) == "BLEDGER") {
		ledgerorbalances = "FINANCIAL REPORT";
		mode.splicer(1, 1, "");
	}else{
		ledgerorbalances = "LEDGER";
	}
//WARNING TODO: check trigraph following;
	var positive = (not authorised(ledgerorbalances ^ " ACCESS", xx, "")) ? var("#") : var("");

////////////
vouchertype:
////////////
	if (mode.substr(1,12) == "VOUCHER.TYPE") {
		doctype = "DAYBOOK";
	} else if (mode.substr(1,13) == "VVOUCHER.TYPE") {
		doctype = "VOUCHER";
		mode.splicer(1, 1, "");
	} else if (mode.substr(1,13) == "IVOUCHER.TYPE") {
		doctype = "INVOICE";
		mode.splicer(1, 1, "");
	}
//L413:
	if (mode.substr(1,12) == "VOUCHER.TYPE") {
		//MULTI=(MODE[-1,1] EQ 'S')
		var multi = mode.index("VOUCHER.TYPES");
		if (mode.index("UNPOSTED")) {
			suffix = "*U";
		}else{
			suffix = "";
		}
		var onlybatches = acctype.index("OB");
		var onlyvouchers = acctype.index("OV");
		var showbatches = acctype.index("B");
		var showvouchers = acctype.index("V");
		var onlyautono = acctype.index("A");
		var nvtypes = fin.definition.a(6).count(VM) + 1;
		var nvtypes2 = 0;
		var selection = "";

		var ncompanies = reqcompanies.count(VM) + (reqcompanies ne "");
		for (var companyn = 1; companyn <= ncompanies; ++companyn) {
			var reqcompany = reqcompanies.a(1, companyn);

			for (var vtypen = 1; vtypen <= nvtypes; ++vtypen) {
				var batchfound = 0;
				var vtype = fin.definition.a(6, vtypen);
				if (not vtype) {
					goto nextvtype;
				}

				//skip if not required daybook style
				if (validaccs) {
					if (not(validaccs.a(1).locateusing(fin.definition.a(1, vtypen), VM, temp))) {
						goto nextvtype;
					}
				}

				//stock vouchers only
				if (reqledgertype == "STOCK") {
					if (not(fin.definition.a(1, vtypen).match("\"ST\"1N"))) {
						goto nextvtype;
					}
				}

				//skip if not allowed to access
				if (not(authorised(doctype ^ " ACCESS " ^ (DQ ^ (vtype ^ DQ)), "", ""))) {
					goto nextvtype;
				}

				//skip if not auto numbered
				if (onlyautono and not fin.definition.a(9, vtypen)) {
					goto nextvtype;
				}

				nvtypes2 += 1;
				selection.r(nvtypes2, fin.definition.a(5, vtypen) ^ VM ^ vtype);
				selection.r(nvtypes2, 9, vtype ^ "*" ^ reqcompany);

				if (showvouchers) {
					//READV LAST.VOUCHER.NO FROM VOUCHERS,'%':VTYPE:'*':CURR.COMPANY:'%',1 ELSE LAST.VOUCHER.NO=''
					if (not(lastvoucherno.readv(fin.vouchers, "%" ^ vtype ^ "*" ^ reqcompany ^ "%", 1))) {
						lastvoucherno = "";
					}
					if (lastvoucherno > 1) {
						selection.r(nvtypes2, 3, lastvoucherno - 1);
					}else{
						if (onlyvouchers) {
badvtype:
							selection.r(nvtypes2, "");
							nvtypes2 -= 1;
							goto nextvtype;
						}
					}
				}

				if (showbatches) {
					//READV LAST.BATCH.NO FROM BATCHES,'%':CURR.COMPANY:'*':VTYPE:SUFFIX:'%',1 THEN
					if (lastbatchno.readv(fin.batches, "%" ^ reqcompany ^ "*" ^ vtype ^ suffix ^ "%", 1)) {
nextbatch:
						//READO TEMP FROM BATCHES,CURR.COMPANY:'*':VTYPE:'*':LAST.BATCH.NO:SUFFIX THEN
						if (temp.read(fin.batches, reqcompany ^ "*" ^ vtype ^ "*" ^ lastbatchno ^ suffix)) {
							lastbatchno += 1;
						}else{
						}else{
							lastbatchno = "";
						}
						if (lastbatchno > 1) {
							selection.r(nvtypes2, 4, lastbatchno - 1);
						}else{
							if (onlybatches) {
								goto badvtype;
							}
						}
						//READV first.BATCH.NO FROM BATCHES,'%':CURR.COMPANY:'*':VTYPE:SUFFIX:'%*',1 THEN
						if (firstbatchno.readv(fin.batches, "%" ^ reqcompany ^ "*" ^ vtype ^ suffix ^ "%*", 1)) {

							//pick up any earlier ones
							if (suffix) {
								while (true) {
									//t=CURR.COMPANY:'*':vtype:'*':first.batch.no-1:SUFFIX
									var tt = reqcompany ^ "*" ^ vtype ^ "*" ^ firstbatchno - 1 ^ suffix;
									if (not(tt.read(fin.batches, tt))) {
										tt = "";
									}
								///BREAK;
								if (not tt) break;;
									firstbatchno -= 1;
								}//loop;
							}

							if (firstbatchno >= lastbatchno) {
								goto badvtype;
							}

							selection.r(nvtypes2, 3, firstbatchno);
							selection.r(nvtypes2, 5, reqcompany);
							selection.r(nvtypes2, 9, vtype ^ "*" ^ reqcompany);

						}
					}

nextvtype:;
				};//vtypen;
			};//companyn;

			if (not nvtypes2) {

				if (mode.index("UNPOSTED2")) {
					return "";
				}

				//print char(7):
				if ((validaccs == "JO") and onlyautono) {
					msg = "There are no journal type daybooks with|auto numbering that you can access";
				}else{
					msg = "There are no suitable items|that you can access";
				}
				call mssg(msg);
				ANS = "";
				return 0;
			}

			if (mode.index("UNPOSTED2")) {
				for (var ii = 1; ii <= 9999; ++ii) {
				///BREAK;
				if (not(selection.a(ii))) break;;
					selection.r(ii, selection.a(ii, 2));
				};//ii;
				ANS = selection;
				return 0;
			}

			var options = "1:40:L\\2:5:L";
			if (showbatches) {
				options ^= "\\4:10:R::Last batch";
			}
			if (showvouchers) {
				options ^= "\\3:12:R::Last voucher";
			}
			if (suffix) {
				options ^= "\\5:7:L::Company";
			}
			var question = "Which voucher type";
			if (multi) {
				question ^= "(s)";
			}
			question ^= " do you want ?|";
			if (suffix) {
				question ^= "(Unposted batches)";
			}
			if (selection[-1] == FM) {
				selection.splicer(-1, 1, "");
			}
//WARNING TODO: check trigraph following;
			var returncoln = suffix ? var(9) : var(2);

			if (nvtypes2 == 1) {
				ANS = selection.a(1, returncoln);
			}else{
				var yy = CRTHIGH / 2 - selection.count(FM) / 2 - 4;
				if (yy < 0) {
					yy = 0;
				}
				var ans = pop_up(5, yy, "", selection, options, "R", multi, question, "", "", "", "P");
				for (var ii = 1; ii <= 9999; ++ii) {
					var optionn = ans.a(ii);
				///BREAK;
				if (not optionn) break;;
					ans.r(ii, selection.a(optionn, returncoln));
				};//ii;

				ANS = ans;
			}
			return 0;
		}

	/////////////////////////////////////////////////////////
	// ACCOUNT mode - first ask if the account number is known
	/////////////////////////////////////////////////////////
		if (mode == "ACCOUNT.OR.MANY") {
			many = 1;
			mode = "ACCOUNT";
		}else{
			many = 0;
		}
		if (mode == "ACCOUNT") {
			ANS = validaccs;
inponeacc:
			valid = 1;
			msg = "Which " ^ accstk;
			if (many) {
				msg ^= "(s)";
			}
			msg ^= " do you want ?||Give the " ^ accstk2 ^ " or name,|or press [Enter] if not known.|(separate multiple entries with spaces)";
			if (interactive) {
				ANS = "";
				call note(msg, "RCE", ANS, "");
			}else{
				if (inpaccnos.unassigned()) {
					inpaccnos = "";
				}
				ANS = inpaccnos;
			}
			if (ANS == var().chr(27)) {
msgexit:
				ANS = "";
				return 0;
			}
			if (ANS == "") {
				ANS = "";
				mode = "ACCOUNTSX";
			}else{
				var replies = ANS.trim();
				replies.converter(" ", FM);

				var nreplies = replies.count(FM) + 1;
				for (var replyn = 1; replyn <= nreplies; ++replyn) {
					reply = replies.a(replyn);

					temp = reply.field(",", 2);
					if (temp) {
						var temp2;
						if (not(temp2.read(gen.companies, temp))) {
							msg = DQ ^ (temp ^ DQ) ^ " - COMPANY CODE DOES NOT EXIST";
							gosub msg();
							if (interactive) {
								goto inponeacc;
							}
							goto msgexit;
						}
						//curr.company=temp
						call initcompany(temp);
					}

					//bill of materials explosion
					var bomfile;
					if (bomfile.open("BOM", "")) {
						var bom;
						if (bom.read(bomfile, reply.a(1, 1, 1).oconv("[ACNO]"))) {
							if (not(bom.a(1))) {
								temp = bom.a(3);
								temp.converter(VM, FM);
								replies.inserter(replyn + 1, temp);
								nreplies = replies.count(FM) + 1;
							}
						}
					}

					//if req.ledger.type='STOCK' then t='STOCK' else t=''
					//if req.ledger.type='JOB' then t='JOB'
					var tt = reqledgertype;
//WARNING TODO: check trigraph following;
					var t2 = many ? var("MANY") : var("");
					if (not(valacc(t2, reply, reply, tt, msg, acctype))) {
						if (msg) {
							//print char(7):
							call mssg(msg);
						}
						if (interactive) {
							goto inponeacc;
						}
						goto msgexit;
					}

					replies.r(replyn, reply.a(1, 1, 2));

				};//replyn;

				replies.converter(TM, FM);

				//check security
				nreplies = replies.count(FM) + 1;
				for (var replyn = 1; replyn <= nreplies; ++replyn) {
					var acno = replies.a(replyn);

					if (fin.account.read(fin.accounts, acno)) {

						//check allowed to access the ledger
						if (not(authorised(positive ^ ledgerorbalances ^ " ACCESS " ^ (DQ ^ (fin.account.a(2) ^ DQ)), msg, ""))) {
							//print char(7):
							call mssg(msg);
							if (interactive) {
								goto inponeacc;
							}
							goto msgexit;
						}

						//check authorisation for the account
						if (not(authorised("!" ^ ledgerorbalances ^ " ACCESS ACCOUNT " ^ (DQ ^ (reply.a(1, 1, 1) ^ DQ)), msg, ""))) {
							//print char(7):
							call mssg(msg);
							var().stop();
						}

					}

				};//replyn;

				ANS = replies;
				goto returnans;
			}
		}

	//accounts specified
	///////////////////
		if (validaccs and validaccs.substr(1,5) ne "CHART") {
			//select from a list of valid accounts
			var nvalidaccs = validaccs.count(FM) + 1;

			//remove accounts for the wrong company
			if (reqcompanies or acctype) {
				for (var ii = 1; ii <= nvalidaccs; ++ii) {
					if (fin.account.read(fin.accounts, "." ^ validaccs.a(ii))) {

						if (reqcompanies) {
							if (fin.account.a(5)) {
								if (not(reqcompanies.a(1).locateusing(fin.account.a(5), VM, temp))) {
									goto badaccount;
								}
							}
						}

						if (acctype ne "") {
							if (acctype == "-") {
								if (fin.account.a(6)) {
badaccount:
									validaccs.eraser(ii);
									nvalidaccs -= 1;
									ii -= 1;
								}
							}else{
								if (fin.account.a(8)) {
									if (not(acctype.a(1).locateusing(fin.account.a(8), VM, temp))) {
										goto badaccount;
									}
								}
							}
						}
					}
				};//ii;
				if (not validaccs) {
					goto nosuitable;
				}
			}
			var listofaccs = validaccs;
			goto selacc;
		}

	//ledgers specified
	//////////////////
		var rejects = "";
		if (validaccs.index("#")) {
			validaccs.transfer(rejects);
			rejects.converter("#", "");
		}
		if (validaccs) {
			var ledgercodes = validaccs.substr(7,9999);
			var nledgers = ledgercodes.count(FM) + (ledgercodes ne "");

			var ans = "";

			//remove ledgers for wrong company or account type
			//check security always
			if ((var(1) or reqcompanies) or acctype ne "") {
				for (var ii = 1; ii <= nledgers; ++ii) {
					var possibleledgercode = ledgercodes.a(ii);

					//remove ledgers not allowed access to
					if (not(authorised(positive ^ ledgerorbalances ^ " ACCESS " ^ (DQ ^ (possibleledgercode ^ DQ)), "", ""))) {
						goto badledger;
					}

					var ledger;
					if (not(ledger.read(fin.ledgers, possibleledgercode))) {
						goto badledger;
					}

					if (reqcompanies) {
						if (ledger.a(6)) {
							if (not(reqcompanies.a(1).locateusing(ledger.a(6), VM, temp))) {
								goto badledger;
							}
						}
					}

					if (acctype ne "") {

						if (acctype == "-") {
							if (ledger.a(2) ne "") {
badledger:
								ledgercodes.eraser(ii);
								nledgers -= 1;
								ii -= 1;
							}else{
							}else{
								if ((acctype == "B") and (ledger.a(9) == "")) {
									ledger.r(9, "O");
								}
								if (ledger.a(9)) {
									if (not(acctype.a(1).locateusing(ledger.a(9), VM, temp))) {
										goto badledger;
									}
								}
							}

						}

						if (not interactive) {
							ledger.r(2, ledger.a(2, 1, 1));
							//convert \FEFDFC\ to \FDFCFB\ in ledger
							ledger = lower(ledger);
							//ans<i>=possible.ledger.code:vm:field(ledger,vm,1,10)
							//ans<i>=possible.ledger.code:vm:ledger
							ans.r(-1, possibleledgercode ^ VM ^ ledger);
						}

nextledgerx:;
					};//ii;
					if (not nledgers) {
						goto nosuitable;
					}
				}

				if (not interactive) {
					ans.transfer(ANS);
					return 0;
				}

				//select ledger if more than one
				if (nledgers > 1) {
					//select from a list of charts
					//leave in the order given GOSUB SORT.LEDGER.CODES
					if (mode.index("LEDGERS")) {
						onemany = "O";
						ss = "(es)";
					}else{
						onemany = 0;
						ss = "";
					}
					var params = "1:30:::Ledger";
					params ^= "\\2:35::ACNO2:Control";
					params ^= "\\0:10:::Code";
					params ^= "\\6:8:::Company";
					params ^= "\\9:4:::Type";
					var yy = CRTHIGH / 2 - ledgercodes.count(FM) / 2 - 4;
					if (yy < 0) {
						yy = 0;
					}
					ledgercode = pop_up(0, yy, fin.ledgers, ledgercodes, params, "T", onemany, "Which ledger" ^ ss ^ " do you want ?", "", "", "1", "K");
					if (not ledgercode) {
						return 0;
					}
				}else{
					ledgercode = ledgercodes;
				}

			}

	//no ledgers specified
	/////////////////////
			if (not ledgercode) {
				//get ledgers
				ledgercodes = "";
				//SELECT LEDGERS
				call safeselect("SSELECT LEDGERS WITH AUTHORISED BY CONTROL_ACC_LINENO BY CHART_NAME BY CHART_CODE (S)");
				nledgers = 0;
				ans = "";
nextledgercode:
				if (readnext(ledgercode)) {

					//skip any ledgers specifically rejected
					if (rejects) {
						if (rejects.locateusing(ledgercode, FM, temp)) {
							goto nextledgercode;
						}
					}

					if (ledgercode[1] == "%") {
						goto nextledgercode;
					}
					//remove ledgers not allowed access to
					//done in select now
					//IF SECURITY(positive:ledgerorbalances:' ACCESS ':QUOTE(LEDGER.CODE),'','') ELSE GOTO NEXTLEDGERCODE

					var ledger;
					if (ledger.read(fin.ledgers, ledgercode)) {

						var ledgertype = ledger.a(5).field(";", 2);

						//allow stock ledgers if they have a control account
						//IF LEDGER<2> NE '' AND REQ.LEDGER.TYPE EQ '' AND LEDGER.TYPE EQ 'STOCK' THEN GOTO LEDGER.TYPE.OK:

						//remove ledgers of the wrong type
						//(if the ledger type is in brackets is means all EXCEPT that ledger type)
						if (reqledgertype ne "*") {
							if (reqledgertype[1] == "(") {
								if (reqledgertype == ("(" ^ ledgertype ^ ")")) {
									goto nextledgercode;
								}
							}else{
								if (reqledgertype ne ledgertype) {
									goto nextledgercode;
								}
							}
						}
ledgertypeok:

						//remove ledgers of the wrong company
						if (reqcompanies) {
							if (ledger.a(6)) {
								if (not(reqcompanies.a(1).locateusing(ledger.a(6), VM, temp))) {
									goto nextledgercode;
								}
							}
						}

						//remove ledgers of the wrong type (open item/ balance forward)
						if (acctype ne "") {
							if (acctype == "-") {
								if (ledger.a(2)) {
									goto nextledgercode;
								}
							}else{
								if ((acctype == "B") and (ledger.a(9) == "")) {
									ledger.r(9, "O");
								}
								if (ledger.a(9)) {
									if (not(acctype.a(1).locateusing(ledger.a(9), VM, temp))) {
										goto nextledgercode;
									}
								}
							}
						}

						nledgers += 1;
						ledgercodes.r(-1, ledgercode);

						if (not interactive) {
							ledger.r(2, ledger.a(2, 1, 1));
							//convert \FEFDFC\ to \FDFCFB\ in ledger
							ledger = lower(ledger);
							//send everything now
							//ans<nledgers>=ledger.code:vm:field(ledger,vm,1,10)
							ans.r(nledgers, ledgercode ^ VM ^ ledger);
						}

					}

					goto nextledgercode;
				}

				if (not nledgers) {
					goto nosuitable;
				}
				gosub sort.ledger.codes();

				if (not interactive) {
					ans.transfer(ANS);
					return 0;
				}

				if (nledgers == 1) {
					ledgercode = ledgercodes;
				}else{
					if (mode.index("LEDGERS")) {
						onemany = "O";
						ss = "(s)";
					}else{
						onemany = 0;
						ss = "";
					}
					var params = "1:35:::Ledger";
					params ^= "\\2:8::ACNO2:Control";
					params ^= "\\0:10:::Code";
					params ^= "\\6:8:::Company";
					params ^= "\\9:4:::Type";
					var yy = CRTHIGH / 2 - ledgercodes.count(FM) / 2 - 4;
					if (yy < 0) {
						yy = 0;
					}
					ledgercode = pop_up(0, yy, fin.ledgers, ledgercodes, params, "T", onemany, "Which ledger" ^ ss ^ " do you want ?", "", "", "1", "K");
					if (not ledgercode) {
						return 0;
					}
				}
			}

			if (mode.index("LEDGER")) {
				ANS = ledgercode;
				goto returnans;
			}

	//get the chart
	//////////////
			ANS = "";
//WARNING TODO: check trigraph following;
			if (not((*temp*).matread(fin.charts, ledgercode))) {
				msg = DQ ^ (ledgercode ^ DQ) ^ " CHART IS MISSING";
				gosub msg();
				return 0;
			}

			//review required ledger type
			temp = chart(5).field(";", 2);
			if (temp) {
				reqledgertype = temp;
				gosub get.acctype.desc();
			}

	//select from chart
	//////////////////
			if (mode.substr(1,5) == "CHART") {
				if (mode == "CHARTPART") {
					reply = 4;
				}else{
					if (not(decide(chart(1) ^ "||Do you want all " ^ accstk ^ "s ?", "All" _VM_ "Selected" _VM_ "Range (From-To)", reply))) {
						ANS = "";
						return 0;
					}
				}
				var onemulti = "O";
				if (reply == 1) {
					ANS = ledgercode;
				}else{
					gosub getchart();
selchart:
					if (reply == 2) {
						temp = "Which " ^ accstk ^ "(s) do you want ?";
					}else{
						if (reply == 3) {
							temp = "Please choose the first line that you want,|and the last line if you do not want the whole group.";
						}else{
							temp = "Which part do you want ?   (choose a heading line)";
							onemulti = "";
						}
					}
					var yy = CRTHIGH / 2 - selection.count(FM) / 2 - 4;
					if (yy < 0) {
						yy = 0;
					}
					ANS = pop_up(0, yy, "", selection, "1:40:L\\2:5:L\\3:12:L", "R", onemulti, temp ^ "|" ^ chart(1) ^ "|", "", "", "", "P");
					if (not ANS) {
						return 0;
					}

					//generate range of line numbers
					if (reply == 3) {
						temp = ANS.count(FM) + 1;
						if (temp > 2) {
							msg = "PLEASE SELECT ONE OR TWO LINES ONLY.";
							gosub msg();
							goto selchart;
						}
						var start = ANS.a(1);
						var finish = ANS.a(2);

						if (finish) {
							if (start > finish) {
								temp = finish;
								finish = start;
								start = temp;
							}
							ANS = start;
							start += 1;
							for (temp = start; temp <= finish; ++temp) {
								ANS ^= FM ^ temp;
							};//temp;
						}else{
							var nlines = chart(3).count(VM) + 1;
							temp = chart(3).a(1, start);
							var LEVEL = temp.length() - temp.trimf().length();
							ANS = start;
							start += 1;
							for (var finish = start; finish <= nlines; ++finish) {
								temp = chart(3).a(1, finish, 1);
								var temp2 = temp;
								//not now may non latin instead of ibm
								temp2.converter("-=._~ ", "");
								if (temp2) {
									newlevel = temp.length() - temp.trimf().length();
								}else{
									newlevel = 9999;
								}
							///BREAK;
							if (not(newlevel > LEVEL)) break;;
								ANS ^= FM ^ finish;
							};//finish;
						}
					}

					ANS.splicer(1, 0, ledgercode ^ FM);
				}
				return 0;
			}

			//job accounts - select one from chart with headings
			if (mode == "ACCOUNTX") {
				gosub getchart();
				temp = "Which " ^ accstk ^ " do you want ?";
				var yy = CRTHIGH / 2 - selection.count(FM) / 2 - 4;
				if (yy < 0) {
					yy = 0;
				}
				ANS = pop_up(0, yy, "", selection, "1:40:L\\2:5:L\\3:12:L", "R", "", temp ^ "|" ^ chart(1) ^ "|", "", "", "3", "");
				return 0;
			}

	//select an account
	//////////////////

			var listofaccs = chart(4);
			while (true) {
			///BREAK;
			if (not(listofaccs.index(VM ^ VM))) break;;
				listofaccs.swapper(VM ^ VM, VM);
			}//loop;
			while (true) {
			///BREAK;
			if (not(listofaccs[1] == VM)) break;;
				listofaccs.splicer(1, 1, "");
			}//loop;

selacc:
			if (mode.index("ACCOUNTS")) {
				onemany = "O";
				ss = "(s)";
			}else{
				onemany = 0;
				ss = "";
			}
			if (not listofaccs) {
				goto nosuitable;
			}
			if (mode == "ACCOUNTSX") {
				reply = 1;
				if (not(decide("Which accounts do you want ?", "All" _VM_ "Selected", reply))) {
					return 0;
				}
				if (reply == 1) {
					ANS = listofaccs;
					goto returnans;
				}
			}
			var yy = CRTHIGH / 2 - listofaccs.count(VM) / 2 - 4;
			if (yy < 1) {
				yy = 1;
			}
			if (interactive) {
				ANS = pop_up(1, yy, fin.accounts, listofaccs, "1:45\\10:10\\4:3", "T", onemany, "Which " ^ accstk ^ ss ^ " do you want ?|" ^ chart(1) ^ "|", "", "", "", "K");
			}else{
				ANS = "ACCOUNTSEARCH " ^ listofaccs;
			}

returnans:
			ANS.converter(FM, VM);
			return 0;

//////////////////
sortledgercodes:
//////////////////
	// NLEDGER.CODES=COUNT(LEDGER.CODES,FM)+1
	// NEW.LEDGER.CODES=''
	// FOR I=1 TO NLEDGER.CODES
	// TEMP=LEDGER.CODES<I>
	// LOCATE TEMP IN NEW.LEDGER.CODES BY 'AL' USING FM SETTING FN ELSE
	// NEW.LEDGER.CODES=INSERT(NEW.LEDGER.CODES,FN,0,0,TEMP)
	// END
	// NEXT I
	// LEDGER.CODES=NEW.LEDGER.CODES
	// NLEDGER.CODES=COUNT(LEDGER.CODES,FM)+1
			return 0;

}

subroutine getchart() {
			call note("Getting chart.|Please wait ...", "UB", buffer, "");
			selection = chart(3);
			if (selection.index(SVM)) {
				var nn = selection.count(VM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					temp = selection.a(1, ii);
					if (temp.index(SVM)) {
						selection.r(1, ii, temp.a(1, 1, 1));
					}
				};//ii;
			}
			var sep = (var().chr(240) ^ VM).str(selection.count(VM) + 1);
			selection = selection  sep  chart(7)  sep  chart(10);
			selection.converter(var().chr(240) ^ VM, VM ^ FM);
			selection.splicer(-1, 1, "");
			call mssg("", "DB", buffer, "");
			return;

///////////
nosuitable:
///////////
			msg = "There are no suitable";
			if (acctype == "O") {
				msg ^= " Open Item";
			}
			if (acctype == "B") {
				msg ^= " Balance Forward";
			}
			if (acctype == "-") {
				msg ^= " General";
			}
			if (mode) {
				temp = mode;
			}else{
				temp = accstk ^ "s";
			}
			if (temp == "ACCOUNTX") {
				temp = "account";
			}
			//CONVERT @LOWER.CASE TO @UPPER.CASE IN TEMP
			temp.lcaser();
			if (temp[-1] ne "s") {
				temp ^= "s";
			}
			msg ^= " " ^ temp;
			msg ^= "|that you can access.";
			temp = reqcompanies;
			temp.swapper(VM, "\", \"");
			if (reqcompanies) {
				msg ^= "|for company " ^ (DQ ^ (temp ^ DQ));
			}
			gosub msg();
			ANS = "";
			return;

}

subroutine msg() {
			//print char(7):
			call mssg(msg);
			return;

/////////////////
getacctypedesc:
/////////////////
			if (reqledgertype == "STOCK") {
				accstk = "stock item";
				accstk2 = "stock code";
			} else if (reqledgertype == "JOB") {
				accstk = "job account";
				accstk2 = "job number";
			} else {
				accstk = "account";
				accstk2 = "account number";
			}
			return;

}


libraryexit()
