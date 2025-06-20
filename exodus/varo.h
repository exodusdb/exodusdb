#ifndef VAR_OS_H
#define VAR_OS_H

// gendoc: var - OS functions

namespace exo {

	class rex;
	class var;
	class dim;

class PUBLIC var_os : public var_stg {

public:

	// Inherit constructors
	using var_stg::var_stg;

	// Inherit assignment operators
	using var_stg::operator=;

	// Inherit conversion to var;
	using var_stg::operator var;

//	using VAR    =       var;
//	using VARREF =       var&;
//	using CVR    = const var&;
//	using TVR    =       var&&;

	using in     = const var_os&;
	using out    =       var_os&;
	using io     =       var_os&;

	// Implicitly convert var_os to var

//	// Type conversion since reference will be optimised away by -O2
//	// Note that most var_base function directly return a var since var's have full functionality
//	// and can be implicitly used as var_base but not vice versa.
//	CONSTEXPR
//	operator var&() &;

	///// OS TIME/DATE:
	//////////////////

	// obj is var()

	// Get the current date in internal format.
	// Internal format is the number of whole days since pick epoch 1967-12-31 00:00:00 UTC. Dates prior to that are numbered negatively.
	// return: A number. e.g. 20821 represents 2025-01-01 00:00:00 UTC for 24 hours.
	//
	// `let today1 = var::date();
	//  // or
	//  let today2 = date();`
	//
	ND static var  date();

	// Get the current time in internal format.
	// Internal time is the number of whole seconds since the last midnight 00:00:00 (UTC).
	// return: A number in the range 0 - 86399 since there are 24*60*60 seconds in a day. e.g. 43200 if time is 12:00:00
	//
	// `let now1 = var::time();
	//  // or
	//  let now2 = time();`
	//
	ND static var  time();

	// Get the current time in high resolution internal format.
	// High resolution internal time is the number of fractional seconds since the last midnight 00:00:00 (UTC).
	// return: A floating point with approx. nanosecond resolution depending on hardware.
	// e.g. 23343.704387955 approx. 06:29:03 UTC
	//
	// `let now1 = var::ostime();
	//  // or
	//  let now2 = ostime();`
	//
	ND static var  ostime();

	// Get the current timestamp in internal format.
	// Internal timestamp is the number of fractional days since pick epoch 1967-12-31 00:00:00 UTC. Negative for dates before.
	// return: A floating point with approx. nanosecond resolution depending on hardware.
	// e.g. Was 20821.99998842593 around 2025-01-01 23:59:59 UTC
	//
	// `let now1 = var::ostimestamp();
	//  // or
	//  let now2 = ostimestamp();`
	//
	ND static var  ostimestamp();

	// Get the timestamp for a given date and time
	// vardate: Internal date from date(), iconv("D") etc.
	// ostime: Internal time from time(), ostime(), iconv("MT") etc.
	// obj is vardate
	//
	// `let idate = iconv("2025-01-01", "D"), itime = iconv("23:59:59", "MT");
	//  let ts1 = idate.ostimestamp(itime); // 20821.99998842593
	//  // or
	//  let ts2 = ostimestamp(idate, itime);`
	//
	ND var  ostimestamp(in ostime) const;

	// Sleep/pause/wait
	// milliseconds: How to long to sleep.
	// Release the processor if not needed for a period of time or a delay is required.
	//
	// `var::ossleep(100); // sleep for 100ms
	//  // or
	//  ossleep(100);`
	//
	   static void ossleep(const int milliseconds);

	// Sleep/pause/wait up for a file system event
	// file_dir_list: An FM delimited list of OS files and/or dirs to monitor.
	// milliseconds: How long to wait. Any terminal input (e.g. a key press) will also terminate the wait.
	// return: An FM array of event information is returned. See below.
	// Multiple events may be captured and are returned in multivalues.
	// obj is file_dir_list
	//
	// `let v1 = ".^/etc/hosts"_var.oswait(100); /// e.g. "IN_CLOSE_WRITE^/etc^hosts^f"_var
	//  // or
	//  let v2 = oswait(".^/etc/hosts"_var, 100);`
	//
	// Returned dynamic array fields:
	// 1. Event type codes
	// 2. dirpaths
	// 3. filenames
	// 4. d=dir, f=file
	//
	// Possible event type codes:
	// * IN_CLOSE_WRITE * A file opened for writing was closed
	// * IN_ACCESS      * Data was read from file
	// * IN_MODIFY      * Data was written to file
	// * IN_ATTRIB      * File attributes changed
	// * IN_CLOSE       * File was closed (read or write)
	// * IN_MOVED_FROM  * File was moved away from watched directory
	// * IN_MOVED_TO    * File was moved into watched directory
	// * IN_MOVE        * File was moved (in or out of directory)
	// * IN_CREATE      * A file was created in the directory
	// * IN_DELETE      * A file was deleted from the directory
	// * IN_DELETE_SELF * Directory or file under observation was deleted
	// * IN_MOVE_SELF   * Directory or file under observation was moved
	//
	   var  oswait(const int milliseconds) const;

