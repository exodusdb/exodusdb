#include <exodus/library.h>
libraryinit()

function main(io filename, in mode, out filetitle, out triggers) {
    //c gen io,in,out,out

    //purpose:
    //1. given a filename and mode (READ/READO/READU/WRITE/WRITEU/DELETE)
    //return the filetitle or '' if filename not valid or not allowed
    //and a list of pre and post routines
    //2. also called for SELECT to fix up the filename (_->. etc)

    filetitle = "";
    triggers = "";

    //determine pre and post delete routines
    var preread = "";
    var prereadmode = "PREREAD";
    var postread = "";
    var postreadmode = "POSTREAD";
    var updatesubs = "";
    var replaceread = "";
    var replacewrite = "";
    var replacedelete = "";

    //////
    //exit:
    //////

    if (mode.index("READ")) {
        //1/2
        if (preread) {
            triggers.r(1, preread ^ FM ^ prereadmode);
        }
        //3/4
        if (postread) {
            triggers.r(3, postread ^ FM ^ postreadmode);
        }
        //5/6
        if (replaceread) {
            triggers.r(5, replaceread ^ FM ^ "READ");
        }
    } else if (mode.index("WRITE")) {
        if (updatesubs) {
            triggers.r(1, updatesubs ^ FM ^ "PREWRITE");
            triggers.r(3, updatesubs ^ FM ^ "POSTWRITE");
            if (replacewrite) {
                triggers.r(5, updatesubs ^ FM ^ "WRITE");
            }
        }
    } else if (mode.index("DELETE")) {
        if (updatesubs) {
            triggers.r(1, updatesubs ^ FM ^ "PREDELETE");
            triggers.r(3, updatesubs ^ FM ^ "POSTDELETE");
            if (replacedelete) {
                triggers.r(5, updatesubs ^ FM ^ "DELETE");
            }
        }
    } else if (mode == "LOCK") {
    } else if (mode == "RELOCK") {
    } else if (mode == "UNLOCK") {
    } else if (mode == "SELECT") {
    } else if (mode == "GETINDEXVALUES") {
    } else {
        call mssg(mode.quote() ^ " is invalid in LISTEN3");
        return 0;
    }

    //allow all files for the time being
    if (filetitle == "") {
        filetitle = filename;
    }

    //c++ variation
    if (not(VOLUMES)) {
        for (var ii = 1; ii <= 5; ii+=2) {
            var tt = triggers.a(ii);
            tt.lcaser();
            tt.converter(".", "");
            triggers.r(ii, tt);
        };//ii;
    }

    return 0;
}

libraryexit()

