#ifndef EXODUS_LIBEXODUS_EXODUS_TIMEBANK_H_
#define EXODUS_LIBEXODUS_EXODUS_TIMEBANK_H_

// EXODUS_TIMEBANK must be defined to create functional Timers
// e.g. cmake . -DEXODUS_TIMEBANK=1

#ifndef EXODUS_TIMEBANK
////////////////////////
// DUMMY TIMEBANK HEADER
////////////////////////

struct Timer{
	explicit Timer(int){
	}
};

CONSTEVAL_OR_CONSTEXPR
auto get_timeacno(const char* /*arg1*/) -> int {return 0;}

#else
/////////////////////////
// ACTUAL TIMEBANK HEADER
/////////////////////////

#include <chrono>
#include <array>
#include <cstring> // for strcmp

namespace exodus {

void get_timeacc0();

//consteval auto get_timeacno(const char* arg1) -> int;
inline
struct TimeBank {

	struct Acc {
		uint64_t count = 0;
		uint64_t ns = 0;
	};

	std::array<Acc, 1000> timeaccs_;
	std::array<std::string, 1000> timeacc_names_;

	~TimeBank();

} timebank;

struct Timer {

	int time_acno_ = 0;

	std::chrono::system_clock::time_point start_;

	Timer() = delete;

	explicit Timer(const int time_acno)
		:
		time_acno_(time_acno) {

		//std::cout << time_acno_ << " ~" << std::endl;
		start_ = std::chrono::high_resolution_clock::now();
	}

	~Timer() {
		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_).count();
		//std::cout << time_acno_ << " +" << ns << std::endl;
		timebank.timeaccs_[time_acno_].ns += ns;
		timebank.timeaccs_[time_acno_].count++;
	}


};

