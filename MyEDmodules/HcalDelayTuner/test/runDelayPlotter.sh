#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
fi
#export OLD=../data/hotable.csv
#export FMT='%s %d %d %d %f'
#export NEW=../data/SettingsHO_091118postSplash09tune_floatingRings.csv

#export OLD=../data/hbhetable_08tuneInEta.csv
#export OLD=../data/hbhetable_091108tune.csv
#export NEW=../data/hbhetable_091118tune.csv

#export NEW=../data/SettingsHBHE_091118tune_patchfix15chan.csv
export NEW=hfdelpatgen_0.csv
export NEWFMT='%*s %*d %*d %*d %s %d %d %d %d'

CFGFILE=${0%.sh}_cfg.py
cat > ${CFGFILE} << EOF

import FWCore.ParameterSet.Config as cms

process = cms.Process("DELAYTUNER")

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
    fileName = cms.string("DelaySettings.root"),
    closeFileFast = cms.untracked.bool(False)
)

process.load("MyEDmodules.HcalDelayTuner.delaysettingplotter_cfi")
process.plotdelays.oldSettingParameters.fileNames = cms.untracked.vstring("${OLD}")
process.plotdelays.newSettingParameters.fileNames = cms.untracked.vstring("${NEW}")
process.plotdelays.oldSettingParameters.settingScanFmt=cms.untracked.string("${OLDFMT}")
process.plotdelays.newSettingParameters.settingScanFmt=cms.untracked.string("${NEWFMT}")
process.p = cms.Path(process.plotdelays)

EOF

cmsRun ${CFGFILE} 2>&1

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

rm ${CFGFILE}
