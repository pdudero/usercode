#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
fi
#export OLD=../data/hotable.csv
#export FMT='%s %d %d %d %f'
#export NEW=../data/SettingsHO_091118postSplash09tune_floatingRings.csv

#export OLD=../data/hbhetable_08etaTune.csv
#export OLD=../data/hbhetable_091108tune.csv
#export OLD=../data/hbhetable_08tuneInEta.csv
#export OLD=../data/hbhetable_091108tune.csv
#export OLD=jittertrial1_hf.csv
#export OLD=jittertrial2_hf.csv
#export OLD=depthfixsettings.csv
#export OLD=removebadflagcutfix.csv
export OLD=../data/SettingsHBHE_091118tune_patchfixHBM14rm4.csv

# The following columns must be defined at minimum, and have the following
# format and appear in the following order (not necessarily consecutively)
#
# RBX name => %s
# rm number (1-5, 5=CM for HBHEHO,4=CM for HF) => %d
# card number (1-3, HBHEO or 1-4 for HF) => %d
# qie/adc number (0-5) => %d
#
#export OLDFMT='%*s %*d %*d %*d %s %d %d %d %d'
#export NEWFMT='%*s %*d %*d %*d %s %d %d %d %d'
export OLDFMT='%s %d %d %d %d'
export NEWFMT='%s %d %d %d %d'

#export NEW=../data/hbhetable_091118tune.csv
#export NEW=../data/SettingsHBHE_091118tune_patchfix15chan.csv
#export NEW=../data/dirHFscanVEoptSettings-5/vetable_offset=-5.csv
#export NEW=../data/dirHFscanVEoptSettingsTrial1jitter/vetable_offset=0.csv
#export NEW=../data/dirHFscanOptSettings-10ns/h2d_optDelaysHFall_offset=-10.csv
#export NEW=depthfixsettings.csv
#export NEW=removebadflagcutfix.csv
#export NEW=../data/HBHE_delaysTunedFromCollision2010.csv
export NEW=../data/bricks_Sep4th_collision2010.csv

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
