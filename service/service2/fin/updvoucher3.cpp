#include <exodus/library.h>
libraryinit()


#include <gen.h>

var ii;
var voucher;

function main(in mode, in vouchers, in voucherorfield, in voucherkey, in fieldno=0) {
	//c fin in,in,in,in,0

	//only really required if one has not prelocked the voucher to be updated

	//hard core write to vouchers by-passing any indexing
	//which would cause hang if indexed and locked by another user

	//call rtp57(3, '', field2(vouchers,vm,-1), voucherkey, 2, voucher, state)

	var vouchers2 = vouchers;

	//simple method is to delete lease lock record
	var locks;
	if (locks.open("LOCKS", "")) {
		locks.deleterecord("VOUCHERS*" ^ voucherkey);
		

		//otherwise update without updating secondary indexes (not good in all cases)
	}else{
		vouchers2 = vouchers.a(1, 1);

		if (vouchers2.a(1, 1).locateusing("SI.MFS", VM, ii)) {
			vouchers2.eraser(1, 1, ii);
			vouchers2.r(1, -1, field2(vouchers, VM, -1));
		}else{
			vouchers2 = vouchers;
		}

	}

	if (fieldno.unassigned()) {
		fieldno = 0;
	}
	if (fieldno) {
		var voucher;
		if (not(voucher.read(vouchers, voucherkey))) {
			voucher = "";
		}
		voucher.r(fieldno, voucherorfield);
	}else{
		voucher = voucherorfield;
	}

	voucher.write(vouchers2, voucherkey);

	return 0;

	// <vm> <cr> <tm> mean those characters

	/* handle without secondary indexing;
	..\DATA\ADLINEL\ACCOUNTS;
	VOUCHERS;
	GLOBAL;
	SHADOW.MFS<vm>RTP57;
	<cr>0000A6000072..\DATA\ADLINEL\ACCOUNTS\VOUCHERS.LK;
	*/

	/* handle WITH secondary indexing;
	..\DATA\ADLINEL\ACCOUNTS;
	VOUCHERS;
	GLOBAL;
	SI.MFS<vm>SHADOW.MFS<vm>RTP57;
	VOUCHERS<vm>RTP57<tm><cr>0000A9000001..\DATA\ADLINEL\ACCOUNTS\REV20066.LK<vm><cr>0000AC000072;
	*/

}


libraryexit()
