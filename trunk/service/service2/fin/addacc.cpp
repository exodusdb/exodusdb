#include <exodus/library.h>
libraryinit()

#include <addcent.h>

#include <fin.h>
#include <gen.h>

var sortkey;
var value;
var indent;
var ledgertype;

var vn;
var accs;
var naccs;
var naccs2;

function main(in mode, io ledgercode, in newaccno0, in newaccount, in sortorder, in params0, out msg) {
	//y2k
	//chartmv.fns must be changed in all places in ABP and ABP2
	var interactive = not SYSTEM.a(33);
	var newaccno = newaccno0.a(1, 1, 1);
	//if newaccno<1,1,2>='' then newaccno<1,1,2>=newaccno<1,1,1>
	var newaccname = newaccount.a(1);
	var params=params0;

	var update = mode == "ADD";
	msg = "";

	//check does not already exist in any ledger
	if (fin.account.read(fin.accounts, "." ^ newaccno)) {
		return 0;
	}
	// msg=''
	//preexist:
	// msg=quote(newaccno):msg:' ACCOUNT NUMBER ALREADY EXISTS'
	// return 0
	// end

	if (fin.account.read(fin.accounts, newaccno)) {
		return 0;
	}
	// msg=' ORIGINAL'
	// goto preexist
	// end

	//make the ledger code
	var origledgercode = ledgercode;
	var origparams = params;
	gosub expandledgercodeparams(ledgercode, params);

	//if chart doesnt exist then walk back through the years
	//zzz should walk back through quarters or periods too but isnt implemented
	//if by period or quarter will work if all periods/quarters exist in one year
	var tt;
	if (not(tt.read(fin.charts, ledgercode))) {
		var targetledgercode = ledgercode;

		//contruct prior year ledgercode
getprevledgercode:
		params.r(1, (addcent(params.a(1)) - 1).substr(-2, 2));
		ledgercode = origledgercode;
		gosub expandledgercodeparams(ledgercode, params);

		//just quit if back to original code after searching backwards through 07-06
		if (ledgercode == targetledgercode) {
			ledgercode = targetledgercode;

		}else{

			//if prior year chart doesnt exist go back to look for another
			var chart;
			if (not(chart.read(fin.charts, ledgercode))) {
				goto getprevledgercode;
			}

			//found previous chart so create new chart

			//change previous year if in chart name
			var tt = chart.a(1);
			tt.swapper(addcent(params.a(1)), addcent(origparams.a(1)));
			tt.swapper(params.a(1), origparams.a(1));
			chart.r(1, tt);

			ledgercode = targetledgercode;
			//zzz should really lock to prevent create unless exclusive
			//but will not really cause a problem probs
			gosub createchart(chart, ledgercode);

		}

	}

	//find the highest numbered ledger within that group
	var chart = "";
	var ledgerno = -1;
	origledgercode = ledgercode;
	var newaccnumber = newaccno;
	if (newaccnumber.match("\"JOB\"0N")) {
		newaccnumber.splicer(1, 3, "");
	}
	if (newaccnumber.match("\"SCH\"0N")) {
		newaccnumber.splicer(1, 3, "");
	}
nextledgerno:
	ledgerno += 1;
	var newledgercode = origledgercode ^ ledgerno.oconv("MD00Z");
	var nextledgerno = ledgerno + 1;
	if (nextledgerno < 2) {
		nextledgerno = 2;
	}
	var nextledgercode = origledgercode ^ nextledgerno;
	if (chart.read(fin.charts, newledgercode)) {

	/*;
			//if numeric account numbers and any account is gt the new account number
			// then this is the ledger
			if num(newaccnumber) then;
				naccs=count(chart<3>,vm)+1;
				for accn=naccs to 1 step -1;
					accno=chart<10,accn>;
					if accno matches '"JOB"0N' then accno[1,3]='';
					if accno matches '"SCH"0N' then accno[1,3]='';
					if accno then;
						if num(accno) and accno>newaccnumber then;
							ledgercode=newledgercode;
							goto foundledger;
							end;
						end;
					next accn;
				end;
	*/

		ledgercode = newledgercode;
		goto nextledgerno;
	}

	//allow for number :1 missing and jump to :2
	if (ledgerno < 2) {
		//ledgerno+=1
		goto nextledgerno;
	}
//foundledger:

	//check that the ledger exists and get the chart
	if (not chart) {
		msg = "Cannot create Account " ^ (DQ ^ (newaccno ^ DQ)) ^ " automatically because|";
		msg ^= "ledger " ^ (DQ ^ (ledgercode ^ DQ)) ^ " must be created first";
		goto exit;
	}

lockit:
	//lock and reread the chart for update
	if (not(lockrecord("CHARTS", fin.charts, ledgercode, 60))) {
		msg = "Cannot create account " ^ (DQ ^ (newaccno ^ DQ)) ^ " automatically because|";
		msg ^= "somebody is updating |" ^ (DQ ^ (chart.a(1) ^ DQ));
		goto exit;
	}
	if (not(chart.read(fin.charts, ledgercode))) {
		chart = "";
	}

	//add new chart if existing is too big

	//if interactive and len(chart)>32000 and newledgercode then
	if (chart.length() > 32000 and newledgercode) {

		//unlock the old chart because we are going to create a new one
		var xx = unlockrecord("CHARTS", fin.charts, ledgercode);

		//switch to the new ledger code
		ledgercode = newledgercode;
		newledgercode = "";

		gosub createchart(chart, ledgercode);

		goto lockit;

	}

	//prevent chartsize exceeding 32KB
	//NB "TOO BIG" is a trigger phrase hard coded in IMPORTX so dont change it
	msg = "THE|" ^ chart.a(1) ^ " (" ^ ledgercode ^ ")|LEDGER IS GETTING TOO BIG";
	msg ^= "||!!! PLEASE OPEN A NEW LEDGER|WITH LEDGER CODE " ^ (DQ ^ (newledgercode ^ DQ)) ^ "|";
	if (chart.length() > 36000) {
		msg.splicer(1, 0, "!CANNOT OPEN NEW ACCOUNT " ^ (DQ ^ (newaccno ^ DQ)) ^ " BECAUSE ");
		msg.swapper("GETTING ", "");
		goto exit;
	}
	//warning if chartsize nearing 32KB
	if (chart.length() > 32000) {
		//if mode='VALIDATE' then call note(msg)
		if (mode == "VALIDATE") {
			goto exit;
		}
	}
	msg = "";

	///
	if (mode == "VALIDATE") {
		goto exit;
	}
	///

	//determine the sort key
	if (sortorder == "NAME") {
		sortkey = newaccname;
	}else{
		sortkey = newaccname;
	}

	//calc the number of lines
	accs = chart.a(4);
	naccs = accs.count(VM) + (accs ne "");
	naccs2 = (chart.a(3)).count(VM) + (chart.a(3) ne "");
	if (naccs2 > naccs) {
		naccs = naccs2;
	}

	//default to the end of the ledger
	vn = naccs + 1;

	//find value number to insert at
	if (sortorder) {

		//insert before the first larger account number or name
		for (var accn = 1; accn <= naccs; ++accn) {

			if (sortorder == "NAME") {
				value = chart.a(3, accn);
			}else{
				value = chart.a(10, accn);
			}

			if (value) {
				if (value.trimf() > sortkey) {

					vn = accn;
					accn = naccs;

					//insert a blank line
					var nn = fin.chartmvfns.dcount(VM);
					for (var ii = 1; ii <= nn; ++ii) {
						var tt = fin.chartmvfns.a(1, ii);
						if (tt) {
							chart.inserter(tt, vn, "");
							}
					};//ii;

				}
			}
		};//accn;

	}

	//update the chart of accounts
	if (vn == 1) {
		indent = "";
	}else{
		//indent like previous line
		var temp = chart.a(3, vn - 1);
		indent = (temp.length() - (temp.trimf()).length()).space();
		//if no account number on previous line then indent by one more
		if (chart.a(4, vn - 1) == "") {
			indent ^= " ";
		}
	}
	chart.r(3, vn, indent ^ newaccname);
	chart.r(4, vn, newaccno);
	chart.r(10, vn, newaccno);
	//terms
	//main currency
	//tax code
	if (newaccount.a(33)) {
		chart.r(33, vn, newaccount.a(33));
	}
	if (newaccount.a(4)) {
		chart.r(7, vn, newaccount.a(4));
	}
	if (newaccount.a(14)) {
		chart.r(14, vn, newaccount.a(14));
	}
	if (update) {
		call cropper(chart);
		chart.write(fin.charts, ledgercode);
	}

	//create the account record
	fin.account = newaccount;
	fin.account.r(2, ledgercode);
	fin.account.r(5, chart.a(6));
	fin.account.r(6, chart.a(2, 1, 2));
	fin.account.r(8, chart.a(9));
	fin.account.r(10, newaccno);
	fin.account.r(23, chart.a(5));
	//account<26>=
	if (update) {
		fin.account.write(fin.accounts, newaccno);
	}

	//create the other account record
	fin.account.r(10, newaccno);
	if (update) {
		fin.account.write(fin.accounts, "." ^ newaccno);
	}

exit:
	/////
	var xx = unlockrecord("CHARTS", fin.charts, ledgercode);

	return 0;

}

