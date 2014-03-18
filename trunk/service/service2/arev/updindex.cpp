#include <exodus/library.h>
libraryinit()

#include <trimvoucherindex.h>

#include <fin.h>

var fn;//num

function main() {
	//y2k *UPDATE.INDEX:
	//NEEDS DATE/LINE/REF AND KEY
	//LOCK POSTINGS.INDEX,INDEX.KEY ELSE
	// CALL MSG('TRYING TO LOCK POSTING INDEX ':INDEX.KEY,'T1','','')
	// GOTO UPDATE.INDEX
	// END
	var index;
	if (not(index.read(postingsindex, indexkey))) {
		index = "";
	}
	if (index.locatebyusing(daterefline, "AL", fn, FM)) {
		if (deleting) {
			index.eraser(fn);
			if (index.length()) {
				index.write(postingsindex, indexkey);
			}else{
				postingsindex.deleterecord(indexkey);
				}
			}
		}else{

		//sjb2008/2/3 try to intelligently trim index in case clear open item not done
		//but means that open item statements for older period will be wrong
		//since it deletes the oldest 10 by final date of allocation
		if (index.length() > 65500) {
			call trimvoucherindex(index, indexkey);
		}

		//sjb2000/02/13 prevent failure if clear open items is never run
		//but means that even recent open item statements may be wrong (old ones will)
		if (index.length() > 65500) {
			//bad - delete oldest three items ASSUMING that they are fully allocated
			//and allocated off in an old period
			index = index.field(FM, 4, 99999);
			fn -= 3;
		}

		var hexdate = daterefline.substr(1, fin.hexdatesize);
		while (true) {
		///BREAK;
		if ((index.a(fn)).substr(1, fin.hexdatesize) ne hexdate) break;;
			fn += 1;
		}//loop;
		index.inserter(fn, daterefline);
		index.write(postingsindex, indexkey);
	}
	//UNLOCK POSTINGS.INDEX,INDEX.KEY
	return 0;

}


libraryexit()