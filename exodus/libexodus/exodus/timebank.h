#ifndef EXODUS_LIBEXODUS_EXO_TIMEBANK_H_
#define EXODUS_LIBEXODUS_EXO_TIMEBANK_H_

// EXO_TIMEBANK must be defined to create functional Timers
// e.g. cmake . -DEXO_TIMEBANK=1

#ifndef EXO_TIMEBANK
////////////////////////
// DUMMY TIMEBANK HEADER
////////////////////////

struct Timer{
	explicit Timer(int){
	}
};

CONSTEVAL_OR_CONSTEXPR
auto get_timebank_acno(const std::string_view /*arg1*/) -> int {return 0;}
//auto get_timebank_acno(const const * /*arg1*/) -> int {return 0;}

#else
/////////////////////////
// ACTUAL TIMEBANK HEADER
/////////////////////////

#if EXO_MODULE
	import std;
// slows all compilations down but std.cppm seems to be missing some export using clause
//#	include <chrono>
#else
#	include <chrono>
#	include <array>
#	include <cstring> // for strcmp
#	include <time.h>
#	include <stdio.h>
#endif

namespace exo {

void get_timebank_acc0();

// A global array of timebank accounts
//consteval auto get_timebank_acno(const char* arg1) -> int;
//class TimeBank
//struct TimeBank
inline
struct PUBLIC TimeBank {

	struct Acc {
		std::uint64_t count = 0;
		std::uint64_t ns = 0;
	};

	// Array of a thousand accounts
	std::array<Acc, 1000> timeaccs_;
	std::array<std::string, 1000> timeacc_names_;

