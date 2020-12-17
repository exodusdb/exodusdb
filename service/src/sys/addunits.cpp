#include <exodus/library.h>
libraryinit()

#include <split.h>

var acode;

function main(in a0, io bb, in sep) {
	//c sys in,io,in

	//BP  ADXTAB  sm
	//BP  ANALSCH vm
	//GBP NLIST   vm

	if (a0 == "") {
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
	if (sep == SVM) {
		aa.converter(SVM, VM);
		bb.converter(SVM, VM);
	}

	var na = aa.count(VM) + (aa ne "");
	for (var an = 1; an <= na; ++an) {

		var anum = split(aa.a(1, an), acode);
		var bcode = "";

		var nb = bb.count(VM) + (bb ne "");
		//cause assist c++ decompiler to lift bn out of loop
		var bn = 1;
		for (var bn = 1; bn <= nb; ++bn) {
			var bnum = split(bb.a(1, bn), bcode);
			//call msg(na:' ':nb:' ':an:' ':bn:' ':acode:' ':bcode)
			if (bcode == acode) {
				//garbagecollect;
				if (bnum.length() or anum.length()) {
					var ndecs = anum.field(".", 2).length();
					var bndecs = bnum.field(".", 2).length();
					if (bndecs > ndecs) {
						ndecs = bndecs;
					}
					if (anum.isnum() and bnum.isnum()) {
						bb.r(1, bn, (bnum + anum).oconv("MD" ^ ndecs ^ "0P") ^ acode);
					}
				}else{
					bb.r(1, bn, acode);
				}
			}
		//until bcode=acode
			///BREAK;
			if (bcode >= acode) break;
		};//bn;
		//if bcode<>acode and len(anum) then
		if (bcode ne acode) {
			bb.inserter(1, bn, anum ^ acode);
		}

	};//an;

	//work as if vms
	if (sep == SVM) {
		bb.converter(VM, SVM);
	}

	return 0;
}

libraryexit()