	///// OS FILE I/O:
	/////////////////

	// Open an OS file handle.
	// Allow random read and write operations.
	// Open for writing if possible, otherwise read-only.
	// osfilevar[out]: Handle for subsequent osbread() and osbwrite() calls.
	// osfilename: Path and name of an existing OS file.
	// utf8: True (default) removes partial UTF-8 sequences from osbread() ends; false returns raw data.
	// return: True if opened successfully, false if file doesn’t exist or isn’t accessible.
	// obj is osfilevar
	//
	// `let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  if (oswrite("" on osfilename)) ... ok /// Create an empty OS file
	//  var ostempfile;
	//  if (ostempfile.osopen(osfilename)) ... ok
	//  // or
	//  if (osopen(osfilename to ostempfile)) ... ok`
	//
	ND bool osopen(in osfilename, const bool utf8 = true) const;

	// Random write data to an OS file.
	// At a specified position.
	// strvar: Data to write.
	// osfilevar: Handle from osopen() or a path/filename; creates file if offset is 0 and it’s new, fails if offset isn’t 0.
	// offset: [in/out] Start position (0-based); updated to end of written data; -1 appends.
	// return: True if write succeeds, false if file isn’t accessible, updateable, or creatable.
	// obj is strvar
	//
	// `let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  let text = "aaa=123\nbbb=456\n";
	//  var offset = -1; /// -1 means append.
	//  if (text.osbwrite(osfilename, offset)) ... ok // offset -> 16
	//  // or
	//  if (not osbwrite(text on osfilename, offset)) ...`
	//
	ND bool osbwrite(in osfilevar, io offset) const;

	// Random read data from an OS file
	// From a specified position.
	// strvar[out]: Data read.
	// osfilevar: Handle from osopen() or a path/filename.
	// offset: [in/out] Start position (0-based); updated to end of read data.
	// length: Chars to read; with utf8=true (default), may return less to ensure complete UTF-8 code points.
	// return: True if read succeeds, false if file doesn’t exist or isn’t accessible or offset >= file size.
	// obj is strvar
	//
	// `let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  var text, offset = 0;
	//  if (text.osbread(osfilename, offset, 8)) ... ok // text -> "aaa=123\n" // offset -> 8
	//  // or
	//  if (osbread(text from osfilename, offset, 8)) ... ok // text -> "bbb=456\n" // offset -> 16`
	//
	ND bool osbread(in osfilevar, io offset, const int length);

	// Close an osfilevar.
	// Remove an osfilevar handle from the internal memory cache of OS file handles. This frees up both exodus process memory and operating system resources.
	// It is advisable to osclose any file handles after use, regardless of whether they were specifically opened using osopen or not, especially in long running programs. Exodus performs caching of internal OS file handles per thread and OS file. If not closed, then the operating system will probably not flush deleted files from storage until the process is terminated. This can potentially create an memory issue or file system resource issue especially if osopening/osreading/oswriting many perhaps temporary files in a long running process.
	//
	// `var osfilevar; if (osfilevar.osopen(ostempfile())) ... ok
	//  osfilevar.osclose();
	//  // or
	//  osclose(osfilevar);`
	//
	   void osclose() const;

	// obj is strvar

	// Create a complete OS file from a var.
	// strvar: The text or data to be used to create the file.
	// osfilename: Absolute or relative path and filename to be written. Any existing OS file is removed first.
	// codepage: If specified then output is converted from UTF-8 to that codepage before being written. Otherwise no conversion is done.
	// return: True if successful or false if not possible for any reason. e.g. Path is not writeable, permissions etc
	//
	// `let text = "aaa = 123\nbbb = 456";
	//  let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  if (text.oswrite(osfilename)) ... ok
	//  // or
	//  if (oswrite(text on osfilename)) ... ok`
	//
	ND bool oswrite(in osfilename, const char* codepage = "") const;

