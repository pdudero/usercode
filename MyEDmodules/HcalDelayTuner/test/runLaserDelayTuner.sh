#!/bin/bash

if [ $# -eq 0 ]
then
  echo "---------------------------------------"
  echo "  Usage: $0 run1 run2 run3 ... "
  echo "  Usage: $0 run1 events = -1"
  echo "---------------------------------------"
  exit
fi

ENTHRESH=25 #Energy threshold
#ENTHRESH=5 #Energy threshold

TDCCENT=1142
TDCWINDOW=2 #TDC Mean +/- window

CORRECTEDTIMEMODCEILING=9999 #Phase skip? Subtract off 25ns if greater than
TIMEMODCEILING=9999 #Phase skip? Subtract off 25ns if greater than

FIRSTSAMPLE=2 #Simple reconstructor
SAMPLESTOADD=5 #Simple reconstructor

GLOBALOFFSETNS=-65
#GLOBALOFFSETNS=0

EVENTS=-1
#EVENTS=10

#if [ $# -eq 2 ]
#then
#  EVENTS=$2
#fi


#if [ $# -gt 2 ]
if [ $# -gt 1 ]
then
# RUNS="\""
 for runnum in "$@"
 do
   RUN=`printf \"file:/bigspool/usc/USC_%06d.root\", $runnum`
   RUNS=${RUNS}${RUN}
 done
# RUNS=${RUNS}"\""
else
RUNS=`printf \"file:/bigspool/usc/USC_%06d.root\" $1`
fi
echo ${RUNS}

CFGFILE=runLaserDelayTuner_$1_cfg.py
cat > ${CFGFILE} << EOF

import FWCore.ParameterSet.Config as cms

process = cms.Process("LTA")

#----------------------------
# Event Source
#-----------------------------
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( ${EVENTS} )
)

process.source = cms.Source( "HcalTBSource",
   fileNames = cms.untracked.vstring( ${RUNS} ),
   streams = cms.untracked.vstring( #HBHEa,b,c:
#'HCAL_DCC700','HCAL_DCC701','HCAL_DCC702','HCAL_DCC703','HCAL_DCC704','HCAL_DCC705',
#'HCAL_DCC706','HCAL_DCC707','HCAL_DCC708','HCAL_DCC709','HCAL_DCC710','HCAL_DCC711',
#'HCAL_DCC712','HCAL_DCC713','HCAL_DCC714','HCAL_DCC715','HCAL_DCC716','HCAL_DCC717',
                                    #HF:
'HCAL_DCC718','HCAL_DCC719','HCAL_DCC720','HCAL_DCC721','HCAL_DCC722','HCAL_DCC723',
                                    #HO:
#'HCAL_DCC724','HCAL_DCC725','HCAL_DCC726','HCAL_DCC727','HCAL_DCC728','HCAL_DCC729',
#'HCAL_DCC730','HCAL_DCC731',
'HCAL_Trigger','HCAL_SlowData'
   )
)

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#process.load("CalibCalorimetry.HcalPlugins.Hcal_Conditions_forGlobalTag_cff")
#
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cfi")
process.hcal_db_producer = cms.ESProducer("HcalDbProducer",
     dump = cms.untracked.vstring(''),
     file = cms.untracked.string('')
)
 
process.es_hardcode = cms.ESSource("HcalHardcodeCalibrations",
     toGet = cms.untracked.vstring('GainWidths', 
         'channelQuality', 
         'ZSThresholds')
)

process.GlobalTag.globaltag = 'GR10_P_V0::All'  ### Update GlobalTag as necessary; https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideFrontierConditions
process.prefer("GlobalTag")

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1)

#-----------------------------
# Hcal Digis and RecHits
#-----------------------------

process.tbunpack = cms.EDFilter("HcalTBObjectUnpacker",
    IncludeUnmatchedHits = cms.untracked.bool(False),
    HcalTriggerFED       = cms.untracked.int32(1)
)

process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")
#process.load("RecoLocalCalo.HcalRecProducers.HcalSimpleReconstructor_hbhe_cfi")
#process.hbhereco.firstSample = ${FIRSTSAMPLE}
#process.hbhereco.samplesToAdd = ${SAMPLESTOADD}
#process.load("RecoLocalCalo.HcalRecProducers.HcalSimpleReconstructor_ho_cfi")
#process.horeco.firstSample = ${FIRSTSAMPLE}
#process.horeco.samplesToAdd = ${SAMPLESTOADD}
process.load("RecoLocalCalo.HcalRecProducers.HcalSimpleReconstructor_hf_cfi")
process.hfreco.firstSample = ${FIRSTSAMPLE}
process.hfreco.samplesToAdd = ${SAMPLESTOADD}

process.hcalLaserReco = cms.EDProducer( "HcalLaserReco" )

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("laserTimingAnal_run$1_TDC${TDCWINDOW}_ENERGY${ENTHRESH}_GLOFFSET${GLOBALOFFSETNS}.root"),
    closeFileFast = cms.untracked.bool(False)
)

process.load("MyEDmodules.HcalDelayTuner.laserdelaytuner_cfi")
process.hfdelayser.minHitGeV = cms.double(${ENTHRESH})
process.hfdelayser.TDCpars.TDCCutCenter = cms.double(${TDCCENT})
process.hfdelayser.TDCpars.TDCCutWindow = cms.double(${TDCWINDOW})
process.hfdelayser.TDCpars.CorrectedTimeModCeiling = cms.int32(${CORRECTEDTIMEMODCEILING})
process.hfdelayser.TDCpars.TimeModCeiling = cms.int32(${TIMEMODCEILING})
#process.hfdelayser.eventDataPset.verbose = cms.untracked.bool(True)
process.hfdelayser.globalTimeOffset = cms.double(${GLOBALOFFSETNS})
process.hfdelayser.TrecoParams.firstSample = cms.int32(2)

#LogicalMapFilename = cms.untracked.string("HCALmapHBEF_Jun.19.2008.txt")
   
process.p = cms.Path(process.hcalDigis * 
                     process.tbunpack  *
                     process.hcalLaserReco *
                     process.hfreco *
                     process.hfdelayser )
 
# Output module configuration
#process.out = cms.OutputModule("PoolOutputModule",
#    fileName = cms.untracked.string('laserTimingAnal_run$1_TDC${TDCWINDOW}_ENERGY${ENTHRESH}-pool.root'),
#    # save only events passing the full path
#    #SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
#    #outputCommands = cms.untracked.vstring('drop *')
#)

  
#process.e = cms.EndPath(process.out)
EOF

cmsRun ${CFGFILE} 2>&1 | tee ./logs/log_run$1_TDC${TDCWINDOW}_ENERGY${ENTHRESH}.txt

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

#rm ${CFGFILE}
