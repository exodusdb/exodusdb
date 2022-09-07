#include <exodus/library.h>
libraryinit()

#include <gethtml.h>
#include <getmark.h>
#include <getcss.h>
#include <docmods.h>
#include <timedate2.h>

#include <sys_common.h>

var cmdline;//num
var nrows;//num
var ncols;//num
var file;
var dictvoc;
var rowfields;
var nrowfields;
dim rowdict;
dim rowfieldismv;
var rowfn;//num
var rowfield;
var totcol;
var coln;//num
var colorder;
var coldict;
var datadict;
var prefixn;//num
var prefix;
var recn;//num
var tt;
var nmvs;//num
var xx;
var datavals;//num
var nrowvals;//num
var rowvals;
var fieldname;
var nn;
var colvals;
var ncolvals;
var rowvaln;//num
var rowval;
var rown;//num
var msg;
var colvaln;//num
var colval;
var rownx;//num
var oldval;//num
var colconv;
var rowconv;
var printptr;//num
var topmargin;//num
var tx;
var system;
var printfilename;
var html;
var ownprintfile;//num
var ptx_filenamelen;
var ptx_random;
var printfile;
var letterhead;
var pagelns;
var bodyln;//num
var realpagen;//num
var pagen;//num
var newpage;//num
var bottomline;
var printtxmark;
var rfmt;
var head;
var foot;
var ntxlns;//num
var nbodylns;//num
var ptx_temp;
var headx;
var newpagetag;
var ptx_css;
var cssver;
var style;
var stylennx;//num
var htmltitle;
var head_or_foot;
var footx;
var head1;
var optioncharn;//num
var optionchars;
var optionchar;
var newoptions;
var printtx_ii;//num
var spaceoptionsize;