	// Read a complete OS file into a var.
	// osfilename: Absolute or relative path and filename to be read.
	// codepage: If specified then input is converted from that codepage to UTF-8 after being read. Otherwise no conversion is done.
	// strvar[out]: Is currently set to "" in case of any failure but this is may be changed in a future release to either force var to be unassigned or to leave it untouched. To guarantee future behaviour either add a line 'xxxx.defaulter("")' or set var manually in case osread() returns false. Or use the one argument free function version of osread() which always returns "" in case of failure to read.
	// return: True if successful or false if not possible for any reason. e.g. File doesnt exist, insufficient permissions etc.
	//
	// `var text;
	//  let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  if (text.osread(osfilename)) ... ok // text -> "aaa = 123\nbbb = 456"
	//  // or
	//  if (osread(text from osfilename)) ... ok
	//  let text2 = osread(osfilename);`
	//
	ND bool osread(const char* osfilename, const char* codepage = "");

	// TODO check if it calls osclose on itself in case removing a varfile

	// obj is osfile_or_dirname

	// Rename an OS file or dir.
	// In the OS file system.
	// The source and target must exist in the same storage device.
	// osfile_or_dirname: Absolute or relative path and file or dir name to be renamed.
	// new_dirpath_or_filepath: Will not overwrite an existing OS file or dir.
	// return: True if successful or false if not possible for any reason. e.g. Target already exists, path is not writeable, permissions etc.
	// Uses std::filesystem::rename internally.
	//
	// `let from_osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  let to_osfilename = from_osfilename ^ ".bak";
	//  if (not osremove(ostempdir() ^ "xo_gendoc_test.conf.bak")) {}; // Cleanup first
	//
	//  if (from_osfilename.osrename(to_osfilename)) ... ok
	//  // or
	//  if (osrename(from_osfilename, to_osfilename)) ...`
	//
	ND bool osrename(in new_dirpath_or_filepath) const;

	// "Move" an OS file or dir.
	// Within the OS file system.
	// Attempt osrename first, then oscopy plus osremove original.
	// osfile_or_dirname: Absolute or relative path and file or dir name to be moved.
	// to_osfilename: Will not overwrite an existing OS file or dir.
	// return: True if successful or false if not possible for any reason. e.g. Source doesnt exist or cannot be accessed, target already exists, source or target is not writeable, permissions, storage space etc.
	//
	// `let from_osfilename = ostempdir() ^ "xo_gendoc_test.conf.bak";
	//  let to_osfilename = from_osfilename.cut(-4);
	//
	//  if (not osremove(ostempdir() ^ "xo_gendoc_test.conf")) {}; // Cleanup first
	//  if (from_osfilename.osmove(to_osfilename)) ... ok
	//  // or
	//  if (osmove(from_osfilename, to_osfilename)) ...`
	//
	ND bool osmove(in to_osfilename) const;

	// Copy an OS file or directory.
	// Including subdirs.
	// osfile_or_dirname: Absolute or relative path and file or dir name to be copied.
	// to_osfilename: Will overwrite an existing OS file or merge into an existing dir.
	// return: True if successful or false if not possible for any reason. e.g. Source doesnt exist or cannot be accessed, target is not writeable, permissions, storage space, etc.
	// Uses std::filesystem::copy internally with recursive and overwrite options
	//
	// `let from_osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  let to_osfilename = from_osfilename ^ ".bak";
	//
	//  if (from_osfilename.oscopy(to_osfilename)) ... ok;
	//  // or
	//  if (oscopy(from_osfilename, to_osfilename)) ... ok`
	//
	ND bool oscopy(in to_osfilename) const;

	// Remove/delete an OS file.
	// From the OS file system.
	// Will not remove directories. Use osrmdir() to remove directories
	// osfilename: Absolute or relative path and file name to be removed.
	// return: True if successful or false if not possible for any reason. e.g. Target doesnt exist, path is not writeable, permissions etc.
	// If osfilename is an osfilevar then it is automatically closed.
	// obj is osfilename
	//
	// `let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  if (osfilename.osremove()) ... ok
	//  // or
	//  if (osremove(osfilename)) ...`
	//
	ND bool osremove() const;

	///// OS DIRECTORIES:
	////////////////////

	// List files and directories

	// obj is dirpath

