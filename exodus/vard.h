#ifndef VAR_DB_H
#define VAR_DB_H

// gendoc: var - DB functions

#include <exodus/varb.h>
//#include <exodus/vars.h>
#include <exodus/varo.h>
//#include <exodus/var.h>

namespace exo {

	class rex;
	class var;
	class dim;

class PUBLIC var_db : public var_os {

public:

	// Inherit constructors
	using var_os::var_os;

	// Inherit assignment operators
	using var_os::operator=;

	// Inherit conversion to var;
	using var_os::operator var;

//	using VAR    =       var;
//	using VARREF =       var&;
//	using CVR    = const var&;
//	using TVR    =       var&&;

	using in     = const var_db&;
	using out    =       var_db&;
	using io     =       var_db&;

	///// DATABASE ACCESS:
	/////////////////////

	// obj is dbconn

	// Establish a connection to a database.
	// conninfo: The DB connection string parameters are merged from the following places in descending priority.
	// 1. Provided in connect()'s conninfo argument. See the last option. for the complete list of parameters.
	// 2. Any environment variables EXO_HOST EXO_PORT EXO_USER EXO_DATA EXO_PASS EXO_TIME
	// 3. Any parameters found in a configuration file at ~/.config/exodus/exodus.cfg
	// 4. The default conninfo is "host=127.0.0.1 port=5432 dbname=exodus user=exodus password=somesillysecret connect_timeout=10"
	// Setting environment variable EXO_DBTRACE=1 will cause tracing of DB interface including SQL commands.
	// dbconn[out]: Becomes a reference or handle for future functions that require a connection argument.
	// For all the various DB function calls, the dbconn or operative var can be either:
	// * A DB connection created with dbconnect().
	// * A file var created with open().
	// * Any var. A default connection will be established on the fly.
	//
	// `var dbconn = "exodus";
	//  if (not dbconn.connect("dbname=exodus user=exodus password=somesillysecret")) ...;
	//  // or
	//  if (not connect()) ...
	//  // or
	//  if (not connect("exodus")) ...`
	//
	ND bool connect(in conninfo = "");

	// Attach filename(s) to a specific DB connection.
	// Any following use of the given filename(s) without specifying a connection will be directed to the specified connection until process termination.
	// It is not necessary to attach files before opening them but the act of opening them also attaches them.
	// The files must exist in the specified connection.
	// Attachments can changed by calling attach() or open() on a different connection or they can be removed by calling detach().
	// dbconn: The connection to which the filename(s) should be attached. Defaults to the default connection.
	// filenames: FM separated list.
	// return: False if any filename does not exist and cannot be opened on the given connection. All filenames that can be opened on the connection are attached even if some cannot.
	// Internally, attach merely opens each filename on the given connection causing them to be added to an internal cache.
	//
	// `var dbconn = "exodus";
	//  let filenames = "xo_clients^dict.xo_clients"_var;
	//  if (dbconn.attach(filenames)) ... ok
	//  // or
	//  if (attach(filenames)) ... ok`
	//
	ND bool attach(in filenames) const;

	// Remove files from the internal cache created by previous open() and attach() calls.
	// filenames: FM separated list.
	//
	   void detach(in filenames);

	// Begin a DB transaction.
	// return:
	// * True  * Successfully begun.
	// * False * DB refused. See lasterror() for info.
	//
	// `var dbconn = "exodus";
	//  if (not dbconn.begintrans()) ...
	//  // or
	//  if (not begintrans()) ...`
	//
	ND bool begintrans() const;

	// Check if a DB transaction is in progress.
	// return: True or False.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.statustrans()) ... ok
	//  // or
	//  if (statustrans()) ... ok`
	//
	ND bool statustrans() const;

	// Rollback a DB transaction.
	// return:
	// * True  * Successfully rolled back or there was no transaction in progress.
	// * False * DB refused. See lasterror() for info.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.rollbacktrans()) ... ok
	//  // or
	//  if (rollbacktrans()) ... ok`
	//
	ND bool rollbacktrans() const;

	// Commit a DB transaction.
	// return:
	// * True  * Successfully committed or there was no transaction in progress.
	// * False * Transaction was in error or DB refused. See lasterror() for info.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.committrans()) ... ok
	//  // or
	//  if (committrans()) ... ok`
	//
	ND bool committrans() const;

