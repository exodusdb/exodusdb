#!/bin/bash
#[ "$INSTALL_SH" ] || [ "$0" = bash ] || { INSTALL_SH=1 exec bash "$0" "$@" 2>&1 | tee -a "$(echo ${0##*/}.$*.$(date +%H%M%S).log | tr ' :@' '.__')>
set -euxom pipefail
PS4='+ [./fix.foreign.dbname.sh:$LINENO ${1-} ${SECONDS}s] '
: $0 "$@"
:
: Ensure foreign dbname suffix _test matches the dbname suffix _test or lack thereof
: ─────────────────────────────────────────────────────────────────────────────────
: Command line
	EXO_DATA=${1:?exo_data is required}
:
: Get foreign server id if any and quit if none
	foreign_server_id=`sudo -u postgres psql $EXO_DATA -t -c "\des+" | awk '{ print $1}'`
	if ! [ $foreign_server_id ]; then
		: No foreign server declared.
		exit 0
	fi
:
: Info
: ────────────────────────────────────────
: 'Also remove the host so the connection are fast linux socket connections instead of tcp/ip'
:
: 'Only the foreign dbname is important. The actual foreign server name is just an ID or handle and is irrelevent.'
:
: 'Changing a foreign servers dbname changes the attachment of all foreign tables that were associated with that foreign server'
:
: 'No superuser ability is required'
:
: "batesk_test=> \des+"
: "                                                         List of foreign servers"
: "  Name  | Owner  | Foreign-data wrapper | Access privileges | Type | Version |                 FDW options                  | Description "
: "--------+--------+----------------------+-------------------+------+---------+----------------------------------------------+-------------"
: " batesd | exodus | postgres_fdw         |                   |      |         | (dbname 'batesd_test', host '', port '5432') | "
:
: Verify only one since we can handle only one currently
: ────────────────────────────────────────
	if [ "`echo $foreign_server_id | wc -l`" -gt 1 ]; then
		: "ERROR: Reattaching foreign tables expects only one foreign server!"
		exit 1
	fi
:
: Foreign dbname suffix must match that of the dbname
: ────────────────────────────────────────
	if [[ $EXO_DATA =~ _test ]]; then
		foreign_dbname=${foreign_server_id%_test}_test
	else
		foreign_dbname=${foreign_server_id%_test}
	fi
:
: Alter the foreign dbname
: ────────────────────────────────────────
	if ! sudo -u postgres psql $EXO_DATA -t -c "ALTER SERVER $foreign_server_id OPTIONS (SET host '', SET dbname '$foreign_dbname');"; then
	    : "ERROR: $EXO_DATA. Problem changing foreign database to $foreign_dbname"
	    exit 1
	fi
:
: Dump the foreign declarations. NOTE: Server/Server Name is only an ID. The dbname is the important thing.
: ────────────────────────────────────────
	sudo -u postgres psql $EXO_DATA -c "\des+;" -c "\det+"
:
: Finished "$@"
: ────────────────────────────────────────
