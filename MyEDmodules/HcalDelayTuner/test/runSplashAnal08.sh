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

echo "  1 - Run 62063 - Sep.10, 2008 (from +Z)"
echo "  2 - Run 62068 - Sep.10, 2008 (from -Z)"
echo "  3 - Run 63198 - Sep.18, 2008 (from +Z)"
       echo '   '

read VAR1

case $VAR1 in
  "1") export RUN=62063
       export FILES="'rfio:/castor/cern.ch/user/d/dudero/Splash2008skims/run62063-Sept10fromPlusZ-rhskim.root'"
       export RUNDESCR="'Run 62063 Splash from +Z'"
       export SPLASHZSIDEPLUS=True
       export GLOBALTOFFSET=32.53
       ;;
  "2") export RUN=62068
       export FILES="'rfio:/castor/cern.ch/user/d/dudero/Splash2008skims/run62068-Sept10fromMinusZ-rhskim.root'"
       export RUNDESCR="'Run 62068 Splash from -Z'"
       export SPLASHZSIDEPLUS=False
       export GLOBALTOFFSET=0.0
       ;;
  "3") export RUN=63198;
       export FILES="'rfio:/castor/cern.ch/user/d/dudero/Splash2008skims/run63198-Sept18fromPlusZ-rhskim.root'"
       export RUNDESCR="'Run 63198 Splash from +Z'"
       export SPLASHZSIDEPLUS=True
       export GLOBALTOFFSET=15.86
       ;;
esac

echo "Processing $RUN..."

FIRSTSAMPLE=2 #Simple reconstructor
SAMPLESTOADD=5 #Simple reconstructor

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
process.hetimeanal = timeanal.clone(subdet=cms.untracked.string("HE"));
process.hotimeanal = timeanal.clone(subdet=cms.untracked.string("HO"));
process.hotimeanal.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("")
process.hotimeanal.eventDataPset.hoRechitLabel = cms.untracked.InputTag("horeco")

process.hbtimeanal.globalTimeOffset  = cms.double(${GLOBALTOFFSET})
process.hetimeanal.globalTimeOffset  = cms.double(${GLOBALTOFFSET})
process.hotimeanal.globalTimeOffset  = cms.double(${GLOBALTOFFSET})

#LogicalMapFilename = cms.untracked.string("HCALmapHBEF_Jun.19.2008.txt")
   
process.p = cms.Path(process.hbtimeanal*
                     process.hetimeanal*
                     process.hotimeanal)
 
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
