DROP TABLE IF EXISTS DICT.DEFINITIONS CASCADE;
CREATE TABLE IF NOT EXISTS DICT.DEFINITIONS
(
 key text primary key,
 data text
);
ALTER TABLE DICT.DEFINITIONS OWNER to exodus;
INSERT INTO DICT.DEFINITIONS (key,data) values (E'MEDIA_DOWNPAYMENT_ACCNO',E'F21MEDIA DOWNPAYMENT ACCNOSL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_PARAMETERS',E'F7ParametersSL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LOCKS',E'F11TaskLocksM2T20');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'USERS',E'F1UsersM1T201');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'ACCRUALS_ACCNO',E'F14ACCRUALS ACCNOSL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'COSTS_ACCNO',E'F12COSTS ACNOSL151');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'CODE',E'F0CODESL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'REVERSE_THE_USERS',E'SREVERSE THE USERSSdeclare function invertarray,reverse@record=invertarray(reverse(invertarray(@record)))L00');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'MEDIA_INCOME_ACCNO',E'F20MEDIA INCOME ACCNOSL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'CHEQUEDESIGN_IS_DEFAULT',E'SChequeIs DefaultS@ans=@record<14>if @ans then @ans=\'Default\'L10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'F9',E'F9FIELD 9ML101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_STATION',E'F3WorkstationSL201');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_COMMENT2',E'SCommentsScommon /system/ system@ans=\'\'if field(@id,\'*\',2)=system<32> then @ans<1,-1>=\'Current user session\'if trim(@record<3>)=trim(@station) then @ans<1,-1>=\'Current workstation\'if @record<5>=@username then @ans<1,-1>=\'Current user\'T300');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'INCOME_ACCNO',E'F10INCOME ACNOSL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LIC_UPTODATE',E'F1Upto DateM[DATE,4*]R10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'USER_CODE_HTML',E'SUser CodeM1usercodes={USERS}emails=xlate(\'USERS\',usercodes,7,\'X\')nusers=count(usercodes,@vm)+1for usern=1 to nusers usercode=usercodes<1,usern> if usercode then  if emails<1,usern>=\'\' then   usercodes<1,usern>=\'<B>\':usercode:\'</B>\'   end  end next usernreturn usercodesT200');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESSES',E'GWITH @ID STARTING \'PROCESS*\' PROCESS_CATEGORY PROCESS_DATE PROCESS_TIME PROCESS_STATION PROCESS_DATASET PROCESS_USER PROCESS_PARAMETERS PROCESS_DATE2 PROCESS_TIME2 PROCESS_RESULT PROCESS_COMMENT20');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_DATASET',E'F4DatasetSL8');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_DATE',E'F1StartDateS[DATE,4]R10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'MEDIA_COSTS_ACCNO',E'F22MEDIA COSTS ACCNOSL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'KEY1',E'SKEY1S@ans=field(@id,\'*\',1)L100');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LIC_GRACEDAYS',E'F5Grace DaysMR10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'USER_EXPIRY_DATE',E'F3User ExpiryDateM1[DATE,*4]R12');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LAST_LOGIN_AGE',E'SLast LoginAge (days)M1@ans=xlate(\'USERS\',{USER_CODE_WITH_EMAIL},13,\'X\')nn=count(@ans,@vm)+1for ii=1 to nn logindate=int(@ans<1,ii>) if logindate then  @ans<1,ii>=(date()-logindate) \'MD00P\'  end nextR50');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_COMMENT',E'F14CommentsST30');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_COUNT',E'STotalS@ans=1R100');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_TIME',E'F2StartTimeSMTR5(MT)');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'WIP_ACCNO',E'F13WIP ACCNOSL151');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_RESULT',E'F13ResultSL10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LIC_FROMDATE',E'F2From DateM[DATE,4*]R10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'EMAIL_ADDRESS',E'SEmailM1@ans=xlate(\'USERS\',@record<1>,7,\'X\')T200');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'MEDIA_WIP_ACCNO',E'F23MEDIA WIP ACCNOSL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_DATE2',E'F11StopDateS[DATE,4]L10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'DATE_TIME',E'F999999DATETIMES[DATETIME,4*,MTS]R101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LIC_DATABASE',E'F4Database IdML10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'SHORT_DATES',E'F4SHORT DATESSBL10(B)1');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'SPLIT_EXTRAS',E'F5SPLIT EXTRASSBL10(B)1');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LIC_FIELDS',E'GLIC_MODULE LIC_FROMDATE LIC_UPTODATE LIC_DATABASE LIC_GRACEDAYS ID-SUPP0');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LAST_LOGIN_LOCATION',E'SLast LoginLocationM1return xlate(\'USERS\',{USER_CODE_WITH_EMAIL},14,\'X\')L200');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LAST_LOGIN_DATE_TIME',E'SLast LoginM1[DATETIME]return xlate(\'USERS\',{USER_CODE_WITH_EMAIL},13,\'X\')R200');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'F8',E'F8FIELD 8ML101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_USER',E'F5UserSL10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LIC_MODULE',E'F3ModuleML10');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'FULL_NAME',E'SFull NameM1@ans=xlate(\'USERS\',@record<1>,1,\'X\')nn=count(@ans,@vm)+1for ii=1 to nn if @ans<1,ii>=@record<1,ii> then  @ans<1,ii>=\'\'  end nextT200');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LAST_BROWSER',E'SLast BrowserM1return xlate(\'USERS\',{USER_CODE_WITH_EMAIL},\'LAST_BROWSER\',\'X\')L200');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'USER_CODE_WITH_EMAIL',E'SUSER CODE WITH EMAILMusercodes=@record<1,@mv>emails=@record<7,@mv>nusers=count(usercodes,@vm)+1for usern=1 to nusers if emails<1,usern>=\'\' then  usercodes<1,usern>=\'\'  end next usernreturn usercodesL100');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_CATEGORY',E'F6CategorySL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'TASKS',E'F10TasksM2T40');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_DURATION',E'SDurationHrs:Min:SecSsecsperday=24*60*60uptodate=@record<11>uptotime=@record<12>if uptodate=\'\' then uptodate=date()if uptotime=\'\' then uptotime=time()daysecs=(uptodate-@record<1>)*secsperdaytimesecs=uptotime-@record<2>totsecs=daysecs+timesecshours=int(totsecs/60/60)mins=mod(int(totsecs/60),60)secs=int(mod(totsecs,60))@ans=hours:\':\':mins \'R(0)#2\':\':\':secs \'R(0)#2\':\'.\':field(totsecs,\'.\',2) \'R#2\'R120');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_WEEKCOMMENCING',E'SWeekStartingS[DATE,4]@ANS=@record<1>-mod(@RECORD<1>-1,7)R100');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'CHEQUEDESIGN_ACCOUNT_NAME',E'SChequeA/c NameS@ans=xlate(\'ACCOUNTS\',field(@id,\'*\',2),1,\'X\')T200');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'DOWNPAYMENT_ACCNO',E'F11DOWNPAYMENT ACCNOSL151');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'MEDIA_ACCRUALS',E'F24MEDIA ACCRUALSSL101');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'KEY2',E'SKEY2S@ans=field(@id,\'*\',2)L100');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PROCESS_TIME2',E'F12StopTimeSMTR8(MT)');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'KEYS',E'F2UserKeysM1T201');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'PASSWORD_AGE',E'SPasswordAge (days)M1@ans=xlate(\'USERS\',{USER_CODE_WITH_EMAIL},36,\'X\')nn=count(@ans,@vm)+1for ii=1 to nn passworddate=@ans<1,ii> if passworddate then  @ans<1,ii>=(date()-passworddate) \'MD00P\'  end nextR50');
INSERT INTO DICT.DEFINITIONS (key,data) values (E'LAST_OS',E'SLast O/SM1return xlate(\'USERS\',{USER_CODE_WITH_EMAIL},\'LAST_OS\',\'X\')L200');