	// Execute an sql command.
	// return:
	// * True  * Successfully executed.
	// * False * DB refused. See lasterror() for info.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.sqlexec("select 1")) ... ok
	//  // or
	//  if (sqlexec("select 1")) ... ok`
	//
	ND bool sqlexec(in sqlcmd) const;

	// Execute an SQL command and capture the response.
	// return:
	// * True  * Successfully executed.
	// * False * DB refused. See lasterror() for info.
	// response: Any rows and columns returned are separated by RM and FM respectively. The first row is the column names.
	// It is *ecommended* that you do not use sql directly unless you must, perhaps to manage or configure a database.
	//
	// `var dbconn = "exodus";
	//  let sqlcmd = "select 'xxx' as col1, 'yyy' as col2";
	//  var response;
	//  if (dbconn.sqlexec(sqlcmd, response)) ... ok // response -> "col1^col2\x1fxxx^yyy"_var /// \x1f is the Record Mark (RM) char. The backtick char is used here by gendoc to deliminate source code.
	//  // or
	//  if (sqlexec(sqlcmd, response)) ... ok`
	//
	ND bool sqlexec(in sqlcmd, io response) const;

	// Close a DB connection
	// Free process resources both locally and in the database server.
	//
	// `var dbconn = "exodus";
	//  dbconn.disconnect();
	//  // or
	//  disconnect();`
	//
	   void disconnect();

	// Close all DB connections
	// Free process resources both locally and in the database server(s).
	// All connections are closed automatically when a process terminates.
	//
	// `var dbconn = "exodus";
	//  dbconn.disconnectall();
	//  // or
	//  disconnectall();`
	//
	   void disconnectall();

	///// DATABASE MANAGEMENT:
	/////////////////////////

	// obj is dbconn

	// Create a named database on a particular connection.
	// Optionally copies an existing database from the same connection
	// return: True or False. See lasterror() for errors.
	// * Target database must not already exist.
	// * Source database must exist on the same connection.
	// * Source database cannot have any current connections.
	//
	// `var dbconn = "exodus";
    //  if (not dbdelete("xo_gendoc_testdb")) {}; // Cleanup first
	//  if (dbconn.dbcreate("xo_gendoc_testdb")) ... ok
	//  // or
	//  if (dbcreate("xo_gendoc_testdb")) ...`
	//
	ND bool dbcreate(in new_dbname, in old_dbname = "") const;

	// Create a named database as a copy of an existing database.
	// return: True or False. See lasterror() for errors.
	// * Target database must not already exist.
	// * Source database must exist on the same connection.
	// * Source database cannot have any current connections.
	//
	// `var dbconn = "exodus";
    //  if (not dbdelete("xo_gendoc_testdb2")) {}; // Cleanup first
	//  if (dbconn.dbcopy("xo_gendoc_testdb", "xo_gendoc_testdb2")) ... ok
	//  // or
	//  if (dbcopy("xo_gendoc_testdb", "xo_gendoc_testdb2")) ...`
	//
	ND bool dbcopy(in from_dbname, in to_dbname) const;

	// Get a list of available databases.
	// A list for a given connection or the default connection.
	// return: An FM delimited list.
	//
	// `var dbconn = "exodus";
	//  let v1 = dbconn.dblist();
	//  // or
	//  let v2 = dblist();`
	//
	ND var  dblist() const;

	// Delete (drop) a named database.
	// return: True or False. See lasterror() for errors.
	// * Database does not exist
	// * Database has active connections.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.dbdelete("xo_gendoc_testdb2")) ... ok
	//  // or
	//  if (dbdelete("xo_gendoc_testdb2")) ...`
	//
	ND bool dbdelete(in dbname) const;

	// Create a named DB file.
	// filenames ending with "_temp" only last until the connection is closed.
	// return: True or False. See lasterror() for errors.
	// * Filename is invalid.
	// * Filename already exists.
	//
	// `let filename = "xo_gendoc_temp", dbconn = "exodus";
	//  if (dbconn.createfile(filename)) ... ok
	//  // or
	//  if (createfile(filename)) ...`
	//
	ND bool createfile(in filename) const;

	// Rename a DB file.
	// return: True or False. See lasterror() for errors.
	// * Source filename does not exist.
	// * New filename is invalid.
	// * New filename already exists.
	//
	// `let dbconn = "exodus", filename = "xo_gendoc_temp", new_filename = "xo_gendoc_temp2";
	//  if (dbconn.renamefile(filename, new_filename)) ... ok
	//  // or
	//  if (renamefile(filename, new_filename)) ...`
	//
	ND bool renamefile(in filename, in newfilename) const;

