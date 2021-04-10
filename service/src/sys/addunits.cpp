#include <exodus/library.h>
libraryinit()

#include <split.h>

var acode;

function main(in a0, io bb, in sep) {
	//c sys in,io,in

	//BP  ADXTAB  sm
	//BP  ANALSCH vm
	//GBP NLIST   vm

	if (a0 eq "") {
		return 0;
	}
	//add a to b
	//a and b can be a mv or sv list of amounts with unit codes eg 200STG]300YEN]100USD
	//b must be ascii alphabetic order

	//quick calc and exit if both are plain numeric
	if (a0.isnum() and bb.isnum()) {
		bb += a0;
		return 0;
	}

	var aa = a0;

	//work as if vms
	if (sep eq SVM) {
		aa.converter(SVM, VM);
		bb.converter(SVM, VM);
	}

	var na = aa.count(VM) + (aa ne "");
	for (var an = 1; an <= na; ++an) {

		var anum = split(aa.a(1, an), acode);
		var bcode = "";

		var nb = bb.count(VM) + (bb ne "");
		//assist ADECOM c++ decompiler to lift bn out of loop
		//bn=1
		//for bn=1 to nb
		var bn = 0;
		while (true) {
			bn += 1;
			///BREAK;
			if (not(bn le nb)) break;
			var bnum = split(bb.a(1, bn), bcode);
			//call msg(na:' ':nb:' ':an:' ':bn:' ':acode:' ':bcode)

			if (bcode eq acode) {
				//garbagecollect;
				if (bnum.length() or anum.length()) {
					var ndecs = anum.field(".", 2).length();
					var bndecs = bnum.field(".", 2).length();
					if (bndecs gt ndecs) {
						ndecs = bndecs;
					}
					if (anum.isnum() and bnum.isnum()) {
						bb.r(1, bn, (bnum + anum).oconv("MD" ^ ndecs ^ "0P") ^ acode);
					}
				} else {
					bb.r(1, bn, acode);
				}
				break;
			}

			//could be faster if input was guaranteed to be in order
			//until bcode>=acode

			//next bn
		}//loop;

		//if bcode<>acode and len(anum) then
		if (bcode ne acode) {
			bb.inserter(1, bn, anum ^ acode);
		}

	} //an;

	//work as if vms
	if (sep eq SVM) {
		bb.converter(VM, SVM);
	}

	return 0;
}

libraryexit()