CONSTEVAL_OR_CONSTEXPR
auto get_timeacno(const char* arg1) -> int {
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
		awk '{print "else if ( ! std::strcmp(arg1, \"" $0 "\")) acno = " ++i ";"}' q > r
	*/

	if ( ! std::strcmp(arg1, "dim var::split(SV sepchar = FM)")) acno = 1;
	else if ( ! std::strcmp(arg1, "var dim::split(CVR str1, SV sepchar = FM)")) acno = 2;
	else if ( ! std::strcmp(arg1, "bool dim::read(CVR filevar, CVR key)")) acno = 3;
	else if ( ! std::strcmp(arg1, "bool dim::write(CVR filevar, CVR key) const")) acno = 4;
	else if ( ! std::strcmp(arg1, "bool dim::osread(CVR osfilename, const char* codepage DEFAULT_EMPTY)")) acno = 5;
	else if ( ! std::strcmp(arg1, "bool dim::oswrite(CVR osfilename, const char* codepage DEFAULT_EMPTY)")) acno = 6;
	else if ( ! std::strcmp(arg1, "var var::sort(SV sepchar = FM)")) acno = 7;
	else if ( ! std::strcmp(arg1, "var var::reverse(SV sepchar = FM)")) acno = 8;
	else if ( ! std::strcmp(arg1, "var var::timestamp(CVR ostime) const")) acno = 9;
	else if ( ! std::strcmp(arg1, "bool var::connect(CVR conninfo")) acno = 10;
	else if ( ! std::strcmp(arg1, "bool var::attach(CVR filenames")) acno = 11;
	else if ( ! std::strcmp(arg1, "bool var::detach(CVR filenames")) acno = 12;
	else if ( ! std::strcmp(arg1, "bool var::disconnect()")) acno = 13;
	else if ( ! std::strcmp(arg1, "bool var::disconnectall()")) acno = 14;
	else if ( ! std::strcmp(arg1, "bool var::open(CVR filename, CVR connection)")) acno = 15;
	else if ( ! std::strcmp(arg1, "bool var::open cache_miss")) acno = 16;
	else if ( ! std::strcmp(arg1, "void var::close()")) acno = 17;
	else if ( ! std::strcmp(arg1, "bool var::readc(CVR filehandle,CVR key)")) acno = 18;
	else if ( ! std::strcmp(arg1, "void var::writec(CVR filehandle,CVR key)")) acno = 19;
	else if ( ! std::strcmp(arg1, "bool var::deletec(CVR key)")) acno = 20;
	else if ( ! std::strcmp(arg1, "bool var::read(CVR filehandle,CVR key)")) acno = 21;
	else if ( ! std::strcmp(arg1, "bool var::reado cache_miss")) acno = 22;
	else if ( ! std::strcmp(arg1, "var var::hash() const")) acno = 23;
	else if ( ! std::strcmp(arg1, "var var::lock(CVR key) const")) acno = 24;
	else if ( ! std::strcmp(arg1, "void var::unlock(CVR key) const")) acno = 25;
	else if ( ! std::strcmp(arg1, "void var::unlockall() const")) acno = 26;
	else if ( ! std::strcmp(arg1, "bool var::sqlexec(CVR sqlcmd, VARREF response) const")) acno = 27;
	else if ( ! std::strcmp(arg1, "bool var::writef(CVR filehandle,CVR key,const int fieldno) const")) acno = 28;
	else if ( ! std::strcmp(arg1, "bool var::write(CVR filehandle, CVR key) const")) acno = 29;
	else if ( ! std::strcmp(arg1, "bool var::updaterecord(CVR filehandle,CVR key) const")) acno = 30;
	else if ( ! std::strcmp(arg1, "bool var::insertrecord(CVR filehandle,CVR key) const")) acno = 31;
	else if ( ! std::strcmp(arg1, "bool var::deleterecord(CVR key) const")) acno = 32;
	else if ( ! std::strcmp(arg1, "bool var::cleardbcache() const")) acno = 33;
	else if ( ! std::strcmp(arg1, "bool var::begintrans() const")) acno = 34;
	else if ( ! std::strcmp(arg1, "bool var::rollbacktrans() const")) acno = 35;
	else if ( ! std::strcmp(arg1, "bool var::committrans() const")) acno = 36;
	else if ( ! std::strcmp(arg1, "bool var::statustrans() const")) acno = 37;
	else if ( ! std::strcmp(arg1, "bool var::dbcreate(CVR from_dbname, CVR to_dbname)")) acno = 38;
	else if ( ! std::strcmp(arg1, "bool var::dbdelete(CVR dbname)")) acno = 39;
	else if ( ! std::strcmp(arg1, "bool var::createfile(CVR filename)")) acno = 40;
	else if ( ! std::strcmp(arg1, "bool var::renamefile(CVR filename, CVR newfilename)")) acno = 41;
	else if ( ! std::strcmp(arg1, "bool var::deletefile(CVR filename)")) acno = 42;
	else if ( ! std::strcmp(arg1, "bool var::clearfile(CVR filename)")) acno = 43;
	else if ( ! std::strcmp(arg1, "bool var::select(CVR sortselectclause) const")) acno = 44;
	else if ( ! std::strcmp(arg1, "bool var::deletelist(CVR listname) const")) acno = 45;
	else if ( ! std::strcmp(arg1, "bool var::savelist(CVR listname)")) acno = 46;
	else if ( ! std::strcmp(arg1, "bool var::getlist(CVR listname) const")) acno = 47;
	else if ( ! std::strcmp(arg1, "bool var::formlist(CVR keys, CVR fieldno)")) acno = 48;
	else if ( ! std::strcmp(arg1, "bool var::makelist(CVR listname)")) acno = 49;
	else if ( ! std::strcmp(arg1, "bool var::hasnext() const")) acno = 50;
	else if ( ! std::strcmp(arg1, "bool var::readnext(VARREF key, VARREF valueno) const")) acno = 51;
	else if ( ! std::strcmp(arg1, "bool var::readnext(VARREF record, VARREF key, VARREF valueno) const")) acno = 52;
	else if ( ! std::strcmp(arg1, "bool var::createindex(CVR fieldname, CVR dictfile) const")) acno = 53;
	else if ( ! std::strcmp(arg1, "bool var::deleteindex(CVR fieldname) const")) acno = 54;
	else if ( ! std::strcmp(arg1, "var var::listfiles() const")) acno = 55;
	else if ( ! std::strcmp(arg1, "var var::dblist() const")) acno = 56;
	else if ( ! std::strcmp(arg1, "bool var::cursorexists()")) acno = 57;
	else if ( ! std::strcmp(arg1, "var var::listindex(CVR filename) const")) acno = 58;
	else if ( ! std::strcmp(arg1, "var var::reccount(CVR filename_or_handle_or_null) const")) acno = 59;
	else if ( ! std::strcmp(arg1, "var var::flushindex(CVR filename=) const")) acno = 60;
	else if ( ! std::strcmp(arg1, "bool var::input()")) acno = 61;
	else if ( ! std::strcmp(arg1, "bool var::input(CVR prompt")) acno = 62;
	else if ( ! std::strcmp(arg1, "bool var::inputn(const int nchars")) acno = 63;
	else if ( ! std::strcmp(arg1, "VARREF var::default_to(CVR defaultvalue) const")) acno = 64;
	else if ( ! std::strcmp(arg1, "VARREF var::default_from(CVR defaultvalue)")) acno = 65;
	else if ( ! std::strcmp(arg1, "char var::toChar() const")) acno = 66;
	else if ( ! std::strcmp(arg1, "std::string var::toString() &&")) acno = 67;
	else if ( ! std::strcmp(arg1, "std::string var::toString() const&")) acno = 68;
	else if ( ! std::strcmp(arg1, "var var::len() const")) acno = 69;
	else if ( ! std::strcmp(arg1, "var var::textlen()")) acno = 70;
	else if ( ! std::strcmp(arg1, "std::u32string var::to_u32string() const")) acno = 71;
	else if ( ! std::strcmp(arg1, "VARREF var::trim(SV trimchars)")) acno = 72;
	else if ( ! std::strcmp(arg1, "VARREF var::trimmer(SV trimchars)")) acno = 73;
	else if ( ! std::strcmp(arg1, "VARREF var::trimfirst(SV trimchars) const&")) acno = 74;
	else if ( ! std::strcmp(arg1, "VARREF var::trimmerfirst(SV trimchars)")) acno = 75;
	else if ( ! std::strcmp(arg1, "VARREF var::trimlast(SV trimchars) const&")) acno = 76;
	else if ( ! std::strcmp(arg1, "VARREF var::trimmerlast(SV trimchars)")) acno = 77;
	else if ( ! std::strcmp(arg1, "VARREF var::trimboth(SV trimchars) const&")) acno = 78;
	else if ( ! std::strcmp(arg1, "VARREF var::trimmerboth(SV trimchars)")) acno = 79;
	else if ( ! std::strcmp(arg1, "VARREF var::inverter()")) acno = 80;
	else if ( ! std::strcmp(arg1, "VARREF var::ucaser()")) acno = 81;
	else if ( ! std::strcmp(arg1, "VARREF var::lcaser()")) acno = 82;
	else if ( ! std::strcmp(arg1, "VARREF var::tcaser()")) acno = 83;
	else if ( ! std::strcmp(arg1, "VARREF var::fcaser()")) acno = 84;
	else if ( ! std::strcmp(arg1, "VARREF var::normalizer()")) acno = 85;
	else if ( ! std::strcmp(arg1, "var var::unique()")) acno = 86;
	else if ( ! std::strcmp(arg1, "var var::seq() const")) acno = 87;
	else if ( ! std::strcmp(arg1, "var var::textseq() const")) acno = 88;
	else if ( ! std::strcmp(arg1, "VARREF var::paster(const int pos1, const int length, SV insertstr)")) acno = 89;
	else if ( ! std::strcmp(arg1, "VARREF var::paster(const int pos1, SV insertstr)")) acno = 90;
	else if ( ! std::strcmp(arg1, "var var::prefix(SV insertstr)")) acno = 91;
	else if ( ! std::strcmp(arg1, "VARREF var::prefixer(SV insertstr)")) acno = 92;
	else if ( ! std::strcmp(arg1, "VARREF var::popper()")) acno = 93;
	else if ( ! std::strcmp(arg1, "VARREF var::move(VARREF tovar)")) acno = 94;
	else if ( ! std::strcmp(arg1, "var var::str(const int num) const")) acno = 95;
	else if ( ! std::strcmp(arg1, "var var::space() const")) acno = 96;
	else if ( ! std::strcmp(arg1, "VARREF var::cropper()")) acno = 97;
	else if ( ! std::strcmp(arg1, "VARREF var::lowerer()")) acno = 98;
	else if ( ! std::strcmp(arg1, "VARREF var::raiser()")) acno = 99;
	else if ( ! std::strcmp(arg1, "VARREF var::converter(SV fromchars,SV tochars)")) acno = 100;
	else if ( ! std::strcmp(arg1, "VARREF var::converter(CVR fromchars,CVR tochars)")) acno = 101;
	else if ( ! std::strcmp(arg1, "VARREF var::parser(char sepchar)")) acno = 102;
	else if ( ! std::strcmp(arg1, "var var::fcount(SV sep) const")) acno = 103;
	else if ( ! std::strcmp(arg1, "var var::count(SV str) const")) acno = 104;
	else if ( ! std::strcmp(arg1, "var var::index(SV substr,const int startindex) const")) acno = 105;
	else if ( ! std::strcmp(arg1, "var var::indexr(SV substr,const int startindex) const")) acno = 106;
	else if ( ! std::strcmp(arg1, "var var::index(SV substr) const")) acno = 107;
	else if ( ! std::strcmp(arg1, "var var::xlate(CVR filename,CVR fieldno, const char* mode) const")) acno = 108;
	else if ( ! std::strcmp(arg1, "var var::numberinwords(in number, in langname_or_locale_id)")) acno = 109;
	else if ( ! std::strcmp(arg1, "var var::iconv(const char* conversion) const")) acno = 110;
	else if ( ! std::strcmp(arg1, "var var::oconv(const char* conversion) const")) acno = 111;
	else if ( ! std::strcmp(arg1, "var var::mod(CVR limit) const")) acno = 112;
	else if ( ! std::strcmp(arg1, "var var::mod(double limit) const")) acno = 113;
	else if ( ! std::strcmp(arg1, "var var::mod(const int limit) const")) acno = 114;
	else if ( ! std::strcmp(arg1, "var var::abs() const")) acno = 115;
	else if ( ! std::strcmp(arg1, "var var::sin() const")) acno = 116;
	else if ( ! std::strcmp(arg1, "var var::cos() const")) acno = 117;
	else if ( ! std::strcmp(arg1, "var var::tan() const")) acno = 118;
	else if ( ! std::strcmp(arg1, "var var::atan() const")) acno = 119;
	else if ( ! std::strcmp(arg1, "var var::loge() const")) acno = 120;
	else if ( ! std::strcmp(arg1, "var var::sqrt() const")) acno = 121;
	else if ( ! std::strcmp(arg1, "var var::pwr(CVR exponent) const")) acno = 122;
	else if ( ! std::strcmp(arg1, "var var::exp() const")) acno = 123;
	else if ( ! std::strcmp(arg1, "var var::osshell() const")) acno = 124;
	else if ( ! std::strcmp(arg1, "var var::osshellread() const")) acno = 125;
	else if ( ! std::strcmp(arg1, "var var::osshellwrite(CVR oscmd) const")) acno = 126;
	else if ( ! std::strcmp(arg1, "bool var::osopen(CVR osfilename, const char* locale)")) acno = 127;
	else if ( ! std::strcmp(arg1, "bool var::osopenx(CVR osfilename, const char* locale)")) acno = 128;
	else if ( ! std::strcmp(arg1, "bool var::osread(const char* osfilename, const char* codepage")) acno = 129;
	else if ( ! std::strcmp(arg1, "bool var::to_codepage(const char* codepage) const")) acno = 130;
	else if ( ! std::strcmp(arg1, "bool var::from_codepage(const char* codepage) const")) acno = 131;
	else if ( ! std::strcmp(arg1, "bool var::oswrite(CVR osfilename, const char* codepage) const")) acno = 132;
	else if ( ! std::strcmp(arg1, "bool var::osbwrite(CVR osfilevar, VARREF offset) const")) acno = 133;
	else if ( ! std::strcmp(arg1, "bool var::osbread(CVR osfilevar, VARREF offset, const int bytesize")) acno = 134;
	else if ( ! std::strcmp(arg1, "bool var::osrename(CVR new_dirpath_or_filepath) const")) acno = 135;
	else if ( ! std::strcmp(arg1, "bool var::oscopy(CVR new_dirpath_or_filepath) const")) acno = 136;
	else if ( ! std::strcmp(arg1, "bool var::osmove(CVR new_dirpath_or_filepath) const")) acno = 137;
	else if ( ! std::strcmp(arg1, "bool var::osrmdir(bool evenifnotempty) const")) acno = 138;
	else if ( ! std::strcmp(arg1, "var var::oslist(CVR globpattern, const int mode) const")) acno = 139;
	else if ( ! std::strcmp(arg1, "var var::oscwd(const char* newpath) const")) acno = 140;
	else if ( ! std::strcmp(arg1, "var var::oscwd() const")) acno = 141;
	else if ( ! std::strcmp(arg1, "void var::ossleep(const int milliseconds) const")) acno = 142;
	else if ( ! std::strcmp(arg1, "var var::rnd() const")) acno = 143;
	else if ( ! std::strcmp(arg1, "void var::initrnd() const")) acno = 144;
	else if ( ! std::strcmp(arg1, "bool var::osgetenv(const char* envcode)")) acno = 145;
	else if ( ! std::strcmp(arg1, "bool var::ossetenv(const char* envcode) const")) acno = 146;
	else if ( ! std::strcmp(arg1, "CVR var::put(std::ostream& ostream1) const")) acno = 147;
	else if ( ! std::strcmp(arg1, "var var::match(SV regex, SV regex_options) const")) acno = 148;
	else if ( ! std::strcmp(arg1, "VARREF var::replacer(SV what, SV with)")) acno = 149;
	else if ( ! std::strcmp(arg1, "var var::replace(const rex& regex, SV replacement) const")) acno = 150;
	else if ( ! std::strcmp(arg1, "var var::field(SV separatorx,const int fieldnx,const int nfieldsx) const")) acno = 151;
	else if ( ! std::strcmp(arg1, "VARREF var::fieldstorer(SV separator0,const int fieldnx,const int nfieldsx, CVR replacementx)")) acno = 152;
	else if ( ! std::strcmp(arg1, "bool var::locate(CVR target, VARREF setting, const int fieldno/*=0*/,const int valueno/*=0*/) const")) acno = 153;
	else if ( ! std::strcmp(arg1, "bool var::locate(CVR target, VARREF setting, const int fieldno/*=0*/,const int valueno/*=0*/) const")) acno = 154;
	else if ( ! std::strcmp(arg1, "bool var::locate(CVR target) const")) acno = 155;
	else if ( ! std::strcmp(arg1, "bool var::locateby(const char* ordercode, CVR target, VARREF setting) const")) acno = 156;
	else if ( ! std::strcmp(arg1, "bool var::locateby(const char* ordercode, CVR target, VARREF setting, const int fieldno, const int valueno/*=0*/) const")) acno = 157;
	else if ( ! std::strcmp(arg1, "bool var::locatebyusing(const char* ordercode, const char* usingchar, CVR target, VARREF setting, const int fieldno=0, const int valueno=0, const int valueno=0) const")) acno = 158;
	else if ( ! std::strcmp(arg1, "bool var::locateusing(const char* usingchar, CVR target) const")) acno = 159;
	else if ( ! std::strcmp(arg1, "bool var::locateusing(const char* usingchar, CVR target, VARREF setting, const int fieldno/*=0*/, const int valueno/*=0*/, const int subvalueno/*=0*/) const")) acno = 160;
	else if ( ! std::strcmp(arg1, "var var::f(const int argfieldn, const int argvaluen, const int argsubvaluen) const")) acno = 161;
	else if ( ! std::strcmp(arg1, "VARREF var::remover(int fieldno,int valueno,int subvalueno)")) acno = 162;
	else if ( ! std::strcmp(arg1, "VARREF var::r(int fieldno,int valueno,int subvalueno,CVR replacement)")) acno = 163;
	else if ( ! std::strcmp(arg1, "VARREF var::inserter(const int fieldno,const int valueno,const int subvalueno,const VARREF insertion)")) acno = 164;
	else if ( ! std::strcmp(arg1, "bool var::starts(SV str) const")) acno = 165;
	else if ( ! std::strcmp(arg1, "bool var::ends(SV str) const")) acno = 166;
	else if ( ! std::strcmp(arg1, "bool var::contains(SV str) const")) acno = 167;
	else if ( ! std::strcmp(arg1, "var var::first(const size_t length) const")) acno = 168;
	else if ( ! std::strcmp(arg1, "VARREF var::firster(const size_t length)")) acno = 169;
	else if ( ! std::strcmp(arg1, "var var::last(const size_t length) const")) acno = 170;
	else if ( ! std::strcmp(arg1, "VARREF var::laster(const size_t length)")) acno = 171;
	else if ( ! std::strcmp(arg1, "var var::cut(const int length) const")) acno = 172;
	else if ( ! std::strcmp(arg1, "VARREF var::cutter(const int length)")) acno = 173;
	else if ( ! std::strcmp(arg1, "VARREF var::substrer(const int startindex1,const int length)")) acno = 174;
	else if ( ! std::strcmp(arg1, "var var::at(const int charno) const")) acno = 175;
	else if ( ! std::strcmp(arg1, "var var::multivalued(const char* opcode, CVR var2) const")) acno = 176;
	else if ( ! std::strcmp(arg1, "var var::substr(const int startindex1, VARREF delimiterchars, int& endindex) const")) acno = 177;
	else if ( ! std::strcmp(arg1, "var var::substr2(VARREF startindex1, VARREF delimiterno) const")) acno = 178;
	else if ( ! std::strcmp(arg1, "var var::sumall() const")) acno = 179;
	else if ( ! std::strcmp(arg1, "var var::sum() const")) acno = 180;
	else if ( ! std::strcmp(arg1, "var var::sum(SV separator) const")) acno = 181;
	else if ( ! std::strcmp(arg1, "bool var::setxlocale() const")) acno = 182;
	else if ( ! std::strcmp(arg1, "VARREF var::quoter()")) acno = 183;
	else if ( ! std::strcmp(arg1, "VARREF var::squoter()")) acno = 184;
	else if ( ! std::strcmp(arg1, "VARREF var::unquoter()")) acno = 185;
	else if ( ! std::strcmp(arg1, "CVR var::swap(CVR var2) const")) acno = 186;
	else if ( ! std::strcmp(arg1, "VARREF var::swap(VARREF var2)")) acno = 187;
	else if ( ! std::strcmp(arg1, "var var::textwidth() const")) acno = 188;
	else if ( ! std::strcmp(arg1, "var var::first() const")) acno = 189;
	else if ( ! std::strcmp(arg1, "var var::last() const")) acno = 190;
	else if ( ! std::strcmp(arg1, "VARREF var::firster()")) acno = 191;
	else if ( ! std::strcmp(arg1, "VARREF var::laster()")) acno = 192;
	else if ( ! std::strcmp(arg1, "var var::search(SV regex, int startchar1, SV regex_options) const")) acno = 193;
	else if ( ! std::strcmp(arg1, "std::wstring var::to_wstring() const")) acno = 194;

	else {
		//throw std::runtime_error("");
		//throw 500;
		std::cerr << "Error: " << __PRETTY_FUNCTION__ << ": Unrecognised time account title: '" << arg1 << "'. Must be added to timebank.h and timebank.txt" << std::endl;
		throw arg1;
		//static_assert(false);
		//static_assert(arg1);
	}

	return acno + 499;
}

} // namespace exodus
#endif // EXODUS_TIMEBANK
#endif // EXODUS_LIBEXODUS_EXODUS_TIMEBANK_H_