	// Get a list of all files in a database.
	// return: An FM separated list.
	//
	// `var dbconn = "exodus";
	//  if (not dbconn.listfiles()) ...
	//  // or
	//  if (not listfiles()) ...`
	//
	ND var  listfiles() const;

	// Delete all records in a DB file.
	// return: True or False. See lasterror() for errors.
	// * File name does not exist.
	//
	// `let dbconn = "exodus", filename = "xo_gendoc_temp2";
	//  if (not dbconn.clearfile(filename)) ...
	//  // or
	//  if (not clearfile(filename)) ...`
	//
	ND bool clearfile(in filename) const;

	// Delete a DB file.
	// return: True or False. See lasterror() for errors.
	// * File name does not exist.
	//
	// `let dbconn = "exodus", filename = "xo_gendoc_temp2";
	//  if (dbconn.deletefile(filename)) ... ok
	//  // or
	//  if (deletefile(filename)) ...`
	//
	ND bool deletefile(in filename) const;

	// obj is conn_or_file

	// Get the approx number of records in a DB file.
	// Might return -1 if not known.
	// Not very accurate inside transactions.
	// return: An approximate number.
	//
	// `let dbconn = "exodus", filename = "xo_clients";
	//  var nrecs1 = dbconn.reccount(filename);
	//  // or
	//  var nrecs2 = reccount(filename);`
	//
	ND var  reccount(in filename = "") const;

	// Call the DB maintenance function.
	// For one file or all files.
	// Ensure that reccount() function is reasonably accurate.
	// Despite the name, this doesnt flush any index.
	// return: True or False. See lasterror() for errors.
	// * Cannot perform maintenance while a transaction is active.
	//
	   bool flushindex(in filename = "") const;

	///// DATABASE FILE I/O:
	///////////////////////

	// obj is file

	// Open a DB file.
	// To a var which can be used in subsequent DB function calls to access a specific file using a specific connection.
	// dbconn: If dbconn is *not* specified, and the filename is present in an internal cache of filenames and connections created by previous calls to open() or attach() then open() returns true. If it is not present in the cache then the default connection will be checked.
	// return: See lasterror().
	// * True  * The filename was present in the cache OR the DB connection reports that the file is present.
	// * False * The DB connection reports that the file does not exist.
	//
	// `var file, filename = "xo_clients";
	//  if (not file.open(filename)) ...
	//  // or
	//  if (not open(filename to file)) ...`
	//
	ND bool open(in dbfilename, in dbconn = "");

	// Close a DB file.
	// Does nothing currently since database file vars consume no resources
	//
	// `var file = "xo_clients";
	//  file.close();
	//  // or
	//  close(file);`
	//
	   void close() const;

	// Create a secondary index.
	// For a given DB file and field name.
	// The fieldname must exist in a dictionary file. The default dictionary is "dict." ^ filename.
	// return: False if the index cannot be created for any reason. See lasterror().
	// * Index already exists
	// * File does not exist
	// * The dictionary file does not have a record with a key of the given field name.
	// * The dictionary file does not exist. Default is "dict." ^ filename.
	// * The dictionary field defines a calculated field that uses an exodus function. Using a psql function is OK.
	//
	// `var filename = "xo_clients", fieldname = "DATE_CREATED";
	//  if (not deleteindex("xo_clients", "DATE_CREATED")) {}; // Cleanup first
	//  if (filename.createindex(fieldname)) ... ok
	//  // or
	//  if (createindex(filename, fieldname)) ...`
	//
	ND bool createindex(in fieldname, in dictfile = "") const;

	// List secondary indexes.
	// In a database or for a DB file.
	// return: An FM separated list.
	// * DB file or fieldname does not exist.
	// obj is file|dbconn
	//
	// `var dbconn = "exodus";
	//  if (dbconn.listindex()) ... ok // include "xo_clients__date_created"
	//  // or
	//  if (listindex()) ... ok`
	//
	ND var  listindex(in file_or_filename = "", in fieldname = "") const;

