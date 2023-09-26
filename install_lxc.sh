#!/bin/bash
set -euxo pipefail
:
: Syntax
: ------
:
:	$0 " <UBUNTU_VER> [<PG_VER>] [<STAGES>]"
:
: UBUNTU_VER e.g. 22.04
:
: PG_VER e.g. 14 The default depends on apt an d Ubuntu version.
:
: STAGE is letters. Default is "'bBiIT'"
:
: b = Dependencies for Build
: B = Build
:
: i = Dependencies for Install
: I = Install
:
: T = Test
:
: Config
: ------
:
	#export DEBIAN_FRONTEND=noninteractive
	#export NEEDRESTART_MODE=a
	UBUNTU_VER=${1:?UBUNTU_VER is required. e.g. 22.04}
	PG_VER=${2:-}
	STAGES=${3:-bBiIT}

	if [[ ! $PG_VER =~ ^[0-9]*$ ]]; then
		echo Postgres version is only digits or blank. Check syntax above.
		exit 1
	fi

	if [[ ! $STAGES =~ ^[bBiIT]*$ ]]; then
		echo STAGES has only letters bBiIT. Check syntax above.
		exit 1
	fi

:
: Function to do one stage
: ------------------------
:
function stage {
:
	STAGE=$1
	OLD_C=t4-${UBUNTU_VER//./}-$(($1 - 1))
	NEW_C=t4-${UBUNTU_VER//./}-$1
:
: Create/Overwrite container
:
	lxc rm $NEW_C --force || true
	if [[ $STAGE == 1 ]]; then
		lxc launch ubuntu:$UBUNTU_VER $NEW_C
	else
		lxc stop $OLD_C --force || true
		lxc cp $OLD_C $NEW_C
		lxc start $NEW_C
	fi
:
: Update container
:
	lxc file push * $NEW_C/root/exodus --recursive --create-dirs --quiet
:
: Run the stage script
:
	STAGE_LETTERS=bBiIT
	STAGE_LETTER=${STAGE_LETTERS:$((STAGE-1)):1}
	lxc exec $NEW_C -- bash -c "ls && cd exodus && ./install.sh ${PG_VER:-''} $STAGE_LETTER" || exit 1
}
:
: ----
: Main
: ----
:
	[[ $STAGES =~ b ]] && stage 1 || true # Dependencies for build
	[[ $STAGES =~ B ]] && stage 2 || true # Build
	[[ $STAGES =~ i ]] && stage 3 || true # Dependencies for install
	[[ $STAGES =~ I ]] && stage 4 || true # Install
	[[ $STAGES =~ T ]] && stage 5 || true # Test
:
: ------------------------------------------------------------
: Finished $0 $* in $((SECONDS/60)) mins and $((SECONDS%60)) secs
: ------------------------------------------------------------
