#include <exodus/library.h>
libraryinit()

//this function replicates the main BFS low level file handler in Advanced Revelation
//where fs contains a list of file handlers to be called sequentially for the file in question
//MFS=modifying file system called before BFS
//BFS=base file system - the last in the chain
//each handler is responsible for removing itself from the front of the fs and handle list and calling the next fs and handler in sequence (if it chooses to do so)
//examples:
//SI.MFS - creates and maintains secondary index files used by select processor - functionality replaced by postgresql indexes
//SHADOW.MFS - keeps a record in the SHADOW file of what records have been updated - used for replication
//RTP57 - BFS raw linear hash database file handler - represented by mvdppostgres.cpp implementation of var

//the chaining of file handlers in not currently implemented in c++
//all file handling commands are hard codes to call postgres in mvdbpostgresql
//since chaining of file handlers was used only for secondary indexing and shadow/replication
//both of which are not required, we can get away with hard coding for postgresql unless and until
//we wish to interactive with alternative database backends or have multiple file handlers called per file i/o

function main(in code, in fs, io handle, in keyorfilename, in fmc, io record, io status) {
	true||code||fs||handle||keyorfilename||fmc||record||status;
	//code 3 write record on file, key
	//code 4 delete key

	//code 22 install/mount

	//TODO implement lock and unlock for listen/listen2
	//these are global locks not related to files
	//code 23 lock key
	//code 24 unlock key

	switch (int(code)) {

	//write record on file,key
	case 3:
		status=record.write(handle,keyorfilename);
		break;

	//delete file,key
	case 4:
		status=handle.deleterecord(keyorfilename);
		break;

	//lock file,key
	case 5:
		status=handle.lock(keyorfilename);
		break;

	//unlock file,key
	case 6:
		status=handle.unlock(keyorfilename);
		break;

	//open filename to filehandle
	case 11:
		status=handle.open(keyorfilename);
		break;

	//global lock
	case 23:
		status=var("").lock(keyorfilename);
		break;

	//global unlock
	case 24:
		status=var("").unlock(keyorfilename);
		break;

	default:
		throw MVException(code ^ " invalid code in rtp57");
		break;

	}
	return 0;
}

libraryexit()

