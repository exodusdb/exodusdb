/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/
#ifndef MVWINDOW_H_
#define MVWINDOW_H_

#include <vector>
#include <exodus/exodus.h>
//#include <exodus/mvenvironment.h>

namespace exodus
{

class MvWindow;
#ifdef NEO_MVWINDOW_CPP
	//plus 1 to allow main thread to have its own (0)
	std::vector<MvWindow*> global_wins(NTHREADS+1);
#else
	extern std::vector<MvWindow*> global_wins;
#endif

class MvWindow
{
public:

    MvWindow(MvEnvironment& mv): mv(mv)
    {
        //std::wcout<<L"ctor MvWindow::MvWindow(MvEnvironment& mv1)"<<std::endl;
    }

    ~MvWindow()
    {
        //std::wcout<<L"dtor MvWindow::MvWindow(MvEnvironment& mv1)"<<std::endl;
    }

    MvEnvironment& mv;

	var srcfile;
	var srcdict;
	var xrefwindow;
	var windxy;
	var label;
	var keyfull;
	var prompts;
	var templatex;
	var vspvy;
	var xrverify;
	var softkeys;
	var softdata;
	var amv;
	var amvvars;
	var recalc;
	var detailhelp;
	var joinedlocks;
	var redisplaylist;
	var windowaction;
	var vspvlen;
	var deleterec;
	var idnext;
	var browsenext;
	var saverec;
	var displayaction;
	var recordfound;
	var helpprotect;
	var intkeys;
	var relatedkeys;
	var relateddata;
	var freeform;
	var prec;
	var blabel;
	var preproc;
	var postproc;
	var templatefilename;
	var pseudofile;
	var joinedhandles;
	var browselist;
	var maintkey;
	var browsepopup;
	var xcnt;
	var tablewidth;
	var tutorial;
	var winaccount;
	var upswd;
	var ulvl;
	var invertsexist;
	var wlist;
	var ubrowselist;
	var ringptr;
	var mprmode;
	var delim;
	var vspvdepth;
	var isorig;
	var ymax;
	var stoplist;
	var sigwidth;
	var multiple;
	var reproc;
	var perproc;
	var wc;
	var wstchar;
	var scbdepth;
	var wi;
	var recmap;
	var orec;
	var pkey;
	var xmax;
	var keycnt;
	var keymap;
	var wcnt;
	var si;
	var is;
	var isdflt;
	var tsx;
	var virtualx;
	var wlx;
	var wly;
	var wrx;
	var wry;
	var wdone;
	var wchange;
	var reqfields;
	var reqcnt;
	var reset;
	var lockingon;
	var wlocked;
	var wexitkeys;
	var mainmenu;
	var atrovr;
	var appldomain;
	var joinedkeys;
	var recalccnt;
	var datafile;
	var vwindow;
	var vdx;
	var vdy;
	var vprev;
	var wexplode;
	var valid;
	var forcechange;
	var tableexitmode;
	var scribeflags;
	var exceptkeys;
	var pseudo;
	var labelimage;
	var wenv;
	var winext;
	var ww;
	var vspvx;
	var virtualon;
	var ringnext;
	var tableon;
	var newid;
	var arevstart;
	var datafilenext;
	var newbrowse;
	var newdatafile;
	var crtvx;
	var crtvy;
	var crtvlen;
	var crtvdepth;
	var prompttype;
	var mvnext;
	var curramvgroup;
	var mvx;
	var scribestate;
	var vhead;
	var amvaction;
	var unkeys;
	var labelcnt;
	var registerx[10];

	//var calculate(const var& dictid) const;

    bool security(const var& mode);
    bool security2(const var& mode, const var& op0);
    bool security3(const var& op, const var& op2);
    var securitysubs(const var& mode);
    void securitysubs_changepassx(const var& newpassword, const var& usercode, const var& usern);
    void securitysubs_cleartemp(var& record);

    bool invalidq(const var& msg);
    bool invalid();
    bool invalid(const var&);

    //following cannot be moved to MvEnvironment because used window variables for postread
    //void MvWindow::select2(var& filenamex, var& linkfilename2, var& sortselect, var& dictids, var& options, var& datax, var& response, var& limitfields, var& limitchecks, var& limitvalues);
    void select2(const var& filenamex, const var& linkfilename2, const var& sortselect, const var& dictids, const var& options, var& datax, var& response, const var& limitfields, const var& limitchecks, const var& limitvalues);

};

}

#endif /*MVWINDOW_H_*/
