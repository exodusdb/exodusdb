module;

#include <exodus/var.h>
#include <exodus/dim.h>
#include <exodus/rex.h>

export module var;

export namespace exo {

	using exo::var_base;
	using exo::var_mid;
	using exo::var;
	using exo::var_iter;

	using exo::dim;
	using exo::dim_iter;
	using exo::rex;
	using exo::VarError;
	using exo::operator""_var;
	using exo::operator""_rex;
	using exo::SMALLEST_NUMBER;

	using exo::var_proxy1;
	using exo::var_proxy2;
	using exo::var_proxy3;

	// Not actually required to export since they will be exported along with all var members anyway
	// Only available for specific export if specifically listed in var.h
	// Not sufficient if only declared via friendship
//	using exo::begin;
//	using exo::end;

	// Exported automatically since they are friended in class var
	// and cannot be reexported since they are not actually declared outside of the friend declarations?
//	using exo::operator==;
//	using exo::operator!=;
//	using exo::operator<;
//	using exo::operator>;
//	using exo::operator<=;
//	using exo::operator>=;
//	using exo::operator<=>;

	using exo::varint_t;

	using exo::VarDBException;
	using exo::VarDebug;
	using exo::VarDivideByZero;
	using exo::VarError;
	using exo::VarInvalidPointer;
	using exo::VarNonNumeric;
	using exo::VarNonPositive;
	using exo::VarNotImplemented;
	using exo::VarNumOverflow;
	using exo::VarNumUnderflow;
	using exo::VarOutOfMemory;
	using exo::VarUnassigned;
	using exo::VarUndefined;

	using exo::DimDimensionedZero;
	using exo::DimIndexOutOfBounds;
	using exo::DimNotDimensioned;

	using exo::CVR;
	using exo::VARREF;
	using exo::SV;

//	using exo::EOL;
//	using exo::OSSLASH;
//	using exo::OSSLASH_IS_BACKSLASH;

//	using exo::_CPP_STANDARD;
//	using exo::PLATFORM;

	using exo::_OS_NAME;
	using exo::_OS_VERSION;

	using exo::RM;
	using exo::FM;
	using exo::VM;
	using exo::SM;
	using exo::TM;
	using exo::ST;