	// Get a list of OS files and/or dirs.
	// dirpath: Absolute or relative dir path.
	// globpattern: e.g. *.conf to be appended to the dirpath or a complete path plus glob pattern e.g. /etc/ *.conf.
	// mode:
	// * 0 * Any regular OS file or dir (Default).
	// * 1 * Only regular OS files.
	// * 2 * Only dirs.
	// return: An FM delimited string containing all matching dir entries given a dir path
	//
	// `var entries1 = "/etc/"_var.oslist("*.cfg"); /// e.g. "adduser.conf^ca-certificates.con^... etc."
	//  // or
	//  var entries2 = oslist("/etc/" "*.conf");`
	ND var  oslist(SV globpattern = "", const int mode = 0) const;

	// Get a list of OS files.
	// See oslist() for info.
	ND var  oslistf(SV globpattern = "") const;

	// Get a list of OS dirs.
	// See oslist() for info.
	ND var  oslistd(SV globpattern = "") const;

	// Get dir info about an OS file or dir.
	// return: A short string containing size ^ FM ^ modified_time ^ FM ^ modified_time or "" if not a regular file or dir.
	// mode: 0: Default. 1: Must be a regular OS file. 2: Must be an OS dir.
	// See also osfile() and osdir()
	// obj is osfile_or_dirpath
	//
	// `var info1 = "/etc/hosts"_var.osinfo(); /// e.g. "221^20597^78309"_var
	//  // or
	//  var info2 = osinfo("/etc/hosts");`
	//
	ND var  osinfo(const int mode = 0) const;

	// Get dir info of an OS file.
	// osfilename: Absolute or relative path and file name.
	// return: A short string containing size ^ FM ^ modified_time ^ FM ^ modified_time or "" if not a regular file.
	// Alias for osinfo(1)
	// obj is osfilename
	//
	// `var fileinfo1 = "/etc/hosts"_var.osfile(); /// e.g. "221^20597^78309"_var
	//  // or
	//  var fileinfo2 = osfile("/etc/hosts");`
	//
	ND var  osfile() const;

	// Get dir info of an OS dir.
	// dirpath: Absolute or relative path and dir name.
	// return: A short string containing FM ^ modified_time ^ FM ^ modified_time or "" if not a dir.
	// Alias for osinfo(2)
	// obj is dirpath
	//
	// `var dirinfo1 = "/etc/"_var.osdir(); /// e.g. "^20848^44464"_var
	//  // or
	//  var dirinfo2 = osfile("/etc/");`
	//
	ND var  osdir() const;

	// Create a new OS file system directory.
	// Parent dirs wil be created if necessary.
	// dirpath: Absolute or relative path and dir name.
	// return: True if successful.
	// obj is dirpath
	//
	// `let osdirname = "xo_test/aaa";
	//  if (osrmdir("xo_test/aaa")) {}; // Cleanup first
	//  if (osdirname.osmkdir()) ... ok
	//  // or
	//  if (osmkdir(osdirname)) ...`
	//
	ND bool osmkdir() const;

	// Change the current working dir.
	// newpath: An absolute or relative dir path and name.
	// return: True if successful or false if not. e.g. Invalid dirpath, insufficient permission etc.
	// obj is var()
	//
	// `let osdirname = "xo_test/aaa";
	//  if (osdirname.oscwd()) ... ok
	//  // or
	//  if (oscwd(osdirname)) ... ok
	//  if (oscwd("../..")) ... ok /// Change back to avoid errors in following code.`
	//
	ND static bool oscwd(SV newpath);

	// Get the current working dir path and name.
	// return: The current working dir path and name.
	// e.g. "/root/exodus/cli/src/xo_test/aaa"
	// obj is var()
	//
	// `var cwd1 = var().oscwd();
	//  // or
	//  var cwd2 = oscwd();`
	//
	ND static var  oscwd();

	// Remove (deletes) an OS dir,
	// eventifnotempty: If true any subdirs will also be removed/deleted recursively, otherwise the function will fail and return false.
	// return: Returns true if successful or false if not. e.g dir doesnt exist, insufficient permission, not empty etc.
	//
	// `let osdirname = "xo_test/aaa";
	//  if (osdirname.osrmdir()) ... ok
	//  // or
	//  if (osrmdir(osdirname)) ...`
	//
	ND bool osrmdir(bool evenifnotempty = false) const;

	///// OS SHELL/ENVIRONMENT:
	//////////////////////////