	~TimeBank();

} timebank;

// Utility function to get a timestamp in ns
// Keep in mind that the precision of the clock_gettime function is not guaranteed to be nanosecond-level,
// and the actual precision may vary depending on the system and the clock being used.
static inline long long get_time_ns() {

	struct timespec ts;

	// system clock_gettime function returns the time in seconds and nanoseconds,
	clock_gettime(CLOCK_REALTIME, &ts);

	// Multiply the seconds by 1000000000 and add the nanoseconds
	// to get the total time in nanoseconds.
	return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

struct Timer {

	int timebank_acno_ = 0;

//	using Clock = std::chrono::high_resolution_clock;
//	using Time_Point = Clock::time_point;
//	Time_Point timebank_start_;
//	Time_Point timebank_start_ = Clock::now();

	// Default initialisation captures an initial ns timestamp
	long long timebank_start_ = get_time_ns();

	// Prevent default construction. Is this necessary since we provide a specific constructor?
	Timer() = delete;

	// Explicit constructor for a timebank account number
	// Captures account number;
	explicit Timer(const int timebank_acno)
		:
		timebank_acno_(timebank_acno) {
//		timebank_start_ = Clock::now();
	}

	// Destructor
	// 1. Calculate the elapsed time in ns
	// 2. Add it to the timebank account total.
	~Timer() {

//		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - timebank_start_).count();
		auto elapsed_ns = get_time_ns() - timebank_start_;

//		//std::cout << timebank_acno_ << " +" << ns << std::endl;
		timebank.timeaccs_[timebank_acno_].ns += elapsed_ns;
		timebank.timeaccs_[timebank_acno_].count++;
	}


};

CONSTEVAL_OR_CONSTEXPR
auto get_timebank_acno(const std::string_view arg1) -> int {
	int acno = 0;

	// To renumber timebank.txt sequentially e.g. after inserting new elements
	/*
		cut -d' ' -f2-999 timebank.txt > x
		awk '{print NR+499 " " $0}' x > timebank.txt
	*/

	// Create this code block using
	/*
		cut -d' ' -f2-999 timebank.txt > p
		sed 's#\s*$##g' p > q
//		awk '{print "else if (++acno && arg1 == \"" $0 "\")) acno = " ++i ";"}' q > r
		awk '{print "else if (++acno && arg1 == \"" $0 "\") \{\}"}' q > r
	*/

	     if (++acno && arg1 == "dim& dim::splitter(in str1, SV sepchar = FM)") {}
	else if (++acno && arg1 == "xxxxxxxx unused") {}
	else if (++acno && arg1 == "bool dim::read(in filevar, in key)") {}
	else if (++acno && arg1 == "bool dim::write(in filevar, in key) const") {}
	else if (++acno && arg1 == "bool dim::osread(in osfilename, const char* codepage DEFAULT_EMPTY)") {}
	else if (++acno && arg1 == "bool dim::oswrite(in osfilename, const char* codepage DEFAULT_EMPTY)") {}
	else if (++acno && arg1 == "var  var::sort(SV sepchar = FM)") {}
	else if (++acno && arg1 == "var  var::reverse(SV sepchar = FM)") {}
	else if (++acno && arg1 == "var  var::timestamp(in ostime) const") {}
	else if (++acno && arg1 == "bool var::connect(in conninfo") {}
	else if (++acno && arg1 == "bool var::attach(in filenames") {}
	else if (++acno && arg1 == "bool var::detach(in filenames") {}
	else if (++acno && arg1 == "bool var::disconnect()") {}
	else if (++acno && arg1 == "bool var::disconnectall()") {}
	else if (++acno && arg1 == "bool var::open(in filename, in connection)") {}
	else if (++acno && arg1 == "bool var::open cache_miss") {}
	else if (++acno && arg1 == "void var::close()") {}
	else if (++acno && arg1 == "bool var::readc(in filehandle, in key)") {}
	else if (++acno && arg1 == "void var::writec(in filehandle, in key)") {}
	else if (++acno && arg1 == "bool var::deletec(in key)") {}
	else if (++acno && arg1 == "bool var::read(in filehandle, in key)") {}
	else if (++acno && arg1 == "bool var::reado cache_miss") {}
	else if (++acno && arg1 == "var  var::hash() const") {}
	else if (++acno && arg1 == "var  var::lock(in key) const") {}
	else if (++acno && arg1 == "void var::unlock(in key) const") {}
	else if (++acno && arg1 == "void var::unlockall() const") {}
	else if (++acno && arg1 == "bool var::sqlexec(in sqlcmd, io response) const") {}
	else if (++acno && arg1 == "bool var::writef(in filehandle, in key, const int fieldno) const") {}
	else if (++acno && arg1 == "bool var::write(in filehandle, in key) const") {}
	else if (++acno && arg1 == "bool var::updaterecord(in filehandle, in key) const") {}
	else if (++acno && arg1 == "bool var::insertrecord(in filehandle, in key) const") {}
	else if (++acno && arg1 == "bool var::deleterecord(in key) const") {}
	else if (++acno && arg1 == "bool var::cleardbcache() const") {}
	else if (++acno && arg1 == "bool var::begintrans() const") {}
	else if (++acno && arg1 == "bool var::rollbacktrans() const") {}
	else if (++acno && arg1 == "bool var::committrans() const") {}
	else if (++acno && arg1 == "bool var::statustrans() const") {}
	else if (++acno && arg1 == "bool var::dbcreate(in from_dbname, in to_dbname)") {}
	else if (++acno && arg1 == "bool var::dbdelete(in dbname)") {}
	else if (++acno && arg1 == "bool var::createfile(in filename)") {}
	else if (++acno && arg1 == "bool var::renamefile(in filename, in newfilename)") {}
	else if (++acno && arg1 == "bool var::deletefile(in filename)") {}
	else if (++acno && arg1 == "bool var::clearfile(in filename)") {}
	else if (++acno && arg1 == "bool var::select(in sortselectclause) const") {}
	else if (++acno && arg1 == "bool var::deletelist(in listname) const") {}
	else if (++acno && arg1 == "bool var::savelist(in listname)") {}
	else if (++acno && arg1 == "bool var::getlist(in listname) const") {}
	else if (++acno && arg1 == "bool var::formlist(in keys, in fieldno)") {}
	else if (++acno && arg1 == "bool var::makelist(in listname)") {}
	else if (++acno && arg1 == "bool var::hasnext() const") {}
	else if (++acno && arg1 == "bool var::readnext(io key, io valueno) const") {}
	else if (++acno && arg1 == "bool var::readnext(io record, io key, io valueno) const") {}
	else if (++acno && arg1 == "bool var::createindex(in fieldname, in dictfile) const") {}
	else if (++acno && arg1 == "bool var::deleteindex(in fieldname) const") {}
	else if (++acno && arg1 == "var  var::listfiles() const") {}
	else if (++acno && arg1 == "var  var::dblist() const") {}
	else if (++acno && arg1 == "bool var::cursorexists()") {}
	else if (++acno && arg1 == "var  var::listindex(in filename) const") {}
	else if (++acno && arg1 == "var  var::reccount(in filename_or_handle_or_null) const") {}
	else if (++acno && arg1 == "var  var::flushindex(in filename=) const") {}
	else if (++acno && arg1 == "bool var::input()") {}
	else if (++acno && arg1 == "bool var::input(in prompt") {}
	else if (++acno && arg1 == "bool var::inputn(const int nchars") {}
	else if (++acno && arg1 == "io   var::default_to(in defaultvalue) const") {}
	else if (++acno && arg1 == "io   var::default_from(in defaultvalue)") {}
	else if (++acno && arg1 == "char var::toChar() const") {}
	else if (++acno && arg1 == "str  var::toString() &&") {}
	else if (++acno && arg1 == "str  var::toString() const&") {}
	else if (++acno && arg1 == "var  var::len() const") {}
	else if (++acno && arg1 == "var  var::textlen()") {}
	else if (++acno && arg1 == "std::u32string var::to_u32string() const") {}
	else if (++acno && arg1 == "io   var::trim(SV trimchars)") {}
	else if (++acno && arg1 == "io   var::trimmer(SV trimchars)") {}
	else if (++acno && arg1 == "io   var::trimfirst(SV trimchars) const&") {}
	else if (++acno && arg1 == "io   var::trimmerfirst(SV trimchars)") {}
	else if (++acno && arg1 == "io   var::trimlast(SV trimchars) const&") {}
	else if (++acno && arg1 == "io   var::trimmerlast(SV trimchars)") {}
	else if (++acno && arg1 == "io   var::trimboth(SV trimchars) const&") {}
	else if (++acno && arg1 == "io   var::trimmerboth(SV trimchars)") {}
	else if (++acno && arg1 == "io   var::inverter()") {}
	else if (++acno && arg1 == "io   var::ucaser()") {}
	else if (++acno && arg1 == "io   var::lcaser()") {}
	else if (++acno && arg1 == "io   var::tcaser()") {}
	else if (++acno && arg1 == "io   var::fcaser()") {}
	else if (++acno && arg1 == "io   var::normalizer()") {}
	else if (++acno && arg1 == "var  var::unique()") {}
	else if (++acno && arg1 == "var  var::seq() const") {}
	else if (++acno && arg1 == "var  var::textseq() const") {}
	else if (++acno && arg1 == "io   var::paster(const int pos1, const int length, SV insertstr)") {}
	else if (++acno && arg1 == "io   var::paster(const int pos1, SV insertstr)") {}
	else if (++acno && arg1 == "var  var::prefix(SV insertstr)") {}
	else if (++acno && arg1 == "io   var::prefixer(SV insertstr)") {}
	else if (++acno && arg1 == "io   var::popper()") {}
	else if (++acno && arg1 == "io   var::move(io tovar)") {}
	else if (++acno && arg1 == "var  var::str(const int num) const") {}
	else if (++acno && arg1 == "var  var::space() const") {}
	else if (++acno && arg1 == "io   var::cropper()") {}
	else if (++acno && arg1 == "io   var::lowerer()") {}
	else if (++acno && arg1 == "io   var::raiser()") {}
	else if (++acno && arg1 == "io   var::converter(SV fromchars, SV tochars)") {}
	else if (++acno && arg1 == "io   var::converter(in fromchars, in tochars)") {}
	else if (++acno && arg1 == "io   var::parser(char sepchar)") {}
	else if (++acno && arg1 == "var  var::fcount(SV sep) const") {}
	else if (++acno && arg1 == "var  var::count(SV str) const") {}
	else if (++acno && arg1 == "var  var::index(SV substr, const int startindex) const") {}
	else if (++acno && arg1 == "var  var::indexr(SV substr, const int startindex) const") {}
	else if (++acno && arg1 == "var  var::index(SV substr) const") {}
	else if (++acno && arg1 == "var  var::xlate(in filename, in fieldno, const char* mode) const") {}
	else if (++acno && arg1 == "var  var::numberinwords(in number, in langname_or_locale_id)") {}
	else if (++acno && arg1 == "var  var::iconv(const char* conversion) const") {}
	else if (++acno && arg1 == "var  var::oconv(const char* conversion) const") {}
	else if (++acno && arg1 == "var  var::mod(in limit) const") {}
	else if (++acno && arg1 == "var  var::mod(double limit) const") {}
	else if (++acno && arg1 == "var  var::mod(const int limit) const") {}
	else if (++acno && arg1 == "var  var::abs() const") {}
	else if (++acno && arg1 == "var  var::sin() const") {}
	else if (++acno && arg1 == "var  var::cos() const") {}
	else if (++acno && arg1 == "var  var::tan() const") {}
	else if (++acno && arg1 == "var  var::atan() const") {}
	else if (++acno && arg1 == "var  var::loge() const") {}
	else if (++acno && arg1 == "var  var::sqrt() const") {}
	else if (++acno && arg1 == "var  var::pwr(in exponent) const") {}
	else if (++acno && arg1 == "var  var::exp() const") {}
	else if (++acno && arg1 == "var  var::osshell() const") {}
	else if (++acno && arg1 == "var  var::osshellread() const") {}
	else if (++acno && arg1 == "var  var::osshellwrite(in oscmd) const") {}
	else if (++acno && arg1 == "bool var::osopen(in osfilename, const char* locale)") {}
	else if (++acno && arg1 == "bool var::osopenx(in osfilename, const char* locale)") {}
	else if (++acno && arg1 == "bool var::osread(const char* osfilename, const char* codepage") {}
	else if (++acno && arg1 == "bool var::to_codepage(const char* codepage) const") {}
	else if (++acno && arg1 == "bool var::from_codepage(const char* codepage) const") {}
	else if (++acno && arg1 == "bool var::oswrite(in osfilename, const char* codepage) const") {}
	else if (++acno && arg1 == "bool var::osbwrite(in osfilevar, io offset) const") {}
	else if (++acno && arg1 == "bool var::osbread(in osfilevar, io offset, const int bytesize") {}
	else if (++acno && arg1 == "bool var::osrename(in new_dirpath_or_filepath) const") {}
	else if (++acno && arg1 == "bool var::oscopy(in new_dirpath_or_filepath) const") {}
	else if (++acno && arg1 == "bool var::osmove(in new_dirpath_or_filepath) const") {}
	else if (++acno && arg1 == "bool var::osrmdir(bool evenifnotempty) const") {}
	else if (++acno && arg1 == "var  var::oslist(in globpattern, const int mode) const") {}
	else if (++acno && arg1 == "var  var::oscwd(const char* newpath) const") {}
	else if (++acno && arg1 == "var  var::oscwd() const") {}
	else if (++acno && arg1 == "void var::ossleep(const int milliseconds) const") {}
	else if (++acno && arg1 == "var  var::rnd() const") {}
	else if (++acno && arg1 == "void var::initrnd() const") {}
	else if (++acno && arg1 == "bool var::osgetenv(const char* envcode)") {}
	else if (++acno && arg1 == "bool var::ossetenv(const char* envcode) const") {}
	else if (++acno && arg1 == "CVR  var::put(std::ostream& ostream1) const") {}
	else if (++acno && arg1 == "var  var::match(SV regex_str, SV regex_options) const") {}
	else if (++acno && arg1 == "io   var::replacer(SV what, SV with)") {}
	else if (++acno && arg1 == "var  var::replace(const rex& regex, SV replacement) const") {}
	else if (++acno && arg1 == "var  var::field(SV separatorx, const int fieldnx, const int nfieldsx) const") {}
	else if (++acno && arg1 == "io   var::fieldstorer(SV separator0, const int fieldnx, const int nfieldsx, in replacementx)") {}
	else if (++acno && arg1 == "bool var::locate(in target, io setting, const int fieldno/*=0*/, const int valueno/*=0*/) const") {}
	else if (++acno && arg1 == "bool var::locate(in target, io setting, const int fieldno/*=0*/, const int valueno/*=0*/) const") {}
	else if (++acno && arg1 == "bool var::locate(in target) const") {}
	else if (++acno && arg1 == "bool var::locateby(const char* ordercode, in target, io setting) const") {}
	else if (++acno && arg1 == "bool var::locateby(const char* ordercode, in target, io setting, const int fieldno, const int valueno/*=0*/) const") {}
	else if (++acno && arg1 == "bool var::locatebyusing(const char* ordercode, const char* usingchar, in target, io setting, const int fieldno=0, const int valueno=0, const int valueno=0) const") {}
	else if (++acno && arg1 == "bool var::locateusing(const char* usingchar, in target) const") {}
	else if (++acno && arg1 == "bool var::locateusing(const char* usingchar, in target, io setting, const int fieldno/*=0*/, const int valueno/*=0*/, const int subvalueno/*=0*/) const") {}
	else if (++acno && arg1 == "var  var::f(const int argfieldn, const int argvaluen, const int argsubvaluen) const") {}
	else if (++acno && arg1 == "io   var::remover(int fieldno, int valueno, int subvalueno)") {}
	else if (++acno && arg1 == "io   var::r(int fieldno, int valueno, int subvalueno, in replacement)") {}
	else if (++acno && arg1 == "io   var::inserter(const int fieldno, const int valueno, const int subvalueno, in insertion)") {}
	else if (++acno && arg1 == "bool var::starts(SV str) const") {}
	else if (++acno && arg1 == "bool var::ends(SV str) const") {}
	else if (++acno && arg1 == "bool var::contains(SV str) const") {}
	else if (++acno && arg1 == "var  var::first(const std::size_t length) const") {}
	else if (++acno && arg1 == "io   var::firster(const std::size_t length)") {}
	else if (++acno && arg1 == "var  var::last(const std::size_t length) const") {}
	else if (++acno && arg1 == "io   var::laster(const std::size_t length)") {}
	else if (++acno && arg1 == "var  var::cut(const int length) const") {}
	else if (++acno && arg1 == "io   var::cutter(const int length)") {}
	else if (++acno && arg1 == "io   var::substrer(const int startindex1, const int length)") {}
	else if (++acno && arg1 == "var  var::at(const int charno) const") {}
	else if (++acno && arg1 == "var  var::multivalued(const char* opcode, in var2) const") {}
	else if (++acno && arg1 == "var  var::substr(const int startindex1, io delimiterchars, int& endindex) const") {}
	else if (++acno && arg1 == "var  var::substr2(io startindex1, io delimiterno) const") {}
	else if (++acno && arg1 == "var  var::sumall() const") {}
	else if (++acno && arg1 == "var  var::sum() const") {}
	else if (++acno && arg1 == "var  var::sum(SV separator) const") {}
	else if (++acno && arg1 == "bool var::setxlocale() const") {}
	else if (++acno && arg1 == "io   var::quoter()") {}
	else if (++acno && arg1 == "io   var::squoter()") {}
	else if (++acno && arg1 == "io   var::unquoter()") {}
	else if (++acno && arg1 == "CVR  var::swap(in var2) const") {}
	else if (++acno && arg1 == "io   var::swap(io var2)") {}
	else if (++acno && arg1 == "var  var::textwidth() const") {}
	else if (++acno && arg1 == "var  var::first() const") {}
	else if (++acno && arg1 == "var  var::last() const") {}
	else if (++acno && arg1 == "io   var::firster()") {}
	else if (++acno && arg1 == "io   var::laster()") {}
	else if (++acno && arg1 == "var  var::search(SV regex_str, int startchar1, SV regex_options) const") {}
	else if (++acno && arg1 == "std::wstring var::to_wstring() const") {}
	else if (++acno && arg1 == "const char* var::c_str() const") {}

	else if (++acno && arg1 == "str  var::oconv_T(in format) const") {}
	else if (++acno && arg1 == "str  var::oconv_MD(const char* conversion) const") {}
	else if (++acno && arg1 == "str  var::oconv_LRC(in format) const") {}
	else if (++acno && arg1 == "str  var::oconv_TX(const char* conversion) const") {}
	else if (++acno && arg1 == "str  var::oconv_HEX(const int) const") {}

	else if (++acno && arg1 == "var  var::iconv_D(const char* conversion) const") {}
	else if (++acno && arg1 == "str  var::oconv_D(const char* conversion) const") {}
	else if (++acno && arg1 == "io   var::oconv_MR(const char* conversion) const") {}
	else if (++acno && arg1 == "var  var::iconv_MT() const") {}

	else if (++acno && arg1 == "str  var::oconv_MT(const char* conversion) const") {}
	else if (++acno && arg1 == "var  var::iconv_HEX(const int ioratio) const") {}
	else if (++acno && arg1 == "var  var::iconv_TX(const char* conversion) const") {}
	else if (++acno && arg1 == "var  var::quote() const&") {}
	else if (++acno && arg1 == "var  var::squote() const&") {}
	else if (++acno && arg1 == "var  var::unquote() const&") {}
	else if (++acno && arg1 == "str  var::oconv_MT(const char* conversion) const") {}

	else if (++acno && arg1 == "var  var::format(SV fmt_str, Args&&... args) const") {}
	else if (++acno && arg1 == "var  format(SV fmt_str, Args&&... args)") {}
	//else if (++acno && arg1 == "auto fmt::formatter::parse(...)") {}
	else if (++acno && arg1 == "auto fmt::formatter::format(var) const") {}

//	else if (++acno && arg1 == "")) acno = ;


	else {

		// This is the only way to indicate to the compiler that an error occurred during a compile time execution of this function
		// Dont add any other type of activity that is not consteval/constexpr
		//
		//throw std::runtime_error("");
		//throw 500;
		std::cerr << "Error: " << __PRETTY_FUNCTION__ << ": Unrecognised time account title: '" << arg1 << "'. Must be added to timebank.h and timebank.txt" << std::endl;
		throw arg1;
		//static_assert(false);
		//static_assert(arg1);
	}

	return acno + 499;
}

} // namespace exo
#endif // EXO_TIMEBANK
#endif // EXODUS_LIBEXODUS_EXO_TIMEBANK_H_
