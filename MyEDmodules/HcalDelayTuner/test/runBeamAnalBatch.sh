#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
fi

if [[ -e ./anal_setup.rc ]] 
then
    source ./anal_setup.rc
fi

CFGFILE=./beamTimingAnal_run${RUN}_cfg.py
LOGFILE=./beamTimingAnal_run${RUN}.log
#ANALOUTPUT=$2
#ARG1=$1
#ANALOUTPUT=`printf "${ANALOUTPUTFMT}" ${ARG1} ${ARG1}`
ANALOUTPUT=beamTimingAnal_run${RUN}.root
INCFILE="${INCLUDEFILE%.py}"

if (( ${#EVENTLIMIT} == 0 )) 
then
    EVENTLIMIT="-1";
fi

echo "Processing $RUN..."

cat > ${CFGFILE} << EOF1

import FWCore.ParameterSet.Config as cms

process = cms.Process("BEAMTIMEANAL")
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( ${EVENTLIMIT} )
)

process.load("MyEDmodules.HcalDelayTuner.${INCFILE}")
#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#process.load("CalibCalorimetry.HcalPlugins.Hcal_Conditions_forGlobalTag_cff")
#
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("${ANALOUTPUT}"),
    closeFileFast = cms.untracked.bool(False)
)

#process.load("MyEDmodules.HcalDelayTuner.L1skimdef_Bits42or43_cfi")
process.load("MyEDmodules.HcalDelayTuner.beamtiminganal_cfi")
process.hbtimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.hbtimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.hbtimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.hbtimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.hbtimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.hbtimeanal.minHitGeV            = cms.double(${MINHITGEV})
process.hbtimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.hbtimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)
#process.hbtimeanal.unravelHBatIeta      = cms.int32(14)
#process.hbtimeanal.eventDataPset.verbose=cms.untracked.bool(True)
#
process.hetimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.hetimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.hetimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.hetimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.hetimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.hetimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.hetimeanal.minHitGeV            = cms.double(${MINHITGEV})
process.hetimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)

process.hftimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.hftimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.hftimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.hftimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
#process.hftimeanal.detIds2mask          = cms.vint32(-6,20,4,-6,10,4)
process.hftimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.hftimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.hftimeanal.minHitGeV            = cms.double(${MINHITGEV})
process.hftimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)
EOF1

### Mode-dependent part

if [[ "${MODE}" == "HASDIGIS" ]]
    then
cat >> ${CFGFILE}<<EOF2
# If you want Digis...
process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")
process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = 'FIRSTCOLL::All'
process.hbtimeanal.eventDataPset.hbheDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hetimeanal.eventDataPset.hbheDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hftimeanal.eventDataPset.hfDigiLabel=cms.untracked.InputTag("hcalDigis")
EOF2
    else
echo Assume no digis in input.
fi    

cat >> ${CFGFILE}<<EOF3
process.allAnals=cms.Sequence(
                     process.hbtimeanal+
                     process.hetimeanal+
                     process.hftimeanal)

process.p = cms.Path(process.allAnals)
EOF3

if [[ "${RUNMODE}" == "BATCH" ]]
    then
~/private/bin/lsfbare.perl ${PWD} ${CFGFILE} ${LOGFILE} ${ANALOUTPUT} ${CASTOROUTPUTLOC} ${LOCALRT}
    else
    cmsRun ${CFGFILE} 2>&1 >${LOGFILE}
fi
#rm ${CFGFILE}
