#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>

var id;

function main(in mode, io tx, in arg3, io arg4, in arg5) {
	//c sys in,io,in,io,in

	if (mode == "INITMENUS") {

		tx = "<!DOCTYPE html>";
		tx ^= FM ^ "<html>";
		tx ^= FM ^ "<head>";
		tx ^= FM ^ "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />";
		tx ^= FM ^ "</head>";
		tx ^= FM ^ "<body style=\"text-align: left\">";
		tx ^= FM ^ "";
		tx ^= FM ^ "    <div id=\"menudiv\" class=\"menu\" style=\"display: none; text-align: left\">";

	} else if (mode == "ADDMENU") {

		#define menu arg3
		if (arg4.unassigned()) {
			id = "";
		}else{
			id = arg4;
		}

		//<a id="menu_SUPPORT">_Support</a>
		//<div class="menu">

		//use menu title as menu id
		if (not id) {
			id = var(menu).ucase();
			id.converter("_", "");
		}

		//convert _x to <u>x</u> to indicate keyboard shortcut
		var tt = var(menu).index("_");
		if (tt) {
			var(menu).splicer(tt, 2, "<u>" ^ var(menu)[tt + 1] ^ "</u>");
		}

		tx ^= FM ^ "<a id=\"menu_" ^ id ^ "\">" ^ menu ^ "</a>";
		tx ^= FM ^ " <div class=\"menu\">";

		id = "";

	} else if (mode == "SUBMENU") {

		//<a>_Reports</a>
		//<div class="menu">

		var submenu = arg3;

		//convert _x to <u>x</u> to indicate keyboard shortcut
		var tt = submenu.index("_");
		if (tt) {
			submenu.splicer(tt, 2, "<u>" ^ submenu[tt + 1] ^ "</u>");
		}

		tx ^= FM ^ "<a>" ^ submenu ^ "</a>";
		//submenus are same class as menus
		tx ^= FM ^ " <div class=\"menu\">";

	} else if (mode == "ADDITEM") {

		var item = arg3;
		#define href arg4
		#define onclick arg5

		//<a href="../media/schedules.htm?filename=PLANS&amp;plantype=1&ratings=1"><u>M</u>onth Plan File</a><br />
		var props = "";
		//if unassigned(id) else props:=' id=':quote(id)
		if (not(var(onclick).unassigned())) {
			props ^= " onclick = " ^ (var(onclick).quote());
		}

		//convert _x to <u>x</u> to indicate keyboard shortcut
		var tt = item.index("_");
		if (tt) {
			item.splicer(tt, 2, "<u>" ^ item[tt + 1] ^ "</u>");
		}

		tx ^= FM ^ " <a" ^ props ^ " href=\"" ^ href ^ "\">" ^ item ^ "</a><br />";

	} else if (mode == "ADDSEP") {

		tx ^= FM ^ "<hr class=\"menu_hr\" />";

	} else if (mode == "ENDSUBMENU") {

		tx ^= FM ^ "</div>";
		tx ^= FM ^ "<br />";

	} else if (mode == "ENDMENU") {

		tx ^= FM ^ "</div>";

	} else if (mode == "EXITMENUS") {

		tx ^= FM ^ "<button tabindex=\"-1\" style=\"background-color: white; height: 1px; width: 1px; border-style: none; margin: 0; padding: 0\" accesskey=\"M\"";
		var onclickx = "javascript:if (typeof menuonmouseover!=\'undefined\') menuonmouseover(\'click\')";
		tx ^= FM ^ " onclick=" ^ (onclickx.quote()) ^ ">";
		tx ^= FM ^ "</button>";
		tx ^= FM ^ "</body>";

	} else {
		call sysmsg(mode.quote() ^ " invalid mode in MENU.SUBS");
	}

	//prevent href being carried forwards to ADDITEM which use onclick instead
	arg4 = "";

	return 0;
}

libraryexit()
