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
       echo '   '

read VAR1

SETUPFILE=anal_setup.rc

case $VAR1 in
    "1") echo "RUN=130910bdskim"        >${SETUPFILE}
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
esac

echo "ANALOUTPUTFMT='bta_%s.root'"      >>${SETUPFILE}

./runBeamAnalBatch.sh
