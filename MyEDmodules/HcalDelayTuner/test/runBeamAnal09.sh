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

echo "  1 - Run 122314 - Nov. 23, 2009"
echo "  2 - Run 123567 - Dec.  4, 2009"
echo "  3 - Run 123575 - Dec.  5, 2009"
echo "  4 - Run 123596 - Dec.  6, 2009"
       echo '   '

read VAR1

SETUPFILE=anal_setup.rc

case $VAR1 in
    "1") echo "RUN=122314xpres"         >${SETUPFILE}
#	echo "RUN=122314preskim"        >>${SETUPFILE}
#	echo "RUN=122314myhltskim"      >>${SETUPFILE}
	echo "GLOBALTOFFSET=0.0"        >>${SETUPFILE}
#       echo "BAD_EVENT_LIST="          >>${SETUPFILE}
	echo "BXNUMS=51"                >>${SETUPFILE}
	echo "GLOBAL_FLAG_MASK=0xC0003" >>${SETUPFILE}
	echo "TIMEWINDOWMIN=-11"        >>${SETUPFILE}
	echo "TIMEWINDOWMAX=11"         >>${SETUPFILE}
	echo "MINHITGEV=2."             >>${SETUPFILE}
	echo "MAXGEV2PLOT=100"          >>${SETUPFILE}
	echo "MAXEVENTNUM=30000000"     >>${SETUPFILE}
	echo "RUNDESCR='Run 122314 Min-Bias Trigger, 142 events'"  >>${SETUPFILE}
       ;;
    "2")  echo "RUN=123567xpres"        >${SETUPFILE}
	echo "GLOBALTOFFSET=0.0"        >>${SETUPFILE}
#       echo "BAD_EVENT_LIST="          >>${SETUPFILE}
#       echo "BXNUMS=51"                >>${SETUPFILE}
	echo "BXNUMS=942"               >>${SETUPFILE}
	echo "GLOBAL_FLAG_MASK=0xC0003" >>${SETUPFILE}
	echo "TIMEWINDOWMIN=-11"        >>${SETUPFILE}
	echo "TIMEWINDOWMAX=11"         >>${SETUPFILE}
	echo "MINHITGEV=2".       .     >>${SETUPFILE}
	echo "MAXGEV2PLOT=100"          >>${SETUPFILE}
	echo "MAXEVENTNUM=1000000"      >>${SETUPFILE}
	echo "RUNDESCR='Run 123567 BSC,HFTTP Triggers, 2x2'" >>${SETUPFILE}
	;;
    "3")
	echo "RUN=123575myhltskim"      >${SETUPFILE}
#	echo "RUN=123575xpres"          >>${SETUPFILE}
	echo "GLOBALTOFFSET=0.0"        >>${SETUPFILE}
#       echo "BAD_EVENT_LIST="          >>${SETUPFILE}
	echo "BXNUMS=51"                >>${SETUPFILE}
	echo "GLOBAL_FLAG_MASK=0xC0003" >>${SETUPFILE}
	echo "TIMEWINDOWMIN=-11"        >>${SETUPFILE}
	echo "TIMEWINDOWMAX=11"         >>${SETUPFILE}
	echo "MINHITGEV=2."             >>${SETUPFILE}
	echo "MAXGEV2PLOT=100"          >>${SETUPFILE}
	echo "MAXEVENTNUM=8000000"      >>${SETUPFILE}
	echo "RUNDESCR='Run 123575 BSC,HFTTP Triggers, 1x1'" >>${SETUPFILE}
	;;
    "4")
	echo "RUN=123596myhltskim"      >${SETUPFILE}
#	echo "RUN=123575xpres"          >>${SETUPFILE}
	echo "GLOBALTOFFSET=0.0"        >>${SETUPFILE}
#       echo "BAD_EVENT_LIST="          >>${SETUPFILE}
# file already prefiltered for bx's
	echo "BXNUMS="                  >>${SETUPFILE}
	echo "GLOBAL_FLAG_MASK=0xC0003" >>${SETUPFILE}
	echo "TIMEWINDOWMIN=-11"        >>${SETUPFILE}
	echo "TIMEWINDOWMAX=11"         >>${SETUPFILE}
	echo "MINHITGEV=1."             >>${SETUPFILE}
	echo "MAXGEV2PLOT=100"          >>${SETUPFILE}
	echo "MAXEVENTNUM=8000000"      >>${SETUPFILE}
	echo "RUNDESCR='Run 123596 BSC Triggers, 4x4'" >>${SETUPFILE}
	;;
esac

./runBeamAnalBatch.sh
