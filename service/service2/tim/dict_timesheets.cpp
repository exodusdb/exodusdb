#include <exodus/library.h>

var usern;
var rate;//num
var msg;

libraryinit(activity_name)

function main()  {
	return calculate("ACTIVITY_CODE").xlate("JOB_TYPES", 1, "X");
}
libraryexit(activity_name)


libraryinit(amount)

function main()  {
	var username = ID.field("*", 1);
	if (SECURITY.locate(username, usern, 1)) {
		rate = SECURITY.a(5, usern);
		if (rate == "") {
			rate = 1;
		}
	}else{
		rate = 1;
	}
	var hours = RECORD.a(2, MV);
	var nvs = hours.count(VM) + 1;
	ANS = "";
	for (var vn = 1; vn <= nvs; ++vn) {
		ANS.r(1, vn, (hours.a(1, vn) * rate).oconv("MD20P"));
	};//vn;
	return ANS;
}
libraryexit(amount)


libraryinit(authorised)
#include <validcode2.h>

function main()  {
	var xx;
	if (validcode2(calculate("COMPANY_CODE"), "", calculate("BRAND_CODE").a(1, 1), xx, msg)) {
		ANS = 1;
	}else{
		ANS = 0;
	}
	return ANS;
}
libraryexit(authorised)


libraryinit(authorised1)

function main()  {
	ANS = 1;
	//force authorised to avoid adding WITH AUTHORISED in select2
	return ANS;
}
libraryexit(authorised1)


libraryinit(brand_code)

function main()  {
	return calculate("JOB_NO").xlate("JOBS", 2, "X");
}
libraryexit(brand_code)


libraryinit(brand_name)

function main()  {
	return xlate("JOBS", RECORD.a(1, MV), 2, "X").xlate("BRANDS", 2, "C");
}
libraryexit(brand_name)


libraryinit(client_code)

function main()  {
	return calculate("BRAND_CODE").xlate("BRANDS", 1, "X");
}
libraryexit(client_code)


libraryinit(client_name)

function main()  {
	return xlate("JOBS", RECORD.a(1, MV), 2, "X").xlate("BRANDS", 3, "C");
}
libraryexit(client_name)


libraryinit(company_code)

function main()  {
	return calculate("JOB_NO").xlate("JOBS", 14, "X");
}
libraryexit(company_code)


libraryinit(company_name)

function main()  {
	return calculate("COMPANY_CODE").xlate("COMPANIES", 1, "X");
}
libraryexit(company_name)


libraryinit(day)

function main()  {
	return (ID.field("*", 2)).oconv("D2/E").substr(1,2) + 0;
}
libraryexit(day)


libraryinit(day_of_week)

function main()  {
	return var("Monday/Tuesday/Wednesday/Thursday/Friday/Saturday/Sunday").field("/", calculate("DATE") - 1 % 7 + 1);
}
libraryexit(day_of_week)


libraryinit(department)

#include <generalsubs.h>
function main()  {
	call generalsubs("GETUSERDEPTX," ^ ID.field("*", 1));
	return ANS;
}
libraryexit(department)


libraryinit(department2)

#include <generalsubs.h>
function main()  {
	call generalsubs("GETUSERDEPTX," ^ ID.field("*", 1));
	return capitalise(ANS);
}
libraryexit(department2)


libraryinit(department_code)

function main()  {
	return ID.field("*", 1).xlate("USERS", 5, "X");
}
libraryexit(department_code)


libraryinit(department_code2)

function main()  {
	return ID.field("*", 1).xlate("USERS", 21, "X");
}
libraryexit(department_code2)


libraryinit(details2)

function main()  {
	var activitynames = calculate("ACTIVITY_NAME");
	var ans = calculate("DETAILS");
	var nlines = ans.count(VM) + 1;
	for (var linen = 1; linen <= nlines; ++linen) {
		var detail = ans.a(1, linen);
		detail.swapper(activitynames.a(1, linen), "");
		ans.r(1, linen, detail.trim());
	};//linen;
	return ans;
}
libraryexit(details2)


libraryinit(job_description)

function main()  {
	//only the first line of the description is wanted
	var jobnos = RECORD.a(1, MV);
	var nvs = jobnos.count(VM) + 1;
	var ans = "";
	for (var vn = 1; vn <= nvs; ++vn) {
		ans.r(1, vn, jobnos.a(1, vn).xlate("JOBS", 9, "X").a(1, 1, 1));
	};//vn;
	return ans;
}
libraryexit(job_description)


libraryinit(month)

function main()  {
	return (ID.field("*", 2)).oconv("D2/E").substr(4,2) + 0;
}
libraryexit(month)


libraryinit(period)

function main()  {
	return calculate("JOB_NO").xlate("JOBS", 1, "X");
}
libraryexit(period)


libraryinit(person_name)

function main()  {
	return capitalise(calculate("PERSON_CODE"));
}
libraryexit(person_name)


libraryinit(rate)

function main()  {
	var username = ID.field("*", 1);
	if (SECURITY.locate(username, usern, 1)) {
		ANS = SECURITY.a(5, usern);
	}else{
		ANS = "";
	}
	if (ANS == "") {
		ANS = 1;
	}
	return ANS;
}
libraryexit(rate)


libraryinit(total_hours)

function main()  {
	return (calculate("HOURS")).sum();
}
libraryexit(total_hours)


libraryinit(user_rank)

function main()  {
	return ID.field("*", 1).xlate("USERS", 11, "X");
}
libraryexit(user_rank)


libraryinit(year)

function main()  {
	return (ID.field("*", 2)).oconv("D/E").substr(-4,4) + 0;
}
libraryexit(year)


libraryinit(year_period)

function main()  {
	//date=field(@id,'*',2) 'D2/E'
	//@ans=field(date,'/',3):'.':('00':field(date,'/',2))[-2,2]
	var periods = calculate("PERIOD").outputl("PERIOS=");
	//@ANS=XLATE('JOBS',{JOB_NO},1,'X')
	var nn = periods.count(VM) + (periods ne "");
	var ans = "";
	var period;
	for (var ii = 1; ii <= nn; ++ii) {
		period = periods.a(1, ii);
		ANS.r(1, ii, period.field("/", 2) ^ "." ^ ("00" ^ period.field("/", 1)).substr(-2,2));
	};//ii;
	return ANS;

}

libraryexit(year_period)

