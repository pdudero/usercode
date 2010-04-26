#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
fi

export WRITEBRICKS=False
export GLOBALOFFSET=0
export LASTCUTDIR="hftimeanal/HF/cut8keepGoodHits"
#export LASTCUTDIR="hftimeanal/HF/cut4outOfTime"

CFGFILE=${0%.sh}_cfg.py
cat > ${CFGFILE} << EOF

import FWCore.ParameterSet.Config as cms

process = cms.Process("HFSCANPOSTAN")

#----------------------------
# Event Source
#-----------------------------

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(0))
process.source    = cms.Source("EmptySource")

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);

#-----------------------------
# Hcal Digis and RecHits
#-----------------------------

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("hfscanpostanal.root"),
    closeFileFast = cms.untracked.bool(False)
)

process.load("MyEDmodules.HcalDelayTuner.hfscanruns_cfi")
process.hfscananal.writeBricks = cms.untracked.bool(${WRITEBRICKS})
process.hfscananal.globalOffsetns = cms.int32(${GLOBALOFFSET})
process.hfscananal.lastCutDir = cms.untracked.string("${LASTCUTDIR}")

process.p = cms.Path(process.hfscananal)

EOF

cmsRun ${CFGFILE} 2>&1

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

#rm ${CFGFILE}
