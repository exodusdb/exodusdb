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

ALTER TABLE ONLY dict.changelog DROP CONSTRAINT changelog_pkey;
DROP TABLE dict.changelog;
SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: changelog; Type: TABLE; Schema: dict; Owner: exodus
--

CREATE TABLE dict.changelog (
    key text NOT NULL,
    data text
);


ALTER TABLE dict.changelog OWNER TO exodus;

--
-- Data for Name: changelog; Type: TABLE DATA; Schema: dict; Owner: exodus
--

COPY dict.changelog (key, data) FROM stdin;
@CRT	GDATE KEYWORDS TEXT0
DATE	F1Date[DATE,4*]R101
DATE_TIME	F4Date_time[DATETIME,4*,MTS]R101
DISTRIBUTION	F5DistributionM0L1011
KEYWORD	F2KeywordM1L10N1
KEYWORDS	SKeywordsS@ans=@record<2>swap @vm with ", " in @ansswap @svm with ", " in @ansL200
NUMBER	F0No.R101
TEXT	F3DescriptionST601
TEXT2	SDescriptionS@ans=@record<3>distrib=@record<5>convert @svm to ':' in distribif distrib and not(index(distrib,'User',1)) then @ans[1,0]=field(distrib,':',1):': 'T600
\.


--
-- Name: changelog changelog_pkey; Type: CONSTRAINT; Schema: dict; Owner: exodus
--

ALTER TABLE ONLY dict.changelog
    ADD CONSTRAINT changelog_pkey PRIMARY KEY (key);


--
-- PostgreSQL database dump complete
--

