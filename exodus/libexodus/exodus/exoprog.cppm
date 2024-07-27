module;

#include <exodus/exoimpl.h>
#include <exodus/exocallable.h>
#include <exodus/exoprog.h>

#include <exodus/format.h>
#include <exodus/exofuncs.h>
//#include <fmt/core.h>

//#include <exodus/exomacros.h>
#include <exodus/range.h>

export module exoprog;

export import var;

namespace exo {
//	export using exo::callable_ioconv_custom;
	export using exo::ExodusProgramBase;
//	export using exo::ExodusProgramBase::Callable;
	export using exo::Callable;

	export using exo::NamedCommon;
	export using exo::ExoEnv;

	export using exo::lasterror;

	export using exo::MVStop;
	export using exo::MVAbort;
	export using exo::MVAbortAll;
	export using exo::MVLogoff;

	export using exo::range;
	export using exo::reverse_range;

	// free functions from exofuncs.h

	export using exo::exodus_main;
	export using exo::osgetenv;
	export using exo::osgetenv;
	export using exo::ossetenv;
	export using exo::ostempdirpath;
	export using exo::ostempfilename;
	export using exo::assigned;
	export using exo::unassigned;
	export using exo::move;
	export using exo::swap;
	export using exo::date;
	export using exo::time;
	export using exo::ostime;
	export using exo::timestamp;
	export using exo::timestamp;
	export using exo::ossleep;
	export using exo::oswait;
	export using exo::osopen;
	export using exo::osclose;
	export using exo::osbread;
	export using exo::osbwrite;
	export using exo::osbread;
	export using exo::osbwrite;
	export using exo::oswrite;
	export using exo::osread;
	export using exo::osread;
	export using exo::osremove;
	export using exo::osrename;
	export using exo::oscopy;
	export using exo::osmove;
	export using exo::oslist;
	export using exo::oslistf;
	export using exo::oslistd;
	export using exo::osinfo;
	export using exo::osfile;
	export using exo::osdir;
	export using exo::osmkdir;
	export using exo::osrmdir;
	export using exo::oscwd;
	export using exo::oscwd;
	export using exo::osflush;
	export using exo::ospid;
	export using exo::ostid;
	export using exo::osshell;
	export using exo::osshellwrite;
	export using exo::osshellread;
	export using exo::osshellread;
	export using exo::backtrace;
	export using exo::setxlocale;
	export using exo::getxlocale;
	export using exo::isnum;
	export using exo::num;
	export using exo::abs;
	export using exo::pwr;
	export using exo::exp;
	export using exo::sqrt;
	export using exo::sin;
	export using exo::cos;
	export using exo::tan;
	export using exo::atan;
	export using exo::loge;
	export using exo::mod;
	export using exo::mod;
	export using exo::mod;
	export using exo::integer;
	export using exo::floor;
	export using exo::round;
	export using exo::rnd;
	export using exo::initrnd;
	export using exo::getprompt;
	export using exo::setprompt;
	export using exo::input;
	export using exo::input;
	export using exo::inputn;
	export using exo::isterminal;
	export using exo::hasinput;
	export using exo::eof;
	export using exo::echo;
	export using exo::breakon;
	export using exo::breakoff;
	export using exo::len;
	export using exo::textlen;
	export using exo::textwidth;
	export using exo::convert;
	export using exo::converter;
	export using exo::textconvert;
	export using exo::textconverter;
	export using exo::replace;
	export using exo::replacer;
	export using exo::replace;
	export using exo::replacer;
	export using exo::ucase;
	export using exo::ucaser;
	export using exo::lcase;
	export using exo::lcaser;
	export using exo::tcase;
	export using exo::tcaser;
	export using exo::fcase;
	export using exo::fcaser;
	export using exo::normalize;
	export using exo::normalizer;
	export using exo::uniquer;
	export using exo::unique;
	export using exo::invert;
	export using exo::inverter;
	export using exo::invert;
	export using exo::lower;
	export using exo::lowerer;
	export using exo::raise;
	export using exo::raiser;
	export using exo::paste;
	export using exo::paster;
	export using exo::paste;
	export using exo::paster;
	export using exo::prefix;
	export using exo::prefixer;
	export using exo::pop;
	export using exo::popper;
	export using exo::quote;
	export using exo::quoter;
	export using exo::squote;
	export using exo::squoter;
	export using exo::unquote;
	export using exo::unquoter;
	export using exo::fieldstore;
	export using exo::fieldstorer;
	export using exo::trim;
	export using exo::trimfirst;
	export using exo::trimlast;
	export using exo::trimboth;
	export using exo::trimmer;
	export using exo::trimmerfirst;
	export using exo::trimmerlast;
	export using exo::trimmerboth;
	export using exo::first;
	export using exo::last;
	export using exo::first;
	export using exo::last;
	export using exo::chr;
	export using exo::textchr;
	export using exo::match;
	export using exo::seq;
	export using exo::textseq;
	export using exo::str;
	export using exo::space;
	export using exo::fcount;
	export using exo::count;
	export using exo::substr;
	export using exo::substr;
	export using exo::substrer;
	export using exo::substrer;
	export using exo::starts;
	export using exo::end;
	export using exo::contains;
	export using exo::index;
	export using exo::indexn;
	export using exo::indexr;
	export using exo::field;
	export using exo::field2;
	export using exo::substr2;
	export using exo::split;
	export using exo::join;
	export using exo::pickreplace;
	export using exo::pickreplace;
	export using exo::pickreplace;
	export using exo::extract;
	export using exo::insert;
	export using exo::insert;
	export using exo::insert;
	export using exo::remove;
	export using exo::pickreplacer;
	export using exo::pickreplacer;
	export using exo::pickreplacer;
	export using exo::inserter;
	export using exo::inserter;
	export using exo::inserter;
	export using exo::remover;
	export using exo::locate;
	export using exo::locate;
	export using exo::locate;
	export using exo::locateby;
	export using exo::locateby;
	export using exo::locateby;
	export using exo::locateby;
	export using exo::locateusing;
	export using exo::locateusing;
	export using exo::locateusing;
	export using exo::sum;
	export using exo::sum;
	export using exo::crop;
	export using exo::cropper;
	export using exo::sort;
	export using exo::sorter;
	export using exo::reverse;
	export using exo::reverser;
	export using exo::shuffle;
	export using exo::shuffler;
	export using exo::parser;
	export using exo::parser;
	export using exo::connect;
	export using exo::disconnect;
	export using exo::disconnectall;
	export using exo::dbcreate;
	export using exo::dblist;
	export using exo::dbcopy;
	export using exo::dbdelete;
	export using exo::createfile;
	export using exo::deletefile;
	export using exo::clearfile;
	export using exo::renamefile;
	export using exo::listfiles;
	export using exo::reccount;
	export using exo::createindex;
	export using exo::deleteindex;
	export using exo::listindex;
	export using exo::begintrans;
	export using exo::statustrans;
	export using exo::rollbacktrans;
	export using exo::committrans;
	export using exo::cleardbcache;
	export using exo::lock;
	export using exo::unlock;
	export using exo::unlockall;
	export using exo::open;
	export using exo::open;
	export using exo::read;
	export using exo::readc;
	export using exo::readf;
	export using exo::write;
	export using exo::writec;
	export using exo::writef;
	export using exo::updaterecord;
	export using exo::insertrecord;
	export using exo::dimread;
	export using exo::dimwrite;
	export using exo::xlate;
	export using exo::xlate;
	export using exo::lasterror;
	export using exo::loglasterror;

	export using exo::output;
	export using exo::outputl;
	export using exo::outputt;

	export using exo::errput;
	export using exo::errputl;

	export using exo::logput;
	export using exo::logputl;

	export using exo::printx;
	export using exo::printl;
	export using exo::printt;

	export using exo::vprint;
	export using exo::vprintln;
	export using exo::xvformat;
	export using exo::xformat;

}
