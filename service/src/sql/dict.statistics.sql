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

ALTER TABLE ONLY dict.statistics DROP CONSTRAINT statistics_pkey;
DROP TABLE dict.statistics;
SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: statistics; Type: TABLE; Schema: dict; Owner: exodus
--

CREATE TABLE dict.statistics (
    key text NOT NULL,
    data text
);


ALTER TABLE dict.statistics OWNER TO exodus;

--
-- Data for Name: statistics; Type: TABLE DATA; Schema: dict; Owner: exodus
--

COPY dict.statistics (key, data) FROM stdin;
@CRT	GID-SUPP BY DATE BY HOUR BY LAST_ACCESS_TIME DATE HOUR SESSION DEPARTMENT USER_CODE IP_NO DATABASE REQUESTS LAST_ACCESS_TIME0
DATABASE	F0DatabaseS1L101
DATE	F0DateS5[DATE,4*]R101
DEPARTMENT	SDepartmentS@ans=@record<3>*may be missing try to get from user fileif @ans else @ans=xlate('USERS',field(@id,'*',3),5,'X')*use database as better than blank/unknownif @ans else if field(@id,'*',3)='EXODUS' then  @ans='EXODUS' end else  @ans=field(@id,'*',1)  end endT200
HOUR	F0HourS6R21
IP_NO	F0IP-No.S4L101
LAST_ACCESS	F1Last AccessS[DATETIME,4*,DOS]R101
LAST_ACCESS_TIME	SLast-Access-TimeS[TIME2]@ans=mod(@record<1>*86400+@sw<1>,86400)R100
REQUESTS	F2RequestsSR101
SESSION	F0SessionS2R101
USER_BROWSER	SUser-BrowserSusercode=field(@id,'*',3)@ans=xlate('USERS',usercode,39,'X')<1,6>T200
USER_CODE	SUserS@ans=field(@id,'*',3)swap ' ' with '_' in @ansL100
USER_DEPT	F3USER DEPTSL101
USER_NAME	SUser-NameSusercode=field(@id,'*',3)@ans=xlate('USERS',usercode,1,'X')if @ans='' then @ans=usercode swap ' ' with '_' in @ansend else if @ans<>usercode then @ans:=' (':usercode:')' endT200
\.


--
-- Name: statistics statistics_pkey; Type: CONSTRAINT; Schema: dict; Owner: exodus
--

ALTER TABLE ONLY dict.statistics
    ADD CONSTRAINT statistics_pkey PRIMARY KEY (key);


--
-- PostgreSQL database dump complete
--

