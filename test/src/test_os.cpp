#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <exodus/program.h>

programinit()

function main() {

	var tempdir="t_exotemp746.dir";
	var tempfilename=tempdir^OSSLASH^"temp1";

	{
		printl("Waiting for 500ms for any event on the log dir");
		var events = oswait(500, "/var/log");
		TRACE(events)
	}

	{
		// check osshell failures

		var badcmd = "usdcmwelefhayasflkj";
		assert(osshell("ls"));
		assert(not osshell(badcmd));
		assert(not osshellread(badcmd));
		assert(not osshellread(RECORD from badcmd));
		assert(not osshellwrite("abc" on badcmd));
	}

	//test copying files forced overwrite)
	assert(osmkdir("d1/d1"));
	assert(oswrite("f1","d1/f1"));
	assert(oswrite("f2","d1/f2"));
	assert(oscopy("d1/f1","d1/f2"));
	var data;
	assert(osread(data,"d1/f2"));
	printl(data=="f1");

	//test copying directories (recursive)
	assert(oswrite("f1","d1/d1/f1"));
	assert(oscopy("d1","d2"));
	assert(osread(data,"d2/d1/f1"));
	assert(data=="f1");

	//cannot copy a directory to a file
	assert(oscopy("d1","d2/d1/f1")==0);

	// Cannot oswrite on a dir path
	assert(not oswrite("xxx" on "d2"));

	// Check can rm directories with contents without setting recursive true
	assert(not osrmdir("d1"));

	// Check can rm dir with contents by setting recursive true
	assert(osrmdir("d1",true));
	assert(osrmdir("d2",true));

	// Cannot oswrite to a file in a non-existent dir
	assert(not oswrite("xxx" on "d2/abc"));

	osrmdir("test_main.1");
	osrmdir("test_main.2");

	assert(osmkdir("test_main.1"));
	assert(osmkdir("test_main.2"));

	if (not SLASH_IS_BACKSLASH) {

		//TRACE: dir1=".gitignore^a.out^alloc_traits.h.gcov^allocator.h.gcov^basic_string.h.gcov^basic_string.tcc.gcov^calblur8.html^char_traits.h.gcov^cmake_install.cmake^CMakeCache.txt^CMakeFiles^CMakeLists.txt^cpp_type_traits.h.gcov^CTestTestfile.cmake^DartConfiguration.tcl^exodusfuncs.h.gcov^exprtk.cpp^genregress2.cpp^gthr-default.h.gcov^hashtable_policy.h.gcov^hashtable.h.gcov^iomanip.gcov^ios_base.h.gcov^Makefile^move.h.gcov^mv.h.gcov^new_allocator.h.gcov^p4.cpp^ptr_traits.h.gcov^simple_decimal_conversion.h.gcov^std_mutex.h.gcov^stl_algobase.h.gcov^stl_iterator_base_funcs.h.gcov^stl_iterator_base_types.h.gcov^string_conversions.h.gcov^t_calblur8utf8.html^t_codep.bin^t_cp_allo.txt^t_cp_allo4.txt^t_EN_RU_UA.txt^t_GreekEnglFile.txt^t_GreekLocalFile.txt^t_GreekUTF-8File.txt^t_temp1234.txt^t_temp5.txt^t_test_MIXTURE.txt^t_test_OUTPUT_1251.txt^t_test_OUTPUT_UTF8.txt^t_utf-8-test.txt^t_utf8copy.html^t_utf8utf8.html^t_x.txt^test_asyncupd^test_asyncupd.cpp^test_comp^test_comp.cpp^test_db^test_db.cpp^test_dict^test_dict.cpp^test_dim^test_dim.cpp^test_isnum^test_isnum.cpp^test_main^test_main.1^test_main.2^test_main.cpp^test_main.out^test_math^test_math.cpp^test_mod^test_more^test_more.cpp^test_multilang^test_multilang.cpp^test_mvfuncs^test_mvfuncs.cpp^test_mvmv^test_mvmv.cpp^test_numfmt^test_numfmt.cpp^test_osopen^test_osopen.cpp^test_precision^test_precision.cpp^test_regress^test_regress.cpp^test_select^test_select.cpp^test_sort^test_sort.cpp^test_sortarray^test_sortarray.cpp^test_zzzclean^test_zzzclean.cpp^Testing^tests^type_traits.gcov^type_traits.h.gcov^unordered_map.h.gcov^utf-8-test.txt^utf8.html^x^y^z"

		assert(not oslist("a98s7d6c98as7d6c9sa876"));

		//unordered files and directories
		var dir1 = oslist("*").sort();

		var dir2 = osshellread("ls . -AU1").convert("\n\r", _FM).trim(_FM).sort();
		//TRACE(dir1)
		//TRACE(dir2)
		assert(dir1 eq dir2);

		{
			//unordered files and directories
			var dir1 = oslist("*").sort();

			//separate dir and glob
			assert(oslist(".","*").sort() eq dir1);

			var result;
			assert(osshellread(result from "ls . -AU1"));
			var dir2 = result.convert("\n\r", _FM).trim(_FM).sort();

			//TRACE(dir1)
			//TRACE(dir2)
			assert(dir1 eq dir2);
		}

		//unordered directories
		var dirs = oslistd("*").sort();
		//TRACE(dirs)
		assert(dir1 ne dirs);

		//separate dir and glob
		assert(oslistd(".","*").sort() eq dirs);

		//unordered files
		var files = oslistf("*").sort();
		//TRACE(files)
		assert(dir1 ne files);

		//separate dir and glob
		assert(oslistf(".","*").sort() eq files);

		//check oslist = oslistd ^ oslistf (both sorted)
		dir2 = (dirs ^ FM ^ files).sort();
		assert(dir1 eq dir2);

		// ls xxx*.yyy returns a sorted list regardless of the -U unordered option
		dir1 = oslist("test_*.cpp").sort();
		dir2 = osshellread("ls test_*.cpp -AU1").convert("\n\r", _FM ).trim(_FM).sort();
		//TRACE(dir1)
		//TRACE(dir2)
		assert(dir1 eq dir2);

		//files (not directories)
		assert(oslistf("*").convert(FM,"") == osshellread("find . -maxdepth 1 ! -path . ! -type d -printf '%f\n'").convert("\n\r",""));

		//directories (not files)
		assert(oslistd("*").convert(FM,"") == osshellread("find . -maxdepth 1 ! -path . -type d -printf '%f\n\'").convert("\n\r",""));
	}

	osrmdir("test_main.1");
	osrmdir("test_main.2");

	printl();
	assert(osdir(OSSLASH).match(_FM "\\d{5}" _FM "\\d{1,5}"));

	//root directories

	//check one step multilevel subfolder creation (requires boost version > ?)
	var topdir1=OSSLASH^"exodus544";
	var topdir1b=topdir1^"b";
	var subdir2=topdir1^OSSLASH^"abcd";
	var subdir2b=topdir1b^OSSLASH^"abcd";

	osrmdir(tempdir,true);

	//try to remove any old versions (subdir first to avoid problems)
	osrmdir(topdir1b,true);
	osrmdir(topdir1);
	osrmdir(subdir2b,true);
	osrmdir(subdir2);

	//need oermission to test root directory access
	if (osmkdir(subdir2)) {

		//assert(osmkdir(subdir2));

		printl("\nCheck CANNOT rename multilevel root folders");
		assert(not osrename(topdir1,topdir1b));

		printl("\nCheck CANNOT force delete root folders");
		assert(not osrmdir(topdir1,true));
		printl();

		//check can remove root folders one by one without force
		assert(osrmdir(subdir2));
		assert(osrmdir(topdir1));

		//printl(osdir("c:\\config.sys"));

		//relative directories ie not-root
		if (osdir(tempdir))
			assert(osrmdir(tempdir,true));

		//check mkdir
		assert(osmkdir(tempdir));
		assert(osdir(tempdir));
		assert(not osmkdir(tempdir));

		//check rmdir
		assert(osrmdir(tempdir));
		assert(not osdir(tempdir));
	}

	{
		// Verify cannot move a file to an existing file
		assert(oswrite("", "t_move1.txt"));
		assert(oswrite("", "t_move2.txt"));
		assert(not osmove("t_move1.txt", "t_move2.txt"));

		// Verify cannot rename to an existing file
		assert(not osrename("t_move1.txt","t_move2.txt"));

		// Verify can move a file
		assert(osremove("t_move2.txt"));
		assert(osmove("t_move1.txt", "t_move2.txt"));

		// Verify cannot rename non-existent file
		osremove("t_move3.txt");
		assert(not osrename("t_move1.txt","t_move3.txt"));

		// Verify cannot remove non-existent file
		assert(not osremove("t_move1.txt"));

		// Verify cannot osrmdir a file
		assert(not osrmdir("t_move2.txt"));

		// Verify can remove a file
		assert(osremove("t_move2.txt"));

	}

	{
		// Verify cannot move a dir to an existing dir
		osrmdir("t_move1.dir");
		osrmdir("t_move2.dir");
		assert(osmkdir("t_move1.dir"));
		assert(osmkdir("t_move2.dir"));
		assert(not osmove("t_move1.dir", "t_move2.dir"));

		// Verify can move a dir
		assert(osrmdir("t_move2.dir"));
		assert(osmove("t_move1.dir", "t_move2.dir"));

		// Verify cannot move non-existent dir
		osrmdir("t_move3.dir");
		assert(not osmove("t_move1.dir","t_move3.dir"));

		// Verify cannot remove non-existent dir
		assert(not osrmdir("t_move1.dir"));

		// Verify cannot osremove a dir
		assert(not osremove("t_move2.dir"));

		// Verify can remove a dir
		assert(osrmdir("t_move2.dir"));

	}

	{
		//check writing a 1Mb file
		//restrict to ascii characters so size on disk=number of characters in string
		//also restrict to size 1 2 4 8 16 etc
		//var str1="1234ABC\x0160";//Note: you have to prefix strings with L if you want to put multibyte hex chars
		var str1="1234ABCD";
		var filesize=1024*1024/8;
		printl(tempdir);
		assert(osmkdir(tempdir));
		assert(osrmdir(tempdir));
		assert(osmkdir(tempdir));
		printl(tempfilename);
		//printl(str(str1,filesize/len(str1)));
		assert(oswrite(str(str1,filesize/len(str1)),tempfilename));
		var filedate=date();
		assert(osfile(tempfilename));

		var info=osfile(tempfilename);
		assert(info.f(1) eq filesize);
		assert(info.f(2) eq filedate);

		//check copying to a new file
		var tempfilename2=tempfilename^2;
		if (osfile(tempfilename2))
			assert(osremove(tempfilename2));
		assert(oscopy(tempfilename,tempfilename2));
		assert(osfile(tempfilename2) eq info);

		//check renaming
		var tempfilename3=tempfilename^3;
		assert(osrename(tempfilename2,tempfilename3));
		assert(osfile(tempfilename3) eq info);

		//check force delete of subdirectory
		assert(osrmdir(tempdir,true));
	}

	{
		var env=osgetenv("");
		printl(env);
		assert(osgetenv("PATH"));
		assert(osgetenv("HOME"));
		env="Steve";
		env.ossetenv("XYZ");
		assert(osgetenv("XYZ"));

		assert(osgetenv("HOME",RECORD));
		assert(RECORD eq osgetenv("HOME"));

		assert(not osgetenv("test_os" ^ rnd(999999), RECORD));
	}

	{
		var temprecord;
		var tempfilename0="tempfile";
		assert(oswrite("123" on tempfilename0));
		assert(osfile(tempfilename0));
		assert(osread(temprecord from tempfilename0));
		assert(temprecord eq "123");
		assert(osremove(tempfilename0));
		assert(not osfile(tempfilename0));
	}

	{

		// Get current working dir
		var cwd1;
		cwd1=oscwd();
		TRACE(cwd1)
		assert(cwd1);

		// Make a new dir
		//var cwd2 = cwd1 ^ OSSLASH ^ "t_cwd.dir";
		var cwd2 = ostempdirpath() ^ "t_cwd_" ^ rnd(99999999) ^ ".dir";
		TRACE(cwd2)
		assert(osmkdir(cwd2));

		// Change to a new dir
		assert(oscwd(cwd2));
		assert(oscwd() eq cwd2);

		// Change back
		assert(oscwd(cwd1));
		assert(oscwd() eq cwd1);

		// Remove the new dir
		assert(osrmdir(cwd2));

		// Cannot change to deleted dir
		assert(not oscwd(cwd2));
		assert(oscwd() eq cwd1);

	}

	{
		var tempfilename = ostempfilename();
		assert(tempfilename.starts(ostempdirpath()));
		assert(oswrite("" on tempfilename));
		TRACE(tempfilename)
		assert(osremove(tempfilename));
	}

	{
		printl("Init");
		var tempfilename = ostempfilename();
		TRACE(tempfilename)
		var testdata = "some test data";

		printl("Linux only using 'cat'");
		TRACE(testdata)
		assert(osshellwrite(testdata on "cat > " ^ tempfilename));
		TRACE(testdata)

		printl("Check data was processed correctly");
		assert(osread(tempfilename) eq testdata);

		printl("Cleanup");
		osremove(tempfilename);
	}

	printl("osflush()");
	osflush();

	{
		var filename="t_hello.txt";
		assert(oswrite("1234567890abc", filename));
		assert(osfile(filename).f(1) == 13);

		var filex;
		assert(osopen(filename,filex));

		var data;
		var offset=3;// zero based offset
		var length=3;
		assert(osbread(data,filex,offset,length));
		assert(data.outputl() == "456");

		length = 2;
		assert(osbread(data,filex,offset,length));
		assert(data.outputl() == "78");
		//offset is automatically updated according to length of previous osbread
		assert(offset == 8);

		//offset -1 appends on write
		offset = -1;
		assert(osbwrite("qwe",filex,offset));
		assert(offset == 16);
		assert(osread(filename).outputl() == "1234567890abcqwe");

		//offset -2 offset starts on the last byte of the file
		offset = -2;
		assert(osbwrite("XYZ",filex,offset));
		assert(offset == 18);
		assert(osread(filename).outputl() == "1234567890abcqwXYZ");

		// files must be closed manually
		// TODO add a way to autoclose all files/all files opened after a certain point
		// rather like disconnect(nn)
		osclose(filex);

		// Check can remove file
		assert(osremove(filename));
		assert(not osfile(filename));
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()