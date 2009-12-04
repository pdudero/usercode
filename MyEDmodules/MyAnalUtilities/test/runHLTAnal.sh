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

echo "  1 - Run 120015 - Nov.  7, 2009 (from -Z)"
echo "  2 - Run 120042 - Nov.  9, 2009 (from -Z)"
echo "  3 - Run 121943 - Nov. 20, 2009 (from +Z)"
echo "  4 - Run 121964 - Nov. 20, 2009 (from -Z)"
echo "  5 - Run 121993 - Nov. 21, 2009 (from -Z)"
       echo '   '

read VAR1

case $VAR1 in
  "1") export RUN=120015
#       export FILES="'rfio:/castor/cern.ch/user/d/dudero/Splash2009skims/run120015-rhskim-nodigis-59events.root'"
       export FILES="'/store/data/BeamCommissioning09/BeamHalo/RECO/v1/000/120/015/3EDA5631-80CC-DE11-A980-0030487A3C9A.root'"
       export RUNDESCR="'Run 120015 Splash from -Z'"
       export GLOBALTOFFSET=-7.31
       export SPLASHZSIDEPLUS=False
       export BAD_EVENT_LIST=8,51,270
       export BXNUMS=2604
       export GLOBAL_FLAG_MASK=0xC0003
       export TIMEWINDOWMIN=-11
       export TIMEWINDOWMAX=11
       ;;
  "2") export RUN=120042
#      export FILES="'rfio:/castor/cern.ch/user/d/dudero/Splash2009skims/run120042-rhskim-nodigis.root'"
       export FILES="'/store/data/BeamCommissioning09/BeamHalo/RECO/v1/000/120/042/3EE31BC1-FECC-DE11-9FC2-001617C3B79A.root'"
       export RUNDESCR="'Run 120042 Splash from -Z'"
# raw:
#       export GLOBALTOFFSET=0.0
# b4 corrections, zero=barrel center:
#       export GLOBALTOFFSET=-11.67
# with adjusted delay settings applied:
       export GLOBALTOFFSET=-5.67
       export SPLASHZSIDEPLUS=False
       export BXNUMS=100
       export GLOBAL_FLAG_MASK=0xC0003
#      export BAD_EVENT_LIST=8,51,270
       export TIMEWINDOWMIN=-10
       export TIMEWINDOWMAX=5
       export MAXEVENTNUM=1600000
       ;;
  "3") export RUN=121943
#      export FILES="'file:/afs/cern.ch/user/d/dudero/scratch0/data/Splash09skims/run121943-rhskim.root'"
       export FILES="'/store/data/BeamCommissioning09/PhysicsMuonBkg/RECO/v2/000/121/943/80633910-12D6-DE11-8D71-001D09F2932B.root'"
       export RUNDESCR="'Run 121943 Splash from +Z'"
       export GLOBALTOFFSET=34.84
       export SPLASHZSIDEPLUS=True
#      export BXNUMS=339
       export BXNUMS=341
       export GLOBAL_FLAG_MASK=0xC0003
#      export BAD_EVENT_LIST=8,51,270
       export TIMEWINDOWMIN=-10
       export TIMEWINDOWMAX=5
       export MAXEVENTNUM=200
       ;;
  "4") export RUN=121964
#       export FILES="'/store/data/BeamCommissioning09/PhysicsMuonBkg/RECO/v2/000/121/964/96F2B74C-43D6-DE11-8340-0030487A18A4.root'"
       export FILES="'file:../../HcalRecHitFilter/test/run121964-rhskim.root'"
       export RUNDESCR="'Run 121964 Splash from -Z'"
       export GLOBALTOFFSET=5.76
       export SPLASHZSIDEPLUS=False
       export BXNUMS=2607
       export GLOBAL_FLAG_MASK=0xC0003
       export BAD_EVENT_LIST=97
       export TIMEWINDOWMIN=-10
       export TIMEWINDOWMAX=5
       export MAXEVENTNUM=166000
       ;;
  "5") export RUN=121993
       export FILES="'file:~/stage/1E2E81FE-9FD6-DE11-A5B2-0030487A18A4.root'"
       export RUNDESCR="'Run 121993 Splash from -Z'"
       export GLOBALTOFFSET=0.0
       export SPLASHZSIDEPLUS=False
       export BXNUMS=342
       export GLOBAL_FLAG_MASK=0xC0003
       export BAD_EVENT_LIST=97
       export TIMEWINDOWMIN=-10
       export TIMEWINDOWMAX=5
       export MAXEVENTNUM=-1
       ;;
esac

echo "Processing $RUN..."

EVENTS=$MAXEVENTNUM

#if [ $# -eq 2 ]
#then
#  EVENTS=$2
#fi

CFGFILE=$0_$1_cfg.py
cat > ${CFGFILE} << EOF

import FWCore.ParameterSet.Config as cms

process = cms.Process("MYHLTANAL")

#----------------------------
# Event Source
#-----------------------------
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( ${EVENTS} )
)

process.source = cms.Source( "PoolSource",
   fileNames = cms.untracked.vstring($FILES)
)

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);
#process.MessageLogger.cerr.threshold = cms.untracked.string("DEBUG")
#process.MessageLogger.debugModules = cms.untracked.vstring("*")
process.load("HLTrigger.HLTcore.triggerSummaryAnalyzerAOD_cfi")

process.p = cms.Path(process.triggerSummaryAnalyzerAOD)
EOF

cmsRun ${CFGFILE} 2>&1 | tee ./logs/hltAnal_run$RUN.log

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

#rm ${CFGFILE}
