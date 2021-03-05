#include <mv.h>
#include <string>

namespace exodus {

//implicit conversion to var
var_brackets_proxy::operator var() const {
	//before first character?
	if (charindex1 < 1) {
		charindex1 += str1.length();
		//overly negative - return ""
		if (charindex1 < 1)
			charindex1 = 1;
	}
	//off end - return ""
	else if (uint(charindex1) > str1.length())
		return "";

	//within range
	return str1[charindex1 - 1];
}

//operator assign a char
void var_brackets_proxy::operator=(char char1) {
	//before first character?
	if (charindex1 < 1) {
		charindex1 += str1.length();
		//overly negative - return ""
		if (charindex1 < 1)
			charindex1 = 1;
	}
	//off end - return ""
	else if (uint(charindex1) > str1.length()) {
		str1.push_back(char1);
	} else {
		str1[charindex1 - 1] = char1;
	}

	return;
}

}  //namespace exodus
