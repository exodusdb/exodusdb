DROP TABLE IF EXISTS DICT.PROCESSES CASCADE;
CREATE TABLE IF NOT EXISTS DICT.PROCESSES
(
 key text primary key,
 data text
);
ALTER TABLE DICT.PROCESSES OWNER to exodus;
INSERT INTO DICT.PROCESSES (key,data) values (E'NOT_INTERACTIVE',E'F31NotInteractiveSL101');
INSERT INTO DICT.PROCESSES (key,data) values (E'CONNECTION',E'F54ConnectionML101');
INSERT INTO DICT.PROCESSES (key,data) values (E'LAST_UPDATED2',E'SLast UpdatedS/*dostimenow=date()+24873+time()/86400elapsedsecs=int((dostimenow-@record<27>)*86400)ans=\'\'interval=604800intervalname=\'weeks\'gosub addintervaldescinterval=interval/7intervalname=\'days\'gosub addintervaldescinterval=interval/24intervalname=\'hours\'gosub addintervaldescinterval=interval/60intervalname=\'mins\'gosub addintervaldescinterval=1intervalname=\'secs\'gosub addintervaldescreturn trim(ans)****************addintervaldesc:****************if interval=1 or elapsedsecs>interval then nintervals=int(elapsedsecs/interval) elapsedsecs-=nintervals*interval if nintervals=1 then intervalname[-1,1]=\'\' ans:=\' \':nintervals:\' \':intervalname endreturn*/declare function elapsedtimetextfromdate=int(@record<27>)-24873fromtime=field(@record<27>,\'.\',2)/86400return elapsedtimetext(fromdate,fromtime,uptodate,uptotime)T100');
INSERT INTO DICT.PROCESSES (key,data) values (E'PROCESS_NO',E'F0ProcessNoS0R31');
INSERT INTO DICT.PROCESSES (key,data) values (E'MAINTENANCE',E'SMaintenanceSif @record<33> then @ans=\'\' else @ans=\'Yes\'L100');
INSERT INTO DICT.PROCESSES (key,data) values (E'PID',E'SPidS@ans=@record<54,5>R50');
INSERT INTO DICT.PROCESSES (key,data) values (E'@CRT',E'GID-SUPP BY PROCESS_NO PROCESS_NO SYSTEM_CODE DATABASE_CODE PID LAST_UPDATED2 STATUS0');
INSERT INTO DICT.PROCESSES (key,data) values (E'LAST_UPDATED',E'F27LastUpdatedS[DATETIME,DOS*,DOSMTS]R201');
INSERT INTO DICT.PROCESSES (key,data) values (E'SYSTEM_CODE',E'F51SystemCodeSL101');
INSERT INTO DICT.PROCESSES (key,data) values (E'DATABASE_CODE',E'F17DatabaseCodeSL101');
INSERT INTO DICT.PROCESSES (key,data) values (E'STATUS',E'SStatusS*NB add any new statuses to MONITORING programif @record<52> then @ans=\'Closed\' return @ans endcall processlocking(\'ISLOCKED\',@id,islocked)if islocked else @ans=\'Crashed\' return @ans enddostimenow=date()+24873+time()/86400*10 minshungtime=10*60/86400r33=@record<33>if r33 and (dostimenow-@record<27>) gt hungtime then @ans=\'Hung\' return @ans endif r33 then @ans=\'OK\'end else @ans=\'Maintenance\' endif @record<53> then @ans:=\' \':@record<53>return @ansL100');
