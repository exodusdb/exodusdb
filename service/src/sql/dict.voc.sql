--
-- PostgreSQL database dump
--

-- Dumped from database version 12.9 (Ubuntu 12.9-0ubuntu0.20.04.1)
-- Dumped by pg_dump version 12.9 (Ubuntu 12.9-0ubuntu0.20.04.1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

ALTER TABLE ONLY dict.voc DROP CONSTRAINT voc_pkey;
DROP TABLE dict.voc;
SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: voc; Type: TABLE; Schema: dict; Owner: exodus
--

CREATE TABLE dict.voc (
    key text NOT NULL,
    data text
);


ALTER TABLE dict.voc OWNER TO exodus;

--
-- Data for Name: voc; Type: TABLE DATA; Schema: dict; Owner: exodus
--

COPY dict.voc (key, data) FROM stdin;
#	RLISTNOT
(	RLIST(
)	RLIST)
<	RLISTLT
<=	RLISTLE
<>	RLISTNOT
=	RLISTEQ
=<	RLISTLE
=>	RLISTGE
>	RLISTGT
>=	RLISTGE
@CRT	GTYPE FMC PART DISPLAY2 SM CONV JUST LEN MASTER_FLAG0
@ID	F0Ref.S0T20
A	RLIST
AFTER	RLISTGT
ALINES	SALINESS0@ANS=COUNT(@RECORD,\\\\0A\\\\)+1IF @RECORD EQ '' THEN @ANS=0R60
ALL	SALLSANS=@ID:@FM:@RECORDreturn ansL200
ALLUPPERCASE	SALLSconvert @lower.case to @upper.case in @recordreturn @record*@ANS=@RECORD*convert @lower.case to @upper.case in @ansANS=@RECORDconvert @lower.case to @upper.case in ans*can cause out of memory*1. doesnt help*declare function memspace*mem=memspace(999999)*2. dont pass back in @ans variable seems to solve it!*transfer @ans to ans*if you return something then it uses it, otherwise it uses what is in @ansreturn @recordT300
ALPHA	SALPHAS0@ANS=@IDCONVERT '0123456789' TO '' IN @ANSL100
AN	RLIST
AND	RLISTAND
ANY	RLIST
ANYDEBUG	SANYDEBUGSif index('*$',@id[1,1],1) then return 0if @id='DEBUG' then return 0if @id='MSG' then return 0if @id='RTP25' then return 0if @id='TEST' then return 0if @id='TESTBASIC' then return 0*upper=@record*convert @lower.case to @upper.case in upperequ upper to @recordif index(upper,'debug',1) then return 1if @id='SENDMAIL' then convert "'EXODUS.ID'" to '' in upperif index(upper,"'EXODUS.ID'",1) and @id<>'INSTALLALLOWHOSTS' then return 1return 0L100
ARE	RLIST
ARENT	RLISTNOT
AVERAGE	RLISTAVERAGE
BECAUSE	RLISTWITH
BEFORE	RLISTLT
BETWEEN	RLISTBETWEEN
BREAK-ON	RLISTBREAK-ON
BY	RLISTBY
BY-DSND	RLISTBY-DSND
BY-EXP	RLISTBY
BY-EXP-DSND	RLISTBY-DSND
CASERATIO	SCASERATIOSMD00Px=@recordconvert @lower.case to '' in xy=@recordconvert @upper.case to '' in yu=len(@record)-len(y)l=len(@record)-len(x)if u>l then @ans=u/(l+1) else @ans=-(l/(u+1))R100
CH	RLISTCH
CLASS	F1CLASSSL1"F""S""G"CHAR(1)
COL-HDR-SUPP	RLISTCS
COLHEAD	RLISTCH
COLUMN_NAME	F0COLUMN_NAMES2L20
COMPUTED_INDEX	F27COMPUTED_INDEXSL10VARCHAR1
CONTAINING	RLIST[]
CONTAINS	RLIST[]
CONV	F7CONVS0L9
CONV2UTF8	SCONV2UTF8Sorigrecord=@recordorigid=@idcall conv2utf8(msg,@recur0,'')@recur1+=1print @(0):@(-4):@recur1:'. ':@rec.count:@ans=msgif @ans else @ans=@record<>origrecord or @id<>origid end*restore otherwise select will not find it again@id=origidT100
CONVERSION	F7CONVERSIONSL10VARCHAR
COUNT	SCountS@ANS=1R100
CROSSREFERENCED	F22CROSSREFERENCEDSR1BOOLEAN1
C_ARGS	SC ARGSS@ans=@record<2>if @ans[1,3] ne '*c ' then @ans=''T100
DATA	SDATAS0@ANS=@RECORDCONVERT \\\\FFFEFDFCFBFAF9F8\\\\ TO '' IN @ANSL100
DATETIME_CREATED	SDate TimeCreatedS[DATETIME]@ans={DATETIME_UPDATED}<1,1>R200
DBL-SPC	RLISTDB
DEFAULT	F29DEFAULTSL10"NULL""CURRENT""USER"'"'0X'"'VARCHAR1
DESC	F14DESCS0T20
DESCRIPTION	F14DESCRIPTIONMT20VARCHAR
DET-SUPP	RLISTDS
DICT	RLISTDICT
DIFFERENT	SDIFFERENTS$INSERT GBP,GENERAL.COMMON*COMMON /DIFFERENT/ LAST.RECCOUNT, COMPARE.FILE*IF COMPARE.FILE EQ '' THEN LAST.RECCOUNT = 9999**IF @RECCOUNT LT LAST.RECCOUNT THEN**  FN=''**  CALL MSG('DIFFERENT FROM WHAT FILE','RC',FN,'')*fn='QFILE'OPEN '',FN TO COMPARE.FILE ELSE STOP 'CANNOT OPEN ':FN*ENDLAST.RECCOUNT = @RECCOUNTREAD REC FROM COMPARE.FILE, @ID THEN  IF @RECORD EQ REC THEN @ANS = '' ELSE   IF @ID[1,1]='$' THEN    REC.DATETIME=FIELD2(@RECORD,@FM,-1)    REC.DATETIME=TRIM(FIELD(REC.DATETIME,' ',2,9)):' ':FIELD(REC.DATETIME,' ',1)    REC.DATETIME=ICONV(REC.DATETIME,'DT')    CMP.DATETIME=FIELD2(REC,@FM,-1)    CMP.DATETIME=TRIM(FIELD(CMP.DATETIME,' ',2,9)):' ':FIELD(CMP.DATETIME,' ',1)    CMP.DATETIME=ICONV(CMP.DATETIME,'DT')    IF REC.DATETIME AND CMP.DATETIME ELSE GOTO CHANGED    IF REC.DATETIME EQ CMP.DATETIME THEN GOTO CHANGED    IF REC.DATETIME GT CMP.DATETIME THEN     @ANS='REPLACES'    END ELSE     @ANS='REPLACED'     END   END ELSECHANGED:    @ANS = 'CHANGED'    END   ENDEND ELSE  @ANS = 'NEW REC'ENDR10
DISPLAY	F3HEADINGS0L13
DISPLAY2	SHEADINGS@ans=@record<3>convert @vm to ' ' in @ansT130
DISPLAY_LENGTH	F10DISPLAY_LENGTHSL100N_(0,65537)INTEGER
DOES	RLIST
DOESNT	RLISTNOT
DT	SDTS0D2-@ANS=DATE()L10(D)0
EACH	RLISTEVERY
ENDING	RLIST[
EQ	RLISTEQ
EQUAL	RLISTEQ
EVERY	RLISTEVERY
EXCEPT	RLISTWITH NOT
EXCLUDE	RLISTWITH NOT
EXCLUDING	RLISTWITH NOT
EXECUTIVE_EMAIL	SExecutive EmailSexecutivecode={EXECUTIVE_CODE}convert @lower.case to @upper.case in executivecode*@ in executive name assume is an email emailif index(executivecode,'@',1) then @ans=executivecode convert ' ,' to ';;' in @ansend else *1) look for user code directly user=xlate('USERS',executivecode,'','X') *2) look for user name if user else user=xlate('USERS','%':executivecode:'%','','X') *3) try to use the first word of the executive code as the username *first name only if user else user=xlate('USERS',field(executivecode,' ',1),'','X') if user<35> and date() ge user<35> then  *expired  @ans='' end else  *not expired  @ans=user<7>  end *runtime users email if @ans else  @ans=xlate('USERS',@username,7,'X')  end endL100
LOG_SOURCE1	SSourceS@ans=field(@record<1>,' ',1)L200
WITHOUT	RLISTWITH NOT
EXECUTIVE_NAME	SExecutiveS*ans={EXECUTIVE_CODE}*ans2=ans*convert @lower.case to @upper.case in ans2*@ans=xlate('USERS',ans2,1,'X')*if @ans else @ans=ansdeclare function capitaliseans={EXECUTIVE_CODE}ans2=ansconvert @lower.case to @upper.case in ans2if ans2 ne ans then transfer ans to @ansend else *@ans=xlate('USERS','%':ans2:'%',1,'X') @ans=xlate('USERS',ans2,1,'X') if @ans then  ans2=@ans  convert @lower.case to @upper.case in ans2  if ans2=@ans then   @ans=capitalise(@ans)   end end else  transfer ans to @ans  @ans=capitalise(@ans)  end endT200
EXPORTABLE	G@IDTYPEFMCDISPLAYSNJUSTLEN0
F0_1	F0Key Part 1S1L10
F0_2	F0Key Part 2S2L10
F0_3	F0Key Part 3S3L10
F1	F1F1L10
F10	F10F10ML10
F10M	F10F10ML10
F11	F11F11ML10
F11M	F11F11ML10
F12	F12F12ML10
F12M	F12F12ML10
F14	F14FIELD 14ML10
F1M	F1F1ML10
F1_R	F1FIELD 1S0R10
F2	F2F2ML10
F2M	F2F2ML10
F3	F3F3ML10
F3M	F3F3ML10
F4	F4F4ML10
F4M	F4F4ML10
F5	F5F5ML10
F5M	F5F5ML10
F6	F6F6ML101
F6M	F6F6ML10
F7	F7F7ML10
F7M	F7F7ML10
F8	F8F8ML10
F8M	F8F8ML10
F9	F9F9ML10
F9M	F9F9ML10
FIELD	SFIELDMIF @RECUR0 ELSE CALL MSG('WHICH FIELD NUMBER ?','R',@RECUR0,'') END@ANS=@RECORD<@RECUR0>L100
FILE	RLIST
FINDALL	STestMif @record<1> then data=@record<8> convert @vm to @fm in dataend else data=@record endn=count(data,@fm)+1convert @lower.case to @upper.case in dataconvert '"' to "'" in data@ans=''for fn=1 to n tx=trim(data<fn>) if index(tx,'xlate(',1) and (index(tx,"','C')",1) or index(tx,"','X')",1)) then  *gosub change2  @ans<1,-1>=tx  end *if index(tx,'MSG(',1) and ( index(tx,",'U",1) or index(tx,",'D",1) or index(tx,",'R",1) ) then * gosub change * @ans<1,-1>=tx * end *if index(tx,'NOTE(',1) and ( index(tx,",'U",1) or index(tx,",'D",1) or index(tx,",'R",1) or index(tx,",'T",1)) then * gosub change * @ans<1,-1>=tx * end next fnreturn @ans/*change: tx=data<fn> swap 'MSG(' with 'msg2(' in tx declare function decide2 if decide2(@id:@fm:tx:'','',reply,2) else stop if reply=1 then  open 'TEMP' to file else call fsmsg();stop  *writev tx on file,@id,fn  if @record<1>='S' then   @record<8,fn>=tx  end else   @record<fn>=tx   end  write @record on file,@id  end return*/T600
FMC	F2FMCS0R3
FOOTING	RLISTFOOTING
FOR	RLIST
FOREIGN_TYPE	F15FOREIGN TYPESL101
FOREIGN_ATTRIBUTES	F18FOREIGN_ATTRIBUTESSL10VARCHAR1
FOREIGN_MAP	F17FOREIGN_MAPSL10VARCHAR1
FOREIGN_NAME	F16FOREIGN_NAMESL10VARCHAR1
FORMULA	F8FORMULAS0T65
FRIDAY	RLISTFRI
FROM	RLISTFROM
GE	RLISTGE
GENERIC_TYPE	F12GENERIC TYPESL15
GRAND-TOTAL	RLISTGRAND-TOTAL
GREATER	RLISTGT
GT	RLISTGT
GTOT-SUPP	RLISTGTS
HAS	RLIST
HDR-SUPP	RLISTHS
HEADING	RLISTHEADING
ID	F0Ref.S0T20
ID-SUPP	RLISTIS
ID_R	F0Ref.S0R20
IF	RLISTWITH
IN	RLIST
INCLUDE	RLISTWITH
INCLUDING	RLISTWITH
INDEXED	F6INDEXEDSL1"1""0"BOOLEAN
INDEX_DEPENDANCIES	F21INDEX_DEPENDANCIESSL10VARCHAR1
INPUT_TRUNCATION	F39INPUT_TRUNCATIONSL10VARCHAR1
IS	RLIST
ISCPP	SISCPPS@ans=@record<2>[1,3]='*c 'L100
ISNT	RLISTNOT
IT	RLIST
ITEMS	RLIST
ITS	RLIST
JUST	F9JSTS0L3
JUSTIFICATION	F9JUSTIFICATIONSL10"L""R""C""T"CHAR(10)
JUSTLEN	RLISTJL
KEY1	SKEY1S@ans=field(@id,'*',1)L100
KEY2	SKEY2S@ans=field(@id,'*',2)L100
KEY3	SKEY3S@ans=field(@id,'*',3)L100
KEY4	SKEY4S@ans=field(@id,'*',4)L100
KEYLEN	SKEYLENS@ans=len(@id)R100
KEYPART	F5KEYPARTSR50N_(0,32768)INTEGER
KEYSIZE	SKey SizeS@ANS=LEN(@ID)R80
LAST	RLISTLAST
LATENT	RLISTNR
LATER	RLISTGT
LE	RLISTLE
LEN	F10LENS0R3LENGTH
LESS	RLISTLT
LIKE	RLISTEQ
LIMIT	RLISTLIMIT
LINEMARKS	SLINEMARKSSif index('$*',@id[1,1],1) then return ''end else if index(@record,'linemark',1) then tt=@record convert ' ' to @fm in tt return index(tt,@fm:'linemark',1)end else @ans=0 endreturn @ansL100
LINES	SLINESM0@ANS=@RECORDCONVERT @FM TO @VM IN @ANSL600
LISTDICT.REP	GCOLHEAD "FIELD NAME" TYPE FMC PART MASTER_FLAG SM GENERIC_TYPE JUST LEN HEADING "'FS22DS22'PAGE'PP'"0
LOG@CRT	GLOG_DATE LOG_TIME LOG_USERNAME LOG_WORKSTATION LOG_SOURCE LOG_MESSAGE20
LOG_DATE	F0DateS3[DATE,4]R12[DATE]1
LOG_MESSAGE	F2MessageST30
LOG_MESSAGE2	SMessageSdeclare function trim2@ans=trim2(@record<2>,@vm)T300
LOG_SOURCE	F1SourceSL20
LOG_TIME	F0TimeS4MTSR5(MT)1
LOG_USERNAME	F0UsernameS2T201
LOG_WORKSTATION	F0WorkstationS1L201
LONG_FORMULA	F8FORMULAS0T60FORMULA
LOWERCASE_INDEX	F26LOWERCASE_INDEXSR1BOOLEAN1
LPTR	RLIST(P)
LT	RLISTLT
MASTER_FLAG	F28MASTER FLAGSBYes,L41
MASTER_DEFINITION	F28MASTER_DEFINITIONSR1BOOLEAN
MATCH	RLISTMATCH
MATCHES	RLISTMATCH
MATCHING	RLISTMATCH
MAX_LENGTH	F33MAX_LENGTHSR50N_(1,65537)INTEGER1
MONDAY	RLISTMON
MV	SMVMreturn @mvR30
NE	RLISTNOT
NEXT	RLISTNEXT
NFIELDS	SNFIELDSS@ans=count(@record,@fm)+1R100
NFIELDS0	SNFIELDSSconvert \\\\FE20\\\\ to \\\\20FE\\\\ in @record@ans=count(trim(@record),' ')+1L100
NFIELDS00	SNFIELDS00S*remove comment blocksloop pos1=index(@record,'/*',1)while pos1 pos2=index(@record,'*/',1) if pos2 else pos2=len(@record)+1 @record[pos1,pos2-pos1+2]='' repeat*call msg('x')*remove comment linesnfields=count(@record,@fm)+1*dim x(nfields)*matparse @record into xfor i=1 to nfields *if trim(x(i))[1,1]='*' then x(i)='' if trim(@record<i>)[1,1]='*' then @record<i>='' next*@record=matunparse(x)*             call msg('y')*remove blank linesconvert \\\\FE20\\\\ to \\\\20FE\\\\ in @recordnfields=count(trim(@record),' ')+1@ans=nfieldsR100
NLINES	SNLINESS0@ANS=COUNT(@RECORD,@FM)+1IF @RECORD EQ '' THEN @ANS=0R60
NO	RLISTNOT
NOPAGE	RLIST(N)
NORESOLVE	RLISTNR
NOT	RLISTNOT
NULL_FLAG	F31NULL_FLAGSR1BOOLEAN1
NULL_TRUNCATION	F20NULL_TRUNCATIONSL10VARCHAR1
NUM	SNUMS0@ANS=@IDHOLD=@IDCONVERT '0123456789' TO '' IN HOLDCONVERT HOLD TO '' IN @ANSR100
NUMID	F0NUMIDS0R10
OBJECT	GWITH  @ID "$]"0
OBJECTCODETYPE	SOBJECTCODETYPES@ans=seq(@record<1>)L100
OCONV	RLISTOC
OF	RLIST
ONLY	RLISTONLY
OR	RLIST
ORDINAL	F2ORDINALSR50N_(0,32768)INTEGER
OUTPUT_TRUNCATION	F40OUTPUT_TRUNCATIONSL10VARCHAR1
OVER	RLISTGT
PAGE	RLIST
PART	F5PARTS0R2
PATRN	F11PATRNS0T10
PG	RLIST
PHYSICAL_MAP_DOMAIN	F19PHYSICAL_MAP_DOMAINSL10VARCHAR1
PROGRAM_DATE	SPROGRAM DATES[DATE,4]ans={TIMEDATE}@ans=iconv(ans[11,99],'D')R120
PROMPT	F12PROMPTS0L12
PROTECTED	F25PROTECTEDSL10BOOLEAN1
RECNUM	SRECNUMS*@reccount is the same thing@ANS=@REC.COUNTL100
RECORD	SRECORDS@ANS=@RECORDT300
REDUCE	RLISTRP
REFERENCES	F30REFERENCESML10VARCHAR1
REFERENTIAL_INDEX	F23REFERENTIAL_INDEXSL10VARCHAR1
REFERRED	F24REFERREDSL10VARCHAR1
REPEAT	F4REPEATSL1"S""M"CHAR(1)
SATURDAY	RLISTSAT
SHOW	RLIST
SHOWING	RLIST
SIZE	SSIZES@ANS=LEN(@RECORD) + LEN(@ID) + 5R80
SM	F4SMS0L4
SOURCE	GWITH NOT @ID "$]" "*]"0
SPACER	S---S@ans=''L100
STARTING	RLIST]
STARTS	RLIST]
SUNDAY	RLISTSUN
SUPP	RLISTHS
TABLE_NAME	F0TABLE_NAMES1L20VARCHAR
TEMP	STEMPS@ans=count(@record,'xlate(')R100
THAN	RLIST
THAT	RLISTWITH
THE	RLIST
THURSDAY	RLISTTHU
TIME	S3TIMEMTHS@ANS=FIELD(DIR(@ID),@FM,3)L90
TIMEDATE	STIMEDATES$INSERT GBP,GENERAL.COMMONANS = FIELD2(@RECORD,@FM,-1)IF ANS[1,1] = 'V' THEN ANS = FIELD2(@RECORD,@FM,-2) ENDIF NOT(ANS[1,2] MATCHES '2N') THEN @ANS='' ELSE @ANS = ANSL210
TO	RLISTTO
TODAY	RLISTDAY
TOMMORROW	RLISTTOM
TOTAL	RLISTTOTAL
TUESDAY	RLISTTUE
TYPE	F1TYPES0L4
UNDER	RLISTLT
UNIQUE_FLAG	F32UNIQUE_FLAGSR1BOOLEAN1
UNLESS	RLISTWITH NOT
USER1	F41USER1SL101
USER2	F42USER2SL101
USER3	F43USER3SL101
USER4	F44USER4SL101
USER5	F45USER5SL101
USERNAME	SOperatorS@ANS=@USERNAMET200
USERNAME_CREATED	SUsernameCreatedS@ans={USERNAME_UPDATED}<1,1>L100
USING	RLISTUSING
VALIDATION	F11VALIDATIONML10VARCHAR
VERSION	SVERSIONSdeclare function field2if index(@dict,'VOC',1) then @ans=xlate(@record<3>,'$':@record<4>,'VERSION','X')end else @ANS = FIELD2(@RECORD,@FM,-1) *IF @ANS[1,1] = 'V' THEN *  @ANS=@ANS[2,99] *END ELSE *  @ANS = '' ENDL70
VERSION_DATE	SVERSION DATES[DATE,4]@ans=iconv(field(trim({VERSION}),' ',2,3),'D')R100
VOC_ID	F0VOC.IDL10
WEDNESDAY	RLISTWED
WHEN	RLISTWITH
WHENEVER	RLISTWITH
WHICH	RLISTWITH
WITH	RLISTWITH
YESTERDAY	RLISTYST
[	RLIST[
[]	RLIST[]
]	RLIST]
{	RLIST(
}	RLIST)
\.


--
-- Name: voc voc_pkey; Type: CONSTRAINT; Schema: dict; Owner: exodus
--

ALTER TABLE ONLY dict.voc
    ADD CONSTRAINT voc_pkey PRIMARY KEY (key);


--
-- PostgreSQL database dump complete
--

