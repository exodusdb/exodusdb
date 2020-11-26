DROP TABLE IF EXISTS DICT_CHANGELOG CASCADE;
CREATE TABLE IF NOT EXISTS DICT_CHANGELOG
(
 key text primary key,
 data text
);
ALTER TABLE DICT_CHANGELOG OWNER to exodus;
INSERT INTO DICT_CHANGELOG (key,data) values (E'DATE_TIME',E'F4Date_time[DATETIME,4*,MTS]R101');
INSERT INTO DICT_CHANGELOG (key,data) values (E'DISTRIBUTION',E'F5DistributionM0L1011');
INSERT INTO DICT_CHANGELOG (key,data) values (E'NUMBER',E'F0No.R101');
INSERT INTO DICT_CHANGELOG (key,data) values (E'KEYWORD',E'F2KeywordM1L10N1');
INSERT INTO DICT_CHANGELOG (key,data) values (E'@CRT',E'GDATE KEYWORDS TEXT0');
INSERT INTO DICT_CHANGELOG (key,data) values (E'TEXT',E'F3DescriptionST601');
INSERT INTO DICT_CHANGELOG (key,data) values (E'DATE',E'F1Date[DATE,4*]R101');
INSERT INTO DICT_CHANGELOG (key,data) values (E'TEXT2',E'SDescriptionS@ans=@record<3>distrib=@record<5>convert @svm to \':\' in distribif distrib and not(index(distrib,\'User\',1)) then @ans[1,0]=field(distrib,\':\',1):\': \'T600');
INSERT INTO DICT_CHANGELOG (key,data) values (E'KEYWORDS',E'SKeywordsS@ans=@record<2>swap @vm with ", " in @ansswap @svm with ", " in @ansL200');
