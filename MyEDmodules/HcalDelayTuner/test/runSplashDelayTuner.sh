#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
fi

EVENTS=-1
#EVENTS=10
WRITEBRICKS=False

#if [ $# -eq 2 ]
#then
#  EVENTS=$2
#fi

CFGFILE=$0_$1_cfg.py
cat > ${CFGFILE} << EOF

import FWCore.ParameterSet.Config as cms

process = cms.Process("DELAYTUNER")

#----------------------------
# Event Source
#-----------------------------

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(0))
process.source    = cms.Source("EmptySource")

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#process.load("CalibCalorimetry.HcalPlugins.Hcal_Conditions_forGlobalTag_cff")
#

#process.GlobalTag.globaltag = 'CRAFT_ALL_V11::All'  ### Update GlobalTag as necessary; https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideFrontierConditions
#process.prefer("GlobalTag")

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);

#-----------------------------
# Hcal Digis and RecHits
#-----------------------------

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("splashDelayTuner_run$1.root"),
    closeFileFast = cms.untracked.bool(False)
)

process.load("MyEDmodules.HcalDelayTuner.splashdelaytuner_cfi")
process.tunedelays.writeBricks  = cms.untracked.bool(${WRITEBRICKS})
process.tunedelays.rootFilename = cms.untracked.string("splashTimingAnal_run120015.root")
#process.tunedelays.hbRMprofileHistos = cms.untracked.vstring("hbtimeanal/HB/cutBadEvents/p1d_avgTimePerRMd1HB",
#                                                             "hbtimeanal/HB/cutBadEvents/p1d_avgTimePerRMd2HB")
process.tunedelays.heRMprofileHisto = cms.untracked.string("hetimeanal/HE/cutBadEvents/p1d_avgTimePer2RMsHE")
#process.tunedelays.hoRMprofileHisto = cms.untracked.string("hotimeanal/HO/cutBadEvents/p1d_avgTimePerRMd1HO")
#process.tunedelays.oldSettingFilenames = cms.untracked.vstring("../data/hbtable.csv")
process.tunedelays.oldSettingFilenames = cms.untracked.vstring("../data/hetable.csv")

#LogicalMapFilename = cms.untracked.string("HCALmapHBEF_Jun.19.2008.txt")
   
process.p = cms.Path(process.tunedelays)
 
# Output module configuration
#process.out = cms.OutputModule("PoolOutputModule",
#    fileName = cms.untracked.string('laserDelayTuner_run$1_TDC${TDCWINDOW}_ENERGY${ENTHRESH}-pool.root'),
#    # save only events passing the full path
#    #SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
#    #outputCommands = cms.untracked.vstring('drop *')
#)

  
#process.e = cms.EndPath(process.out)
EOF

cmsRun ${CFGFILE} 2>&1 | tee ./logs/log.txt

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

#rm ${CFGFILE}
