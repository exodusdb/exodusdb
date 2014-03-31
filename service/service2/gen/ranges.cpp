#include <exodus/library.h>
libraryinit()


var basekey;
var lists;
var listn;//num
var keys;
var key;

function main(io ranges0, in basekey0, in listid="") {
	//jbase

	if (basekey0.unassigned()) {
		basekey = "";
	}else{
		basekey = basekey0;
	}

	if (listid) {
		var lists;
		if (not(lists.open("LISTS", ""))) {
			call fsmsg();
			var().stop();
		}
		listn = "";
		keys = "";
	}

	var ranges = ranges0.trim();
	//convert ',/' to ' ' in ranges
	//dont convert / since an invoice number may contain a slash
	ranges.converter(",", " ");
	ranges.trimmer();
	ranges.converter(" ", FM);
	ranges.swapper(" -", "-");
	ranges.swapper("- ", "-");

	var nn = ranges.count(FM) + (ranges ne "");

	var allchars = "";
	for (var ii = 32; ii <= 255; ++ii) {
		allchars ^= var().chr(ii);
	};//ii;
	allchars.converter("0123456789-", "");

	for (var ii = nn; ii >= 1; --ii) {

		var rangex = ranges.a(ii);

		//if enter something like P13/1000-2000 then
		//prefix=P13/ start=1000 stop=2000
		//ie TRAILING number range but wont work if trailing year like 1000/13
		//startx=field(temp,'-',1)
		var startx = "";
		var prefix = rangex.field("-", 1);
		while (true) {
			var lastchar = prefix[-1];
		///BREAK;
		if (not(var("0123456789").index(lastchar, 1))) break;;
			startx.splicer(1, 0, lastchar);
			prefix.splicer(-1, 1, "");
		}//loop;

		//range of numbers
		var temp0 = rangex;
		var temp = temp0;
		temp.converter(allchars, "");

		if (temp.match("1N0N\"-\"1N0N")) {

			//finish=field(temp,'-',2)
			var finish = "";
			var tt = ranges.a(ii).field("-", 2);
			while (true) {
				var lastchar = tt[-1];
			///BREAK;
			if (not(var("0123456789").index(lastchar, 1))) break;;
				finish.splicer(1, 0, lastchar);
				tt.splicer(-1, 1, "");
			}//loop;

			if (finish < startx) {
				ranges0 = "";
				call mssg(startx ^ "-" ^ finish ^ " range is not consecutive");
				return 0;
			}
			var range = "";
			for (var i2 = startx; i2 <= finish; ++i2) {
				if (esctoexit()) {
					goto exit;
				}

				if (basekey and not prefix) {
					key = basekey;
					key.swapper("%", i2);
				}else{
					key = prefix ^ i2;
				}

				if (listid) {
					keys ^= FM ^ key;
					//assumes key is not too long
					if (keys.length() > 32000) {
						gosub writelist(listid);
					}

				}else{
					if (range.length() + ranges.length() > 65000) {
						//call msg('Range exceeds maximum of 65,000 characters|Please enter a smaller range')
						call mssg("Range exceeds maximum|Please enter a smaller range");
						ranges = "";
						goto exit;
					}
					range ^= " " ^ key;
				}

			};//i2;

			if (not listid) {
				range.splicer(1, 1, "");
				ranges.r(ii, range);
			}

			//not range
		}else{
			if (listid) {

				if (basekey and not prefix) {
makekey:
					key = basekey;
					key.swapper("%", temp);
				} else if (basekey) {
					temp = rangex;
					goto makekey;
				}else{
					//key=prefix:i2
					key = rangex;
				}


				keys ^= FM ^ key;
			}

		}

	};//ii;

exit:
	/////
	if (listid) {
		if (keys) {
			//return nlistparts in vnos to indicate success
			var vnos = listn;
			if (not vnos) {
				vnos = 1;
			}
			gosub writelist(listid);
		}else{
			//return '' in vnos if none found (unlikely)
			var vnos = "";
		}
	}else{
		ranges0 = ranges;
		ranges0.converter(FM, " ");
	}

	return 0;

}

subroutine writelist(in listid) {
	if (not keys.length()) {
		return;
	}
	keys.splicer(1, 1, "");
	keys.write(lists, listid ^ listn);
	keys = "";
	listn += 1;
	return;

}


libraryexit()
