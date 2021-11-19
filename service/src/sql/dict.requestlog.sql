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

ALTER TABLE ONLY dict.requestlog DROP CONSTRAINT requestlog_pkey;
DROP TABLE dict.requestlog;
SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: requestlog; Type: TABLE; Schema: dict; Owner: exodus
--

CREATE TABLE dict.requestlog (
    key text NOT NULL,
    data text
);


ALTER TABLE dict.requestlog OWNER TO exodus;

--
-- Data for Name: requestlog; Type: TABLE DATA; Schema: dict; Owner: exodus
--

COPY dict.requestlog (key, data) FROM stdin;
@CRT	GID-SUPP DATABASE DATE TIME PROCESS_NO IPNO USERCODE REQUEST1 REQUEST2 REQUEST3 REQUEST4 REQUEST5 REQUEST6 REQUEST7 RESPONSE_TIME SECS RESPONSE0
DATABASE	F0DatabaseS1L81
DATE	F0DateS2[DATE,4*]R121
IPNO	F2IPNoS0L151
PROCESS_NO	F0PS4R3
REQUEST1	F11Request1ST101
REQUEST2	F12Request2SL101
REQUEST3	F13Request3SL101
REQUEST4	F14Request4SL101
REQUEST5	F15Request5SL101
REQUEST6	F16Request6SL101
REQUEST7	F17Request7SL101
RESPONSE	F6ResponseST201
RESPONSE_TIME	F5Response TimeSMTSR81
SECS	SSecsSresponsetime=@record<5>if len(responsetime) then @ans=@record<5>-field(@id,'*',3) if @ans<0 then @ans+=86400 *garbagecollect @ans=oconv(@ans,'MD20P')+0end else *blank if no response time (crashed) @ans='' endR100
SEQ	SSeqSreturn @id/*pgsqlreturn key;*/R100
TIME	F0TimeS3MTSR81
USERCODE	F1UserS0L10
\.


--
-- Name: requestlog requestlog_pkey; Type: CONSTRAINT; Schema: dict; Owner: exodus
--

ALTER TABLE ONLY dict.requestlog
    ADD CONSTRAINT requestlog_pkey PRIMARY KEY (key);


--
-- PostgreSQL database dump complete
--

