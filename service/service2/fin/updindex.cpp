#include <exodus/library.h>
libraryinit()

#include <trimvoucherindex.h>

#include <fin.h>

var fn;//num
var oldentryn2;

function main(in postingsindex, in indexkey, in daterefline, in deleting) {
	//c fin
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

		goto 401;
	}
	if (deleting) {

deletehistorical:
	/////////////////
		for (var oitag = 1; oitag <= 999; ++oitag) {
			var oldindexkey = indexkey.fieldstore("*", 4, 1, oitag);
			var oldindex;
			if (not(oldindex.read(fin.voucherindex, oldindexkey))) {
				oldindex = " ";
			}

			//NB historical indexes can be EMPTY until compacted by clearopenitems
		///BREAK;
		if (not(oldindex ne " ")) break;;

			if (oldindex.locateusing(daterefline, FM, oldentryn2)) {

				oldindex.eraser(oldentryn2);

				//DONT delete empty indexes without compacting them
				//otherwise ledger cant know the end of the list *1 *2 etc
				//They will be compacted in clearopenitems
				oldindex.write(fin.voucherindex, oldindexkey);

				//should not occur in more than one historical
				//but no harm except performance (probably rare) to check all historical
				//so dont return immediately after deleting one
				//return

			}

		};//oitag;

	}else{

		var maxindexsize = 65500;

		//sjb2008/2/3 try to intelligently trim index in case clear open item not done
		//but means that open item statements for older period will be wrong
		//since it deletes the oldest 10 by final date of allocation
		//TODO make it delete only items that are still pending some allocation
		if (index.length() > maxindexsize) {
			//trim older items that have been completely allocated
			call trimvoucherindex(index, indexkey, 0);
			//if still not enough size then trim even currently open items
			if (index.length() > maxindexsize) {
				call trimvoucherindex(index, indexkey, 1);
			}
		}

		//sjb2000/02/13 prevent failure if clear open items is never run
		//but means that even recent open item statements may be wrong (old ones will)
		//should never happen since trimvoucherindex called above
		if (index.length() > 65500) {
			//bad - delete oldest three items HOPING that they are fully allocated
			//and allocated off in an old period
			//BUT trimvoucherindex() above should have done this already if it could
			//so there must be too many items pending allocation
			//at least we will not crash
			index = index.field(FM, 4, 99999);
		}

		//do again after possible deletions above!
		if (index.locatebyusing(daterefline, "AL", fn, FM)) {
			{}
		}

		var hexdate = daterefline.substr(1,fin.hexdatesize);
		while (true) {
		///BREAK;
		if (index.a(fn).substr(1,fin.hexdatesize) ne hexdate) break;;
			fn += 1;
		}//loop;

		index.inserter(fn, daterefline);

		index.write(postingsindex, indexkey);

		//delete from historical indexes
		//in case we are posting/reposting ancient posted/unposted vouchers
		goto deletehistorical;

	}
//L401:

	//UNLOCK POSTINGS.INDEX,INDEX.KEY
	return 0;

}


libraryexit()
