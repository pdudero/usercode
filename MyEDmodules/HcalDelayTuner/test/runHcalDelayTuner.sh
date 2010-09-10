#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
fi

EVENTS=-1
#EVENTS=10
WRITEBRICKS=True
#HBHE:
#TIMECORRFILENAMES="'../data/timeCorrs_run120042_HE.csv','../data/timeCorrs_run120042_HB.csv'"
#TIMECORRSCANFMT='%*s %*d %*d %*d %s %d %d %d %f'
#
# Putting multiple files in the vector cleverly merges/overlays
# settings in the latter to that in the former...
#
OLDSETTINGFILENAMES="'../data/SettingsHBHE_091118tune_patchfixHBM14rm4.csv','../data/HE_region_2ns_timing_aug25.csv'"
#
# ...that is, *if* you set this variable:
OVERWRITEDUPS=True
#
SUBDET=HBHE
#
#HO:
#TIMECORRFILENAMES="'../data/timeCorrs_run120042_HO_noSiPMs.csv'"
#TIMECORRSCANFMT='%*s %*d %*d %*d %s %d %d %d %f'
#OLDSETTINGFILENAMES="'../data/hotable_mapcorrs_noHOX.csv'"
#OLDSETTINGFILENAMES="'../data/hotable.csv'"
#SUBDET=HO
#CLIPATLIMITS=True
CLIPATLIMITS=False

#if [ $# -eq 2 ]
#then
#  EVENTS=$2
#fi

CFGFILE=$0_cfg.py
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

process.load("MyEDmodules.HcalDelayTuner.hcaldelaytuner_cfi")
process.tunedelays.writeBricks         = cms.untracked.bool(${WRITEBRICKS})
process.tunedelays.timecorrFilenames   = cms.untracked.vstring(${TIMECORRFILENAMES})
process.tunedelays.timecorrScanFmt     = cms.untracked.string("${TIMECORRSCANFMT}")
process.tunedelays.fileNames           = cms.untracked.vstring(${OLDSETTINGFILENAMES})
process.tunedelays.overwriteDuplicates = cms.untracked.bool(${OVERWRITEDUPS})

process.tunedelays.clipSettingsAtLimits= cms.untracked.bool(${CLIPATLIMITS})
process.tunedelays.subdet              = cms.untracked.string("${SUBDET}")
   
process.p = cms.Path(process.tunedelays)

EOF

cmsRun ${CFGFILE} 2>&1 | tee ./logs/splashDelayTuner.log

#rm ${CFGFILE}
