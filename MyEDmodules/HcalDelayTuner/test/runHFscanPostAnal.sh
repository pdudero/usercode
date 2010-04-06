#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
fi

export NEWFMT='%*s %*d %*d %*d %s %d %d %d %d'

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

process.load("MyEDmodules.HcalDelayTuner.hfscanpostanal_cfi")
process.p = cms.Path(process.hfscananal)

EOF

cmsRun ${CFGFILE} 2>&1

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

rm ${CFGFILE}
