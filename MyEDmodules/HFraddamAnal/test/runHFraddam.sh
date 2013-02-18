#!/bin/bash

if [ $# -eq 0 ]
then
  echo "---------------------------------------"
  echo "  Usage: ./runHFraddam.sh run1 run2 run3 ... "
  echo "  Usage: ./runHFraddam.sh run1 events = -1"
  echo "---------------------------------------"
  exit
fi

TDCCENT=1075
TDCWINDOW=25 #TDC Mean +/- window

CORRECTEDTIMEMODCEILING=9999 #Phase skip? Subtract off 25ns if greater than
TIMEMODCEILING=9999 #Phase skip? Subtract off 25ns if greater than

FIRSTSAMPLE=2 #Simple reconstructor
SAMPLESTOADD=5 #Simple reconstructor

RUNS=`printf \"file:/bigspool/usc/USC_%06d.root\" $1`
EVENTS=-1
#EVENTS=10

if [ $# -eq 2 ]
then
  EVENTS=$2
fi

if [ $# -gt 2 ]
then
 RUNS="\""
# RUNS="\""
 for runnum in "$@"
 do
   RUN=`printf \"file:/bigspool/usc/USC_%06d.root\", $runnum`
   RUNS=${RUNS}${RUN}
 done
 RUNS=${RUNS}"\""
fi
echo ${RUNS}

CFGFILE=runHFraddam_$1_cfg.py
cat > ${CFGFILE} << EOF

import FWCore.ParameterSet.Config as cms

process = cms.Process("HFRADDAM")

#----------------------------
# Event Source
#-----------------------------
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( ${EVENTS} )
)

process.source = cms.Source( "HcalTBSource",
   fileNames = cms.untracked.vstring( ${RUNS} ),
   streams = cms.untracked.vstring( #HF:
'HCAL_DCC718','HCAL_DCC719','HCAL_DCC720','HCAL_DCC721','HCAL_DCC722','HCAL_DCC723',
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

process.GlobalTag.globaltag = 'GR_P_V42_AN3::All'  ### Update GlobalTag as necessary; https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideFrontierConditions
process.prefer("GlobalTag")

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(10);
process.MessageLogger.cerr.FwkReport.reportEvery = 100

#-----------------------------
# Hcal Digis and RecHits
#-----------------------------

process.tbunpack = cms.EDProducer("HcalTBObjectUnpacker",
    IncludeUnmatchedHits = cms.untracked.bool(False),
    HcalTriggerFED       = cms.untracked.int32(1)
)

process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")

process.hcalLaserReco = cms.EDProducer( "HcalLaserReco" )

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("hfraddam_run$1.root"),
    closeFileFast = cms.untracked.bool(False)
)

process.load("MyEDmodules.HFraddamAnal.hfraddamanal_cfi")
process.hfraddam.TDCpars.TDCCutCenter = cms.double(${TDCCENT})
process.hfraddam.TDCpars.TDCCutWindow = cms.double(${TDCWINDOW})
process.hfraddam.TDCpars.CorrectedTimeModCeiling = cms.int32(${CORRECTEDTIMEMODCEILING})
process.hfraddam.TDCpars.TimeModCeiling = cms.int32(${TIMEMODCEILING})
process.hfraddam.eventDataPset.fedRawDataLabel = cms.untracked.InputTag("")
#process.hfraddam.eventDataPset.verbose = cms.untracked.bool(True)
process.hfraddam.tdcwindowsfile=cms.untracked.string("perchanwin_run$1.txt")

#LogicalMapFilename = cms.untracked.string("HCALmapHBEF_Jun.19.2008.txt")
   
process.p = cms.Path(process.hcalDigis * 
                     process.tbunpack  *
                     process.hcalLaserReco *
                     process.hfraddam )
 
# Output module configuration
#process.out = cms.OutputModule("PoolOutputModule",
#    fileName = cms.untracked.string('hfraddam_run$1-pool.root'),
#    # save only events passing the full path
#    #SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
#    #outputCommands = cms.untracked.vstring('drop *')
#)

  
#process.e = cms.EndPath(process.out)
EOF

cmsRun ${CFGFILE} 2>&1 | tee ./logs/log_run$1.txt

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}! Deleting temp .cfg" 
echo " -----------------------------------------------------------------  " 
echo "                                                                    "

rm ${CFGFILE}
