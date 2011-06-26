#/!bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
BASEDIR=${SCRIPT_DIR}/pkglib

SEARCHDIRS=`pg_config --bindir 2>/dev/null`
if [ ! -d "$SEARCHDIRS" ]
then
echo
echo pg_config not in path
SEARCHDIRS="/Library /opt"
fi

for SEARCHDIR in $SEARCHDIRS
do

echo
echo Searching $SEARCHDIR for pg_config

for PGCONFIG in `find $SEARCHDIR -type f -name pg_config 2>/dev/null`
do

PKGLIBDIR=`$PGCONFIG --pkglibdir`
PGMAJOR=`$PGCONFIG --version|cut -f 2 -d " "|cut -f 1 -d "."`
echo
echo Found version $PGMAJOR using $PGCONFIG

SAVEDIR=$BASEDIR/pgexodus-$PGMAJOR-${MACHTYPE}/

if [ -d "$SAVEDIR" ]
then
echo "cp -a ${SAVEDIR}* ${PKGLIBDIR}/"
cp -a ${SAVEDIR}* ${PKGLIBDIR}/
#ls -l ${SAVEDIR}*
#ls -l $PKGLIBDIR/pgexodus*
else

echo Error: --- No pgexodus lib for $PGMAJOR-$MACHTYPE ---
fi

done

done