	// Execute a shell command.
	// command: An executable command to be interpreted by the default OS shell.
	// return: True if the process terminates with error status 0 and false otherwise.
	// Append "&>/dev/null" to the command to suppress terminal output.
	// obj is command
	//
	// `let cmd = "echo $HOME";
	//  if (cmd.osshell()) ... ok
	//  // or
	//  if (osshell(cmd)) ... ok`
	//
	ND bool osshell() const;

	// Execute a shell command and capture its stdout.
	// return: The stout of the shell command.
	// Append "2>&1" to the command to capture stderr/stdlog output as well.
	// obj is instr
	//
	// `let cmd = "echo $HOME";
	//  var text;
	//  if (text.osshellread(cmd)) ... ok
	//
	//  // or capturing stdout but ignoring exit status
	//  text = osshellread(cmd);`
	//
	ND bool osshellread(in oscmd);

	// Execute a shell command and provide its stdin.
	// return: True if the process terminates with error status 0 and false otherwise.
	// Append "&> somefile" to the command to suppress and/or capture output.
	// obj is outstr
	//
	// `let outtext = "abc xyz";
	//  if (outtext.osshellwrite("grep xyz")) ... ok
	//  // or
	//  if (osshellwrite(outtext, "grep xyz")) ... ok`
	//
	ND bool osshellwrite(in oscmd) const;

	// Run an OS program synchronously.
	// Provide its standard input and capture its output, errors, and exit status.
	// Shell features (e.g., pipes, redirects) are unsupported but can be invoked via an oscmd like "bash -c 'abc|yy $HOME'".
	// oscmd: Executable and arguments; must exist in OS PATH.
	// stdin_for_process: Optional; input data for the program’s standard input.
	// stdout_from_process[out]: Standard output from the program.
	// stderr_from_process[out]: Error/log output from the program.
	// exit_status[out]: Program’s exit status: 0 (normal), -1 (timeout), else (error).
	// timeout_secs: Optional; max runtime in seconds (default 0 = no timeout).
	// return: True if program ran and exited with status 0 (success) or false if program failed to start, timed out, or exited with non-zero status.
	// throw: Pipe creation failed, fork failed, poll failed.
	//
	// `var v_stdout, v_stderr, v_exit_status;
	//  if (var::osprocess("grep xyz", "abc\nxyz 123\ndef", v_stdout, v_stderr, v_exit_status)) ... ok // v_stdout -> "xyz 123\n" // v_exit_status = 0
	//  // or
	//  if (osprocess("grep xyz", "abc\nxyz 123\ndef", v_stdout, v_stderr, v_exit_status)) ... ok`
	//
	ND static bool osprocess(in oscmd, in stdin_for_process, out stdout_from_process, out stderr_from_process, out exit_status, in timeout_secs = 0);

	// Get the tmp dir path.
	// return: A string e.g. "/tmp/"
	// obj is var()
	//
	// `let v1 = var::ostempdir();
	//  // or
	//  let v2 = ostempdir();`
	//
	ND static var  ostempdir();

	// Create a temporary file.
	// return: The name of new temporary file e.g. "/tmp/~exoEcLj3C"
	// obj is var()
	//
	// `var temposfilename1 = var::ostempfile();
	//  // or
	//  var temposfilename2 = ostempfile();`
	//
	ND static var  ostempfile();

	// obj is envvalue

	// Set the value of an environment variable.
	// envcode: The code of the env variable to set.
	// envvalue: The new value to set the env code to.
	// obj is envvalue
	//
	// `let envcode = "EXO_ABC", envvalue = "XYZ";
	//  envvalue.ossetenv(envcode);
	//  // or
	//  ossetenv(envcode, envvalue);`
	//
	   void ossetenv(SV envcode) const;

	// Get the value of an environment variable.
	// envcode: The code of the env variable to get or "" for all.
	// envvalue[out]: Set to the value of the env variable if set otherwise "". If envcode is "" then envvalue is set to a dynamic array of all environment variables LIKE CODE1=VALUE1^CODE2=VALUE2...
	// return: True if the envcode is set or false if not.
	// osgetenv and ossetenv work with a per thread copy of the OS process environment. This avoids multithreading issues but does not change the process environment. Child processes created by var::osshell() will not inherit any env variables set using ossetenv() so the oscommand will need to be prefixed to achieve the desired result.
	// For the actual system environment, see "man environ". extern char **environ; // environ is a pointer to an array of pointers to char* env pairs like xxx=yyy and the last pointer in the array is nullptr.
	// obj is envvalue
	//
	// `var envvalue1;
	//  if (envvalue1.osgetenv("HOME")) ... ok // e.g. "/home/exodus"
	//  // or
	//  let envvalue2 = osgetenv("EXO_ABC"); // "XYZ"`
	//
	ND bool osgetenv(SV envcode);

