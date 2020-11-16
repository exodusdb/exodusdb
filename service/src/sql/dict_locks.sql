DROP TABLE IF EXISTS DICT_LOCKS CASCADE;
CREATE TABLE IF NOT EXISTS DICT_LOCKS
(
 key text primary key,
 data text
);
INSERT INTO DICT_LOCKS (key,data) values (E'STATION',E'F3StationSL101');
INSERT INTO DICT_LOCKS (key,data) values (E'SESSION',E'F5SessionSL101');
INSERT INTO DICT_LOCKS (key,data) values (E'DATETIME_LOCKED',E'F2Date TimeLockedS[DATETIME,4*DOS,MTSDOS]R101');
INSERT INTO DICT_LOCKS (key,data) values (E'DATETIME_EXPIRES',E'F1Date TimeExpiresS[DATETIME,4*DOS,MTSDOS]R101');
INSERT INTO DICT_LOCKS (key,data) values (E'MASTER_LOCK',E'F6MASTER LOCKSL10FILE*KEY*SESSIONID1');
INSERT INTO DICT_LOCKS (key,data) values (E'FILENAME',E'F0FilenameS1L201');
INSERT INTO DICT_LOCKS (key,data) values (E'KEY',E'F0KeyS2L201');
INSERT INTO DICT_LOCKS (key,data) values (E'@CRT',E'GID-SUPP FILENAME KEY2 DATETIME_LOCKED DATETIME_EXPIRES LOCK_EXPIRED STATION USER SESSION0');
INSERT INTO DICT_LOCKS (key,data) values (E'USER',E'F4UsernameSL101');
INSERT INTO DICT_LOCKS (key,data) values (E'LOCK_EXPIRED',E'SLockExpiredS call dostime(dostime);*seconds and hundreds since last midnight *convert to Windows based date/time (ndays since 1/1/1900) *31/12/67 in rev date() format equals 24837 in windows date format dostime=24837+date()+(dostime/24/3600) @ans=(@record<1> le dostime)L100');
INSERT INTO DICT_LOCKS (key,data) values (E'KEY2',E'SKeyS@ans=field(@id,\'*\',2,999)L200');
INSERT INTO DICT_LOCKS (key,data) values (E'WAITING_USERS',E'F7Waiting UsersML101');
