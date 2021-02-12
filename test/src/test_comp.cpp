#include <exodus/program.h>
#include <cassert>

programinit()

//#include <l2.h>

function main() {

	var bigs="10000000000000";
	var bigi=100000000;
	var bigd=1.0e13;
	//printl(bigs);
	//printl(bigi);
	//printl(bigd);

	//var lits ="0.0000000000001";
	var lits ="0.0000000001";
	//var liti =1e-14;
	//var litd =1e-13;
	var litd =1e-10;
	//printl(lits);
	//printl(litd);

	//var tinys="0.00000000000001";
	var tinys="0.00000000001";
	//var tinyi=1e-14;
	var tinyd=1e-14;
	//printl(tinys);
	//printl(tinyd);

	var zeros="0";
	var zeroi=0;
	var zerod=0.0;
	//printl(zeros);
	//printl(zeroi);
	//printl(zerod);


	//big <

	assert(!(bigs<bigs));
	assert(bigs>bigi);
	assert(!(bigs>bigd));

	assert(bigi<bigs);
	assert(!(bigi<bigi));
	assert(bigi<bigd);

	assert(!(bigd<bigs));
	assert(bigd>bigi);
	assert(!(bigd>bigd));

	assert(bigs>lits);
	assert(bigs>litd);

	assert(bigs>tinys);
	assert(bigs>tinyd);



	assert(bigs==bigs);
	assert(bigs!=bigi);
	assert(bigs==bigd);

	assert(bigi!=bigs);
	assert(bigi==bigi);
	assert(bigi!=bigd);

	assert(bigd==bigs);
	assert(bigd!=bigi);
	assert(bigd==bigd);

	assert(bigs!=lits);
	assert(bigs!=litd);

	assert(bigs!=tinys);
	assert(bigs!=tinyd);


	assert(bigi>lits);
	assert(bigi>litd);

	assert(bigi>tinys);
	assert(bigi>tinyd);

	assert(bigd>lits);
	assert(bigd>litd);
	assert(bigd>tinys);
	assert(bigd>tinyd);

	assert(bigs>zeros);
	assert(bigs>zeroi);
	assert(bigs>zerod);




	assert(bigs>0);
	assert(bigi>0);
	assert(bigd>0);

	assert(lits>0);
	//assert(liti>0);
	assert(litd>0);

	assert(!(tinys>0));
	//assert(!(tinyi>0));
	assert(!(tinyd>0));

	assert(!(zeros>0));
	assert(!(zeroi>0));
	//printl(zerod);
	assert(!(zerod>0));

	assert(0<bigs);
	assert(0<bigi);
	assert(0<bigd);

	assert(0<lits);
	//assert(0<liti);
	assert(0<litd);

	assert(!(0<tinys));
	//assert(0<tinyi);
	assert(!(0<tinyd));

	assert(!(0<zeros));
	assert(!(0<zeroi));
	assert(!(0<zerod));

	assert(bigs!=0);
	assert(bigi!=0);
	assert(bigd!=0);

	assert(lits!=0);
	//assert(liti>0);
	assert(litd!=0);

	assert(tinys==0);
	//assert(!(tinyi>0));
	assert(tinyd==0);

	assert(zeros==0);
	assert(zeroi==0);
	assert(zerod==0);

	assert(0!=bigs);
	assert(0!=bigi);
	assert(0!=bigd);

	assert(0!=lits);
	//assert(0<liti);
	assert(0!=litd);

	assert(0==tinys);
	//assert(0<tinyi);
	assert(0==tinyd);

	assert(0==zeros);
	assert(0==zeroi);
	assert(0==zerod);

	printl("Test passed");

	return 0;
}

programexit()

