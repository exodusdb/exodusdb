#!/bin/bash
set -euxo pipefail
:
: $0 $*
: ===========================================================
: Install current dir of exodus in an LXC container in stages
: ===========================================================
:
: Syntax
: ------
:
:	$0 " <SOURCE> <NEW_CONTAINER_NAME> <REQ_STAGES> [g++|clang] [<PG_VER>]"
:
:	SOURCE e.g. an lxc image like ubuntu, ubuntu:22.04 etc. or an existing lxc container code
:
:	NEW_CONTAINER_NAME e.g. u2204 for lxc
:
	ALL_STAGES=bBdDTW
	DEFAULT_STAGES=bBdDT
:
:	REQ_STAGES must be one or more letters as follows -
:
:		A = All "'$DEFAULT_STAGES'" except W
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
:	Container names may be prefixed by lxc host. Recommended both the same to avoid copying across network.
:
:	e.g. ./install_lxc.sh nl22:u2204 nl22:u2204 A g++
:	e.g. ./install_lxc.sh nl22:ubuntu:22.04 nl22:u2204 A
:
:	Each stage of build, install and test will create
:	a new container with name ending "x-1", "x-2" etc. where x is g for g++ or c for clang
:
:	Each compiler will be setup in a new container
:	with name ending g or c for g++ and clang.
:
:	Note: /etc/cloud/cloud.cfg will be run on first boot of a new container to configure it.
:	e.g. Overwriting /etc/apt/sources.list. See that file for more info how to prevent that.

#: Parse command line
: ------------------
:
	#export DEBIAN_FRONTEND=noninteractive
	#export NEEDRESTART_MODE=a
	SOURCE=${1:?SOURCE is required. e.g. ubuntu, ubuntu:22.04 etc. or container code}
	NEW_CONTAINER_NAME=${2:?NEW_CONTAINER_NAME is required. e.g. u2204 for lxc}
	REQ_STAGES=${3:?Stages is required. A for '$DEFAULT_STAGES' - all except Web service}
	COMPILER=${4:-g++}
	PG_VER=${5:-}
:
: Validate
: --------
:
	if [[ $REQ_STAGES = "A" ]]; then
		REQ_STAGES=$DEFAULT_STAGES
	fi
	if [[ ! $ALL_STAGES =~ $REQ_STAGES ]]; then
		echo STAGES "'$REQ_STAGES'" must be one or more consecutive letters from $ALL_STAGES
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

	SSH_OPT="-o BatchMode=yes -o StrictHostKeyChecking=no"

#	IMAGE=u2204-preinstalled
#	lxc launch ubuntu:22.04 u2204-preinstalled
#	lxc start u2204-preinstalled
#	lxc shell u2204-preinstalled <<-V0G0N
#		sudo apt-get update
#		sudo apt-get install -y postgresql-common
#		sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh || true
#		sudo apt-get install -y cmake
#		sudo apt-get install -y g++ libpq-dev libboost-regex-dev libboost-locale-dev
#		sudo apt-get install -y postgresql-server-dev-{14,16}
#		sudo apt-get install -y postgresql-common
#		sudo apt-get install -y postgresql-{14,16} #for pgexodus install
#	V0G0N