	// obj is var()

	// Get the current OS process id.
	// return: A number e.g. 663237.
	// obj is var()
	//
	// `let pid1 = var::ospid(); /// e.g. 663237
	//  // or
	//  let pid2 = ospid();`
	//
	ND static var  ospid();

	// Get the current OS thread process id.
	// return: A number e.g. 663237.
	// obj is var()
	//
	// `let tid1 = var::ostid(); /// e.g. 663237
	//  // or
	//  let tid2 = ostid();`
	//
	ND static var  ostid();

	// Get the exodus library version info.
	// return: The git commit details as at the time the library was built.
	// obj is var()
	//
	// `// e.g.
	//  // Local:  doc 2025-03-19 18:15:31 +0000 219cdad8a
	//  // Remote: doc 2025-03-17 15:03:00 +0000 958f412f0
	//  // https://github.com/exodusdb/exodusdb/commit/219cdad8a
	//  // https://github.com/exodusdb/exodusdb/archive/958f412f0.tar.gz
	//  //
	//  let v1 = var::version();
	//  // or
	//  let v2 = version();`
	//
	ND static var  version();

	// obj is var

	// Set the current thread's default locale.
	// strvar: The new locale codepage code.
	// True if successful
	// obj is strvar
	//
	// `if (var::setxlocale("en_US.utf8")) ... ok
	//  // or
	//  if (setxlocale("en_US.utf8")) ... ok`
	//
	   static bool setxlocale(const char* newlocalecode);

	// Get the current thread's default locale.
	// return: A locale codepage code string.
	//
	// `let v1 = var::getxlocale(); // "en_US.utf8"
	//  // or
	//  let v2 = getxlocale();`
	//
	ND static var  getxlocale();

	///// OUTPUT:
	////////////

	// obj is strvar

	// Output to stdout with optional prefix.
	// Append an NL char.
	// Is FLUSHED, not buffered.
	// The raw string bytes are output. No character or byte conversion is performed.
	//
	// `"abc"_var.outputl("xyz = "); /// Sends "xyz = abc\n" to stdout and flushes.
	//  // or
	//  outputl("xyz = ", "abc"); /// Any number of arguments is allowed. All will be output.`
	//
	   CVR outputl(in prefix = "") const;
	   CVR output(in prefix = "") const;  // Same as outputl() but doesnt append an NL char and is BUFFERED, not flushed.
	   CVR outputt(in prefix = "") const; // Same as outputl() but appends a tab char instead of an NL char and is BUFFERED, not flushed.

	// Output to stdlog with optional prefix.
	// Appends an NL char.
	// Is BUFFERED not flushed.
	// Any of the six types of field mark chars present are converted to their visible versions,
	//
	// `"abc"_var.logputl("xyz = "); /// Sends "xyz = abc\n" to stdlog buffer and is not flushed.
	//  // or
	//  logputl("xyz = ", "abc");; /// Any number of arguments is allowed. All will be output.`
	//
	   CVR logputl(in prefix = "") const;
	   CVR logput(in prefix = "") const; // Same as logputl() but doesnt append an NL char.

	// Output to stderr with optional prefix.
	// Appends an NL char.
	// Is FLUSHED not buffered.
	// Any of the six types of field mark chars present are converted to their visible versions,
	//
	// `"abc"_var.errputl("xyz = "); /// Sends "xyz = abc\n" to stderr
	//  // or
	//  errputl("xyz = ", "abc"); /// Any number of arguments is allowed. All will be output.`
	//
	   CVR errputl(in prefix = "") const;
	   CVR errput(in prefix = "") const; // Same as errputl() but doesnt append an NL char and is BUFFERED not flushed.

	// Output to a given stream.
	// Is BUFFERED not flushed.
	// The raw string bytes are output. No character or byte conversion is performed.
	//
	   CVR put(std::ostream& ostream1) const;

	// Flush any and all buffered output to stdout and stdlog.
	// obj is var()
	//
	// `var().osflush();
	//  // or
	//  osflush();`
	//
	   void osflush() const;

