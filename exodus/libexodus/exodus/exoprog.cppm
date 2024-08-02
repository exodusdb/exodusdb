module;

#include <fmt/format.h>
#include <fmt/core.h>

#include <exodus/vardefs.h>

#include <exodus/exoimpl.h>
#include <exodus/exocallable.h>
#include <exodus/exoprog.h>

#include <exodus/format.h>
#include <exodus/exofuncs.h>

//#include <exodus/exomacros.h>
#include <exodus/range.h>

export module exoprog;

export import var;

export namespace exo {
//	using exo::callable_ioconv_custom;
	using exo::ExodusProgramBase;
//	using exo::ExodusProgramBase::Callable;
	using exo::Callable;

	using exo::NamedCommon;
	using exo::ExoEnv;

	using exo::lasterror;

	using exo::MVStop;
	using exo::MVAbort;
	using exo::MVAbortAll;
	using exo::MVLogoff;

	using exo::range;
	using exo::reverse_range;

	// free functions from exofuncs.h

	using exo::exodus_main;
	using exo::osgetenv;
	using exo::osgetenv;
	using exo::ossetenv;
	using exo::ostempdirpath;
	using exo::ostempfilename;
	using exo::assigned;
	using exo::unassigned;
	using exo::move;
	using exo::swap;
	using exo::date;
	using exo::time;
	using exo::ostime;
	using exo::timestamp;
	using exo::timestamp;
	using exo::ossleep;
	using exo::oswait;
	using exo::osopen;
	using exo::osclose;
	using exo::osbread;
	using exo::osbwrite;
	using exo::osbread;
	using exo::osbwrite;
	using exo::oswrite;
	using exo::osread;
	using exo::osread;
	using exo::osremove;
	using exo::osrename;
	using exo::oscopy;
	using exo::osmove;
	using exo::oslist;
	using exo::oslistf;
	using exo::oslistd;
	using exo::osinfo;
	using exo::osfile;
	using exo::osdir;
	using exo::osmkdir;
	using exo::osrmdir;
	using exo::oscwd;
	using exo::oscwd;
	using exo::osflush;
	using exo::ospid;
	using exo::ostid;
	using exo::osshell;
	using exo::osshellwrite;
	using exo::osshellread;
	using exo::osshellread;
	using exo::backtrace;
	using exo::setxlocale;
	using exo::getxlocale;
	using exo::isnum;
	using exo::num;
	using exo::abs;
	using exo::pwr;
	using exo::exp;
	using exo::sqrt;
	using exo::sin;
	using exo::cos;
	using exo::tan;
	using exo::atan;
	using exo::loge;
	using exo::mod;
	using exo::mod;
	using exo::mod;
	using exo::integer;
	using exo::floor;
	using exo::round;
	using exo::rnd;
	using exo::initrnd;
	using exo::getprompt;
	using exo::setprompt;
	using exo::input;
	using exo::input;
	using exo::inputn;
	using exo::isterminal;
	using exo::hasinput;
	using exo::eof;
	using exo::echo;
	using exo::breakon;
	using exo::breakoff;
	using exo::len;
	using exo::textlen;
	using exo::textwidth;
	using exo::convert;
	using exo::converter;
	using exo::textconvert;
	using exo::textconverter;
	using exo::replace;
	using exo::replacer;
	using exo::replace;
	using exo::replacer;
	using exo::ucase;
	using exo::ucaser;
	using exo::lcase;
	using exo::lcaser;
	using exo::tcase;
	using exo::tcaser;
	using exo::fcase;
	using exo::fcaser;
	using exo::normalize;
	using exo::normalizer;
	using exo::uniquer;
	using exo::unique;
	using exo::invert;
	using exo::inverter;
	using exo::invert;
	using exo::lower;
	using exo::lowerer;
	using exo::raise;
	using exo::raiser;
	using exo::paste;
	using exo::paster;
	using exo::paste;
	using exo::paster;
	using exo::prefix;
	using exo::prefixer;
	using exo::pop;
	using exo::popper;
	using exo::quote;
	using exo::quoter;
	using exo::squote;
	using exo::squoter;
	using exo::unquote;
	using exo::unquoter;
	using exo::fieldstore;
	using exo::fieldstorer;
	using exo::trim;
	using exo::trimfirst;
	using exo::trimlast;
	using exo::trimboth;
	using exo::trimmer;
	using exo::trimmerfirst;
	using exo::trimmerlast;
	using exo::trimmerboth;
	using exo::first;
	using exo::last;
	using exo::first;
	using exo::last;
	using exo::chr;
	using exo::textchr;
	using exo::match;
	using exo::seq;
	using exo::textseq;
	using exo::str;
	using exo::space;
	using exo::fcount;
	using exo::count;
	using exo::substr;
	using exo::substr;
	using exo::substrer;
	using exo::substrer;
	using exo::starts;
	using exo::end;
	using exo::contains;
	using exo::index;
	using exo::indexn;
	using exo::indexr;
	using exo::field;
	using exo::field2;
	using exo::substr2;
	using exo::split;
	using exo::join;
	using exo::pickreplace;
	using exo::pickreplace;
	using exo::pickreplace;
	using exo::extract;
	using exo::insert;
	using exo::insert;
	using exo::insert;
	using exo::remove;
	using exo::pickreplacer;
	using exo::pickreplacer;
	using exo::pickreplacer;
	using exo::inserter;
	using exo::inserter;
	using exo::inserter;
	using exo::remover;
	using exo::locate;
	using exo::locate;
	using exo::locate;
	using exo::locateby;
	using exo::locateby;
	using exo::locateby;
	using exo::locateby;
	using exo::locateusing;
	using exo::locateusing;
	using exo::locateusing;
	using exo::sum;
	using exo::sum;
	using exo::crop;
	using exo::cropper;
	using exo::sort;
	using exo::sorter;
	using exo::reverse;
	using exo::reverser;
	using exo::shuffle;
	using exo::shuffler;
	using exo::parser;
	using exo::parser;
	using exo::connect;
	using exo::disconnect;
	using exo::disconnectall;
	using exo::dbcreate;
	using exo::dblist;
	using exo::dbcopy;
	using exo::dbdelete;
	using exo::createfile;
	using exo::deletefile;
	using exo::clearfile;
	using exo::renamefile;
	using exo::listfiles;
	using exo::reccount;
	using exo::createindex;
	using exo::deleteindex;
	using exo::listindex;
	using exo::begintrans;
	using exo::statustrans;
	using exo::rollbacktrans;
	using exo::committrans;
	using exo::cleardbcache;
	using exo::lock;
	using exo::unlock;
	using exo::unlockall;
	using exo::open;
	using exo::open;
	using exo::read;
	using exo::readc;
	using exo::readf;
	using exo::write;
	using exo::writec;
	using exo::writef;
	using exo::updaterecord;
	using exo::insertrecord;
	using exo::dimread;
	using exo::dimwrite;
	using exo::xlate;
	using exo::xlate;
	using exo::lasterror;
	using exo::loglasterror;

	using exo::output;
	using exo::outputl;
	using exo::outputt;

	using exo::errput;
	using exo::errputl;

	using exo::logput;
	using exo::logputl;

	using exo::printx;
	using exo::printl;
	using exo::printt;

	using exo::format;
	using exo::print;
	using exo::println;

//	using exo::vprint;
//	using exo::vprintln;
//	using exo::xvformat;
//	using exo::xformat;

}
export namespace fmt {
	using fmt::format;
	using fmt::print;
//	using fmt::println;
}