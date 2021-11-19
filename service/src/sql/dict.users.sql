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

ALTER TABLE ONLY dict.users DROP CONSTRAINT users_pkey;
DROP TABLE dict.users;
SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: users; Type: TABLE; Schema: dict; Owner: exodus
--

CREATE TABLE dict.users (
    key text NOT NULL,
    data text
);


ALTER TABLE dict.users OWNER TO exodus;

--
-- Data for Name: users; Type: TABLE DATA; Schema: dict; Owner: exodus
--

COPY dict.users (key, data) FROM stdin;
AUTHORISED	SAuthorisedS$insert gbp,general.commonif @id='EXODUS' then goto unknownlocate @username in userprivs<1> setting usern then if security('TIMESHEET ADMINISTRATOR') then  ans=1  return ans  end *may not be allowed to access higher users/groups if usern>@record<11> then  ans=security('AUTHORISATION UPDATE HIGHER GROUPS') end else  *look for the user in the same group as the active user  for usern2=usern to 9999   user=userprivs<1,usern2>  while user and user<>'---' and user ne @id   next usern2  *if found then same group therefore ok  if user=@id then   ans=1  end else   *not found therefore possibly not allowed access   ans=security('AUTHORISATION UPDATE LOWER GROUPS')   end  endend elseunknown: ans=(@username='EXODUS') endreturn ansL100
AUTHORISED_JOB_UPDATE	SAuthorisedJob UpdateSdeclare function securityif security('JOB UPDATE',msg,xx,@id) then @ans=1end else @ans=0 endL100
AUTHORISED_JOURNAL_POST	SAuthorisedJournal PostSdeclare function securityif security('JOURNAL POST',msg,'',@id) then @ans=1end else @ans=0 endL100
AUTHORISED_TASK_ACCESS	SAuthorisedTask AccessSdeclare function securityif security('TASK ACCESS',msg,'',@id) then @ans=1end else @ans=0 endL100
AUTHORISED_TASK_CREATE	SAuthorisedTask CreateSdeclare function securityif security('TASK CREATE',msg,xx,@id) then @ans=1end else @ans=0 endL100
AUTHORISED_TIMESHEET_ADMINISTRATION	SAuthorisedTimesheet AdministrationSdeclare function securityif security('TIMESHEET ADMINISTRATION',msg,'',@id) then @ans=1end else @ans=0 endL100
COMPANY_CODE	F33Co.ML101
DATETIME_UPDATED	F31DateUpdatedM[DATETIME]L201
DATE_TIME	F12DATETIMES[DATETIME,4*,MTS]R101
DEPARTMENT_CODE	F5Department codeSL10without any digits1
DEPARTMENT_CODE2	F21DepartmentCodeSL10with any digits1
DEPT_AND_USER_NAME	SDepartment and User NameS@ans=@record<5>:' - ':@record<1>T300
EMAIL_ADDRESS	F7EmailAddressSL301
EXPIRY_DATE	F35Expiry DateSR101
HOLIDAY_FROM_DATE	F22HolidayFrom DateM[DATE,4*]R121
HOLIDAY_REASON	F38Holiday ReasonMT201
HOLIDAY_UPTO_DATE	F23HolidayUpto DateM[DATE,4*]R121
HTMLCODES	F19HTMLCODESML101
IPNOS	F40IPNOSSL101
IS_DEPARTMENT	SIS DEPARTMENTSif @record<5> eq @id then return 1 endreturn ''/*pgsqlif split_part(data,FM,5) = key then ANS='1';else ANS='';end if;*/L100
KEYS	SKeysS$insert gbp,general.commonlocate @id in userprivs<1> setting usern then @ans=userprivs<2,usern>end else @ans='' endL200
KEYS2	F41KEYS2SL101
LAST_BROWSER	SLast BrowserSans=@record<39,6>call htmllib2('OCONV.AGENT.BROWSER',ans)return ansT200
LAST_CONNECTION	F39LAST CONNECTIONML101
LAST_LOGIN_DATETIME	F13Last LoginDate/TimeS[DATETIME,4*,MTS]L151
LAST_LOGIN_LOCATION	F14Last LoginLocationSL101
LAST_OS	SLast O/SSans=@record<39,6>call htmllib2('OCONV.AGENT.OS',ans)return ansT200
LAST_UPGRADE_DATE_TIME	F17Last UpgradeDate/TimeS[DATETIME,4*,MTS]L101
LIVE_USER_WITH_EMAIL	SLIVE USER WITH EMAILSif @record<7> then expired=@record<35> if expired and expired<=date() then  @ans=0 end else  @ans=1  endend else @ans='' endL100
MARKET_CODE	F25MarketCodeSL10<MARKETS>1
MENU	F34MenuSL10Legacy from Authorisation File userprivs<3>1
PASSWORD_DATE	F36PasswordDateSL101
PASSWORD_EXPIRY_DATE	F37PasswordExpiryDateS[DATE,4*]R10Just a way to warn user on login without a server requesti.e. calculate as ={PASSWORD_DATE}+userprivs<25> every web READ1
PREFERENCES_FOR	F20PreferencesForSL101
PREVIOUS_LOGIN_DATETIME	F15Previous LoginDate/TimeM[DATETIME,4*,MTS]L151
PREVIOUS_LOGIN_LOCATION	F16Previous LoginLocationML101
PREVIOUS_LOGIN_STATUS	F18Previous Login StatusMT20OK if successful or reason for failure1
RANK	F11RankSR41
STATION_UPDATED	F32StationUpdatedML101
USERNAME_UPDATED	F30UsernameML101
USER_AND_DEPT_NAME	SUser and Dept. NameS@ans=@record<1>:' - ':@record<5>T300
USER_CODE	F0User CodeS0T301
USER_NAME	F1User NameST301
WEEKDAYS_OFF	F24WeekdaysOffSL101
\.


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: dict; Owner: exodus
--

ALTER TABLE ONLY dict.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (key);


--
-- PostgreSQL database dump complete
--

