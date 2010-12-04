/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/

//main is a fairly extensive testbed of exodus
//main sets up several thread of a server that listens for request (via files), read/write the database or calls libs and responds.

//this program is dependent on boost to do threads - but threads will be added to exodus directly
//exodus is designed to be threadsafe (a few global variables are not threadsafe and need coordinated access)

//#include <unicode/ustring.h>
//"visual leak detector" debugger
//#include <vld.h>

#include <iostream>
#include <vector>

//using namespace std;

//BOOST thread library minimal examples http://www.ddj.com/dept/cpp/184401518
//and boost\boost_1_34_0\libs\thread\example
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

//used to prevent threads overlapping cout operations
boost::mutex io_mutex;
///use like this (is rather slow)
//{
// boost::mutex::scoped_lock lock(io_mutex);
// wcout<<"xyz";
//}

//#include <exodus/mv.h>
//#include <exodus/exodus.h>
#include <exodus/program.h>
#include "server.h"

DLL_PUBLIC
boost::thread_specific_ptr<int> tss_environmentns;

using namespace exodus;

//#include <exodus/mvenvironment.h>
#include "mvwindow.h"

//#include "Definition.h"
//#include "Market.h"

/*
http://www.postgresql.org/docs/8.2/interactive/libpq-pgpass.html
The file .pgpass in a user's home directory or the file referenced by PGPASSFILE can contain passwords to be used if the connection requires a password (and no password has been specified otherwise). On Microsoft Windows the file is named %APPDATA%\postgresql\pgpass.conf (where %APPDATA% refers to the Application Data subdirectory in the user's profile).

This file should contain lines of the following format:

hostname:port:database:username:password
*/

//used to provide thread specific data like global data
//boost::thread_specific_ptr<var> value;
//use like this (ie gets a pointer) is very fast
//value.reset(new var(0)); // initialize the thread's storage
//var& xx = *value.get();
//xx=xx+1;

bool init_thread(const int environmentn)
{
		//set this threads environment number
		//some worker threads might be created with the same environment number
		setenvironmentn(environmentn);

		//create a new environment for the thread
		global_environments[environmentn]=new MvEnvironment;

		//and get a reference to it
		MvEnvironment& env=*global_environments[environmentn];

		//and init it
		if (!env.init(environmentn))
			return false;

		//create a new window common for the thread
		//the window structure needs access to the environment so it is part of the contruction
        //tss_wins.reset(new MvWindow(env));
		global_wins[environmentn]=new MvWindow(env);

        //mvlibs.set("DEFINITIONS",new Definition);
        //mvlibs.set("MARKETS",new Market);		

		return true;

}

class MVThread
{
public:

	//CONSTRUCTOR
    //use a one parameter contructor to pass the thread data in
	MVThread(int environmentn) : environmentn(environmentn)
	{
		boost::mutex::scoped_lock lock(io_mutex);
		exodus::printl("MVThread::ctor ",environmentn);

		setenvironmentn(environmentn);
	}

	//DESTRUCTOR
	virtual ~MVThread()
	{
		boost::mutex::scoped_lock lock(io_mutex);
		printl("MVThread::dtor ",environmentn);
	}

    //MvLib
	//declare and define the MvLib that is the thread
	void operator()()
	{

		{
			boost::mutex::scoped_lock lock(io_mutex);
			printl("MVThread::operator() start ",environmentn);
		}

		init_thread(environmentn);

        Server mvs1;
        mvs1.run();

        /*
    //var aa=environmentn;
		//var bb=environmentn;
		value.reset(new var(0));
		for (int i = 0; i < 1000000; ++i)
		{
			//test thread specific storage speed
			var& xx = *value.get();
			xx++;

			//debug one thread
			//if (i eq 5&&environmentn eq 1) i=i/(i-i);

			//ensure single threaded cout
			//boost::mutex::scoped_lock lock(io_mutex);
			printl("thread=",environmentn,": i=", i," ");

		}
        */
		{
			boost::mutex::scoped_lock lock(io_mutex);
			printl("MVThread::operator() stop ", environmentn);
		}
	};

private:
	int environmentn;
};

/* MVConnection holds and manages a connection to a database
*
*/
class MVConnection
{
private:
};

//programinit()
class ExodusProgram : public ExodusProgramBase {

function xyz(in xyzz)
{
	xyzz(2,2,2).outputl();
	return 1;
}

function accrest() {
        var infilename="\\tapex";//=field(sentence()," ",2);
        if (not osopen(infilename,infilename))
                abort("Cant read "^infilename);

        var fms=FM^VM^SM^TM^STM^SSTM;
        var visibles="^]\???";
        var EOL="\n\r";
        var offset=0;
        var blocksize=50000;
        while (true) {
                var block=osbread(infilename,offset,blocksize);
 //printl(offset," ",len(block));
                if (not len(block))
                        break;
                offset+=blocksize;
                converter(block,fms,visibles);
                swapper(block,IM,EOL);
                print(block);
				var xx;
                inputn(xx,1);
        }
        return 0;
}

//program()

function main()
{

	// cannot connectlocal in main and thread until pipes are numbered
	tss_environmentns.reset(new int(0));
	if (!init_thread(0))
		abort("Couldnt init thread 0");

	/*
	//create a temporary using a constructor and its operator() will be called
	//boost::thread thread1(MVThread(1));
	//boost::thread thread2(MVThread(2));
	//boost::thread thread3(MVThread(2));
	//thread1.join();
	//thread2.join();
	//thread3.join();
	*/
	
	printl("Starting ",NTHREADS," threads");
	boost::thread_group threads;
	for (var ii = 0; ii < NTHREADS; ++ii)
	{
		outputl("Creating thread " ^ ii);
		//start from environment number 1 so that main thread has its own environment
		threads.create_thread(MVThread(ii+1));
	}
	threads.join_all();

    stop("Shutting down ...");
    
	return 0;
}

programexit()