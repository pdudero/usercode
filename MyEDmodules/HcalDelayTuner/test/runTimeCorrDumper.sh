#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
fi

# 'GR10_P_V6::All'
#GLOBAL_TAG=MC_38Y_V12::All
#GLOBAL_TAG=GR_R_38X_V14::All
#GLOBAL_TAG=GR10_P_V12::All
GLOBAL_TAG=GR_R_39X_V2::All

RUNNUMBER=149011

CFGFILE=${0%.sh}_cfg.py
cat > ${CFGFILE} << EOF

import FWCore.ParameterSet.Config as cms

process = cms.Process("TCDUMP")

#----------------------------
# Event Source
#-----------------------------

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))
process.source    = cms.Source("EmptySource",
                                firstRun = cms.untracked.uint32(${RUNNUMBER})
)

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = '${GLOBAL_TAG}'
#process.load("CalibCalorimetry.HcalPlugins.Hcal_Conditions_forGlobalTag_cff")

#-----------------------------
# Hcal Digis and RecHits
#-----------------------------

# process.TFileService = cms.Service("TFileService", 
#     fileName = cms.string("DelaySettings.root"),
#     closeFileFast = cms.untracked.bool(False)
# )

process.tcdump = cms.EDAnalyzer("TimeCorrDumper")

process.p = cms.Path(process.tcdump)

EOF

cmsRun ${CFGFILE} 2>&1

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

rm ${CFGFILE}
