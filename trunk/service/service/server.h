/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/
#ifndef SERVER_H_
#define SERVER_H_

#include <exodus/mv.h>
#include <exodus/mvenvironment.h>
#include "mvwindow.h"

namespace exodus
{

	const var fefdfcfb = _FM_ _VM_ _SM_ _TM_;

class Server
{

	var nblocks;

	//would be faster if not a pointer but would require a version of MVProcessdefs to be
	//like mv._USERNAME not mv._username

public:

    //constructor
//    Server():mv(*global_environments[getenvironmentn()]), win(*tss_wins.get()){}
    Server():
	  mv(*global_environments[getenvironmentn()])
		  ,win(*global_wins[getenvironmentn()]
	  ){}

    //holder for pseudo global variables to do with thread
    MvEnvironment& mv;

    //holder for pseudo global variables to do with MvLibs
    MvWindow& win;

    //main procedure
	var run();

private:

	var requestloop();
	var serviceloop();
	var processlink();
	var processrequest();

//	General general;

//	void FileCommuter could be declared "Abtract" with =0 to prevent need to define it
//  in this base class (and therefore force its definition in subclass)
//	virtual void FileCommuter(var& filename, var& mode, var& newfilename, var& routines)=0;
//	virtual void FileCommuter(var& filename, var& mode, var& newfilename, var& routines);

	void gosub_getbakpars();
	void gosub_gettimeouttime();
    void gosub_flagserveractive();
    void gosub_convlogx();
	void gosub_writelogx();
	void gosub_writelogx2();
	void gosub_writelogx3();
	void gosub_getsecurity();
	bool filesecurity(const var& mode);
	void gosub_lock();
	void gosub_unlock();
	void gosub_lockit();
	void gosub_lockit2();
	void gosub_getostime();
	void gosub_unlockit();
	void gosub_properlock();
	void gosub_properunlock();
	void gosub_badfile();
	void gosub_formatresponse();
	void gosub_geterrorresponse();
	void gosub_deleteoldfiles(const var& path,const var& wildcard);

	void gosub_requestinit();
	void gosub_requestexit();
	void gosub_process();
	void gosub_request();
	void gosub_exit();
	void gosub_login();
	void gosub_validate();
	void gosub_respond();
	void select();
	void getindexvalues();

	bool routineexists;

	var datx[4];
    var hexx[256];
	var sleepms;//num
	var origscrn;
	var origattr;
	var locks;
	var logfilename;
	var logfile;
	var logptr;//num
	var xx;
	var logx;
	var timenow;
	var linkfilenames;
	var linkfilename0;
	var charx;
	var buffer;
	var reply;
	var bakreq;
	var lastbakattemptdate;
	var bakdows;
	var minbaktime;
	var maxbaktime;
	var testdata;
	var bakdisk;
	var requeststarttime;//num
	var linkfile1;
	var linkfilename2;
	var invaliduser;
	var nostack;
	var newfilename;
	var ok;
	var createnotallowed;
	var file;
	var lockauthorised;//num
	var printfile;
	var requeststoptime;//num
	var iodatlen;

	var request1;
	var request2;
	var request3;
	var request4;
	var request5;

	var code;
	var response;
	var responsefilename;
	var linkfilename1;
	var linkfilename3;
	var replyfilename;
	var filename;
	var keyx;
	var sessionid;
	var Serverstation;
	var Serverusername;
	var relistlocks;

	var record;
	var state;//num
	var timeouttime;//num
	var lockdurationinmins;//num
	var newsessionid;
	var connection;
	var filename2;
	var msg0;
	var positive;
	//var msg2;
	var timeoutsecs;
	var ostimex;

	var maxstringsize;
	var inblocksize;
	//inblocksize=50000
	var outblocksize;
	var Serverversion;
	var origsentence;
	var md;
	var halt;
	var timex;
	var breaktime;
	var dataset;
	var username;
	var password;
	var firstrequestfieldn;
	var Serverfailure;
	var origprivilege;
	var nrequests;
	var tracing;
	var origbatchmode;

	var renaming;
	var portno;
	var onalertsecs;

	var datasetcode;
	var neopath;
	var globalend;
	var allcols;

	var servername;
	var onserver;
	var waitfor;
	var waitsecs;
	var serverflagfilename;
	var intranet;
	var webpath;
	var inpath;
	var batchmode;
	var ostimenow;//num
	var lockduration;
	var lockkey;
	var lockx;
	//delete files older than ...
	var ageinsecs;
	var lockrec;
	var withlock;
	var updatenotallowed;
	var logpath;
	var onalert;
	var timestarted;

};

}//of namespace exodus

#endif /*Server_H_*/
