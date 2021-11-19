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

ALTER TABLE ONLY dict.locks DROP CONSTRAINT locks_pkey;
DROP TABLE dict.locks;
SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: locks; Type: TABLE; Schema: dict; Owner: exodus
--

CREATE TABLE dict.locks (
    key text NOT NULL,
    data text
);


ALTER TABLE dict.locks OWNER TO exodus;

--
-- Data for Name: locks; Type: TABLE DATA; Schema: dict; Owner: exodus
--

COPY dict.locks (key, data) FROM stdin;
@CRT	GID-SUPP FILENAME KEY2 DATETIME_LOCKED DATETIME_EXPIRES LOCK_EXPIRED STATION USER SESSION0
DATETIME_EXPIRES	F1Date TimeExpiresS[DATETIME,4*DOS,MTSDOS]R101
DATETIME_LOCKED	F2Date TimeLockedS[DATETIME,4*DOS,MTSDOS]R101
FILENAME	F0FilenameS1L201
KEY	F0KeyS2L201
KEY2	SKeyS@ans=field(@id,'*',2,999)L200
LOCK_EXPIRED	SLockExpiredS call dostime(dostime);*seconds and hundreds since last midnight *convert to Windows based date/time (ndays since 1/1/1900) *31/12/67 in rev date() format equals 24837 in windows date format dostime=24837+date()+(dostime/24/3600) @ans=(@record<1> le dostime)L100
MASTER_LOCK	F6MASTER LOCKSL10FILE*KEY*SESSIONID1
SESSION	F5SessionSL101
STATION	F3StationSL101
USER	F4UsernameSL101
WAITING_USERS	F7Waiting UsersML101
\.


--
-- Name: locks locks_pkey; Type: CONSTRAINT; Schema: dict; Owner: exodus
--

ALTER TABLE ONLY dict.locks
    ADD CONSTRAINT locks_pkey PRIMARY KEY (key);


--
-- PostgreSQL database dump complete
--

