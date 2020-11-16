DROP TABLE IF EXISTS DICT_REQUESTLOG CASCADE;
CREATE TABLE IF NOT EXISTS DICT_REQUESTLOG
(
 key text primary key,
 data text
);
INSERT INTO DICT_REQUESTLOG (key,data) values (E'REQUEST1',E'F11Request1SL101');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'TIME',E'F0TimeS3MTSR81');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'RESPONSE',E'F6ResponseST201');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'REQUEST4',E'F14Request4SL101');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'REQUEST6',E'F16Request6SL101');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'PROCESS_NO',E'F0PS4R3');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'RESPONSE_TIME',E'F5Response TimeSMTSR81');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'@CRT',E'GID-SUPP DATABASE DATE TIME PROCESS_NO IPNO USERCODE REQUEST1 REQUEST2 REQUEST3 REQUEST4 REQUEST5 REQUEST6 REQUEST7 RESPONSE_TIME SECS RESPONSE0');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'DATE',E'F0DateS2[DATE,4*]R121');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'DATABASE',E'F0DatabaseS1L81');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'REQUEST2',E'F12Request2SL101');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'USERCODE',E'F1UserS0L10');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'REQUEST7',E'F17Request7SL101');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'SEQ',E'SSeqSreturn @idR100');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'REQUEST3',E'F13Request3SL101');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'IPNO',E'F2IPNoS0L151');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'REQUEST5',E'F15Request5SL101');
INSERT INTO DICT_REQUESTLOG (key,data) values (E'SECS',E'SSecsSresponsetime=@record<5>if len(responsetime) then @ans=@record<5>-field(@id,\'*\',3) if @ans<0 then @ans+=86400 *garbagecollect @ans=oconv(@ans,\'MD20P\')+0end else *blank if no response time (crashed) @ans=\'\' endR100');