	// Delete a secondary index.
	// For a DB file and field name.
	// return: False if the index cannot be deleted for any reason. See lasterror().
	// * File does not exist
	// * Index does not already exists
	//
	// `var file = "xo_clients", fieldname = "DATE_CREATED";
	//  if (file.deleteindex(fieldname)) ... ok
	//  // or
	//  if (deleteindex(file, fieldname)) ...`
	//
	ND bool deleteindex(in fieldname) const;

	// Place a metaphorical DB lock.
	// On a particular record given a DB file and key.
	// This is a advisory lock, not a physical lock, since it makes no restriction on the access or modification of data by other connections.
	// Neither the DB file nor the record key need to actually exist since a lock is just a hash of the DB file name and key combined.
	// If another connection attempts to place an identical lock on the same database it will be denied.
	// Locks can be removed by unlock() or unlockall() or will be automatically removed at the end of a transaction or when the connection is closed.
	// If the same process attempts to place an identical lock more than once it may be denied (if not in a transaction) or succeed but be ignored (if in a transaction).
	// Locks can be used to avoid processing a transaction simultaneously with another connection only to have one of them fail due to mutually updating the same records.
	// Returns:
	// * 0  * Failure: Another connection has already placed the same lock.
	// * "" * Failure: The lock has already been placed.
	// * 1  * Success: A new lock has been placed.
	// * 2  * Success: The lock has already been placed and the connection is in a transaction.
	//
	// `var file = "xo_clients", key = "1000";
	//  if (file.lock(key)) ... ok
	//  // or
	//  if (lock(file, key)) ...`
	//
	ND var  lock(in key) const;

	// Remove a DB lock.
	// A lock placed by the lock function.
	// return: True or False. See lasterror().
	// * Lock cannot be removed while in a transaction.
	// * Lock is not present in the connection.
	//
	// `var file = "xo_clients", key = "1000";
	//  if (file.unlock(key)) ... ok
	//  // or
	//  if (unlock(file, key)) ...`
	//
	   bool unlock(in key) const;

	// Remove all DB locks.
	// All locks placed by the lock function in the specified connection.
	// return: True or False. See lasterror().
	// * Locks cannot be removed while in a transaction.
	//
	// `var dbconn = "exodus";
	//  if (not dbconn.unlockall()) ...
	//  // or
	//  if (not unlockall(dbconn)) ...`
	//
	   bool unlockall() const;

	// obj is record

	// Write a record into a DB file.
	// Given a unique primary key, either inserts a new record or updates an existing record.
	// return: Nothing since writes always succeed.
	// throw: VarDBException if the file does not exist.
	// Any memory cached record is deleted.
	//
	// `let record = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
	//  let file = "xo_clients", key = "GD001";
	//  //if (not "xo_clients"_var.deleterecord("GD001")) {}; // Cleanup first
	//  record.write(file, key);
	//  // or
	//  write(record on file, key);`
	//
	   void write(in file, in key) const;

	// Read a record from a DB file.
	// Given a unique primary key.
	// file: A DB filename or a var opened to a DB file.
	// key: The key of the record to be read.
	// return: False if the key doesnt exist
	// var: Contains the record if it exists or is unassigned if not.
	// A special case of the key being "%RECORDS%" results in a fictitious "record" being returned as an FM separated list of all the keys in the DB file up to a maximum size of 4Mib, sorted in natural order.
	//
	// `var record;
	//  let file = "xo_clients", key = "GD001";
	//  if (not record.read(file, key)) ... // record -> "Client GD^G^20855^30000^1001.00^20855.76539"_var
	//  // or
	//  if (not read(record from file, key)) ...`
	//
	ND bool read(in file, in key);

	// Delete a record from a DB file.
	// Given a unique primary key.
	// return: True or False.
	// * Key doesnt exist.
	// Any memory cached record is deleted.
	// obj is file
	// deleterecord(in file), a one argument free function, is available that deletes multiple records using the currently active select list.
	//
	// `let file = "xo_clients", key = "GD001";
	//  if (file.deleterecord(key)) ... ok
	//  // or
	// //if (deleterecord(file, key)) ...`
	//
	   bool deleterecord(in key) const;

	// Insert a new record in a DB file.
	// Given a unique primary key.
	// return: True or False.
	// * Key already exists
	// Any memory cached record is deleted.
	//
	// `let record = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
	//  let file = "xo_clients", key = "GD001";
	//  if (record.insertrecord(file, key)) ... ok
	//  // or
	//  if (insertrecord(record on file, key)) ...`
	//
	ND bool insertrecord(in file, in key) const;

