// dont use guard since may required multiple times eg in dict_xxxxx.cpp files
//#ifndef SERVICE_SRC_EXO_REQ_COMMON_H_
//#define SERVICE_SRC_EXO_REQ_COMMON_H_

#include <exodus/common.h>

#define req_common_no 1

commoninit(req,req_common_no)

	//not generated automatically - manually add whatever are required
	var srcfile = "";
	var srcdict = "";
	var label = "";
	var templatex = "";
	var amvvars = "";
	var redisplaylist = "";
	var deleterec = "";
	var browsenext = "";
	var saverec = "";
	var displayaction = "";
	var blabel = "";
	var isorig = "";
	var wi = "";
	var orec = "";
	var wcnt = "";
	var si = "";
	var is = "";
	var isdflt = "";
	var reset = "";
	var wlocked = "";
	var datafile = "";
	var valid = "";
	var winext = "";
	var newbrowse = "";
	var curramvgroup = "";
	var mvx = "";
	var amvaction = "";
	var perproc = "";
	var reproc = "";
	var postproc = "";
	var preproc = "";

	// Sadly "Non-static data members can only be initialized with member initializer list or with a default member initializer."
	// dim ww(100);
	// dim registerx(10);
	//
	// Compiles but initializes to 0
	//
	// dim ww{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	// dim registerx{0,0,0,0,0,0,0,0,0,0};
	//
	// Solution is to use default constructor and dimension in a default constructor of this common array
	//
	dim ww;
	dim registerx;
	//
	// Default constructor to properly dimension the dim arrays
	req_common() {
		ww.redim(100);
		registerx.redim(10);
	}

commonexit(req,req_common_no)

//#endif // SERVICE_SRC_EXO_REQ_COMMON_H_
