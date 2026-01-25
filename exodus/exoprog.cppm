module;

#include <var/vardefs.h>
#include <var/format.h>
//#include <var/exodebug.h>

#include <exodus/exoimpl.h>
#include <exodus/exoprog.h>
#include <exodus/exofuncs.h>

export module exoprog;

export import var;

// Unfortunately we need to #include <exodus/varformatter.h>
// in exodus.h in order to generate templates on the fly
// that sadly cannot seem to be generated from a module
// possibly because they need to be in std namespace
import std;
export namespace std {
	using std::formatter;
//	using std::format_to;
	using std::make_format_args;
	using std::format_error;
	using std::string;
	using std::stoi;
	using std::string_view;
}

export namespace exo {

// Here for keywords.sh

// var::f
// var::append
// var::appender
// var::let

// FlowControl::function
// FlowControl::subroutine
// FlowControl::func
// FlowControl::subr
// FlowControl::call
// FlowControl::gosub

// var::on
// var::from
// var::with
// var::to

// Extracted from
// nm -D -S -C /usr/local/lib/libexodus.so|\grep exo::[\\w]+ -oP|awk '{print "\tusing " $1 ";"}'|sort|uniq

// See also for all exo:: classes
// nm -D -S -C /usr/local/lib/libexodus.so|\grep exo::[\\w]+::[\\w]+ -oP|sort|uniq|awk -F: '{print $3}'|uniq

// See also for member functions of classes
// nm -D -S -C /usr/local/lib/libexodus.so|\grep exo::[\\w]+::[\\w]+ -oP|sort|uniq

//	using exo::dim;
//	using exo::rex;

	using exo::ExoEnv;

//	using exo::var;
//	using exo::var_base;
//	using exo::var_iter;
//	using exo::var_stg;
//	using exo::var_proxy1;
//	using exo::var_proxy2;
//	using exo::var_proxy3;

	using exo::BS;
	using exo::DQ;
	using exo::SQ;
	using exo::NL;
	using exo::EOL;

	using exo::OSSLASH;
	using exo::PLATFORM;
	using exo::_OS_NAME;
	using exo::_OS_VERSION;
	using exo::_COMPILER;

	using exo::RM;
	using exo::FM;
	using exo::VM;
	using exo::SM;
	using exo::TM;
	using exo::STM;

	using exo::Callable;
	using exo::ExoProgram;
	using exo::pExoProgram;
	using exo::pExoProgram_MemberFunc;

	using exo::ExoExit;
	using exo::ExoAbort;
	using exo::ExoAbortAll;
	using exo::ExoStop;

	using exo::function;
	using exo::subroutine;
	using exo::func;
	using exo::subr;

	using exo::Job;
	using exo::Jobs;
	using exo::ThreadPool;
	using exo::ThreadSafeQueue;
	using exo::TaskManager;
//	using exo::ExoCommon;

//	using exo::DimIndexOutOfBounds;
//	using exo::DimUndimensioned;

//	using exo::VarDBException;
//	using exo::VarDebug;
//	using exo::VarDivideByZero;
//	using exo::VarError;
//	using exo::VarInvalidPointer;
//	using exo::VarNonNumeric;
//	using exo::VarNonPositive;
//	using exo::VarNotImplemented;
//	using exo::VarNumOverflow;
//	using exo::VarNumUnderflow;
//	using exo::VarOutOfMemory;
//	using exo::VarUnassigned;
//	using exo::VarUnconstructed;

//	using exo::exo_backtrace;
//	using exo::exo_savestack;

	// This is mostly exporting the free functions defined in exofuncs.h

	using exo::reset_range;

	using exo::abs;

	using exo::append;
	using exo::appender;

	using exo::assigned;
	using exo::unassigned;

	using exo::atan;
	using exo::attach;
	using exo::backtrace;

	using exo::begintrans;
	using exo::committrans;
	using exo::rollbacktrans;
	using exo::statustrans;

	using exo::breakoff;
	using exo::breakon;

	using exo::chr;
	using exo::clearcache;
	using exo::clearfile;
	using exo::clone;
	using exo::close;
	using exo::connect;
	using exo::contains;

	using exo::convert;
	using exo::converter;

	using exo::cos;
	using exo::count;
	using exo::createfile;
	using exo::createindex;

	using exo::crop;
	using exo::cropper;

	using exo::cut;
	using exo::cutter; // why was this missing?

	using exo::date;

	using exo::dbcopy;
	using exo::dbcreate;
	using exo::dbdelete;
	using exo::dblist;

//	using exo::debug;
	using exo::defaulter;
	using exo::deletec;
	using exo::deletefile;
	using exo::deleteindex;
//	using exo::deleterecord;

	using exo::disconnect;
	using exo::disconnectall;

	using exo::dump;
	using exo::echo;
	using exo::empty;
	using exo::ends;
	using exo::eof;

	using exo::errput;
	using exo::errputl;

	using exo::exp;
	using exo::extract;

	using exo::fcase;
	using exo::fcaser;

	using exo::fcount;
	using exo::field2;
	using exo::field;

	using exo::fieldstore;
	using exo::fieldstorer;

	using exo::first;
	using exo::floor;

	using exo::from_codepage;
	using exo::to_codepage;

