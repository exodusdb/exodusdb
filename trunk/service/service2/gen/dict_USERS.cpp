#include <exodus/dict.h>

dict(USER_AND_DEPT_NAME) {
//outputl("dict_USERS ... ", RECORD.substr(1,40));
	ANS=RECORD.a(1)^" - "^RECORD.a(5);
//RECORD.substr(1,40).outputl("xRECORD=");
//ID.outputl("xID=");
//DICT.outputl("xDICT=");
//MV.outputl("xMV=");
//ANS.outputl("xANS=");
}

dict(EXAMPLEDICTID2) {
	ANS=RECORD(2)^"x";
}

