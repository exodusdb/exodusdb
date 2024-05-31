#!/bin/bash
set -euxo pipefail
:
: ===========================================================
: Install current dir of exodus in an LXC container in stages
: ===========================================================
:
: Syntax
: ------
:
:	$0 " <SOURCE> <NEW_CONTAINER_NAME> <STAGES> [gcc|clang] [<PG_VER>]"
:
:	SOURCE e.g. an lxc image like ubuntu, ubuntu:22.04 etc. or an existing lxc container code
:
:	NEW_CONTAINER_NAME e.g. u2204 for lxc
:
:	STAGES is letters.
:
:		A = All "'bBiIT'" except W
:
:		b = Get dependencies for build
:		B = Build
:
:		i = Get dependencies for install
:		I = Install
:
:		T = Test
:
:		W = Install www service
:
:	PG_VER e.g. 14 or blank for the the default which depends on the Ubuntu version and apt.
:
: Parse command line
: ------------------
:
	#export DEBIAN_FRONTEND=noninteractive
	#export NEEDRESTART_MODE=a
	SOURCE=${1:?SOURCE is required. e.g. ubuntu, ubuntu:22.04 etc. or container code}
	NEW_CONTAINER_NAME=${2:?NEW_CONTAINER_NAME is required. e.g. u2204 for lxc}
#	STAGES=${3:-bBiIT}
	STAGES=${3:?Stages is required. A for 'bBiIT' - all except Web service}
	COMPILER=${4:-gcc}
	PG_VER=${5:-}
:
: Validate
: --------
:
	if [[ $STAGES = "A" ]]; then
		STAGES=bBiIT
	fi
	if [[ ! $STAGES =~ ^[bBiITW]*$ ]]; then
		echo STAGES has only letters bBiITW. Check syntax above.
		exit 1
	fi
:
	if [[ ! $COMPILER =~ gcc|clang ]]; then
		echo COMPILER must be gcc or clang
		exit 1
	fi
:
	if [[ ! $PG_VER =~ ^[0-9]*$ ]]; then
		echo Postgres version is only digits or blank. Check syntax above.
		exit 1
	fi
:
: Config
: ------
:
#	# Install into user 'ubuntu'
#	TARGET_UID=1000
#	TARGET_GID=1000
#	TARGET_HOME=/home/ubuntu

	# Install into root
	TARGET_UID=0
	TARGET_GID=0
	TARGET_HOME=/root

#	IMAGE=u2204-preinstalled
#	lxc launch ubuntu:22.04 u2204-preinstalled
#	lxc start u2204-preinstalled
#	lxc shell u2204-preinstalled <<-V0G0N
#		sudo apt update
#		sudo apt install -y postgresql-common
#		sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh || true
#		sudo apt install -y cmake
#		sudo apt install -y g++ libpq-dev libboost-regex-dev libboost-locale-dev
#		sudo apt install -y postgresql-server-dev-{14,16}
#		sudo apt install -y postgresql-common
#		sudo apt install -y postgresql-{14,16} #for pgexodus install
#	V0G0N

: =========================
: Function to do each stage
: =========================
:
function stage {
:
	STAGE=$1

	OLD_C=${NEW_CONTAINER_NAME}-$(($1 - 1))
	NEW_C=${NEW_CONTAINER_NAME}-$1
:
: Create/Overwrite container
: --------------------------
:
	if lxc info $NEW_C &> /dev/null; then
		lxc rm $NEW_C --force |& grep -v "already stopped"|| true
	fi
	if [[ $STAGE == 1 ]]; then
		if lxc info $SOURCE &> /dev/null; then
			# copy container using a snapshot
			lxc snapshot $SOURCE install_lxc_sh --reuse || exit 1
			lxc copy $SOURCE/install_lxc_sh $NEW_C || exit 1
			lxc rm $SOURCE/install_lxc_sh || exit 1
			lxc start $NEW_C || exit 1
		else
			# Assume $SOURCE is an image
			lxc launch $SOURCE $NEW_C || exit 1
		fi
	else
		lxc stop $OLD_C --force |& grep -v "already stopped"|| true
		lxc cp $OLD_C $NEW_C || exit 1
		lxc start $NEW_C || exit 1
	fi
:
: Update container with local exodus dir
: --------------------------------------
:
	#lxc file push * $NEW_C/root/exodus --recursive --create-dirs --quiet
	#lxc file push . $NEW_C/root --recursive --create-dirs --quiet
	#lxc file push ~/exodus $NEW_C/root --recursive --create-dirs --quiet --gid 0 --uid 0
	#lxc file push ~/exodus $NEW_C/root --recursive --create-dirs --quiet || exit 1
	lxc file push ~/exodus ${NEW_C}${TARGET_HOME} --recursive --create-dirs --quiet || exit 1
:
	#Avoid git error: "fatal: detected dubious ownership in repository at '.../exodus''
	lxc exec $NEW_C -- bash -c "chown -R $TARGET_UID:$TARGET_GID ${TARGET_HOME}/exodus" || exit 1
:
: Run the stage install script
: ----------------------------
:
	STAGE_LETTERS=bBiITW
	STAGE_LETTER=${STAGE_LETTERS:$((STAGE-1)):1}

	lxc exec $NEW_C  --user $TARGET_UID --group $TARGET_GID -- bash -c "cd $TARGET_HOME/exodus && HOME=${TARGET_HOME} ./install.sh \"$STAGE_LETTER\" \"$COMPILER\" ${PG_VER:-''}" || exit 1
}
:
: ====
: MAIN
: ====
:
	[[ $STAGES =~ b ]] && stage 1 || true # 'Get dependencies for build'
	[[ $STAGES =~ B ]] && stage 2 || true # 'Build'
	[[ $STAGES =~ i ]] && stage 3 || true # 'Get dependencies for install'
	[[ $STAGES =~ I ]] && stage 4 || true # 'Install'
	[[ $STAGES =~ T ]] && stage 5 || true # 'Test'
	[[ $STAGES =~ W ]] && stage 6 || true # 'Install www service'
:
: ===============================================================
: Finished $0 $* in $((SECONDS/60)) mins and $((SECONDS%60)) secs
: ===============================================================
