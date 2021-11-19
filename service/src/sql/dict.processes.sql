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

ALTER TABLE ONLY dict.processes DROP CONSTRAINT processes_pkey;
DROP TABLE dict.processes;
SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: processes; Type: TABLE; Schema: dict; Owner: exodus
--

CREATE TABLE dict.processes (
    key text NOT NULL,
    data text
);


ALTER TABLE dict.processes OWNER TO exodus;

--
-- Data for Name: processes; Type: TABLE DATA; Schema: dict; Owner: exodus
--

COPY dict.processes (key, data) FROM stdin;
@CRT	GID-SUPP BY PROCESS_NO PROCESS_NO SYSTEM_CODE DATABASE_CODE PID LAST_UPDATED2 STATUS0
CONNECTION	F54ConnectionML101
DATABASE_CODE	F17DatabaseCodeSL101
LAST_UPDATED	F27LastUpdatedS[DATETIME,DOS*,DOSMTS]R201
LAST_UPDATED2	SLast UpdatedS/*dostimenow=date()+24873+time()/86400elapsedsecs=int((dostimenow-@record<27>)*86400)ans=''interval=604800intervalname='weeks'gosub addintervaldescinterval=interval/7intervalname='days'gosub addintervaldescinterval=interval/24intervalname='hours'gosub addintervaldescinterval=interval/60intervalname='mins'gosub addintervaldescinterval=1intervalname='secs'gosub addintervaldescreturn trim(ans)****************addintervaldesc:****************if interval=1 or elapsedsecs>interval then nintervals=int(elapsedsecs/interval) elapsedsecs-=nintervals*interval if nintervals=1 then intervalname[-1,1]='' ans:=' ':nintervals:' ':intervalname endreturn*/declare function elapsedtimetextfromdate=int(@record<27>)-24873fromtime=('.':field(@record<27>,'.',2))*86400return elapsedtimetext(fromdate,fromtime,uptodate,uptotime)T100
MAINTENANCE	SMaintenanceSif @record<33> then @ans='' else @ans='Yes'L100
NOT_INTERACTIVE	F31NotInteractiveSL101
PID	SPidS@ans=@record<54,5>R50
PROCESS_NO	F0ProcessNoS0R31
STATUS	SStatusS*NB add any new statuses to MONITORING programif @record<52> then @ans='Closed' return @ans endcall processlocking('ISLOCKED',@id,islocked)if not(islocked) and @volumes then @ans='Crashed' return @ans enddostimenow=date()+24873+time()/86400*10 minshungtime=10.0*60/86400r33=@record<33>if r33 and (dostimenow-@record<27>) gt hungtime then if @VOLUMES then  @ans='Hung' end else  @ans='Closed'  end return @ans endif r33 then @ans='OK'end else @ans='Maintenance' endif @record<53> then @ans:=' ':@record<53>return @ansL100
SYSTEM_CODE	F51SystemCodeSL101
\.


--
-- Name: processes processes_pkey; Type: CONSTRAINT; Schema: dict; Owner: exodus
--

ALTER TABLE ONLY dict.processes
    ADD CONSTRAINT processes_pkey PRIMARY KEY (key);


--
-- PostgreSQL database dump complete
--

