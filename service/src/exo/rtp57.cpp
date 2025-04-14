#include <exodus/library.h>
libraryinit()

	// this function replicates the main BFS low level file handler in Advanced Revelation
	// where fs contains a list of file handlers to be called sequentially for the file in question
	// MFS=modifying file system called before BFS
	// BFS=base file system - the last in the chain
	// each handler is responsible for removing itself from the front of the fs and handle list and calling the next fs and handler in sequence (if it chooses to do so)
	// examples:
	// SI.MFS - creates and maintains secondary index files used by select processor - functionality replaced by postgresql indexes
	// SHADOW.MFS - keeps a record in the SHADOW file of what records have been updated - used for replication
	// RTP57 - BFS raw linear hash database file handler - represented by mvdppostgres.cpp implementation of var

	// the chaining of file handlers in not currently implemented in c++
	// all file handling commands are hard codes to call postgres in mvdbpostgresql
	// since chaining of file handlers was used only for secondary indexing and shadow/replication
	// both of which are not required, we can get away with hard coding for postgresql unless and until
	// we wish to interactive with alternative database backends or have multiple file handlers called per file i/o

func main(in code, in /*fs*/, io handle, in keyorfilename, in /*fmc*/, io record, io status) {

	// clang-format off

/*
             |CODE|    BFS         |     HANDLE     |    NAME        |    FMC         |    RECORD      |    STATUS      |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
READ.RECORD  |  1 | Passed         | Passed         | Passed         | Unused         | Returned       | Returned       |
             |    | FILE.FS        | FILE.HANDLE    | RECORD.NAME    |                | RECORD         | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
READO.RECORD |  2 | Passed         | Passed         | Passed         | Unused         | Returned       | Returned       |
             |    | FILE.FS        | FILE.HANDLE    | RECORD.NAME    |                | RECORD         | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
WRITE.RECORD |  3 | Passed         | Passed         | Passed         | Unused         | Passed         | Returned       |
             |    | FILE.FS        | FILE.HANDLE    | RECORD.NAME    |                | RECORD         | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
DELETE.RECORD|  4 | Passed         | Passed         | Passed         | Unused         | Unused         | Returned       |
             |    | FILE.FS        | FILE.HANDLE    | RECORD.NAME    |                |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
LOCK.RECORD  |  5 | Passed         | Passed         | Passed         | Passed         | Unused         | Returned       |
             |    | FILE.FS        | FILE.HANDLE    | RECORD.NAME    | LOCK.TYPE      |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
UNLOCK.RECORD|  6 | Passed         | Passed         | Passed         | Passed         | Unused         | Returned       |
             |    | FILE.FS        | FILE.HANDLE    | RECORD.NAME    | LOCK.TYPE      |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
SELECT       |  7 | Passed         | Passed         | Unused         | Returned       | Unused         | Returned       |
             |    | FILE.FS        | FILE.HANDLE    |                | SELECT.POINTER |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
READNEXT     |  8 | Passed         | Passed         | Unused         | Pass/Returned  | Returned       | Returned       |
             |    | FILE.FS        | FILE.HANDLE    |                | SELECT.POINTER | SELECT.LIST    | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
CLEARSELECT  |  9 | Passed         | Passed         | Unused         | Returned       | Unused         | Returned       |
             |    | FILE.FS        | FILE.HANDLE    |                | SELECT.POINTER |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
CLEARFILE    | 10 | Passed         | Passed         | Unused         | Unused         | Unused         | Returned       |
             |    | FILE.FS        | FILE.HANDLE    |                |                |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
OPEN.FILE    | 11 | Passed         | Passed         | Passed         | Unused         | Returned       | Returned       |
             |    | FILE.FS        | MAP.HANDLE     | FILE.NAME      |                | FILE.HANDLE    | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
CREATE.FILE  | 12 | Passed         | Passed         | Passed         | Passed         | Unused         | Returned       |
             |    | FILE.FS        | MAP.HANDLE     | FILE.NAME      | FILE.SPECS     |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
RENAME.FILE  | 13 | Passed         | Passed         | Passed         | Passed         | Unused         | Returned       |
             |    | FILE.FS        | MAP.HANDLE     | NEW.FILE.NAME  | OLD.FILE.NAME  |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
MOVE.FILE    | 14 | Passed         | Passed         | Passed         | Passed         | Passed         | Returned       |
             |    | FILE.FS        | NEW.MAP.HANDLE | NEW.FILE.NAME  | OLD.MAP.HANDLE | OLD.FILE.NAME  | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
DELETE.FILE  | 15 | Passed         | Passed         | Passed         | Unused         | Unused         | Returned       |
             |    | FILE.FS        | MAP.HANDLE     | FILE.NAME      |                |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
OPEN.MEDIA   | 16 | Passed         | Unused         | Passed         | Unused         | Returned       | Returned       |
             |    | MAP.FS         |                | MAP.QUALIFIER  |                | MAP.HANDLE     | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
CREATE.MEDIA | 17 | Passed         | Unused         | Passed         | Unused         | Unused         | Returned       |
             |    | MAP.FS         |                | MAP.QUALIFIER  |                |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
READ.MEDIA   | 18 | Passed         | Passed         | Unused         | Unused         | Returned       | Returned       |
             |    | MAP.FS         | MAP.HANDLE     |                |                | MAP.RECORD     | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
WRITE.MEDIA  | 19 | Passed         | Passed         | Unused         | Unused         | Passed         | Returned       |
             |    | MAP.FS         | MAP.HANDLE     |                |                | MAP.RECORD     | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
UNLOCK.ALL   | 20 | Passed         | Unused         | Unused         | Unused         | Unused         | Returned       |
             |    | FS             |                |                |                |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
FLUSH        | 21 | Passed         | Unused         | Unused         | Unused         | Unused         | Returned       |
             |    | FS             |                |                |                |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
INSTALL      | 22 | Passed         | Unused         | Unused         | Unused         | Unused         | Returned       |
             |    | FS             |                |                |                |                | STATUS         |
-------------|----|----------------|----------------|----------------|----------------|----------------|----------------|
26 PURGE CACHE
28 get nrecs
// clang-format on
*/

	// code 3 write record on file, key
	// code 4 delete key

	// code 22 install/mount

	// TODO implement lock and unlock for listen/listen2
	// these are global locks not related to files
	// code 23 lock key
	// code 24 unlock key

	switch (code.toInt()) {

	// write record on file,key
	case 3:
		record.write(handle, keyorfilename);
		status = 1;
		break;

	// delete file,key
	case 4:
		status = handle.deleterecord(keyorfilename);
		break;

	// lock file,key
	case 5:
		status = handle.lock(keyorfilename);
		break;

	// unlock file,key
	case 6:
		status = handle.unlock(keyorfilename);
		break;

	// open filename to filehandle
	case 11:
		status = handle.open(keyorfilename);
		break;

	case 23:
		status = var("").lock(keyorfilename);
		break;

	case 24:
		status = var("").unlock(keyorfilename);
		break;

	default:
		throw VarError(code ^ " invalid code in rtp57");
		//std::unreachable();
		//break;

	}

	return 0;
}

}; // libraryexit()
