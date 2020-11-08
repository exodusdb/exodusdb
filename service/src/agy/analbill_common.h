//to add new types
//increase maxnlevels
//append to the end of level() newcode() and newdesc(), breaktitles, classif
//recompile everything using this common analbill analbill2
//develop analbill, analbill2, anal, analbill.subs
//add to billinganalysis.js ui popup gtotaloptions

	#define nbudgets 10
	#define suppresstotals PSEUDO.A(12)

	#define null ""

	#define maxbreaklev 11

	#define infopar 1
	#define categorypar 2
	#define activitypar 3
	#define titlepar 4
	#define yearpar 5
	#define periodpar 6
	#define discchargespar 18

	#define colinfo cols(coln,infopar)
	#define colcategory cols(coln,categorypar)
	#define colactivities cols(coln,activitypar)
	#define coltitle cols(coln,titlepar)
	#define colyear cols(coln,yearpar)
	#define colperiods cols(coln,periodpar)

	#define colfn1 cols(coln,10)
	#define colfn2 cols(coln,11)
	#define colcalc cols(coln,12)
	#define colhidden cols(coln,13)
	#define colsplit cols(coln,14)
	#define coldeleted cols(coln,15)
	#define colfn3 cols(coln,16)
	#define colfn4 cols(coln,17)
	#define disccharges cols(coln,discchargespar)

//nb must keep the following three blocks in the same seq as classif variable

	#define byclientgroup level(1)
	#define byclientgroup2 level(2)
	#define byclient level(3)
	#define bybrand level(7)
	#define bymediaorbtl level(8)
	#define bygroup level(9)
	#define bygroupx level(10)
	#define bygroupy level(11)
	#define bygroupz level(12)
	#define bytype level(13)
	#define bysupplier level(14)
	#define bymarket level(15)
	#define byvehicle level(16)
	#define bycompany level(17)
	#define bybrandexecutive level(18)
	#define byproductcategory level(19)
	#define byaccount level(20)
	#define bysuppliergroup level(21)
	#define bysubtype level(22)

	#define newclientgroupcode newcode(1)
	#define newclientgroup2code newcode(2)
	#define newclientcode NEWCODE(3)
	#define newbrandcode1 NEWCODE(4)
	#define newbrandcode2 NEWCODE(5)
	#define newbrandcode3 NEWCODE(6)
	#define newbrandcode NEWCODE(7)
	#define newmediaorbtlcode NEWCODE(8)
	#define newgroupcode newcode(9)
	#define newgroupxcode newcode(10)
	#define newgroupycode newcode(11)
	#define newgroupzcode newcode(12)
	#define newtypecode NEWCODE(13)
	#define newsupplcode NEWCODE(14)
	#define newmarketcode NEWCODE(15)
	#define newvehiclecode NEWCODE(16)
	#define newcompanycode newcode(17)
	#define newbrandexecutivecode newcode(18)
	#define newproductcategorycode newcode(19)
	#define newaccountcode newcode(20)
	#define newsupplgroupcode newcode(21)
	#define newsubtypecode newcode(22)

	#define newclientgroupname newdesc(1)
	#define newclientgroup2name newdesc(2)
	#define newclientname NEWDESC(3)
	#define newbrandname1 NEWDESC(4)
	#define newbrandname2 NEWDESC(5)
	#define newbrandname3 NEWDESC(6)
	#define newbrandname NEWDESC(7)
	#define newmediaorbtldesc NEWDESC(8)
	#define newgroupdesc newdesc(9)
	#define newgroupxdesc newdesc(10)
	#define newgroupydesc newdesc(11)
	#define newgroupzdesc newdesc(12)
	#define newtypedesc NEWDESC(13)
	#define newsupplname NEWDESC(14)
	#define newmarketname NEWDESC(15)
	#define newvehiclename NEWDESC(16)
	#define newcompanyname newdesc(17)
	#define newbrandexecutivename newdesc(18)
	#define newproductcategoryname newdesc(19)
	#define newaccountname newdesc(20)
	#define newsupplgroupname newdesc(21)
	#define newsubtypename newdesc(22)


	interactive = not(SYSTEM.a(33));
	dbug = 0;
	ab.maxnlevels = 22;
	ab.maxncols = 100;
	ab.nparams = 18;
	ab.cols.redim(ab.maxncols, ab.nparams);
	ab.levelx.redim(ab.maxnlevels);
	ab.code.redim(ab.maxnlevels);
	ab.newcode.redim(ab.maxnlevels);
	ab.desc.redim(ab.maxnlevels);
	ab.newdesc.redim(ab.maxnlevels);
	ab.line.redim(ab.maxnlevels);
	ab.ntots.redim(ab.maxnlevels);
	ab.analrec.redim(40);
	nbsp = "&nbsp;";
	tr = "<TR ALIGN=RIGHT>";
	trx = "</TR>";
	mthcolfmt = "[TAGHTML,TD]";
	l33 = "[TAGHTML,TD ALIGN=LEFT]";
	indentsize = 5;
	s33 = "<TD>&nbsp;</TD>";
	td = "<TD>";
	tdx = "</TD>";
	th = "<TH>";
	thx = "</TH>";