	// Use convenient << syntax to output anything to an osfile.
	// osfile: An OS path and filename or an osfilevar opened by osopen(). The file will be appended, or created if it does not already exist. osfile can be "stdout" or "stderr" to simulate cout/cerr/clog.
	// obj is osfile
	//
	// `let txtfile = "t_temp.txt";
	//  if (not osremove(txtfile)) {} // Remove any existing file.
	//  txtfile << txtfile << " " << 123.456789 << " " << 123 << std::endl;
	//  let v1 = osread(txtfile);   // "t_temp.txt 123.457 123\n"`
	//
	// All standard c++ io manipulators may be used e.g. std::setw, setfill etc.
	//
	// `let vout = "t_std_iomanip_overview.txt";
	//  if (not osremove(vout)) {}
	//  using namespace std;
	//
	//  vout << boolalpha    << true          << "\ttrue"    << endl;
	//  vout << noboolalpha  << true          << "\t1"       << endl;
	//
	//  vout << showpoint    << 42.0          << "\t42.0000" << endl;
	//  vout << noshowpoint  << 42.0          << "\t42"      << endl;
	//
	//  vout << showpos      << 42            << "\t+42"     << endl;
	//  vout << noshowpos    << 42            << "\t42"      << endl;
	//
	//  vout << skipws       << " " << 42     << "\t 42"     << endl;
	//  vout << noskipws     << " " << 42     << "\t 42"     << endl;
	//
	//  vout << unitbuf      << "a"           << "\ta"       << endl;
	//  vout << nounitbuf    << "b"           << "\tb"       << endl;
	//
	//  vout << setw(6)      << 42            << "\t    42"  << endl;
	//
	//  vout << left         << setw(6) << 42 << "\t42    "  << endl;
	//  vout << right        << setw(6) << 42 << "\t    42"  << endl;
	//  vout << internal     << setw(6) << 42 << "\t    42"  << endl;
	//  vout << setfill('*') << setw(6) << 42 << "\t****42"  << endl;
	//
	//  vout << showbase     << hex << 255    << "\t0xff"    << endl;
	//  vout << noshowbase   << 255           << "\tff"      << endl;
	//
	//  vout << uppercase    << 255           << "\tFF"      << endl;
	//  vout << nouppercase  << 255           << "\tff"      << endl;
	//
	//  vout << oct          << 255           << "\t377"     << endl;
	//  vout << hex          << 255           << "\tff"      << endl;
	//  vout << dec          << 255           << "\t255"     << endl;
	//
	//  vout << fixed        << 42.1          << "\t42.100000"            << endl;
	//  vout << scientific   << 42.1          << "\t4.210000e+01"         << endl;
	//  vout << hexfloat     << 42.1          << "\t0x1.50ccccccccccdp+5" << endl;
	//  vout << defaultfloat << 42.1          << "\t42.1"                 << endl;
	//
	//  vout << std::setprecision(3)      << 42.1567  << "\t42.2"  << endl;
	//  vout << resetiosflags(ios::fixed) << 42.1567  << "\t42.2"  << endl;
	//  vout << setiosflags(ios::showpos) << 42       << "\t+42"   << endl;
	//
	//  // Verify actual v. expected.
	//  var act_v_exp = osread(vout);
	//  act_v_exp.converter("\n\t", FM ^ VM); /// Text to dynamic array
	//  act_v_exp = invertarray(act_v_exp);   /// Columns <-> Rows
	//  assert(act_v_exp.f(1) eq act_v_exp.f(2));`
	//
	CVR operator<<(const auto& value) const {
		std::fstream* fs = this->osopenx(*this, /*utf8*/ true, /*or_throw*/ true);
		(*fs) << value; // Write to the stream
		return *this;
	}

	// Handle all stream manipulators

	CVR operator<<(std::ostream& (*manip)(std::ostream&)) const {
		std::fstream* fs = this->osopenx(*this, /*utf8*/ true, /*or_throw*/ true);
		manip(*fs); // Apply manipulator to the output side
		return *this;
	}

	///// INPUT:
	///////////

	// obj is var

	// Read one line of input from stdin.
	// return: True if successful or false if EOF or user pressed Esc or Ctrl+X in a terminal.
	// var[in]: The default value for terminal input and editing. Ignored if not a terminal.
	// var[out]: Raw bytes up to but excluding the first new line char. In case of EOF or user pressed Esc or Ctrl+X in a terminal it will be changed to "".
	// Prompt: If provided, it will be displayed on the terminal.
	// Multibyte/UTF8 friendly.
	//
	// `// var v1 = "defaultvalue";
	//  // if (v1.input("Prompt:")) ... ok
	//  // or
	//  // var v2 = input();`
	//
	ND bool input(in prompt = "");