subroutine expandledgercodeparams(io ledgercode, in params) {

	//old format numbered params
	for (var ii = 1; ii <= 999; ++ii) {
		var paramno = "%" ^ ii ^ "%";
		var ptr = ledgercode.index(paramno, 1);
	///BREAK;
	if (not ptr) break;;
		ledgercode.splicer(ptr, paramno.length(), params.a(ii));
	};//ii;

	//new style parameter codes
	ledgercode.swapper("<YEAR>", params.a(1));
	ledgercode.swapper("<PERIOD>", params.a(2));
	ledgercode.swapper("<QUARTER>", params.a(3));
	//this to be implemented
	ledgercode.swapper("<COMPANY>", params.a(4));

	return;

}

subroutine createchart(io chart, in ledgercode) {

	//clear the last chart record to make a new chart record
	var nmvfns = fin.chartmvfns.dcount(VM);
	for (var fnn = 1; fnn <= nmvfns; ++fnn) {
		chart.r(fin.chartmvfns.a(1, fnn), "");
	};//fnn;

	chart.write(fin.charts, ledgercode);

	chart.write(fin.ledgers, ledgercode);

	//simulate proper ledger creation
	//storerecord=@record
	//storeorec=orec
	//storeid=@id
	//@record=chart
	//orec=''
	//@id=ledgercode
	//cant call directly since contains common variables unlike this subroutine
	//call ledger.subs('WRITE')
	//@id=storeid
	//orec=storerec
	//@record=storerecord

	var storepseudo = PSEUDO;
	PSEUDO = ledgercode ^ FM ^ chart;
	perform("WINDOWSTUB LEDGER.SUBS PSEUDOWRITE");
	PSEUDO = storepseudo;

	return;

}


libraryexit()