	// Update an existing record in a DB file.
	// Given a unique primary key.
	// return: True or False.
	// * Key does not exists
	// Any memory cached record is deleted.
	//
	// `let record = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
	//  let file = "xo_clients", key = "GD001";
	//  if (not record.updaterecord(file, key)) ...
	//  // or
	//  if (not updaterecord(record on file, key)) ...`
	//
	ND bool updaterecord(in file, in key) const;

	// Update the key of an existing record in a DB file.
	// Given two unique primary keys.
	// return: True or False.
	// * Key does not exists
	// * New key already exists.
	// Any memory cached records of either key are deleted.
	//
	// `let file = "xo_clients", key = "GD001", newkey = "GD002";
	//  if (not file.updatekey(key, newkey)) ...
	//  // or
	//  if (not updatekey(file, newkey, key)) ... // Reverse the above change.`
	//
	ND bool updatekey(in key, in newkey) const;

	// obj is strvar

	// Read a field from a DB file record.
	// Same as read() but only returns a specific field number from the record.
	// fieldno: The field number to return from the DB record.
	// return: A string var.
	//
	// `var field, file = "xo_clients", key = "GD001", fieldno = 2;
	//  if (not field.readf(file, key, fieldno)) ... // field -> "G"
	//  // or
	//  if (not readf(field from file, key, fieldno)) ...`
	//
	ND bool readf(in file, in key, const int fieldno);

	// Write a field to a DB file record.
	// Same as write() but only writes to a specific field number in the record.
	//
	// `var field = "f3", file = "xo_clients", key = "1000", fieldno = 3;
	//  field.writef(file, key, fieldno);
	//  // or
	//  writef(field on file, key, fieldno);`
	//
	   void writef(in file, in key, const int fieldno) const;

	// obj is record

	// Write a record and key into a memory cached "DB file".
	// The actual database file is NOT updated.
	// writec() either updates an existing cache record if the key already exists or otherwise inserts a new record into the cache.
	// It always succeeds so no result code is returned.
	// Neither the DB file nor the record key need to actually exist in the actual DB.
	//
    // `let record = "Client XD^X^20855^30000^1001.00^20855.76539"_var;
    //  let file = "xo_clients", key = "XD001";
	//  record.writec(file, key);
	//  // or
	//  writec(record on file, key);`
	//
	   void writec(in file, in key) const;

	// Read a DB record first looking in a memory cached "DB file".
	// Same as "read() but first reads from a memory cache held per connection.
	// 1. Tries to read from a memory cache. Returns true if successful.
	// 2a. Tries to read from the actual DB file and returns false if unsuccessful.
	// 2b. Writes the record and key to the memory cache and returns true.
	// Cached DB file data lives in exodus process memory and is lost when the process terminates or clearcache() is called.
	//
    // `var record;
    //  let file = "xo_clients", key = "XD001";
	//  if (record.readc(file, key)) ... ok
	//  // or
	//  if (readc(record from file, key)) ... ok
	//
	//  // Verify not in actual database file by using read() not readc()
	//  if (read(record from file, key)) abort("Error: " ^ key ^ " should not be in the actual database file"); // error`
	//
	ND bool readc(in file, in key);

	// obj is dbfile

	// Delete a record from a memory cached "DB file".
	// The actual database file is NOT updated.
	// return: False if the key doesnt exist
	//
	// `var file = "xo_clients", key = "XD001";
	//  if (file.deletec(key)) ... ok
	//  // or
	//  if (deletec(file, key)) ...`
	//
	   bool deletec(in key) const;

	// obj is dbconn

	// Clear the "DB file" memory cache.
	// All cached records for the given connection.
	// All future cache readc() function calls will be forced to obtain records from the actual database and refresh the cache.
	//
	// `let dbconn = "exodus";
	//  dbconn.clearcache();
	//  // or
	// clearcache(dbconn);`
	//
	   void clearcache() const;

	// obj is strvar