function main(in filename, in rowfields0, in colfield, in datafield, io output, io filterdictid, io filterin, io filterout, io allrowvals, io allcolvals, io prefixes, io prefixmvfn) {
	//c sys in,in,in,in,io,io,io,io,io,io,io,io
	//global all

	//for printtx
	//global html,head,foot,cssver,htmltitle,topmargin,bottomline,tx

	#include <system_common.h>

	//if prefixes then it will not select records but read sequentially
	//prefix*1 prefix*2 etc

	cmdline = 0;
		/*;
		if field(@sentence,' ',1)='CROSSTAB' then;
			cmdline=1;
			//convert ',' to vm in @sentence
			filename=field(@sentence,' ',2);
			rowfields=field(@sentence,' ',3);
			colfield=field(@sentence,' ',4);
			datafield=field(@sentence,' ',5);

			gosub init;
			gosub output;

			stop;
			end;
		*/

	//////
	//init:
	//////
	if ((output.unassigned() or allrowvals.unassigned()) or allcolvals.unassigned()) {
		allcolvals = "";
		allrowvals = "";
		output = "";
	}
	if (prefixes.unassigned()) {
		prefixes = "";
		prefixmvfn = "";
	}

	nrows = allrowvals.count(VM) + (allrowvals ne "");
	ncols = allcolvals.count(VM) + (allcolvals ne "");

	if (filterdictid.unassigned()) {
		filterdictid = "";
	}
	if (filterin.unassigned()) {
		filterin = "";
	}
	if (filterout.unassigned()) {
		filterout = "";
	}

	//pairs=''

	if (not(file.open(filename, ""))) {
		call fsmsg();
		stop();
	}
	if (not(DICT.open("DICT." ^ filename, ""))) {
		call fsmsg();
		stop();
	}
	if (not(dictvoc.open("DICT.voc", ""))) {
		call fsmsg();
		stop();
	}

	//selectcmd='SELECT ':filename
	//selectcmd:=' BY ':colfield

	rowfields = rowfields0;
	rowfields.converter(",", VM);
	//nrowfields=count(rowfields,vm)+1
	nrowfields = rowfields.count(VM) + (rowfields ne "");
	rowdict.redim(20);
	rowfieldismv.redim(20);
	for (rowfn = 1; rowfn <= nrowfields; ++rowfn) {
		rowfield = rowfields.f(1, rowfn);
		//selectcmd:=' BY ':rowfield
		if (not(rowdict(rowfn).read(DICT, rowfield))) {
			if (not(rowdict(rowfn).read(dictvoc, rowfield))) {
				call mssg(rowfield.quote() ^ " row field doesnt exist in " ^ filename);
				stop();
			}
		}
		rowfieldismv(rowfn) = rowdict(rowfn).f(4) ne "S";
		if ((rowfieldismv(rowfn) and not(prefixmvfn)) and rowdict(rowfn).f(1) eq "F") {
			prefixmvfn = rowdict(rowfn).f(2);
		}
	} //rowfn;

	totcol = colfield eq "TOTAL";
	coln = 1;
	if (totcol) {
		colorder = "AR";
	} else {
		if (not(coldict.read(DICT, colfield))) {
			if (not(coldict.read(dictvoc, colfield))) {
				call mssg(colfield.quote() ^ " column field doesnt exist in " ^ filename);
				stop();
			}
		}

		if (coldict.f(9) eq "R") {
			colorder = "AR";
		} else {
			colorder = "AL";
		}

	}

	if (not(datadict.read(DICT, datafield))) {
		if (not(datadict.read(dictvoc, datafield))) {
			call mssg(datafield.quote() ^ " data field doesnt exist in " ^ filename);
			stop();
		}
	}

	if (prefixes) {
		prefixn = 0;
nextprefix:
///////////
		prefixn += 1;
		prefix = prefixes.f(1, prefixn);
		if (not prefix) {
			goto exit;
		}
		recn = 0;
		MV = 0;

	} else {
		if (not LISTACTIVE) {
			select(file);
		}
		//perform selectcmd
	}

nextrecord:
///////////
	if (prefixes) {
		recn += 1;
		MV = 0;
		ID = prefix ^ recn;
	} else {
		if (not(readnext(ID, MV))) {
			goto exit;
		}
	}
	if (not(RECORD.read(file, ID))) {
		if (prefixes) {
			goto nextprefix;
		}
		goto exit;
	}

	if (prefixmvfn) {
		tt = RECORD.f(prefixmvfn);
		nmvs = tt.count(VM) + (tt ne "");
nextmv:
///////
		//if prefixes else goto nextrecord
		MV += 1;
		if (MV gt nmvs) {
			goto nextrecord;
		}
	} else {
		MV = 1;
		nmvs = 1;
	}

	if (filterdictid) {
		if (filterdictid.isnum()) {
			tt = RECORD.f(filterdictid, MV);
		} else {
			tt = calculate(filterdictid);
		}
		if (filterin) {
			if (not(filterin.locate(tt, xx))) {
				goto nextmv;
			}
		}
		if (filterout) {
			if (filterout.locate(tt, xx)) {
				goto nextmv;
			}
		}
	}

	if (datafield.length() eq 0) {
		datavals = 1;
	} else if (datafield.match("^\\d*$")) {
		datavals = RECORD.f(datafield);
	} else {
		datavals = calculate(datafield);
	}

	nrowvals = 0;
	rowvals = "";
	if (nrowfields) {
		for (rowfn = 1; rowfn <= nrowfields; ++rowfn) {
			fieldname = rowfields.f(1, rowfn);
			tt = calculate(fieldname);
			if (fieldname eq "HOUR") {
				tt = ("00" ^ tt).substr(-2, 2);
			}
			nn = tt.count(VM) + (tt ne "");
			if (nn gt nrowvals) {
				nrowvals = nn;
			}
			//rowvals(rowfn) = tt;
			pickreplacer(rowvals, rowfn, tt);
		} //rowfn;
	} else {
		rowvals = "Total";
		nrowvals = 1;
	}

	if (totcol) {
		colvals = "Total";
	} else {
		colvals = calculate(colfield);
		if (colfield eq "HOUR") {
			colvals = ("00" ^ colvals).substr(-2, 2);
		}
	}
	ncolvals = colvals.count(VM) + (colvals ne "");
	for (rowvaln = 1; rowvaln <= nrowvals; ++rowvaln) {

		//build row value (multiple fields sm separated)
		if (nrowfields) {
			rowval = "";
			for (rowfn = 1; rowfn <= nrowfields; ++rowfn) {
				if (rowfieldismv(rowfn)) {
					tt = rowvals.f(rowfn, rowvaln);
				} else {
					tt = rowvals.f(rowfn, 1);
				}
				//rowval(1, 1, rowfn) = tt;
				pickreplacer(rowval, 1, 1, rowfn, tt);
			} //rowfn;
		} else {
			rowval = rowvals;
		}

		//determine which row to add into
		if (not(allrowvals.f(1).locateby("AL", rowval, rown))) {
			if (allrowvals.length() + rowval.length() gt 65000) {
toobig:
				clearselect();
				msg = "Crosstab too complex. Please simplify your request.";
				if (filename eq "STATISTICS") {
					msg(-1) = "Perhaps choose Columns=Date and dont select Session or IP No.";
				}
				call mssg(msg);
				stop();
			}
			nrows += 1;
			allrowvals.inserter(1, rown, rowval);
			output.inserter(rown + 1, "");
		}

		for (colvaln = 1; colvaln <= ncolvals; ++colvaln) {

			//determine which column to add into
			colval = colvals.f(1, colvaln);
			if (not(allcolvals.f(1).locateby(colorder, colval, coln))) {
				ncols += 1;
				if (allcolvals.length() + colval.length() gt 65000) {
					goto toobig;
				}
				allcolvals.inserter(1, coln, colval);

				if (output.length() + nrows * 2 gt 65000) {
					goto toobig;
				}

				output.inserter(1, 1 + coln, colval);
				for (rownx = 2; rownx <= nrows + 1; ++rownx) {
					output.inserter(rownx, 1 + coln, "");
				} //rownx;
			}

			if (output.length() + datavals.length() gt 6500) {
				goto toobig;
			}

			oldval = output.f(rown + 1, coln + 1);
			//output(rown + 1, coln + 1) = oldval + datavals;
			//output(1, 1) = output.f(1, 1) + 1;
			pickreplacer(output, rown + 1, coln + 1, oldval + datavals);
			pickreplacer(output, 1, 1, output.f(1, 1) + 1);

			//total column at the end
			if (not totcol) {
				oldval = output.f(rown + 1, ncols + 2);
				//output(rown + 1, ncols + 2) = oldval + datavals;
				pickreplacer(output, rown + 1, ncols + 2, oldval + datavals);
			}

		} //colvaln;

	} //rowvaln;

	/////////
	//recexit:
	/////////
	//if prefixes then goto nextmv
	goto nextmv;
	//goto nextrecord

/////
exit:
/////
	//format the column title values
	if (not totcol) {
		colconv = coldict.f(7);
		if (colconv) {
			//ncolvals=count(allcolvals,vm)+1
			for (coln = 1; coln <= ncols; ++coln) {
				output(1, 1 + coln) = oconv(output.f(1, 1 + coln), colconv);
			} //coln;
		}
	}

	if (totcol) {
		output(1, 1 + coln) = datadict.f(3);
	} else {
		for (coln = 1; coln <= ncols; ++coln) {
			output(1, 1 + coln) = coldict.f(3) ^ " " ^ output.f(1, 1 + coln);
		} //coln;
		output(1, ncols + 2) = "Total " ^ datadict.f(3);
	}

	for (rown = 1; rown <= nrows; ++rown) {
		if (output.length() + allrowvals.length() gt 65000) {
			goto toobig;
		}
		output(rown + 1, 1) = allrowvals.f(1, rown);
	} //rown;

	//format the row title values
	output(1, 1, 1) = "";
	for (rowfn = 1; rowfn <= nrowfields; ++rowfn) {
		//output<1,1,rowfn>=rowfields<1,rowfn>
		output(1, 1, rowfn) = rowdict(rowfn).f(3);

		rowconv = rowdict(rowfn).f(7);
		if (rowconv) {
			//nrows=count(allrowvals,vm)+(allrowvals<>'')
			for (rown = 1; rown <= nrows; ++rown) {
				output(rown + 1, 1, rowfn) = oconv(output.f(rown + 1, 1, rowfn), rowconv);
			} //rown;
		}

	} //rowfn;

	//convert sm row keys into columns
	output.converter(SM, VM);

	return 0;
	/*;
	///////
	output:
	///////
		//equ output to table
		//equ dedup to 1
		//equ html to 1
		dedup=1;
		html=1;
		common /system/ system;

		nrows=count(output,fm)+1;
		ncols=count(output<1>,vm)+1;

		//de-duplicate
		if dedup then;
			row1=output<nrows>;
			for rown=nrows to 3 step -1;
				row2=row1;
				row1=output<rown-1>;
				replaced=0;
				for coln=1 to nrowfields;
				while row1<1,coln>=row2<1,coln>;
					replaced=1;
					row2<1,coln>='-';
					next coln;
				if replaced then;
					output<rown>=row2;
					end;
				next rown;
			end;

		if html then;
			tr='<TR valign=top>';
			trx='</TR>':crlf;
			th='<TH>';
			thx='</TH>':crlf;
			td='<TD>';
			tdx='</TD>':crlf;
			nbsp='&nbsp;';

			tx='<H1 align=center>EXODUS USAGE STATISTICS</H1>';
			gosub printtx;

			tx='<table border=1 class=exodustable';
			tx:=' CELLSPACING=0 CELLPADDING=2 ALIGN=CENTER';
			tx:=' STYLE="font-size:66%"';
			tx:='>';

			tx:='<thead>';

			for coln=1 to ncols;
				tx:='<col';
				if coln>nrowfields then;
					tx:=' align=right';
					end;
				tx:='>';
				next coln;

			tx:=tr;
			for coln=1 to ncols;
				cell=output<1,coln>;
				if len(cell) else cell=nbsp;
				tx:=th:cell:thx;
				next coln;
			tx:=trx;

			tx:='</thead>';

			for rown=2 to nrows;
				row=output<rown>;
				rowtx='';
				for coln=1 to ncols;
					cell=row<1,coln>;
					if dedup and coln<=nrowfields then;
						if cell ne '-' then;
							rowspan=1;
							for rown2=rown+1 to nrows;
							while output<rown2,coln>='-';
								rowspan+=1;
								next rown;
							rowtx:='<TD';
							if rowspan>1 then;
								rowtx:=' rowspan=':rowspan;
								end;
							rowtx:='>';
							if len(cell) else cell=nbsp;
							rowtx:=cell;
							rowtx:=tdx;
							end;
					end else;
						if len(cell) else cell=nbsp;
						rowtx:=td:cell:tdx;
						end;
					next coln;
				tx:=tr:rowtx:trx;
				if len(tx)>64000 then;
					gosub printtx;
					end;
				next rown;
			tx:='</table>':crlf;

			gosub printtx;

		end else;

			system<3>=1;
			sys2=system<2>;
			sys2[-3,3]='xls';
			system<2>=sys2;
			swap fm with crlf in output;
			swap vm with \09\ in output;
			call oswrite(output,system<2>);

			end;

		stop;
	*/

}

libraryexit()