	using exo::getexecpath;
	using exo::gethostname;
	using exo::getprocessn;
	using exo::getprompt;
	using exo::getxlocale;

	using exo::hash;
	using exo::hasinput;

	using exo::index;
	using exo::indexn;
	using exo::indexr;

	using exo::initrnd;

	using exo::input;
	using exo::inputn;

	using exo::insert;
	using exo::inserter;

	using exo::insertrecord;
	using exo::integer;

	using exo::invert;
	using exo::inverter;

	using exo::isnum;
	using exo::isterminal;
	using exo::join;
	using exo::keypressed;
	using exo::last;
	using exo::lasterror;

	using exo::lcase;
	using exo::lcaser;

	using exo::len;
	using exo::listfiles;
	using exo::listindex;

	using exo::listed;
	using exo::locate;
	using exo::locateby;
	using exo::locateusing;

	using exo::lock;
	using exo::loge;

	using exo::loglasterror;
	using exo::logput;
	using exo::logputl;

	using exo::lower;
	using exo::lowerer;

	using exo::match;
	using exo::mod;
	using exo::move;

	using exo::normalize;
	using exo::normalizer;

	using exo::num;
	using exo::open;
	using exo::or_default;

	using exo::osbread;
	using exo::osbwrite;
	using exo::osclose;
	using exo::oscopy;
	using exo::oscwd;
	using exo::osdir;
	using exo::osfile;
	using exo::osflush;
	using exo::osgetenv;
	using exo::osinfo;
	using exo::oslist;
	using exo::oslistd;
	using exo::oslistf;
	using exo::osmkdir;
	using exo::osmove;
	using exo::osopen;
	using exo::ospid;
	using exo::osread;
	using exo::osremove;
	using exo::osrename;
	using exo::osrmdir;
	using exo::ossetenv;
	using exo::osshell;
	using exo::osshellread;
	using exo::osshellwrite;
	using exo::osprocess;
	using exo::ossleep;
	using exo::ostempdir;
	using exo::ostempfile;
	using exo::ostid;
	using exo::ostime;
	using exo::oswait;
	using exo::oswrite;

	using exo::output;
	using exo::outputl;
	using exo::outputt;

	using exo::parse;
	using exo::parser;

	using exo::paste;
	using exo::paster;

	using exo::pop;
	using exo::popper;

	using exo::prefix;
	using exo::prefixer;

	using exo::printl;
	using exo::printt;
	using exo::printx;
	using exo::format;
//#if EXO_FORMAT && __GLIBCXX__
	using exo::print;
	using exo::println;
//#endif

	using exo::pwr;

	using exo::quote;
	using exo::quoter;

	using exo::raise;
	using exo::raiser;

	using exo::read;
	using exo::readc;
	using exo::readf;

	using exo::reccount;

	using exo::remove;
	using exo::remover;

	using exo::renamefile;

	using exo::replace;
	using exo::replacer;

	using exo::reverse;
	using exo::reverser;

	using exo::rnd;
	using exo::round;
	using exo::search;
	using exo::ord;
	using exo::setprompt;
	using exo::setxlocale;
	using exo::setprecision;
	using exo::getprecision;

	using exo::randomize;
	using exo::randomizer;

	using exo::sin;
	using exo::min;
	using exo::max;

	using exo::sort;
	using exo::sorter;

	using exo::space;
	using exo::unpack;
	using exo::split;
	using exo::sqlexec;
	using exo::sqrt;

	using exo::squote;
	using exo::squoter;

	using exo::starts;
	using exo::str;

	using exo::substr2;
	using exo::substr;
	using exo::substrer;

	using exo::sum;
	using exo::sumall;
	using exo::stddev;

	using exo::swap;
	using exo::tan;

	using exo::tcase;
	using exo::tcaser;

	using exo::textchr;
	using exo::textchrname;
	using exo::textconvert;
	using exo::textconverter;
	using exo::textlen;
	using exo::textord;
	using exo::textwidth;

	using exo::time;
	using exo::ostimestamp;

	using exo::trim;
	using exo::trimboth;
	using exo::trimfirst;
	using exo::trimlast;
	using exo::trimmer;
	using exo::trimmerboth;
	using exo::trimmerfirst;
	using exo::trimmerlast;

	using exo::ucase;
	using exo::ucaser;

	using exo::unique;
	using exo::uniquer;

	using exo::unlock;
	using exo::unlockall;

	using exo::unquote;
	using exo::unquoter;

	using exo::update;
	using exo::updater;

	using exo::updaterecord;
	using exo::updatekey;

	using exo::version;

	using exo::write;
	using exo::writec;
	using exo::writef;

	using exo::xlate;

}
#if EXO_FORMAT == 1
//	export namespace std {
//		using std::formatter;
//		using std::format;
//	}
	export namespace stdfmt {
		using std::formatter;
		using std::format;
	//	using std::print;
	//	using std::vprint;
	//	using std::println;
	}
#elif EXO_FORMAT == 2 || EXO_FORMAT == 3
	export namespace stdfmt {
		using std::formatter;
		using std::format;
	//	using std::print;
	//	using std::vprint;
	//	using std::println;
	}
#endif // EXO_FORMAT