	// Read a field given filename, key and field number or name.
	// The xlate ("translate") function is similar to readf() but, when called as an exodus program member function, it can be used efficiently with Exodus file dictionaries using named columns, functions and multivalued data.
	// strvar: The primary key to lookup a field in a given file and field no or field name.
	// filename: The DB file in which to look up data.
	// If var key is multivalued then a multivalued field is returned.
	// fieldno: Which field of the record to return.
	// * nn * Field number nn
	// * 0  * The key.
	// * "" * The whole record.
	// mode: If the record does not exist.
	// * "X" * Returns ""
	// * "C" * Returns the key unconverted.
	//
	// `let key = "SB001";
	//  let client_name = key.xlate("xo_clients", 1, "X").squote(); // "'Client AAA'"
	//  // or
	//  let name_and_type = xlate("xo_clients", key, "NAME_AND_TYPE", "X"); // "Client AAA (A)"`
	//
	ND var  xlate(in filename, in fieldno, const char* mode) const;

	///// DATABASE SORT/SELECT:
	//////////////////////////

	// obj is dbfile

	// Create an active select list of DB keys.
	// The select(command) function searches and orders database records for subsequent processing given an English language-like command.
	// The primary job of a database, beyond mere storage and retrieval of information, is to allow rapid searching and ordering of information on demand.
	// In Exodus, searching and ordering of information is known as "sort/select" and is performed by the select() function.
	// Executing the select() function creates an "active select list" which can then be consumed by the readnext() function.
	// dbfile: A opened database file or file name, or an open connection or an empty var for default connections. Subsequent readnext calls must use the same.
	// sort_select_command: A natural language command using dictionary field names. The command can be blank if a dbfile or filename is given in dbfile or just a file name and all keys will be selected in undefined order.
	// Example: "select xo_clients with type 'B' and with balance ge 100 by type by name"
	// Option: "(R)" appended to the sort_select_command acquires the database records as well.
	// return: True if any records are selected or false if none.
	// throw: VarDBException in case of any syntax error in the command.
	// Active select lists created using var.select()'s member function syntax cannot be consumed by the free function form of readnext() and vice versa.
	//
	// `var clients = "xo_clients";
	//  if (clients.select("with type 'B' and with balance ge 100 by type by name"))
	//      while (clients.readnext(ID))
	//          println("Client code is {}", ID);
	//  // or
	//  if (select("xo_clients with type 'B' and with balance ge 100 by type by name"))
	//      while (readnext(ID))
	//          println("Client code is {}", ID);`
	//
	ND bool select(in sort_select_command = "");

	// Create an active select list from a string of DB keys.
	// Similar to select() but creates the list directly from a var.
	// keys: An FM separated list of keys or key^VM^valueno pairs.
	// return: True if any keys are provided or false if not.
	//
	// `var dbfile = "";
	//  let keys = "A01^B02^C03"_var;
	//  if (dbfile.selectkeys(keys)) ... ok
	//  assert(dbfile.readnext(ID) and ID == "A01");
	//  // or
	//  if (selectkeys(keys)) ... ok
	//  assert(readnext(ID) and ID == "A01");`
	//
	ND bool selectkeys(in keys);

	// Check if a select list is active.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// return: True if a select list is active and false if not.
	// If it returns true then a call to readnext() will return a database record key, otherwise not.
	//
	// `var clients = "xo_clients", key;
	//  if (clients.select()) {
	//      assert(clients.hasnext());
	//  }
	//  // or
	//  if (select("xo_clients")) {
	//      assert(hasnext());
	//  }`
	//
	ND bool hasnext();

	// Acquire and consume one key from an active select list.
	// Each call to readnext consumes one key from the list.
	// Once all the keys in an active select list have been consumed by calls to readnext, the list becomes inactive.
	// See select() for example code.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// key[out]: Returns the first (next) key present in an active select list or "" if no select list is active.
	// return: True if a list was active and a key was acquired, false if not.
	//
	ND bool readnext(out key);

	// Acquire and consume one key and valueno pair from an active select list.
	// Similar to readnext(key) but multivalued.
	// If the active list was ordered by multivalued database fields then pairs of key and multivalue number will be available to the readnext function.
	//
	ND bool readnext(out key, out valueno);

