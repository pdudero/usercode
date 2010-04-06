#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
fi

       echo '   '
       echo '   '
echo "      Please choose"
       echo '   '
       echo '   '

echo "  1 - Run 130910 - Mar. 14, 2010"
echo "  2 - Run 132440 - Mar. 30, 2010"
       echo '   '

read VAR1

SETUPFILE=anal_setup.rc

echo "PROCESSNAME=BEAMTIMEANAL" >${SETUPFILE}

case $VAR1 in
    "1") RUN=130910bdskim
	echo "GLOBALTOFFSET=0.0"        >>${SETUPFILE}
#       echo "BAD_EVENT_LIST="          >>${SETUPFILE}
	echo "BXNUMS=51"                >>${SETUPFILE}
	echo "GLOBAL_FLAG_MASK=0xC0003" >>${SETUPFILE}
#	echo "TIMEWINDOWMIN=-11"        >>${SETUPFILE}
#	echo "TIMEWINDOWMAX=11"         >>${SETUPFILE}
	echo "MINHITGEV=2."             >>${SETUPFILE}
	echo "MAXGEV2PLOT=100"          >>${SETUPFILE}
	echo "MAXEVENTNUM=30000000"     >>${SETUPFILE}
	echo "DOTREE=True"              >>${SETUPFILE}
	echo "DOUNPACKHCAL=1"           >>${SETUPFILE}
	echo "DOHCALRECO=1"             >>${SETUPFILE}
	echo "GLOBALTAG='GR10_P_V3::All'" >>${SETUPFILE}
	echo "RUNDESCR='Run 130910 2.36GeV HFTTP Trigger'"  >>${SETUPFILE}
#
	echo "HBHE_FIRSTSAMPLE=2"       >>${SETUPFILE}
	echo "HBHE_SAMPLESTOADD=5"      >>${SETUPFILE}
	echo "HF_FIRSTSAMPLE=3"         >>${SETUPFILE}
	echo "HF_SAMPLESTOADD=4"        >>${SETUPFILE}
#	echo "EVENTLIMIT=100"           >>${SETUPFILE}
#	echo 'EVRANGES=\"130910:1-130910:10000\",\"130910:10001-130910:20000\"' >> ${SETUPFILE}
	echo 'LSRANGES=\"130910:250-130910:MAX\"' >> ${SETUPFILE}
       ;;

    "2") cat anal_setup_1stPhys.rc >>${SETUPFILE}
	RUN=132440
#	DATASET=/store/express/Commissioning10/ExpressPhysics/FEVT/v7/000
#	DATASET=/store/data/Commissioning10/MinimumBias/RAW-RECO/v7/000
#	echo "GLOBALTOFFSET=0.0"        >>${SETUPFILE}
#       echo "BAD_EVENT_LIST="          >>${SETUPFILE}
	echo "BXNUMS=1"                 >>${SETUPFILE}
#	echo 'LSRANGES=\"132440:124-132440:MAX\"' >> ${SETUPFILE}
#	echo "DOTREE=False"             >>${SETUPFILE}
       ;;
esac
#./runBeamAnalBatch.sh
# Use cron job to pick it up
#
#cat >btajc.rc<<EOF
#ANALFILE=${SETUPFILE}
#DATASET=${DATASET}
#RUNS=${RUN}
#EOF
#tar czvf btajc.tz btajc.rc ${SETUPFILE}
#mv btajc.tz ~/public/inbox
#
./genbtapy.sh ${RUN}
