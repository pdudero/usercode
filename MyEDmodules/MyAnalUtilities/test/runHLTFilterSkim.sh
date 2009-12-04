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

echo "  1 - Run 12XXXX - Dec.  5, 2009"
       echo '   '

read VAR1

case $VAR1 in
  "1") export RUN=
       export FILES=
       export HLTPATHS=
       ;;
esac

echo "Processing $RUN..."

CFGFILE=$0_$1_cfg.py
cat > ${CFGFILE} << EOF

import FWCore.ParameterSet.Config as cms

process = cms.Process("MYHLTFILT")

#----------------------------
# Event Source
#-----------------------------
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32(-1)
)

process.source = cms.Source( "PoolSource",
   fileNames = cms.untracked.vstring($FILES)
)

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);
process.MessageLogger.cerr.threshold = cms.untracked.string("DEBUG")
process.MessageLogger.debugModules = cms.untracked.vstring("*")
process.load("HLTrigger.HLTfilters.hltHighLevel_cfi")
process.hltHighLevel.HLTPaths = cms.vstring("${HLTPATHS}")

process.p = cms.Path(process.hltHighLevel)

 Output module configuration
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('hltskim_run$1_-pool.root'),
    # save only events passing the full path
    SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
#   outputCommands = cms.untracked.vstring('drop *')
)

EOF

cmsRun ${CFGFILE} 2>&1 | tee ./logs/hltHighLevelFilter_run$RUN.log

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

rm ${CFGFILE}