	// Similar to readnext(key, valueno) but acquire the database record as well.
	// record[out]: Returns the next database record from the select list assuming that the select list was created with the (R) option otherwise "" if not.
	// key[out]: Returns the next database record key in the select list.
	// valueno[out]: The multivalue number if the select list was ordered on multivalued database record fields or 1 if not.
	// return: True if a list was active and a key was acquired, false if not.
	//
	// `var clients = "xo_clients";
	//  if (clients.select("with type 'B' and with balance ge 100 by type by name (R)"))
	//      while (clients.readnext(RECORD, ID, MV))
	//          println("Code is {}, Name is {}", ID, RECORD.f(1));
	//  // or
	//  DICT = "dict.xo_clients";
	//  if (select("xo_clients with type 'B' and with balance ge 100 by type by name (R)"))
	//      while (readnext(RECORD, ID, MV))
	//          println("Code is {}, Name is {}", calculate("CODE"), calculate("NAME"));`
	//
	ND bool readnext(out record, out key, out valueno);

	// Deactivate an active select list.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// return: Nothing
	// Has no effect if no select list is active for dbfile.
	//
	// `var clients = "xo_clients";
	//  clients.clearselect();
	//  if (not clients.hasnext()) ... ok
	//  // or
	//  clearselect();
	//  if (not hasnext()) ... ok`
	//
	   void clearselect();

	// Save an active select list for later retrieval.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// listname: A suitable name that will be required for later retrieval.
	// return: True if saved successfully or false if there was no active list to be saved.
	// Any existing list with the same name will be overwritten.
	// Only the remaining unconsumed part of the active select list is saved.
	// Saved lists are stand-alone and are not tied to specific database files although they usually hold keys related to specific files.
	// Saved lists can be created from one file and used to access another.
	// savelist() merely writes an FM separated string of keys as a record in the "lists" database file using the list name as the key of the record.
	// If a saved list is very long, additional blocks of keys for the same list may be stored with keys like listname*2, listname*3 etc.
	// Select lists saved in the lists database file may be created, deleted and listed like database records in any other database file.
	//
	// `var clients = "xo_clients";
	//  if (clients.select("with type 'B' by name")) {
	//  }
	//  // or
	//  if (select("xo_clients with type 'B' by name")) {
	//      if (savelist("mylist")) ... ok
	//  }`
	//
	   bool savelist(SV listname);

	// Retrieve and reactivate a saved select list.
	// dbfile: A file or connection var to be used by subsequent readnext function calls.
	// listname: The name of an existing list in the "lists" database file, either created by savelist or manually.
	// return: True if the list was successfully retrieved and activated, or false if the list name doesnt exist.
	// Any currently active select list is replaced.
	// Retrieving a list does not delete it and a list can be retrieved more than once until specifically deleted.
	//
	// `var file = "";
	//  if (file.getlist("mylist")) {
	//      while (file.readnext(ID))
	//          println("Key is {}", ID);
	//  }
	//  // or
	//  if (getlist("mylist")) {
	//      while (readnext(ID))
	//          println("Key is {}", ID);
	//  }`
	//
	ND bool getlist(SV listname);

	// Delete a saved select list.
	// dbfile: A file or connection to the desired database.
	// listname: The name of an existing list in the "lists" database file.
	// return: True if successful or false if the list name doesnt exist.
	//
	// `var dbconn = "";
	//  if (dbconn.deletelist("mylist")) ... ok
	//  // or
	//  if (deletelist("mylist")) ...`
	//
	   bool deletelist(SV listname) const;

//	// Create a saved list from a string of keys.
//	// Any existing list with the same name will be overwritten.
//	// keys: An FM separated list of keys or key^VM^valueno pairs.
//	// return: True if successful or false if no keys were provided.
//	// If the listname is empty then selectkeys() is called instead. This is obsolete and deprecated behaviour.
//	//
//	// `var dbconn = ""; let keys = "A01^B02^C03"_var;
//	//  if (dbconn.makelist("mylist", keys)) ... ok
//	//  // or
//	//  if (makelist("mylist", keys)) ... ok`
//	//
//	ND bool makelist(SV listname, in keys);
//
//	// Create an active select list from a database file record.
//	//
//	// `var lists = "lists";
//	//  if (lists.formlist("mylist")) ... ok
//	//  while (lists.readnext(ID))
//	//      println("Key is {}", ID);
//	//  // or
//	//  if (formlist("lists", "mylist")) ... ok
//	//  while (readnext(ID))
//	//      println("Key is {}, MV is {}", ID, MV);`
//	//
//	ND bool formlist(in keys, const int fieldno = 0);

private:
	ND bool dbcursorexists(); //database, not terminal
	ND bool selectx(in fieldnames, in sortselectclause);

}; // class var_db

} // namespace exo

#endif // VAR_DB_H
