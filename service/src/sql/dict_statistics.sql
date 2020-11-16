DROP TABLE IF EXISTS DICT_STATISTICS CASCADE;
CREATE TABLE IF NOT EXISTS DICT_STATISTICS
(
 key text primary key,
 data text
);
INSERT INTO DICT_STATISTICS (key,data) values (E'SESSION',E'F0SessionS2R101');
INSERT INTO DICT_STATISTICS (key,data) values (E'DEPARTMENT',E'SDepartmentS@ans=@record<3>*may be missing try to get from user fileif @ans else @ans=xlate(\'USERS\',field(@id,\'*\',3),5,\'X\')*use database as better than blank/unknownif @ans else if field(@id,\'*\',3)=\'NEOSYS\' then  @ans=\'NEOSYS\' end else  @ans=field(@id,\'*\',1)  end endT200');
INSERT INTO DICT_STATISTICS (key,data) values (E'USER_BROWSER',E'SUser-BrowserSusercode=field(@id,\'*\',3)@ans=xlate(\'USERS\',usercode,39,\'X\')<1,6>T200');
INSERT INTO DICT_STATISTICS (key,data) values (E'LAST_ACCESS',E'F1Last AccessS[DATETIME,4*,DOS]R101');
INSERT INTO DICT_STATISTICS (key,data) values (E'@CRT',E'GID-SUPP BY DATE BY HOUR BY LAST_ACCESS_TIME DATE HOUR SESSION DEPARTMENT USER_CODE IP_NO DATABASE REQUESTS LAST_ACCESS_TIME0');
INSERT INTO DICT_STATISTICS (key,data) values (E'LAST_ACCESS_TIME',E'SLast-Access-TimeS[TIME2]@ans=mod(@record<1>*86400+@sw<1>,86400)R100');
INSERT INTO DICT_STATISTICS (key,data) values (E'USER_CODE',E'SUserS@ans=field(@id,\'*\',3)swap \' \' with \'_\' in @ansL100');
INSERT INTO DICT_STATISTICS (key,data) values (E'DATE',E'F0DateS5[DATE,4*]R101');
INSERT INTO DICT_STATISTICS (key,data) values (E'HOUR',E'F0HourS6R21');
INSERT INTO DICT_STATISTICS (key,data) values (E'REQUESTS',E'F2RequestsSR101');
INSERT INTO DICT_STATISTICS (key,data) values (E'DATABASE',E'F0DatabaseS1L101');
INSERT INTO DICT_STATISTICS (key,data) values (E'IP_NO',E'F0IP-No.S4L101');
INSERT INTO DICT_STATISTICS (key,data) values (E'USER_NAME',E'SUser-NameSusercode=field(@id,\'*\',3)@ans=xlate(\'USERS\',usercode,1,\'X\')if @ans=\'\' then @ans=usercode swap \' \' with \'_\' in @ansend else if @ans<>usercode then @ans:=\' (\':usercode:\')\' endT200');
