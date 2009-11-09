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

echo "  1 - Run 120015 - Nov.7, 2009 (from -Z)"
       echo '   '

read VAR1

case $VAR1 in
  "1") export RUN=120015
       export FILES="'file:~/scratch0/data/Splash09skims/run120015-rhskim.root'"
       export RUNDESCR="'Run 120015 Splash from -Z'"
       export GLOBALTOFFSET=-7.31
       export SPLASHZSIDEPLUS=False
       export BAD_EVENT_LIST=8,51,270
#      export TIMEWINDOWMIN=
#      export TIMEWINDOWMIN=
       ;;
esac

echo "Processing $RUN..."

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

process = cms.Process("SPLASHTIMEANAL")

#----------------------------
# Event Source
#-----------------------------
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( ${EVENTS} )
)

process.source = cms.Source( "PoolSource",
   fileNames = cms.untracked.vstring($FILES)
)

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#process.load("CalibCalorimetry.HcalPlugins.Hcal_Conditions_forGlobalTag_cff")
#
process.load("Geometry.CMSCommonData.cmsIdealGeometryXML_cfi")
process.load("Geometry.CaloEventSetup.CaloTopology_cfi")
process.load("Geometry.CaloEventSetup.CaloGeometry_cff")
#process.load("Configuration.StandardSequences.Geometry_cff")

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);

#-----------------------------
# Hcal Digis and RecHits
#-----------------------------

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("splashTimingAnal_run$RUN.root"),
    closeFileFast = cms.untracked.bool(False)
)

from MyEDmodules.HcalDelayTuner.splashtiminganal_cfi import *
process.hbtimeanal = timeanal.clone();
process.hbtimeanal.runDescription = cms.untracked.string(${RUNDESCR})
process.hbtimeanal.splashPlusZside = cms.untracked.bool(${SPLASHZSIDEPLUS})
process.hbtimeanal.badEventList    = cms.vint32(${BAD_EVENT_LIST})
process.hetimeanal = process.hbtimeanal.clone(subdet=cms.untracked.string("HE"));
process.hotimeanal = process.hbtimeanal.clone(subdet=cms.untracked.string("HO"));
process.hotimeanal.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("")
process.hotimeanal.eventDataPset.hoRechitLabel = cms.untracked.InputTag("horeco")
#process.hbtimeanal.timeWindowMinNS   = cms.double(${TIMEWINDOWMIN})
#process.hbtimeanal.timeWindowMaxNS   = cms.double(${TIMEWINDOWMAX})
#process.hetimeanal.timeWindowMinNS   = cms.double(${TIMEWINDOWMIN})
#process.hetimeanal.timeWindowMaxNS   = cms.double(${TIMEWINDOWMAX})
#process.hotimeanal.timeWindowMinNS   = cms.double(${TIMEWINDOWMIN})
#process.hotimeanal.timeWindowMaxNS   = cms.double(${TIMEWINDOWMAX})

process.hbtimeanal.globalTimeOffset  = cms.double(${GLOBALTOFFSET})
process.hetimeanal.globalTimeOffset  = cms.double(${GLOBALTOFFSET})
process.hotimeanal.globalTimeOffset  = cms.double(${GLOBALTOFFSET})
   
process.p = cms.Path(process.hbtimeanal*
                     process.hetimeanal)
#                     process.hotimeanal)
 
# Output module configuration
#process.out = cms.OutputModule("PoolOutputModule",
#    fileName = cms.untracked.string('laserDelayTuner_run$1_TDC${TDCWINDOW}_ENERGY${ENTHRESH}-pool.root'),
#    # save only events passing the full path
#    #SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
#    #outputCommands = cms.untracked.vstring('drop *')
#)

  
#process.e = cms.EndPath(process.out)
EOF

cmsRun ${CFGFILE} 2>&1 | tee ./logs/splashDelayTuner_run$RUN.log

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

rm ${CFGFILE}
