#include <exodus/mvprogram.h>

namespace exodus {

DLL_PUBLIC
	ExodusProgramBase::ExodusProgramBase(MvEnvironment& inmv)
		:mv(inmv)
		{
			cache_dictid_ = L"";
			cache_perform_libid_ = L"";
		};

//virtual var main();

DLL_PUBLIC
	ExodusProgramBase::~ExodusProgramBase()
		{};

var ExodusProgramBase::perform(const var& sentence) {
	//THISIS(L"var MvEnvironment::perform(const var& sentence)")
	//ISSTRING(sentence)

	//return ID^L"*"^dictid;

	//wire up the the library linker to have the current mvenvironment
	//if (!perform_exodusfunctorbase_.mv_)
	//	perform_exodusfunctorbase_.mv_=this;

	var libid = sentence.field(L" ", 1);

	//open the library routine
	if (libid != cache_perform_libid_) {
		cache_perform_libid_ = libid;

		//if (!perform_exodusfunctorbase_.mv_)
			perform_exodusfunctorbase_.mv_ = (&mv);

		std::string str_libname = libid.toString();
		//std::string str_funcname="main";
		//if (!exodusfunctorbase_.init(str_libname.c_str(),str_funcname.c_str()))
		//	throw MVException(L"perform() Cannot find Library "^str_libname^L", or function "^str_funcname^L" is not present");
		if (!perform_exodusfunctorbase_.init2(str_libname.c_str(),
				"exodusprogrambasecreatedelete_"))
			throw MVException(
					L"perform() Cannot find shared library \"" ^ str_libname
							^ L"\", or function \"libraryexit()\" is not present");
	}

	//save some environment
	var savesentence;
	savesentence.transfer(mv.SENTENCE);

	//set new perform environment
	mv.SENTENCE = sentence;

	//move to perform() in efb like calldict? or move both here?

	//same code in "sharedlibsubroutine.h" of callable external functions (returns var, zero arguments version)

	//define a function type (pExodusProgramBaseMemberFunction)
	//that can call the shared library object member function
	//with the right arguments and returning a var
	typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

	//call the shared library object main function with the right args, returning a var
	//std::cout<<"precall"<<std::endl;
	mv.ANS =
			CALLMEMBERFUNCTION(*(perform_exodusfunctorbase_.pobject_),
					((pExodusProgramBaseMemberFunction) (perform_exodusfunctorbase_.pmemberfunction_)))();
	//std::cout<<"postcall"<<std::endl;

	//restore some environment
	//std::cout<<"pretransfer"<<std::endl;
	savesentence.transfer(mv.SENTENCE);
	//std::cout<<"posttransfer"<<std::endl;

	return mv.ANS;

}

var ExodusProgramBase::calculate(const var& dictid, const var& dictfile, const var& id, const var& record, const var& mvno) {
	mv.DICT.exchange(dictfile);
	mv.ID.exchange(id);
	mv.RECORD.exchange(record);
	mv.MV.exchange(mvno);

	var result=calculate(dictid);

	mv.DICT.exchange(dictfile);
	mv.ID.exchange(id);
	mv.RECORD.exchange(record);
	mv.MV.exchange(mvno);

	return result;
}

var ExodusProgramBase::calculate(const var& dictid) {
	//THISIS(L"var MvEnvironment::calculate(const var& dictid)")
	//ISSTRING(dictid)

	//return ID^L"*"^dictid;

	//wire up the the library linker to have the current mvenvironment
	//if (!dict_exodusfunctorbase_.mv_)
		dict_exodusfunctorbase_.mv_ = (&mv);

	//get the dictionary record so we know how to extract the correct field or call the right library
	bool newlibfunc;
	if (cache_dictid_ != dictid) {
		newlibfunc = true;
		if (not mv.DICT)
			throw MVException(
					L"calculate(" ^ dictid
							^ L") mv.DICT file variable has not been set");
		if (not cache_dictrec_.read(mv.DICT, dictid))
			if (not cache_dictrec_.read(mv.DICT, dictid.lcase()))
				throw MVException(
					L"calculate(" ^ dictid ^ L") dictionary record not in mv.DICT "
							^ mv.DICT.quote());
		cache_dictid_ = dictid;
	} else
		newlibfunc = false;

	var dicttype = cache_dictrec_(1);

	//F type dictionaries
	if (dicttype == L"F") {

		//check field number is numeric
		var fieldno = cache_dictrec_(2);
		if (!fieldno.isnum())
			return L"";

		//field no > 0
		if (fieldno)
			return mv.RECORD(fieldno, mv.MV);

		//field no 0
		else {
			var keypart = cache_dictrec_(5);
			if (keypart && keypart.isnum())
				return mv.ID.field(L"*", keypart);
			else
				return mv.ID;

		}
/*
	} else if (dicttype == L"S") {
		//TODO deduplicate various exodusfunctorbase code spread around calculate mvipc* etc
		if (newlibfunc) {
			std::string str_libname = mv.DICT.lcase().toString();
			std::string str_funcname = dictid.toString();
			if (!dict_exodusfunctorbase_.init(str_libname.c_str(),str_funcname.c_str()))
				throw MVException(
						L"calculate() Cannot find Library " ^ str_libname
								^ L", or function " ^ str_funcname
								^ L" is not present");
		}

		return dict_exodusfunctorbase_.calldict();
		//return mv.ANS;
	}
*/

	} else if (dicttype == L"S") {
		//TODO deduplicate various exodusfunctorbase code spread around calculate mvipc* etc
		if (newlibfunc) {
			std::string str_libname = mv.DICT.lcase().toString();
			std::string str_funcname = (L"exodusprogrambasecreatedelete_" ^ dictid.lcase()).toString();
			if (!dict_exodusfunctorbase_.initdict(str_libname.c_str(),str_funcname.c_str()))
				throw MVException(
						L"calculate() Cannot find Library " ^ str_libname
								^ L", or function " ^ dictid.lcase()
								^ L" is not present");
		}

		//return dict_exodusfunctorbase_.calldict();
		//return mv.ANS;

		//define a function type (pExodusProgramBaseMemberFunction)
		//that can call the shared library object member function
		//with the right arguments and returning a var
		typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

		//call the shared library object main function with the right args, returning a var
		//std::cout<<"precall"<<std::endl;
		mv.ANS =
				CALLMEMBERFUNCTION(*(dict_exodusfunctorbase_.pobject_),
						((pExodusProgramBaseMemberFunction) (dict_exodusfunctorbase_.pmemberfunction_)))();
		//std::cout<<"postcall"<<std::endl;

		return mv.ANS;

	}

	throw MVException(
			L"calculate(" ^ dictid ^ L") " ^ mv.DICT ^ L" Invalid dictionary type "
					^ dicttype.quote());
	return L"";

}

}//of namespace exodus

