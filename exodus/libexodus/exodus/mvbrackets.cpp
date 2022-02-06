#include <mv.h>
#include <string>

namespace exodus {

#if 0

//class var_brackets_proxy
class PUBLIC var_brackets_proxy {
   public:
	var& var_;
	int index_;

	// Constructor from a var and an index
	var_brackets_proxy(VARREF var1, int index) : var_(var1), index_(index) {
	};

	// Implicit conversion to var
	operator var() const {
		return var_.at(index_);
	}

//	// Implicit conversion to char
//	operator char() const {
//		return var_.at(index_).toChar();
//	}

	// Operator assign
	void operator=(const char char1) {
		var_.splicer(index_, 1, char1);
	}

	// Operator ==
	template<class T>
	bool operator==(T compare) {
		return var_.at(index_) == compare;
	}

	// Operator !=
	template<class T>
	bool operator!=(T compare) {
		return var_.at(index_) != compare;
	}

	// Operator ^
	template<class T>
	var operator^(T appendage) {
		return var_.at(index_) ^ appendage;
	}

};

// could be used if var_brackets_proxy holds a string perhaps for performance

//implicit conversion to var
var_brackets_proxy::operator var() const {
	//before first character?
	if (index_ < 1) {
		index_ += str_.length();
		//overly negative - return ""
		if (index_ < 1)
			index_ = 1;
	}
	//off end - return ""
	else if (uint(index_) > str_.length())
		return "";

	//within range
	return str_[index_ - 1];
}

//operator assign a char
void var_brackets_proxy::operator=(char char1) {
	//before first character?
	if (index_ < 1) {
		index_ += str_.length();
		//overly negative - return ""
		if (index_ < 1)
			index_ = 1;
	}
	//off end - return ""
	else if (uint(index_) > str_.length()) {
		str_.push_back(char1);
	} else {
		str_[index_ - 1] = char1;
	}

	return;
}
#endif

}  //namespace exodus
