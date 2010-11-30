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

#command line arg overrides RC file
if (( $# > 0 ))
then
    RUN=$1
fi

#define files
CFGFILE=./beamTimingAnal_run${RUN}_cfg.py
LOGFILE=./beamTimingAnal_run${RUN}.log
#ANALOUTPUT=$2
#ARG1=$1
if (( ${#ANALOUTPUTFMT} ))
then
    ANALOUTPUT=`printf "${ANALOUTPUTFMT}" ${RUN}`
fi
if (( ${#SKIMOUTPUTFMT} ))
then
    SKIMOUTPUT=`printf "${SKIMOUTPUTFMT}" ${RUN}`
fi

if (( ${#EVENTLIMIT} == 0 )) 
then
    EVENTLIMIT="-1";
fi

echo "Processing $RUN..."

# Piece together the config file
#==================================================

cat > ${CFGFILE} << EOF1

import FWCore.ParameterSet.Config as cms

process = cms.Process("${PROCESSNAME}")
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( ${EVENTLIMIT} )
)

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("${ANALOUTPUT}"),
    closeFileFast = cms.untracked.bool(False)
)
EOF1

#==================================================

if (( ${#INCLUDEFILE} > 0 ))
then
    INCFILE="${INCLUDEFILE%.py}"
else
    INCFILE=`printf "run%sfiles_cfi" $RUN`
fi
cat >>${CFGFILE} <<EOF2
process.load("MyEDmodules.HcalDelayTuner.${INCFILE}")
EOF2

#==================================================

if (( ${#EVRANGES} > 0 ))
then
cat >>${CFGFILE} <<EOF3
process.source.eventsToProcess = cms.untracked.VEventRange(${EVRANGES})
EOF3
fi

#==================================================

if (( ${#LSRANGES} > 0 ))
then
cat >>${CFGFILE} <<EOF3
process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange(${LSRANGES})
EOF3
fi

#==================================================

if (( ${DOUNPACKALL} ))
then
    cat >>${CFGFILE} <<EOF4
process.load('Configuration/StandardSequences/RawToDigi_Data_cff')
EOF4
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.RawToDigi"
    else
	MYPATH="process.RawToDigi"
    fi
fi

#==================================================

if (( ${#L1TTBITS} > 0 ))
then
    cat >>${CFGFILE} <<EOF5
process.load("L1TriggerConfig.L1GtConfigProducers.L1GtTriggerMaskTechTrigConfig_cff")
process.load("HLTrigger.HLTfilters.hltLevel1GTSeed_cfi")
process.hltLevel1GTSeed.L1TechTriggerSeeding = cms.bool(True)
process.hltLevel1GTSeed.L1SeedsLogicalExpression = cms.string("${L1TTBITS}")
EOF5
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.hltLevel1GTSeed"
    else
	MYPATH="process.hltLevel1GTSeed"
    fi
fi

#==================================================

if (( ${#HLTPATHS} > 0 ))
then
    cat >>${CFGFILE} <<EOF6
process.load("HLTrigger.HLTfilters.hltHighLevel_cfi")
process.hltHighLevel.HLTPaths = cms.vstring(${HLTPATHS})
#process.MessageLogger.debugModules = cms.untracked.vstring("hltHighLevel")
EOF6
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.hltHighLevel"
    else
	MYPATH="process.hltHighLevel"
    fi
fi

#==================================================

if (( ${#INCLUDELS} > 0  ||  ${#EXCLUDELS} > 0  ||  ${#INCLUDEBX} > 0 ))
then
    cat >>${CFGFILE} <<EOF7
process.riFilt = cms.EDFilter("RunInfoFilter",
    eventNumbers = cms.vint32(),
    bunchNumbers = cms.vint32(${INCLUDEBX}),
    includeLSnumbers=cms.vint32(${INCLUDELS}),
    excludeLSnumbers=cms.vint32(${EXCLUDELS}),
    runNumbers=cms.vint32(${RUN})
)
# For use with the RunInfoFilter:
process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("run${RUN}_riskim.root"),
    closeFileFast = cms.untracked.bool(False)
)
EOF7
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.riFilt"
    else
	MYPATH="process.riFilt"
    fi
fi

#==================================================

if (( ${DOUNPACKHCAL} ))
then
    cat >>${CFGFILE} <<EOF8
process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")
EOF8
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.hcalDigis"
    else
	MYPATH="process.hcalDigis"
    fi
fi

#==================================================

if (( ${DOFULLRECO} ))
then
    cat >>${CFGFILE} <<EOF9
process.load('Configuration/StandardSequences/Services_cff')
process.load('Configuration/StandardSequences/GeometryExtended_cff')
process.load('Configuration/StandardSequences/MagneticField_38T_cff')
process.load('Configuration/StandardSequences/Reconstruction_cff')
process.load('Configuration/StandardSequences/EndOfProcess_cff')
process.load('Configuration/EventContent/EventContent_cff')
EOF9
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.reconstruction"
    else
	MYPATH="process.reconstruction"
    fi
elif (( ${DOHCALRECO} ))
then
    cat >>${CFGFILE} <<EOF99
process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")
process.load("CalibCalorimetry.HcalPlugins.Hcal_Conditions_forGlobalTag_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = '${GLOBALTAG}'
process.load("RecoLocalCalo.HcalRecProducers.HcalHitReconstructor_hbhe_cfi")
process.load("RecoLocalCalo.HcalRecProducers.HcalHitReconstructor_ho_cfi")
process.load("RecoLocalCalo.HcalRecProducers.HcalHitReconstructor_hf_cfi")
process.load("RecoLocalCalo.HcalRecProducers.HcalHitReconstructor_zdc_cfi")
process.myreco=cms.Sequence(process.hbhereco+
                            process.horeco+
			    process.hfreco+
			    process.zdcreco)
EOF99
    if (( ${#HBHE_FIRSTSAMPLE} >0 ))
    then
	cat >>${CFGFILE} <<EOF999
process.hbhereco.firstSample=cms.int32(${HBHE_FIRSTSAMPLE})
EOF999
    fi
    if (( ${#HBHE_SAMPLESTOADD} >0 ))
    then
	cat >>${CFGFILE} <<EOF9999
process.hbhereco.samplesToAdd=cms.int32(${HBHE_SAMPLESTOADD})
EOF9999
    fi
    if (( ${#HF_FIRSTSAMPLE} >0 ))
    then
	cat >>${CFGFILE} <<EOF99999
process.hfreco.firstSample=cms.int32(${HF_FIRSTSAMPLE})
EOF99999
    fi
    if (( ${#HF_SAMPLESTOADD} >0 ))
    then
	cat >>${CFGFILE} <<EOF999999
process.hfreco.samplesToAdd=cms.int32(${HF_SAMPLESTOADD})
EOF999999
    fi
    if (( ${#MYPATH} > 0 ))
    then

	MYPATH="${MYPATH}*process.myreco"
    else
	MYPATH="process.myreco"
    fi
fi

#==================================================

if (( ${DOCLEANING} ))
then
    cat >>${CFGFILE} <<EOF999_999
process.oneGoodVertexFilter = cms.EDFilter("VertexSelector",
   src = cms.InputTag("offlinePrimaryVertices"),
#   cut = cms.string("!isFake && tracksSize > 3 && abs(z) <= 15 && position.Rho <= 2"),
   cut = cms.string("!isFake && ndof >= 5 && abs(z) <= 15"),
   filter = cms.bool(True)   # otherwise it won't filter the events, just produce an empty vertex collection.
)

process.noscraping = cms.EDFilter("FilterOutScraping",
                                applyfilter = cms.untracked.bool(True),
                                debugOn = cms.untracked.bool(True),
                                numtrack = cms.untracked.uint32(10),
                                thresh = cms.untracked.double(0.25)
                                )
)
EOF999_999
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.noscraping*process.oneGoodVertexFilter"
    else
	MYPATH="process.noscraping*process.oneGoodVertexFilter"
    fi
fi

#==================================================

cat >>${CFGFILE} <<EOF10
process.load("MyEDmodules.HcalDelayTuner.beamtiminganal_cfi")
process.hbtimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.hbtimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.hbtimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.hbtimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.hbtimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.hbtimeanal.minHitGeV            = cms.double(${HBHE_MINHITGEV})
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
process.hetimeanal.minHitGeV            = cms.double(${HBHE_MINHITGEV})
process.hetimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)

process.hftimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.hftimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.hftimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.hftimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.hftimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.hftimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.hftimeanal.minHitGeV            = cms.double(${HF_MINHITGEV})
process.hftimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)
process.hftimeanal.splitByEventRange    = cms.untracked.bool(True)

process.hotimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.hotimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.hotimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.hotimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.hotimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.hotimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.hotimeanal.minHitGeV            = cms.double(${HO_MINHITGEV})
process.hotimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)

process.zdctimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.zdctimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.zdctimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.zdctimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.zdctimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.zdctimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.zdctimeanal.minHitGeV            = cms.double(${ZDC_MINHITGEV})
process.zdctimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)
EOF10

if (( ${#DOUNPACKALL} || ${#DOUNPACKHCAL} || ${#HASDIGIS} ))
    then
cat >> ${CFGFILE}<<EOF11
# Need conditions to convert digi ADC to fC in the analyzer
process.hbtimeanal.eventDataPset.hbheDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hetimeanal.eventDataPset.hbheDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hftimeanal.eventDataPset.hfDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hotimeanal.eventDataPset.hoDigiLabel=cms.untracked.InputTag("hcalDigis")
process.zdctimeanal.eventDataPset.zdcDigiLabel=cms.untracked.InputTag("hcalDigis")
EOF11
    else
echo Assume no digis in input.
fi    

if (( ${#DOTREE} ))
    then
    echo DOTREE=${DOTREE}
cat >> ${CFGFILE}<<EOF12
process.hbtimeanal.doTree=cms.untracked.bool(${DOTREE})
process.hetimeanal.doTree=cms.untracked.bool(${DOTREE})
process.hftimeanal.doTree=cms.untracked.bool(${DOTREE})
process.hotimeanal.doTree=cms.untracked.bool(${DOTREE})
process.zdctimeanal.doTree=cms.untracked.bool(${DOTREE})
EOF12
fi    

#==================================================

cat >> ${CFGFILE}<<EOF13
process.allAnals=cms.Sequence(
                     process.hbtimeanal+
                     process.hetimeanal+
                     process.hftimeanal+
                     process.hotimeanal+
		     process.zdctimeanal)
EOF13

if (( ${#MYPATH} > 0 ))
then
    MYPATH="${MYPATH}*process.allAnals"
else
    MYPATH="process.allAnals"
fi

cat >>${CFGFILE} <<EOF14
process.p = cms.Path(${MYPATH})
EOF14

if (( ${#SKIMOUTPUT} )) && (( ${#CASTOROUTPUTLOC} ))
then
    if (( ${#KEEPDROP} < 4 ))
    then
	KEEPDROP="keep *"
    fi
cat >>${CFGFILE} <<EOF15
# Output module configuration
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('${SKIMOUTPUT}'),
    # save only events passing the full path
    SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
    outputCommands = cms.untracked.vstring('${KEEPDROP}')
)
process.e = cms.EndPath(process.out)
EOF15
fi

if [[ "${RUNMODE}" == "BATCH" ]]
    then
    if (( ${#BATCH_SEMA4} ))
    then
	~/private/bin/lsfbare.perl ${PWD} ${LOCALRT} ${CFGFILE} ${LOGFILE} ${ANALOUTPUT} ${BATCH_SEMA4} ${SKIMOUTPUT} ${CASTOROUTPUTLOC}
    else
	~/private/bin/lsfbare.perl ${PWD} ${LOCALRT} ${CFGFILE} ${LOGFILE} ${ANALOUTPUT} junk ${SKIMOUTPUT} ${CASTOROUTPUTLOC}
    fi
else
    cmsRun ${CFGFILE} 2>&1 >${LOGFILE}
fi
#rm ${CFGFILE}
