#include <cassert>
#include <exodus/program.h>
programinit()

	var test_error_handling = str("t", OPTIONS.contains("t"));
	var use_async = not OPTIONS.count("s");
	var N1 = COMMAND.f(2) ?: var(4);
	int N2 = COMMAND.f(3) ?: var(1000);

func main() {

	// Quit if no default database connection
	if (not connect()) {
		//Pass if allowed
		if (osgetenv("EXO_NODATA") or true) {
			printx("Test passed. Not really. ");
		}
		printl("No default db connection to perform db testing.");
		return 0;
	}

	printl("test_async says 'Hello World!'");
	printl("Syntax is test_async [reps=1] [N=1000] {OPTIONS}|Options are (s)ync, (t)est error handling, and one of (w)rite, (r)ead, (d)elete, default minimal");

	var mode = OPTIONS.contains("w") ? "write" : (OPTIONS.contains("r") ? "read" : "");
	if (OPTIONS.contains("d"))
		mode = "delete";
	if (not mode)
		mode = "select";

//	/////////////////////////
//	// Test One Parallel task
//	/////////////////////////
//
//	printl("\nStart one parallel Job (thread)");
//
//	run("test_asyncb {" ^ test_error_handling ^ "}");
//
//	printl("Wait for and pick up results. Ignore errors.");
//	for (auto& result : run_results())
//		printl(&result);

	////////////////////////
	// Test Manu Async Tasks
	////////////////////////

	{
		printl("\nCall or async a function. N1:", N1, "N2:", N2, "=", N1 * N2, "Async: ", use_async, "Mode:", mode);
		printl("Note that, unlike run/Job, async tasks do not start immediately. One will be handed the execution baton when the caller yields.");
		var t1 = ostimestamp();
		for (int i = 1; i <= N1; ++i) {
			if (use_async) {
				//////////////////////////////////////////////
				async(&_ExoProgram::main3, this, i, N2, mode);
//yield();
			} else
				main3(i, N2, mode);
		}

		// Run them immediately
//		yield();

		// How long to setup task
		var t2 = (ostimestamp() - t1) / (N1);
		if (not use_async)
			t2 /= N2;
		printl(elapsedtimetext(0, t2));
	}

//	yield();
//	{
//		printl("Yield");
//		var t1 = ostimestamp();
//		yield();
//		var t2 = (ostimestamp() - t1) / (N1);
//		printl(elapsedtimetext(0, t2));
//	}

	if (use_async)
	{
		yield();
		printl("test_async: Collect async results. N1:", N1, "N2:", N2, "=", N1*N2, mode);
		var t1 = ostimestamp();
		// Let the error be rethrown and caught like a normal exception
//		try {
			for (auto& result [[maybe_unused]] : async_results()) {
				printl(result.data.or_default(""));
//				yield();
			}
//		}
//		catch (VarError e) {
//			printl("test_async: Catch async VarError while collecting results.");
//			errputl(e.stack());
//			errputl(e.message);
////			throw;
//		}
//		catch (ExoStop e) {
//			printl("test_async: Catch async ExoStop while collecting results.");
////			errputl(e.stack());
//			errputl(e.message);
////			throw;
//		}
//		catch (ExoAbort e) {
//			printl("test_async: Catch async ExoAbort while collecting results.");
////			errputl(e.stack());
//			errputl(e.message);
////			throw;
//		}
//
//		catch (ExoAbortAll e) {
//			printl("Catch async ExoAbortAll while collecting results.");
////			errputl(e.stack());
//			errputl(e.message);
////			throw;
//		}
//
		printl("How long did each db operation consume on average. (total time/N not elapsed time)");
		printl("(Concurrent tasks but NOT concurrent execution.)");
		var t2 = (ostimestamp() - t1) / (N1 * N2);
		printl(elapsedtimetext(0, t2));
	}

	printl(elapsedtimetext());

	printx("Test passed.");

	return 0;
}

// DB file i/o
func main3(var call_id, var N, var mode) {
	printl(call_id, "test_async's main3 local function says 'Hello World!'");

	if (test_error_handling) {
		var t3;
		printl(t3);
	}

	// Connect and begin trans
	// Separate connection is required per async task
	// otherwise all db i/i queues up on one connection
	// and no async benefit.
	var conn;
	if (not conn.connect("exodus")) {
		loglasterror();
		return 0;
	}
	// Create and open
	var file;
	var filename = "abc" ^ call_id;
	if (mode ne "select") {
		if (not conn.begintrans()) abort(lasterror());
		if (not conn.createfile(filename)) errputl(lasterror());
		if (not file.open(filename, conn)) abort(lasterror());
	}

	// WARNING: DONT use thread locals (i.e globals) for async tasks
	// since they all share one FILE RECORD ID etc.

	if (mode eq "write") {

		// Write N records
		var record = str("x", 1000);
		for (var i : range(1, N)) {
			var id =  "X" ^ i;
			record.write(file, id);
		}
	}
	else if (mode eq "read") {

		// Read N records
		for (var i : range(1, N)) {
			var id =  "X" ^ i;
			var record;
			if (not record.read(file, id))
				abort(lasterror());
		}
	}
	else if (mode eq "delete") {

		// Delete N records
		for (var i : range(1, N)) {
			var id =  "X" ^ i;
			if (not file.deleterecord(id))
				abort(lasterror());
		}
		if (not conn.deletefile(filename)) errputl(lasterror());
	}
	else {

		// Minimal N selects
		for (var i : range(1, N)) {
			var response;
			if (not conn.sqlexec("select " ^ i * N, response))
				abort(lasterror());
		}
	}

	// Commit
	if (mode ne "select" and not conn.committrans())
		abort(lasterror());

	set_async_result(call_id);

	return 0;
}

}; // programexit()
