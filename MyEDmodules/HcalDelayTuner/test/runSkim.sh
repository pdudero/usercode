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

echo "  1 - Run 122314"
echo "  2 - Run 123575"
echo "  3 - Run 123587"
echo "  4 - Run 123596"
       echo '   '

read VAR1

SETUPFILE=skim_setup.rc

case $VAR1 in
  "1") echo "RUN=122314"                     >${SETUPFILE}
#       echo "HLTPATHS='"HLT_Jet15U"'"       >>${SETUPFILE}
       echo "HLTPATHS='\"HLT_HFThreshold\"'" >>${SETUPFILE}
       ;;
  "2") echo "RUN=123575xpres"                >${SETUPFILE}
       echo "HLTPATHS='\"HLT_SplashBSC\"'"   >>${SETUPFILE}
       ;;
  "3") echo "RUN=123587xpres"                >${SETUPFILE}
       echo "HLTPATHS='\"HLT_MinBiasBSC\",\"HLT_MinBiasHcal\"'" >>${SETUPFILE}
       ;;
  "4") echo "RUN=123596xpres"                >${SETUPFILE}
       echo "HLTPATHS='\"HLT_MinBiasBSC\"'"  >>${SETUPFILE}
       ;;
esac

./runSkimBatch.sh