    // Read raw bytes from standard input.
	// Any new line chars are treated like any other bytes.
	// Care must be taken to handle incomplete UTF8 byte sequences at the end of one block and the beginning of the next block.
	// return: The requested number of bytes or fewer if not available.
	// nchars:
	// * nn * Get up to nn bytes or fewer if not available. Caution required with UTF8.
	// *  0 * Get all bytes presently available.
	// *  1 * Same as keypressed(true). Deprecated.
	// * -1 * Same as keypressed(). Deprecated.
	//
	   var&  inputn(const int nchars);

	// Return the code of the current terminal key pressed.
	// wait: Defaults to false. True means wait for a key to be pressed if not already pressed.
	// return: ASCII or key code defined according to terminal protocol.
	// return: "" if stdin is not a terminal.
	// e.g. The PgDn key if pressed might return an escape sequence like "\x1b[6~"
	// It only takes a few µsecs to return false if no key is pressed.
	// `var v1; v1.keypressed();
	//  // or
	//  var v2 = keypressed();`
	//
	   var&  keypressed(const bool wait = false);

	// obj is var()

	// Check if is a terminal or a file/pipe.
	// Can check stdin, stdout, stderr.
	// in_out_err:
	// * 0 * stdin
	// * 1 * stdout (Default)
	// * 2 * stderr.
	// return: True if it is a terminal or false if it is a file or pipe.
	// Note that if the process is at the start or end of a pipeline, then only stdin or stdout will be a terminal.
	// The type of stdout terminal can be obtained from the TERM environment variable.
	//
	// `var v1 = var().isterminal(); /// 1 or 0
	//  // or
	//  var v2 = isterminal();`
	//
	ND bool isterminal(const int in_out_err = 1) const;

	// Check if stdin has any bytes available for input.
	// If no bytes are immediately available, the process sleeps for up to the given number of milliseconds, returning true immediately any bytes become available or false if the period expires without any bytes becoming available.
	// return: True if any bytes are available otherwise false.
	// It only takes a few µsecs to return false if no bytes are available and no wait time has been requested.
	//
	ND bool hasinput(const int milliseconds = 0) const;

	// True if stdin is at end of file
	//
	ND bool eof() const;

	// Set terminal echo on or off.
	// "On" causes all stdin data to be reflected to stdout if stdin is a terminal.
	// Turning terminal echo off can be used to prevent display of confidential information.
	// return: True if successful.
	//
	   bool echo(const bool on_off = true) const;

	// Install various interrupt handlers.
	// Automatically called in program/thread initialisation by exodus_main.
	// * SIGINT  * Ctrl+C -> "Interrupted. (C)ontinue (Q)uit (B)acktrace (D)ebug (A)bort ?"
	// * SIGHUP  * Sets a static variable "RELOAD_req" which may be handled or ignored by the program.
	// * SIGTERM * Sets a static variable "TERMINATE_req" which may be handled or ignored by the program.
	//
	   void breakon() const;

	// Disable keyboard interrupt.
	// Ctrl+C becomes inactive in terminal.
	//
	   void breakoff() const;

	// obj is strvar

	// Convert from codepage encoded text to UTF-8 encoded exodus text
	// codepage: e.g. Codepage "CP1124" (Ukrainian).
	// Use Linux command "iconv -l" for a complete list of code pages and encodings.
	//
	// `let v1 = "\xa4"_var.from_codepage("CP1124"); // "Є"
	//  // or
	//  let v2 = from_codepage("\xa4", "CP1124");
	//  // U+0404 Cyrillic Capital Letter Ukrainian Ie Unicode character`
	//
	ND var  from_codepage(const char* codepage) const;

	// Convert to codepage encoded text from exodus UTF-8 encoded text
	//
	// `let v1 = "Є"_var.to_codepage("CP1124").oconv("HEX"); // "A4"
	//  // or
	//  let v2 = to_codepage("Є", "CP1124").oconv("HEX");`
	//
	ND var  to_codepage(const char* codepage) const;

private:
       bool THIS_IS_OSFILE() const { return ((var_typ & VARTYP_OSFILE) != VARTYP_UNA); }

	ND std::fstream* osopenx(in osfilename, const bool utf8 = true, const bool or_throw = false) const;

}; // class var_os

} // namespace exo

#endif // VAR_OS_H