	using exo::BS;
	using exo::DQ;
	using exo::SQ;


// Generated using
// nm -D -S -C /usr/local/lib/libexodus.so|\grep exo::[\\w]+ -oP|awk '{print "\tusing " $1 ";"}'|sort|uniq

// See also for all exo:: classes
// nm -D -S -C /usr/local/lib/libexodus.so|\grep exo::[\\w]+::[\\w]+ -oP|sort|uniq|awk -F: '{print $3}'|uniq

// See also for member functions of classes
// nm -D -S -C /usr/local/lib/libexodus.so|\grep exo::[\\w]+::[\\w]+ -oP|sort|uniq

///	using exo::BS;
//	using exo::Callable;
///	using exo::DQ;
///	using exo::DimDimensionedZero;
///	using exo::DimIndexOutOfBounds;
///	using exo::DimNotDimensioned;
//	using exo::EOL;
//	using exo::ExoEnv;
//	using exo::ExodusProgramBase;
///	using exo::FM;
//	using exo::MVAbort;
//	using exo::MVAbortAll;
//	using exo::MVLogoff;
//	using exo::MVStop;
//	using exo::OSSLASH;
//	using exo::PLATFORM;
//	using exo::RELOAD_req;
///	using exo::RM;
///	using exo::SM;
///	using exo::SQ;
///	using exo::ST;
//	using exo::TERMINATE_req;
///	using exo::TM;
///	using exo::VM;

///	using exo::VarDBException;
///	using exo::VarDebug;
///	using exo::VarDivideByZero;
///	using exo::VarError;
///	using exo::VarInvalidPointer;
///	using exo::VarNonNumeric;
///	using exo::VarNonPositive;
///	using exo::VarNotImplemented;
///	using exo::VarNumOverflow;
///	using exo::VarNumUnderflow;
///	using exo::VarOutOfMemory;
///	using exo::VarUnassigned;
///	using exo::VarUndefined;

///	using exo::_OS_NAME;
///	using exo::_OS_VERSION;
//	using exo::abs;
//	using exo::assigned;
//	using exo::atan;
//	using exo::backtrace;
//	using exo::begin;
//	using exo::begintrans;
//	using exo::breakoff;
//	using exo::breakon;
//	using exo::chr;
//	using exo::cleardbcache;
//	using exo::clearfile;
//	using exo::committrans;
//	using exo::connect;
//	using exo::contains;
//	using exo::convert;
//	using exo::converter;
//	using exo::cos;
//	using exo::count;
//	using exo::createfile;
//	using exo::createindex;
//	using exo::crop;
//	using exo::cropper;
//	using exo::date;
//	using exo::dbcopy;
//	using exo::dbcreate;
//	using exo::dbdelete;
//	using exo::dblist;
//	using exo::debug;
//	using exo::deletefile;
//	using exo::deleteindex;
///	using exo::dim;
//	using exo::dimread;
//	using exo::dimwrite;
//	using exo::disconnect;
//	using exo::disconnectall;
//	using exo::echo;
//	using exo::end;
//	using exo::eof;
//	using exo::exo_backtrace;
//	using exo::exo_savestack;
//	using exo::exodus_main;
//	using exo::exp;
//	using exo::extract;
//	using exo::fcase;
//	using exo::fcaser;
//	using exo::fcount;
//	using exo::field2;
//	using exo::field;
//	using exo::fieldstore;
//	using exo::fieldstorer;
//	using exo::first;
//	using exo::floor;
//	using exo::getexecpath;
//	using exo::gethostname;
//	using exo::getprocessn;
//	using exo::getxlocale;
//	using exo::global_mutex_threadstream;
//	using exo::hasinput;
//	using exo::index;
//	using exo::indexn;
//	using exo::indexr;
//	using exo::initrnd;
//	using exo::input;
//	using exo::inputn;
//	using exo::insert;
//	using exo::inserter;
//	using exo::insertrecord;
//	using exo::integer;
//	using exo::invert;
//	using exo::inverter;
//	using exo::isnum;
//	using exo::isterminal;
//	using exo::join;
//	using exo::last;
//	using exo::lasterror;
//	using exo::lcase;
//	using exo::lcaser;
//	using exo::len;
//	using exo::listfiles;
//	using exo::listindex;
//	using exo::locate;
//	using exo::locateby;
//	using exo::locateusing;
//	using exo::lock;
//	using exo::loge;
//	using exo::loglasterror;
//	using exo::lower;
//	using exo::lowerer;
//	using exo::match;
//	using exo::mod;
//	using exo::move;
//	using exo::normalize;
//	using exo::normalizer;
//	using exo::num;
//	using exo::open;
//	using exo::operator==;
//	using exo::osbread;
//	using exo::osbwrite;
//	using exo::osclose;
//	using exo::oscopy;
//	using exo::oscwd;
//	using exo::osdir;
//	using exo::osfile;
//	using exo::osflush;
//	using exo::osgetenv;
//	using exo::osinfo;
//	using exo::oslist;
//	using exo::oslistd;
//	using exo::oslistf;
//	using exo::osmkdir;
//	using exo::osmove;
//	using exo::osopen;
//	using exo::ospid;
//	using exo::osread;
//	using exo::osremove;
//	using exo::osrename;
//	using exo::osrmdir;
//	using exo::ossetenv;
//	using exo::osshell;
//	using exo::osshellread;
//	using exo::osshellwrite;
//	using exo::ossleep;
//	using exo::ostempdirpath;
//	using exo::ostempfilename;
//	using exo::ostid;
//	using exo::ostime;
//	using exo::oswait;
//	using exo::oswrite;
//	using exo::parser;
//	using exo::paste;
//	using exo::paster;
//	using exo::pickreplace;
//	using exo::pickreplacer;
//	using exo::pop;
//	using exo::popper;
//	using exo::prefix;
//	using exo::prefixer;
//	using exo::pwr;
//	using exo::quote;
//	using exo::quoter;
//	using exo::raise;
//	using exo::raiser;
//	using exo::read;
//	using exo::readc;
//	using exo::readf;
//	using exo::reccount;
//	using exo::remove;
//	using exo::remover;
//	using exo::renamefile;
//	using exo::replace;
//	using exo::replacer;
//	using exo::reverse;
//	using exo::reverser;
///	using exo::rex;
//	using exo::rnd;
//	using exo::rollbacktrans;
//	using exo::round;
//	using exo::seq;
//	using exo::setxlocale;
//	using exo::shuffle;
//	using exo::shuffler;
//	using exo::sin;
//	using exo::sort;
//	using exo::sorter;
//	using exo::space;
//	using exo::split;
//	using exo::sqrt;
//	using exo::squote;
//	using exo::squoter;
//	using exo::starts;
//	using exo::statustrans;
//	using exo::str;
//	using exo::substr2;
//	using exo::substr;
//	using exo::substrer;
//	using exo::sum;
//	using exo::swap;
//	using exo::tan;
//	using exo::tcase;
//	using exo::tcaser;
//	using exo::textchr;
//	using exo::textconvert;
//	using exo::textconverter;
//	using exo::textlen;
//	using exo::textseq;
//	using exo::textwidth;
//	using exo::time;
//	using exo::timestamp;
//	using exo::trim;
//	using exo::trimboth;
//	using exo::trimfirst;
//	using exo::trimlast;
//	using exo::trimmer;
//	using exo::trimmerboth;
//	using exo::trimmerfirst;
//	using exo::trimmerlast;
//	using exo::ucase;
//	using exo::ucaser;
//	using exo::unassigned;
//	using exo::unique;
//	using exo::uniquer;
//	using exo::unlock;
//	using exo::unlockall;
//	using exo::unquote;
//	using exo::unquoter;
//	using exo::updaterecord;

///	using exo::var;
///	using exo::var_base;
///	using exo::var_iter;
///	using exo::var_mid;
///	using exo::var_proxy1;
///	using exo::var_proxy2;
///	using exo::var_proxy3;

//	using exo::write;
//	using exo::writec;
//	using exo::writef;

}
