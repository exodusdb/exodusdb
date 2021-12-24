#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

//exodus/c++ > uses SMALLEST_NUMBER to compare equality
////0.0001 for pick/arev compatibility

programinit()

//#include <l2.h>

function main() {

	TRACE(SMALLEST_NUMBER)

	var bigs="10000000000000";
	var bigi=100000000;
	var bigd=1.0e13;
	//printl(bigs);
	//printl(bigi);
	//printl(bigd);

#define AREV_ROUNDING
#ifdef AREV_ROUNDING

		//var lits ="0.0000000000001";
		var lits ="0.0001";
		//var liti =1e-14;
		//var litd =1e-13;
		var litd =1e-4;
		//printl(lits);
		//printl(litd);

		//var tinys="0.00000000000001";
		var tinys="0.00001";
		//var tinyi=1e-14;
		var tinyd=1e-5;
		//printl(tinys);
		//printl(tinyd);
#else
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
#endif

	var zeros="0";
	var zeroi=0;
	var zerod=0.0;
	//printl(zeros);
	//printl(zeroi);
	//printl(zerod);


	//big <

	assert(not(bigs<bigs));
	assert(bigs>bigi);
	assert(not(bigs>bigd));

	assert(bigi<bigs);
	assert(not(bigi<bigi));
	assert(bigi<bigd);

	assert(not(bigd<bigs));
	assert(bigd>bigi);
	assert(not(bigd>bigd));

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

	assert(not(tinys>0));
	//assert(not(tinyi>0));
	assert(not(tinyd>0));

	assert(not(zeros>0));
	assert(not(zeroi>0));
	//printl(zerod);
	assert(not(zerod>0));

	assert(0<bigs);
	assert(0<bigi);
	assert(0<bigd);

	assert(0<lits);
	//assert(0<liti);
	assert(0<litd);

	assert(not(0<tinys));
	//assert(0<tinyi);
	assert(not(0<tinyd));

	assert(not(0<zeros));
	assert(not(0<zeroi));
	assert(not(0<zerod));

	assert(bigs!=0);
	assert(bigi!=0);
	assert(bigd!=0);

	assert(lits!=0);
	//assert(liti>0);
	assert(litd!=0);

	//checking a floating point number against zero accurately is impossible
	//because the its effective epsilon cannot be calculated because the
	//original numbers that were used to calculate the result and number
	//,and are needed to calculate the effective epsilon - error range,
	// and that is all we have, are no longer available.
	//
	//comparing every numeric calculation's result at the time
	//would not solve this because the result of series of calculations
	//on large numbers may result in zero only at the last stage
	//
	//the result of any floating point calculation is only known to be
	//within a range of +/- epsilon*the size of the arguments
	//so errors multiply and propagate
	//
	//therefore in exodus we pick a mid-range "SMALLEST NUMBER"
	//and use this to check for being zero
	//and MVeq this does special check if either of the arguments are zero
	//
	//floating point errors occur in the decimal digits about 17/18 digits
	//to the right of the largest digit. They do not occur simply at about 17/18
	//digits to the right of the decimal point as naive expectation might assume.

	//bool using_epsilon = true;
	//if (not using_epsilon) {
		assert(tinys==0);
		//assert(not(tinyi>0));
		assert(tinyd==0);
		assert(tinys==0.0);
		//assert(not(tinyi>0));
		assert(tinyd==0.0);
	//}

	//check we can 
	assert(not tinys);
	assert(not tinyd);

	assert(zeros==0);
	assert(zeroi==0);
	assert(zerod==0);

	assert(0!=bigs);
	assert(0!=bigi);
	assert(0!=bigd);

	assert(0!=lits);
	//assert(0<liti);
	assert(0!=litd);

	//if (not using_epsilon) {
		assert(0==tinys);
		//assert(0<tinyi);
		assert(0==tinyd);
		assert(0.0==tinys);
		//assert(0<tinyi);
		assert(0.0==tinyd);
	//}

	assert(0==zeros);
	assert(0==zeroi);
	assert(0==zerod);

	assert(pwr(10,-10)==0);//.0000000001
	assert(pwr(10,-9)==0);//.000000001
	assert(pwr(10,-8)==0);//.00000001
	assert(pwr(10,-7)==0);//.0000001
	assert(pwr(10,-6)==0);//.000001
	assert(pwr(10,-5)==0);//.00001
	assert(pwr(10,-4)!=0);//.0001
	assert(pwr(10,-3)!=0);//.001
	assert(pwr(10,-2)!=0);//.01
	assert(pwr(10,-1)!=0);//.1
	assert(pwr(10,0)!=0);//1
	assert(pwr(10,1)!=0);//10
	assert(pwr(10,2)!=0);//100
	assert(pwr(10,3)!=0);//1000

	assert(pwr(10,-10)==0.0);//.0000000001
	assert(pwr(10,-9)==0.0);//.000000001
	assert(pwr(10,-8)==0.0);//.00000001
	assert(pwr(10,-7)==0.0);//.0000001
	assert(pwr(10,-6)==0.0);//.000001
	assert(pwr(10,-5)==0.0);//.00001
	assert(pwr(10,-4)!=0.0);//.0001
	assert(pwr(10,-3)!=0.0);//.001
	assert(pwr(10,-2)!=0.0);//.01
	assert(pwr(10,-1)!=0.0);//.1
	assert(pwr(10,0)!=0.0);//1
	assert(pwr(10,1)!=0.0);//10
	assert(pwr(10,2)!=0.0);//100
	assert(pwr(10,3)!=0.0);//1000

	assert(pwr(10,-10)==0.0);//.0000000001
	assert(pwr(10,-9)==0.0);//.000000001
	assert(pwr(10,-8)==0.0);//.00000001
	assert(pwr(10,-7)==0.0);//.0000001
	assert(pwr(10,-6)==0.0);//.000001
	assert(pwr(10,-5)==0.0);//.00001
	assert(pwr(10,-4)!=0.0);//.0001
	assert(pwr(10,-3)!=0.0);//.001
	assert(pwr(10,-2)!=0.0);//.01
	assert(pwr(10,-1)!=0.0);//.1
	assert(pwr(10,0)!=0.0);//1
	assert(pwr(10,1)!=0.0);//10
	assert(pwr(10,2)!=0.0);//100
	assert(pwr(10,3)!=0.0);//1000

	assert(var(0.0000)==var(0.00001));
	assert(var(0.0001/10)==var(0.00001));
	assert(var(0.0001/10.0)==var(0.00001));
	assert(var(0.0001)/10==var(0.00001));
	assert(var(0.0001)/10.0==var(0.00001));
	assert(var(0.0001)/var(10)==var(0.00001));
	assert(var(0.0001)/var(10.0)==var(0.00001));

	var t=1/3;
	printl(var(10.0/3));
	printl(var(10.0/3)*3);
	assert(var(10.0/3)*3==10);
	assert(var(10.0/3*3)==10);
	assert(var(10.0)/var(3)*3==10);
	assert(var(10.0)/var(3)*var(3)==10);

	//on exodus/c++ all numbers different by 0.0001 are considered the same
	//whereas on pick/arev they are considered different down to 
	assert((var(0.0001)>0));
	assert((var(1e-4)>0));

	//on arev gt le ge le comparison does not use the same logic as == and bool
	assert(not(var(0.00001)>0));
	assert(not(var(0.000000000000000001)>0));
	assert(not(var(1e-5)>0));
	assert(not(var(1e-18)>0));
	printl("tt", pwr(10.0,-16));
	assert(not(pwr(10.0,-16) > 0));
	assert(not(pwr(10.0,-17) > 0));
	assert(not(pwr(10.0,-18) > 0));

	assert(not(pwr(10.0,-10)>0));//.0000000001
	assert(not(pwr(10.0,-9)>0));//.000000001
	assert(not(pwr(10.0,-8)>0));//.00000001
	assert(not(pwr(10.0,-7)>0));//.0000001
	assert(not(pwr(10.0,-6)>0));//.000001
	assert(not(pwr(10.0,-5)>0));//.00001
	//
	assert((pwr(10.0,-4)>0));//.0001
	assert((pwr(10.0,-3)>0));//.001
	assert((pwr(10.0,-2)>0));//.01
	assert((pwr(10.0,-1)>0));//.1
	assert((pwr(10.0,0)>0));//1
	assert((pwr(10.0,1)>0));//10
	assert((pwr(10.0,2)>0));//100
	assert((pwr(10.0,3)>0));//1000

	printl("Test passed");

	return 0;
}

programexit()