function do_one_stage {
:
	STAGE_NO=$1
	STAGE_LETTER=${ALL_STAGES:$((STAGE_NO-1)):1}
	NEW_C=${NEW_CONTAINER_NAME}${COMPILER:0:1}-$STAGE_NO
:
: ============
: Do one stage - $STAGE_LETTER in ${NEW_C}
: ============
:
	OLD_C=${NEW_CONTAINER_NAME}${COMPILER:0:1}-$(($1 - 1))
:
: Create/Overwrite container - $NEW_C
: --------------------------
:
	if lxc info $NEW_C &> /dev/null; then
		lxc rm $NEW_C --force |& grep -v "already stopped"|| true
	fi
	if [[ $STAGE_NO == 1 ]]; then
		if lxc info $SOURCE &> /dev/null; then
			lxc snapshot $SOURCE install_lxc_sh --reuse
:
: Copy container using a snapshot
: Note. See note in heading about cloud.cfg which runs on first boot of new container
:
			lxc copy $SOURCE/install_lxc_sh $NEW_C
			lxc rm $SOURCE/install_lxc_sh
			lxc start $NEW_C
		else
			# Assume $SOURCE is an image
			lxc launch $SOURCE $NEW_C
		fi
	else
		lxc stop $OLD_C --force |& grep -v "already stopped"|| true
		lxc cp $OLD_C $NEW_C
		lxc start $NEW_C
	fi
:
: On the first stage copy local exodus to the target container - $NEW_C
: ------------------------------------------------------------
:
: TODO update the source container instead
:
	if [[ $FIRST_STAGE_UPDATE_EXODUS == YES ]]; then

		FIRST_STAGE_UPDATE_EXODUS=NO

# No need to use ssh and rsync because pushing a tar file retains the file and dir timestamps
# Retain code in case we need it in future
#
#:
#: Check if remote lxc host
#: ------------------------
#	:
#		if [[ $NEW_C =~ : ]]; then
#			USE_SSH=NO
#		else
#			USE_SSH=YES
#		fi
#
#		if [[ $USE_SSH == YES ]]; then
#:
#: Wait for ip no
#: --------------
#:
#			for i in {1..100}; do
#				IPNO=`lxc list $NEW_C --format csv --columns 4 | grep -P -o "\d+\.\d+\.\d+\.\d+" || true`
#				if [[ "$IPNO" != "" ]]; then
#					set -x
#					break
#				fi
#				sleep 1
#				set +x
#				echo Waiting for ip no to appear in lxc
#			done
#			echo $IPNO
#
#:
#: Enable ssh login
#: ----------------
#:
#			lxc exec $NEW_C -- bash -c "ssh-keygen -t rsa -N '' -f ~/.ssh/id_rsa <<< y"
#
#			lxc exec $NEW_C -- bash -c "echo '`cat ~/.ssh/*.pub`' >> /root/.ssh/authorized_keys"
#:
#: Check we can now ssh into the container
#:
#			sleep 1
#			if ! ssh $SSH_OPT root@$IPNO pwd ; then
#				sleep 1
#			fi
#
#: Check again
#: -----------
#:
#			if ! ssh $SSH_OPT root@$IPNO pwd; then
#				USE_SSH=NO
#			fi
#		fi
#
#:
#: Update container with local exodus dir
#: --------------------------------------
#:
#		if [[ $USE_SSH == YES ]]; then
#
#			CONNECTION=22
#			SSH_USER=root
#			SOURCE=
#			FILEORFOLDER=exodus
#			TARGET=root@$IPNO
#			PARENTPATH=/root/
#			rsync -avz --links -e "ssh -p ${CONNECTION} $SSH_OPT" `pwd` ${TARGET}:/root
#
#		else

:
: Update container with local exodus dir
: --------------------------------------
			#lxc file push * $NEW_C/root/exodus --recursive --create-dirs --quiet
			#lxc file push . $NEW_C/root --recursive --create-dirs --quiet
			#lxc file push ~/exodus $NEW_C/root --recursive --create-dirs --quiet --gid 0 --uid 0
			#lxc file push ~/exodus $NEW_C/root --recursive --create-dirs --quiet
	#		lxc file push ~/exodus ${NEW_C}${TARGET_HOME} --recursive --create-dirs --quiet

	#		Avoid push unless it is a local container since pushing thousands of files isnt efficient
	#		lxc file push ~/exodus ${NEW_C}${TARGET_HOME} --recursive --create-dirs
:
:	tar local exodus dir
:
			TAR_FILENAME=lxc_exodus.tar.z
			rm ../$TAR_FILENAME -rf
			tar cfz ../$TAR_FILENAME ../exodus
			TAR_SIZE=$(ls -lh ../$TAR_FILENAME|cut -d' ' -f5)
:
:	Push the tar file $TAR_SIZE to $NEW_C
:
			lxc file push ../$TAR_FILENAME ${NEW_C}${TARGET_HOME}/$TAR_FILENAME
			rm ../$TAR_FILENAME
:
:	Untar exodus on the target - $NEW_C
:
			lxc exec $NEW_C -- bash -c "tar xfz $TAR_FILENAME"
:

#		fi #not USE_SSH

:
: Avoid git error: "fatal: detected dubious ownership in repository at '.../exodus'"
: ----------------------------------------------------------------------------------
:
		lxc exec $NEW_C -- bash -c "chown -R $TARGET_UID:$TARGET_GID ${TARGET_HOME}/exodus"

:
: End of updating exodus on first stage - $STAGE_NO $STAGE_LETTER
: -------------------------------------
:
	fi

:
: Run the exodus/install.sh script on the target - $NEW_C
: ----------------------------------------------
:
	lxc exec $NEW_C  --user $TARGET_UID --group $TARGET_GID -- bash -c "cd $TARGET_HOME/exodus && HOME=${TARGET_HOME} ./install.sh \"$STAGE_LETTER\" \"$COMPILER\" ${PG_VER:-''}"
}
:
: ====
: MAIN
: ====
:
	FIRST_STAGE_UPDATE_EXODUS=YES
	[[ $REQ_STAGES =~ b ]] && do_one_stage 1 || true # 'Get dependencies for build'
	[[ $REQ_STAGES =~ B ]] && do_one_stage 2 || true # 'Build'
	[[ $REQ_STAGES =~ d ]] && do_one_stage 3 || true # 'Get dependencies for install'
	[[ $REQ_STAGES =~ D ]] && do_one_stage 4 || true # 'Install'
	[[ $REQ_STAGES =~ T ]] && do_one_stage 5 || true # 'Test'
	[[ $REQ_STAGES =~ W ]] && do_one_stage 6 || true # 'Install www service'
:
: ===============================================================
: Finished $0 $* in $((SECONDS/60)) mins and $((SECONDS%60)) secs
: ===============================================================
