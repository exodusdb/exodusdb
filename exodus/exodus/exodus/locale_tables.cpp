//
//
// this is Windows specific names, to be used as parameter of std::locale() constructor
//
// Mapping LCID to Language: http://www.livio.net/main/charset.asp
// Proper 'localewin' strings: http://docs.moodle.org/en/Table_of_locales
//
namespace exodus
{
char * lcid2localename( int lcid)
{
	switch( lcid)
	{
							//package_name 	lang_name 	locale 	localewin 	localewincharset
							//af_utf8 	Afrikaans 	af_ZA.UTF-8 	Afrikaans_South Africa.1252 	WINDOWS-1252
							//sq_utf8 	Albanian 	sq_AL.UTF-8 	Albanian_Albania.1250 	WINDOWS-1250
							//ar_utf8 	Arabic 	ar_SA.UTF-8 	Arabic_Saudi Arabia.1256 	WINDOWS-1256
							//eu_utf8 	Basque 	eu_ES.UTF-8 	Basque_Spain.1252 	WINDOWS-1252
							//be_utf8 	Belarusian 	be_BY.UTF-8 	Belarusian_Belarus.1251 	WINDOWS-1251
							//bs_utf8 	Bosnian 	bs_BA.UTF-8 	Serbian (Latin) 	WINDOWS-1250
							//bg_utf8 	Bulgarian 	bg_BG.UTF-8 	Bulgarian_Bulgaria.1251 	WINDOWS-1251
							//ca_utf8 	Catalan 	ca_ES.UTF-8 	Catalan_Spain.1252 	WINDOWS-1252
							//hr_utf8 	Croatian 	hr_HR.UTF-8 	Croatian_Croatia.1250 	WINDOWS-1250
							//zh_cn_utf8 	Chinese (Simplified) 	zh_CN.UTF-8 	Chinese_China.936 	CP936
							//zh_tw_utf8 	Chinese (Traditional) 	zh_TW.UTF-8 	Chinese_Taiwan.950 	CP950
							//cs_utf8 	Czech 	cs_CZ.UTF-8 	Czech_Czech Republic.1250 	WINDOWS-1250
							//da_utf8 	Danish 	da_DK.UTF-8 	Danish_Denmark.1252 	WINDOWS-1252
							//nl_utf8 	Dutch 	nl_NL.UTF-8 	Dutch_Netherlands.1252 	WINDOWS-1252
							//en_utf8 	English 	en.UTF-8 	English_Australia.1252 	-empty string-
case 1033:
case 2057: return "English_Australia.1252";		//en_us_utf8 	English (US) 	-parent en_utf8 used- 	-parent en_utf8 used- 	-parent en_utf8 used-
							//et_utf8 	Estonian 	et_EE.UTF-8 	Estonian_Estonia.1257 	WINDOWS-1257
							//fa_utf8 	Farsi 	fa_IR.UTF-8 	Farsi_Iran.1256 	WINDOWS-1256
							//fil_utf8 	Filipino 	ph_PH.UTF-8 	Filipino_Philippines.1252 	WINDOWS-1252
							//fi_utf8 	Finnish 	fi_FI.UTF-8 	Finnish_Finland.1252 	WINDOWS-1252
case 1036: return "French_France.1252";		//fr_utf8 	French 	(fr_FR.UTF-8 or fr_CH.UTF-8 or fr_BE.UTF-8)	French_France.1252 	WINDOWS-1252
							//fr_ca_utf8 	French (Canada) 	fr_CA.UTF-8 	French_Canada.1252 	-parent fr_utf8 used-
							//ga_utf8 	Gaelic 	ga.UTF-8 	Gaelic; Scottish Gaelic 	WINDOWS-1252
							//gl_utf8 	Gallego 	gl_ES.UTF-8 	Galician_Spain.1252 	WINDOWS-1252
							//ka_utf8 	Georgian 	ka_GE.UTF-8 	Georgian_Georgia.65001 	-empty string-
case 1031: return "German_Germany.1252";	//de_utf8 	German 	de_DE.UTF-8 	German_Germany.1252 	WINDOWS-1252
							//de_du_utf8 	German (Personal) 	de_DE.UTF-8 	-parent de_utf8 used- 	-parent de_utf8 used-
case 1032: return "Greek_Greece.1253";		//el_utf8 	Greek 	el_GR.UTF-8 	Greek_Greece.1253 	WINDOWS-1253
							//gu_utf8 	Gujarati 	gu.UTF-8 	Gujarati_India.0 	
							//he_utf8 	Hebrew 	he_IL.utf8 	Hebrew_Israel.1255 	WINDOWS-1255
							//hi_utf8 	Hindi 	hi_IN.UTF-8 	Hindi.65001 	-empty string-
							//hu_utf8 	Hungarian 	hu.UTF-8 	Hungarian_Hungary.1250 	WINDOWS-1250
							//is_utf8 	Icelandic 	is_IS.UTF-8 	Icelandic_Iceland.1252 	WINDOWS-1252
							//id_utf8 	Indonesian 	id_ID.UTF-8 	Indonesian_indonesia.1252 	WINDOWS-1252
							//it_utf8 	Italian 	it_IT.UTF-8 	Italian_Italy.1252 	WINDOWS-1252
							//ja_utf8 	Japanese 	ja_JP.UTF-8 	Japanese_Japan.932 	CP932
							//kn_utf8 	Kannada 	kn_IN.UTF-8 	Kannada.65001 	-empty string-
							//km_utf8 	Khmer 	km_KH.UTF-8 	Khmer.65001 	-empty string-
							//ko_utf8 	Korean 	ko_KR.UTF-8 	Korean_Korea.949 	EUC-KR
							//lo_utf8 	Lao 	lo_LA.UTF-8 	Lao_Laos.UTF-8 	WINDOWS-1257
							//lt_utf8 	Lithuanian 	lt_LT.UTF-8 	Lithuanian_Lithuania.1257 	WINDOWS-1257
							//lv_utf8 	Latvian 	lat.UTF-8 	Latvian_Latvia.1257 	WINDOWS-1257
							//ml_utf8 	Malayalam 	ml_IN.UTF-8 	Malayalam_India.x-iscii-ma 	x-iscii-ma
							//ms_utf8 	Malaysian 	id_ID.UTF-8 	Indonesian_indonesia.1252 	WINDOWS-1252
							//mi_tn_utf8 	Maori (Ngai Tahu) 	mi_NZ.UTF-8 	Maori.1252 	WINDOWS-1252
							//mi_wwow_utf8 	Maori (Waikoto Uni) 	mi_NZ.UTF-8 	Maori.1252 	WINDOWS-1252
							//mn_utf8 	Mongolian 	mn.UTF-8 	Cyrillic_Mongolian.1251 	
							//no_utf8 	Norwegian 	no_NO.UTF-8 	Norwegian_Norway.1252 	WINDOWS-1252
							//no_gr_utf8 	Norwegian (Primary) 	no_NO.UTF-8 	-parent no_utf8 used- 	-parent no_utf8 used-
							//nn_utf8 	Nynorsk 	nn_NO.UTF-8 	Norwegian-Nynorsk_Norway.1252 	WINDOWS-1252
							//pl_utf8 	Polish 	pl.UTF-8 	Polish_Poland.1250 	WINDOWS-1250
							//pt_utf8 	Portuguese 	pt_PT.UTF-8 	Portuguese_Portugal.1252 	WINDOWS-1252
							//pt_br_utf8 	Portuguese (Brazil) 	pt_BR.UTF-8 	Portuguese_Brazil.1252 	WINDOWS-1252
							//ro_utf8 	Romanian 	ro_RO.UTF-8 	Romanian_Romania.1250 	WINDOWS-1250
							//ru_utf8 	Russian 	ru_RU.UTF-8 	Russian_Russia.1251 	WINDOWS-1251
							//sm_utf8 	Samoan 	mi_NZ.UTF-8 	Maori.1252 	WINDOWS-1252
							//sr_utf8 	Serbian 	sr_CS.UTF-8 	Serbian (Cyrillic)_Serbia and Montenegro.1251 	WINDOWS-1251
							//sk_utf8 	Slovak 	sk_SK.UTF-8 	Slovak_Slovakia.1250 	WINDOWS-1250
							//sl_utf8 	Slovenian 	sl_SI.UTF-8 	Slovenian_Slovenia.1250 	WINDOWS-1250
							//so_utf8 	Somali 	so_SO.UTF-8 	not found! 	not found!
							//es_utf8 	Spanish (International) 	es_ES.UTF-8 	Spanish_Spain.1252 	WINDOWS-1252
							//sv_utf8 	Swedish 	sv_SE.UTF-8 	Swedish_Sweden.1252 	WINDOWS-1252
							//tl_utf8 	Tagalog 	tl.UTF-8 	not found! 	not found!
							//ta_utf8 	Tamil 	ta_IN.UTF-8 	English_Australia.1252 	
							//th_utf8 	Thai 	th_TH.UTF-8 	Thai_Thailand.874 	WINDOWS-874
							//to_utf8 	Tongan 	mi_NZ.UTF-8' 	Maori.1252 	WINDOWS-1252
							//tr_utf8 	Turkish 	tr_TR.UTF-8 	Turkish_Turkey.1254 	WINDOWS-1254
case 1058: return "Ukrainian_Ukraine.1251";			//uk_utf8 	Ukrainian 	uk_UA.UTF-8 	Ukrainian_Ukraine.1251 	WINDOWS-1251
							//vi_utf8 	Vietnamese 	vi_VN.UTF-8 	Vietnamese_Viet Nam.1258 	WINDOWS-1258
		case 0:
		default:	return "";
	}

}
}