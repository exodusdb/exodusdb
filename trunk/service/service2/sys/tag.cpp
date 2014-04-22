#include <exodus/library.h>
libraryinit()

//adds html tags to text after basic preprocessing
//eg. output=tag("-1000","td align=right");
//    output becomes <td align right><nobr>somedata</nobr></td>
//    blank cells for td/th get filled with &nbsp;
//		any & characters get replaced with &amp;
//		unless they are part of html codes &nbsp; &amp; &lt; &gt; &infin;

function main(in tag, in text) {

	if (not tag)
		return text;

	var output = text;
	var tagname = tag.field(L" ",1).lcase();

	//empty td/th cells get filled with &nbsp;
	if (output == "") {
		if (tagname == "td" or tagname == "th") {
			output = "&nbsp;";
		}

	//negative numbers get "no break" tags
	} else if (output[1] == L"-") {
		var tagname = tag.field(L" ",1).lcase();
		if (tagname == L"td" or tagname == L"th") {
			if (var(L"1234567890").index(output[2], 1)) {
				output = L"<nobr>" ^ output ^ L"</nobr>";
			}
		}
	}

	//convert "&" to &amp; unless it is for the usual & html codes
	for (var ii = 1; ii <= 99999; ++ii) {

		var charn = output.index(L"&", ii);
		if (not charn)
			break;

		//if (not(var("nbsp amp lt gt infin").locateusing(output.substr(charn + 1,99999).field(";", 1), " "))) {
		//	output.splicer(charn, 1, "&amp;");
		//}
		if (
				 (   output[charn+1]==L'n'
				 and output[charn+2]=='b'
				 and output[charn+3]=='s'
				 and output[charn+4]=='p'
				 and output[charn+5]==';'
				 )
				 or
				 (   output[charn+1]=='a'
				 and output[charn+2]=='m'
				 and output[charn+3]=='p'
				 and output[charn+4]==';'
				 )
				 or
				 (	 output[charn+1]=='l'
				 and output[charn+2]=='t'
				 and output[charn+3]==';'
				 )
				 or
				 (	 output[charn+1]=='g'
				 and output[charn+2]=='t'
				 and output[charn+3]==';'
				 )
				 or
				 (	 output[charn+1]=='i'
				 and output[charn+2]=='n'
				 and output[charn+3]=='f'
				 and output[charn+4]=='i'
				 and output[charn+5]=='n'
				 and output[charn+6]==';'
				 )
			) {
			output.splicer(charn, 1, "&amp;");			
		}
	}

	if (tag ne "") {
		output = "<" ^ tag ^ ">" ^ output ^ "</" ^ tagname ^ ">";
	}

	return output;

}

libraryexit()
