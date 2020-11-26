DROP TABLE IF EXISTS DICT_USERS CASCADE;
CREATE TABLE IF NOT EXISTS DICT_USERS
(
 key text primary key,
 data text
);
ALTER TABLE DICT_USERS OWNER to exodus;
INSERT INTO DICT_USERS (key,data) values (E'RANK',E'F11RankSR41');
INSERT INTO DICT_USERS (key,data) values (E'LAST_LOGIN_DATETIME',E'F13Last LoginDate/TimeS[DATETIME,4*,MTS]L151');
INSERT INTO DICT_USERS (key,data) values (E'USER_CODE',E'F0User CodeS0T301');
INSERT INTO DICT_USERS (key,data) values (E'AUTHORISED_JOURNAL_POST',E'SAuthorisedJournal PostSdeclare function securityif security(\'JOURNAL POST\',msg,\'\',@id) then @ans=1end else @ans=0 endL100');
INSERT INTO DICT_USERS (key,data) values (E'COMPANY_CODE',E'F33CompanyCodeML101');
INSERT INTO DICT_USERS (key,data) values (E'LAST_BROWSER',E'SLast BrowserSans=@record<39,6>call htmllib2(\'OCONV.AGENT.BROWSER\',ans)return ansT200');
INSERT INTO DICT_USERS (key,data) values (E'DEPT_AND_USER_NAME',E'SDepartment and User NameS@ans=@record<5>:\' - \':@record<1>T300');
INSERT INTO DICT_USERS (key,data) values (E'USER_AND_DEPT_NAME',E'SUser and Dept. NameS@ans=@record<1>:\' - \':@record<5>T300');
INSERT INTO DICT_USERS (key,data) values (E'PREVIOUS_LOGIN_LOCATION',E'F16Previous LoginLocationML101');
INSERT INTO DICT_USERS (key,data) values (E'HOLIDAY_FROM_DATE',E'F22HolidayFrom DateM[DATE,4*]R121');
INSERT INTO DICT_USERS (key,data) values (E'MARKET_CODE',E'F25MarketCodeSL10<MARKETS>1');
INSERT INTO DICT_USERS (key,data) values (E'LIVE_USER_WITH_EMAIL',E'SLIVE USER WITH EMAILSif @record<7> then expired=@record<35> if expired and expired<=date() then  @ans=0 end else  @ans=1  endend else @ans=\'\' endL100');
INSERT INTO DICT_USERS (key,data) values (E'IPNOS',E'F40IPNOSSL101');
INSERT INTO DICT_USERS (key,data) values (E'DEPARTMENT_CODE2',E'F21DepartmentCodeSL10with any digits1');
INSERT INTO DICT_USERS (key,data) values (E'DEPARTMENT_CODE',E'F5Department codeSL10without any digits1');
INSERT INTO DICT_USERS (key,data) values (E'WEEKDAYS_OFF',E'F24WeekdaysOffSL101');
INSERT INTO DICT_USERS (key,data) values (E'DATETIME_UPDATED',E'F31DateUpdatedM[DATETIME]L201');
INSERT INTO DICT_USERS (key,data) values (E'MENU',E'F34MenuSL10Legacy from Authorisation File userprivs<3>1');
INSERT INTO DICT_USERS (key,data) values (E'LAST_OS',E'SLast O/SSans=@record<39,6>call htmllib2(\'OCONV.AGENT.OS\',ans)return ansT200');
INSERT INTO DICT_USERS (key,data) values (E'USERNAME_UPDATED',E'F30UsernameML101');
INSERT INTO DICT_USERS (key,data) values (E'LAST_CONNECTION',E'F39LAST CONNECTIONML101');
INSERT INTO DICT_USERS (key,data) values (E'AUTHORISED_JOB_UPDATE',E'SAuthorisedJob UpdateSdeclare function securityif security(\'JOB UPDATE\',msg,xx,@id) then @ans=1end else @ans=0 endL100');
INSERT INTO DICT_USERS (key,data) values (E'EMAIL_ADDRESS',E'F7EmailAddressSL301');
INSERT INTO DICT_USERS (key,data) values (E'PASSWORD_DATE',E'F36PasswordDateSL101');
INSERT INTO DICT_USERS (key,data) values (E'HOLIDAY_REASON',E'F38Holiday ReasonMT201');
INSERT INTO DICT_USERS (key,data) values (E'AUTHORISED_TASK_CREATE',E'SAuthorisedTask CreateSdeclare function securityif security(\'TASK CREATE\',msg,xx,@id) then @ans=1end else @ans=0 endL100');
INSERT INTO DICT_USERS (key,data) values (E'IS_DEPARTMENT',E'SIS DEPARTMENTSif @record<5> eq @id then return 1 endreturn \'\'/*pgsqlif split_part(data,FM,5) = key then ANS=\'1\';else ANS=\'\';end if;*/L100');
INSERT INTO DICT_USERS (key,data) values (E'DATE_TIME',E'F12DATETIMES[DATETIME,4*,MTS]R101');
INSERT INTO DICT_USERS (key,data) values (E'AUTHORISED_TIMESHEET_ADMINISTRATION',E'SAuthorisedTimesheet AdministrationSdeclare function securityif security(\'TIMESHEET ADMINISTRATION\',msg,\'\',@id) then @ans=1end else @ans=0 endL100');
INSERT INTO DICT_USERS (key,data) values (E'HOLIDAY_UPTO_DATE',E'F23HolidayUpto DateM[DATE,4*]R121');
INSERT INTO DICT_USERS (key,data) values (E'USER_NAME',E'F1User NameST301');
INSERT INTO DICT_USERS (key,data) values (E'AUTHORISED',E'SAuthorisedS$insert gbp,general.commonif @id=\'EXODUS\' then goto unknownlocate @username in userprivs<1> setting usern then if security(\'TIMESHEET ADMINISTRATOR\') then  ans=1  return ans  end *may not be allowed to access higher users/groups if usern>@record<11> then  ans=security(\'AUTHORISATION UPDATE HIGHER GROUPS\') end else  *look for the user in the same group as the active user  for usern2=usern to 9999   user=userprivs<1,usern2>  while user and user<>\'---\' and user ne @id   next usern2  *if found then same group therefore ok  if user=@id then   ans=1  end else   *not found therefore possibly not allowed access   ans=security(\'AUTHORISATION UPDATE LOWER GROUPS\')   end  endend elseunknown: ans=(@username=\'EXODUS\') endreturn ansL100');
INSERT INTO DICT_USERS (key,data) values (E'PREVIOUS_LOGIN_DATETIME',E'F15Previous LoginDate/TimeM[DATETIME,4*,MTS]L151');
INSERT INTO DICT_USERS (key,data) values (E'HTMLCODES',E'F19HTMLCODESML101');
INSERT INTO DICT_USERS (key,data) values (E'KEYS',E'SKeysS$insert gbp,general.commonlocate @id in userprivs<1> setting usern then @ans=userprivs<2,usern>end else @ans=\'\' endL200');
INSERT INTO DICT_USERS (key,data) values (E'AUTHORISED_TASK_ACCESS',E'SAuthorisedTask AccessSdeclare function securityif security(\'TASK ACCESS\',msg,\'\',@id) then @ans=1end else @ans=0 endL100');
INSERT INTO DICT_USERS (key,data) values (E'LAST_UPGRADE_DATE_TIME',E'F17Last UpgradeDate/TimeS[DATETIME,4*,MTS]L101');
INSERT INTO DICT_USERS (key,data) values (E'PASSWORD_EXPIRY_DATE',E'F37PasswordExpiryDateS[DATE,4*]R10Just a way to warn user on login without a server requesti.e. calculate as ={PASSWORD_DATE}+userprivs<25> every web READ1');
INSERT INTO DICT_USERS (key,data) values (E'LAST_LOGIN_LOCATION',E'F14Last LoginLocationSL101');
INSERT INTO DICT_USERS (key,data) values (E'PREVIOUS_LOGIN_STATUS',E'F18Previous Login StatusMT20OK if successful or reason for failure1');
INSERT INTO DICT_USERS (key,data) values (E'PREFERENCES_FOR',E'F20PreferencesForSL101');
INSERT INTO DICT_USERS (key,data) values (E'STATION_UPDATED',E'F32StationUpdatedML101');
INSERT INTO DICT_USERS (key,data) values (E'EXPIRY_DATE',E'F35Expiry DateSR101');
INSERT INTO DICT_USERS (key,data) values (E'KEYS2',E'F41KEYS2SL101');
