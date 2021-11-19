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

ALTER TABLE ONLY dict.definitions DROP CONSTRAINT definitions_pkey;
DROP TABLE dict.definitions;
SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: definitions; Type: TABLE; Schema: dict; Owner: exodus
--

CREATE TABLE dict.definitions (
    key text NOT NULL,
    data text
);


ALTER TABLE dict.definitions OWNER TO exodus;

--
-- Data for Name: definitions; Type: TABLE DATA; Schema: dict; Owner: exodus
--

COPY dict.definitions (key, data) FROM stdin;
ACCRUALS_ACCNO	F14ACCRUALS ACCNOSL101
CHEQUEDESIGN_ACCOUNT_NAME	SChequeA/c NameS@ans=xlate('ACCOUNTS',field(@id,'*',2),1,'X')T200
CHEQUEDESIGN_IS_DEFAULT	SChequeIs DefaultS@ans=@record<14>if @ans then @ans='Default'L10
CODE	F0CODESL101
COSTS_ACCNO	F12COSTS ACNOSL151
DATE_TIME	F999999DATETIMES[DATETIME,4*,MTS]R101
DOWNPAYMENT_ACCNO	F11DOWNPAYMENT ACCNOSL151
EMAIL_ADDRESS	SEmailM1@ans=xlate('USERS',@record<1>,7,'X')T200
F8	F8FIELD 8ML101
F9	F9FIELD 9ML101
FULL_NAME	SFull NameM1@ans=xlate('USERS',@record<1>,1,'X')nn=count(@ans,@vm)+1for ii=1 to nn if @ans<1,ii>=@record<1,ii> then  @ans<1,ii>=''  end nextT200
INCOME_ACCNO	F10INCOME ACNOSL101
KEY1	SKEY1S@ans=field(@id,'*',1)L100
KEY2	SKEY2S@ans=field(@id,'*',2)L100
KEYS	F2UserKeysM1T201
LAST_BROWSER	SLast BrowserM1return xlate('USERS',{USER_CODE_WITH_EMAIL},'LAST_BROWSER','X')L200
LAST_LOGIN_AGE	SLast LoginAge (days)M1@ans=xlate('USERS',{USER_CODE_WITH_EMAIL},13,'X')nn=count(@ans,@vm)+1for ii=1 to nn logindate=int(@ans<1,ii>) if logindate then  @ans<1,ii>=(date()-logindate) 'MD00P'  end nextR50
LAST_LOGIN_DATE_TIME	SLast LoginM1[DATETIME]return xlate('USERS',{USER_CODE_WITH_EMAIL},13,'X')R200
LAST_LOGIN_LOCATION	SLast LoginLocationM1return xlate('USERS',{USER_CODE_WITH_EMAIL},14,'X')L200
LAST_OS	SLast O/SM1return xlate('USERS',{USER_CODE_WITH_EMAIL},'LAST_OS','X')L200
LIC_DATABASE	F4Database IdML10
LIC_FIELDS	GLIC_MODULE LIC_FROMDATE LIC_UPTODATE LIC_DATABASE LIC_GRACEDAYS ID-SUPP0
LIC_FROMDATE	F2From DateM[DATE,4*]R10
LIC_GRACEDAYS	F5Grace DaysMR10
LIC_MODULE	F3ModuleML10
LIC_UPTODATE	F1Upto DateM[DATE,4*]R10
LOCKS	F11TaskLocksM2T20
MEDIA_ACCRUALS	F24MEDIA ACCRUALSSL101
MEDIA_COSTS_ACCNO	F22MEDIA COSTS ACCNOSL101
MEDIA_DOWNPAYMENT_ACCNO	F21MEDIA DOWNPAYMENT ACCNOSL101
MEDIA_INCOME_ACCNO	F20MEDIA INCOME ACCNOSL101
MEDIA_WIP_ACCNO	F23MEDIA WIP ACCNOSL101
PASSWORD_AGE	SPasswordAge (days)M1@ans=xlate('USERS',{USER_CODE_WITH_EMAIL},36,'X')nn=count(@ans,@vm)+1for ii=1 to nn passworddate=@ans<1,ii> if passworddate then  @ans<1,ii>=(date()-passworddate) 'MD00P'  end nextR50
PROCESSES	GWITH @ID STARTING 'PROCESS*' PROCESS_CATEGORY PROCESS_DATE PROCESS_TIME PROCESS_STATION PROCESS_DATASET PROCESS_USER PROCESS_PARAMETERS PROCESS_DATE2 PROCESS_TIME2 PROCESS_RESULT PROCESS_COMMENT20
PROCESS_CATEGORY	F6CategorySL101
PROCESS_COMMENT	F14CommentsST30
PROCESS_COMMENT2	SCommentsScommon /system/ system@ans=''if field(@id,'*',2)=system<32> then @ans<1,-1>='Current user session'if trim(@record<3>)=trim(@station) then @ans<1,-1>='Current workstation'if @record<5>=@username then @ans<1,-1>='Current user'T300
PROCESS_COUNT	STotalS@ans=1R100
PROCESS_DATASET	F4DatasetSL8
PROCESS_DATE	F1StartDateS[DATE,4]R10
PROCESS_DATE2	F11StopDateS[DATE,4]L10
PROCESS_DURATION	SDurationHrs:Min:SecSsecsperday=24*60*60uptodate=@record<11>uptotime=@record<12>if uptodate='' then uptodate=date()if uptotime='' then uptotime=time()daysecs=(uptodate-@record<1>)*secsperdaytimesecs=uptotime-@record<2>totsecs=daysecs+timesecshours=int(totsecs/60/60)mins=mod(int(totsecs/60),60)secs=int(mod(totsecs,60))@ans=hours:':':mins 'R(0)#2':':':secs 'R(0)#2':'.':field(totsecs,'.',2) 'R#2'R120
PROCESS_PARAMETERS	F7ParametersSL101
PROCESS_RESULT	F13ResultSL10
PROCESS_STATION	F3WorkstationSL201
PROCESS_TIME	F2StartTimeSMTR5(MT)
PROCESS_TIME2	F12StopTimeSMTR8(MT)
PROCESS_USER	F5UserSL10
PROCESS_WEEKCOMMENCING	SWeekStartingS[DATE,4]@ANS=@record<1>-mod(@RECORD<1>-1,7)R100
REVERSE_THE_USERS	SREVERSE THE USERSSdeclare function invertarray,reverse@record=invertarray(reverse(invertarray(@record)))L00
SHORT_DATES	F4SHORT DATESSBL10(B)1
SPLIT_EXTRAS	F5SPLIT EXTRASSBL10(B)1
TASKS	F10TasksM2T40
USERS	F1UsersM1T201
USER_CODE_HTML	SUser CodeM1usercodes={USERS}emails=xlate('USERS',usercodes,7,'X')nusers=count(usercodes,@vm)+1for usern=1 to nusers usercode=usercodes<1,usern> if usercode then  if emails<1,usern>='' then   usercodes<1,usern>='<B>':usercode:'</B>'   end  end next usernreturn usercodesT200
USER_CODE_WITH_EMAIL	SUSER CODE WITH EMAILMusercodes=@record<1,@mv>emails=@record<7,@mv>nusers=count(usercodes,@vm)+1for usern=1 to nusers if emails<1,usern>='' then  usercodes<1,usern>=''  end next usernreturn usercodesL100
USER_EXPIRY_DATE	F3User ExpiryDateM1[DATE,*4]R12
WIP_ACCNO	F13WIP ACCNOSL151
\.


--
-- Name: definitions definitions_pkey; Type: CONSTRAINT; Schema: dict; Owner: exodus
--

ALTER TABLE ONLY dict.definitions
    ADD CONSTRAINT definitions_pkey PRIMARY KEY (key);


--
-- PostgreSQL database dump complete
--

