#!/bin/bash

if [[ -e ./anal_setup.rc ]] 
then
    source ./anal_setup.rc
fi

#command line arg overrides RC file
if (( $# > 0 ))
then
    RUN=$1
elif (( ${#RUN} == 0 ))
then
    echo "RUN is not defined. Define run in anal_setup.rc or provide run number on the command line."
    exit
fi

#define files
if (( ${#CFGFILE} == 0 )) 
then
    CFGFILE=./bta_run${RUN}_cfg.py
fi

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

echo "Processing $RUN, making $CFGFILE..."

# Piece together the config file
#==================================================

cat > ${CFGFILE} << EOF10

import FWCore.ParameterSet.Config as cms

process = cms.Process("${PROCESSNAME}")
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( ${EVENTLIMIT} )
# test it!
#   input = cms.untracked.int32( 100 ) 
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
EOF10

#==================================================

if (( ${#INCLUDEFILE} > 0 ))
then
    INCFILE="${INCLUDEFILE%.py}"
else
    INCFILE=`printf "run%sfiles_cfi" $RUN`
fi
cat >>${CFGFILE} <<EOF20
process.load("MyEDmodules.HcalDelayTuner.${INCFILE}")
EOF20

#==================================================

if (( ${#EVRANGES} > 0 ))
then
cat >>${CFGFILE} <<EOF30
process.source.eventsToProcess = cms.untracked.VEventRange(${EVRANGES})
EOF30
fi

#==================================================

if (( ${#LSRANGES} > 0 ))
then
cat >>${CFGFILE} <<EOF30
process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange(${LSRANGES})
EOF30
fi

#==================================================

if (( ${DOUNPACKALL} ))
then
    cat >>${CFGFILE} <<EOF40
process.load('Configuration/StandardSequences/RawToDigi_Data_cff')
EOF40
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
    cat >>${CFGFILE} <<EOF50
process.load("L1TriggerConfig.L1GtConfigProducers.L1GtTriggerMaskTechTrigConfig_cff")
process.load("HLTrigger.HLTfilters.hltLevel1GTSeed_cfi")
process.hltLevel1GTSeed.L1TechTriggerSeeding = cms.bool(True)
process.hltLevel1GTSeed.L1SeedsLogicalExpression = cms.string("${L1TTBITS}")
EOF50
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
    cat >>${CFGFILE} <<EOF60
process.load("HLTrigger.HLTfilters.hltHighLevel_cfi")
process.hltHighLevel.HLTPaths = cms.vstring(${HLTPATHS})
#process.MessageLogger.debugModules = cms.untracked.vstring("hltHighLevel")
EOF60
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
    cat >>${CFGFILE} <<EOF70
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
EOF70
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
    cat >>${CFGFILE} <<EOF80
process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")
EOF80
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.hcalDigis"
    else
	MYPATH="process.hcalDigis"
    fi
fi

#==================================================

cat >>${CFGFILE}<<EOF90
process.load("MyEDmodules.HcalDelayTuner.beamtiminganal_cfi")
EOF90

if (( ${DO_HBHE} ))
then
    cat >>${CFGFILE} <<EOF100
process.hbtimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.hbtimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.hbtimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.hbtimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.hbtimeanal.acceptedPkTSnumbers  = cms.vint32(${HBHETSNUMS})
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
process.hetimeanal.acceptedPkTSnumbers  = cms.vint32(${HBHETSNUMS})
process.hetimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.hetimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.hetimeanal.minHitGeV            = cms.double(${HBHE_MINHITGEV})
process.hetimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)
EOF100
    if (( ${DOHCALRECO} ))
    then
	cat >>${CFGFILE}<<EOF101
process.load("RecoLocalCalo.HcalRecProducers.HcalHitReconstructor_hbhe_cfi")
process.hbhetimeanal.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbhereco","","${PROCESSNAME}")
EOF101
    fi
    if (( ${#HBHE_FIRSTSAMPLE} >0 ))
    then
	cat >>${CFGFILE} <<EOF102
process.hbhereco.firstSample=cms.int32(${HBHE_FIRSTSAMPLE})
EOF102
    fi
    if (( ${#HBHE_SAMPLESTOADD} >0 ))
    then
	cat >>${CFGFILE} <<EOF103
process.hbhereco.samplesToAdd=cms.int32(${HBHE_SAMPLESTOADD})
EOF103
    fi
    ANALS="process.hbtimeanal+process.hetimeanal"
    MYRECO="process.hbhereco"
fi

if (( ${DO_HF} ))
then
    cat >>${CFGFILE} <<EOF110
process.hftimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.hftimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.hftimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.hftimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.hftimeanal.acceptedPkTSnumbers  = cms.vint32(${HFTSNUMS})
process.hftimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.hftimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.hftimeanal.minHitGeV            = cms.double(${HF_MINHITGEV})
process.hftimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)
process.hftimeanal.splitByEventRange    = cms.untracked.bool(True)
EOF110
    if (( ${#ANALS} > 0 ))
    then
	ANALS="${ANALS}+process.hftimeanal"
	if (( ${DOHCALRECO} ))
	then
	    MYRECO="${MYRECO}+process.hfreco"
	fi
    else
	ANALS="process.hftimeanal"
	if (( ${DOHCALRECO} ))
	then
	    MYRECO="process.hfreco"
	fi
    fi
    if (( ${DOHCALRECO} ))
    then
	cat >>${CFGFILE}<<EOF111
process.hftimeanal.eventDataPset.hfRechitLabel = cms.untracked.InputTag("hfreco","","${PROCESSNAME}")
process.load("RecoLocalCalo.HcalRecProducers.HcalHitReconstructor_hf_cfi")
EOF111
	if (( ${#HF_FIRSTSAMPLE} >0 ))
	then
	    cat >>${CFGFILE} <<EOF112
process.hfreco.firstSample=cms.int32(${HF_FIRSTSAMPLE})
EOF112
	fi
	if (( ${#HF_SAMPLESTOADD} >0 ))
	then
	cat >>${CFGFILE} <<EOF113
process.hfreco.samplesToAdd=cms.int32(${HF_SAMPLESTOADD})
EOF113
	fi
    fi
    if (( ${DOHFREFLAG} ))
    then
	cat >>${CFGFILE} <<EOF114
from RecoLocalCalo.HcalRecAlgos.hcalrechitreflagger_cfi import *
process.hfrecoReflagged = hcalrechitReflagger.clone()
process.hfrecoReflagged.hf_Algo3test = False  # S9/S1 algorithm (current default)
process.hfrecoReflagged.hf_Algo2test = True # PET algorithm
# Turns thresholding of PMT hits way down, makes PMT reflagger more aggressive about ID-ing PMT hits
#process.hfrecoReflagged.hf_PET_params.PET_EnergyThreshLong=cms.untracked.vdouble([5.])
#process.hfrecoReflagged.hf_PET_params.PET_EnergyThreshShort=cms.untracked.vdouble([5.])
process.hftimeanal.eventDataPset.hfRechitLabel = cms.untracked.InputTag("hfrecoReflagged")
EOF114
	if (( ${DOHCALRECO} ))
	then
	    cat >>${CFGFILE} <<EOF115
process.hfrecoReflagged.hfInputLabel = cms.untracked.InputTag("hfreco","","${PROCESSNAME}")
EOF115
	fi
    fi
fi

if (( ${DO_HO} ))
then
    cat >>${CFGFILE} <<EOF120
process.hotimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.hotimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.hotimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.hotimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.hotimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.hotimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.hotimeanal.minHitGeV            = cms.double(${HO_MINHITGEV})
process.hotimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)
EOF120
    if (( ${#ANALS} > 0 ))
    then
	ANALS="${ANALS}+process.hotimeanal"
	if (( ${DOHCALRECO} ))
	then
	    MYRECO="${MYRECO}+process.horeco"
	fi
    else
	ANALS="process.hotimeanal"
	if (( ${DOHCALRECO} ))
	then
	    MYRECO="process.horeco"
	fi
    fi
    if (( ${DOHCALRECO} ))
    then
	cat >>${CFGFILE}<<EOF121
process.hotimeanal.eventDataPset.hoRechitLabel = cms.untracked.InputTag("horeco","","${PROCESSNAME}")
process.load("RecoLocalCalo.HcalRecProducers.HcalHitReconstructor_ho_cfi")
EOF121
    fi
fi

if (( ${DO_ZDC} ))
then
    cat >>${CFGFILE} <<EOF130
process.zdctimeanal.runDescription       = cms.untracked.string("${RUNDESCR}")
process.zdctimeanal.globalRecHitFlagMask = cms.int32(${GLOBAL_FLAG_MASK})
process.zdctimeanal.badEventList         = cms.vint32(${BAD_EVENT_LIST})
process.zdctimeanal.acceptedBxNums       = cms.vint32(${BXNUMS})
process.zdctimeanal.acceptedPkTSnumbers  = cms.vint32(${HFTSNUMS})
process.zdctimeanal.maxEventNum2plot     = cms.int32(${MAXEVENTNUM})
process.zdctimeanal.globalTimeOffset     = cms.double(${GLOBALTOFFSET})
process.zdctimeanal.minHitGeV            = cms.double(${ZDC_MINHITGEV})
process.zdctimeanal.recHitEscaleMaxGeV   = cms.double(${MAXGEV2PLOT}.5)
EOF130
    if (( ${#ANALS} > 0 ))
    then
	ANALS="${ANALS}+process.zdctimeanal"
	if (( ${DOHCALRECO} ))
	then
	    MYRECO="${MYRECO}+process.zdcreco"
	fi
    else
	ANALS="process.zdctimeanal"
	if (( ${DOHCALRECO} ))
	then
	    MYRECO="process.zdcreco"
	fi
    fi
    if (( ${DOHCALRECO} ))
    then
	cat >>${CFGFILE}<<EOF131
process.zdctimeanal.eventDataPset.zdcRechitLabel = cms.untracked.InputTag("zdcreco","","${PROCESSNAME}")
process.load("RecoLocalCalo.HcalRecProducers.HcalHitReconstructor_zdc_cfi")
EOF131
    fi
fi

#==================================================

if (( ${DOFULLRECO} ))
then
    cat >>${CFGFILE} <<EOF140
process.load('Configuration/StandardSequences/Services_cff')
process.load('Configuration/StandardSequences/GeometryExtended_cff')
process.load('Configuration/StandardSequences/MagneticField_38T_cff')
process.load('Configuration/StandardSequences/Reconstruction_cff')
process.load('Configuration/StandardSequences/EndOfProcess_cff')
process.load('Configuration/EventContent/EventContent_cff')
EOF140
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.reconstruction"
    else
	MYPATH="process.reconstruction"
    fi
elif (( ${DOHCALRECO} ))
then
    cat >>${CFGFILE} <<EOF141
process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")
process.load("CalibCalorimetry.HcalPlugins.Hcal_Conditions_forGlobalTag_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cfi")
process.GlobalTag.globaltag = '${GLOBALTAG}'
process.myreco=cms.Sequence(${MYRECO})
EOF141
    if (( ${#MYPATH} > 0 ))
    then

	MYPATH="${MYPATH}*process.myreco"
    else
	MYPATH="process.myreco"
    fi
fi

#==================================================

if (( ${DOHFREFLAG} ))
then
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.hfrecoReflagged"
    else
	MYPATH="process.hfrecoReflagged"
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
                                debugOn = cms.untracked.bool(False),
                                numtrack = cms.untracked.uint32(10),
                                thresh = cms.untracked.double(0.25)
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

if (( ${#DOUNPACKALL} || ${#DOUNPACKHCAL} || ${#HASDIGIS} ))
    then
cat >> ${CFGFILE}<<EOF143
# Need conditions to convert digi ADC to fC in the analyzer
process.hbtimeanal.eventDataPset.hbheDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hetimeanal.eventDataPset.hbheDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hftimeanal.eventDataPset.hfDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hotimeanal.eventDataPset.hoDigiLabel=cms.untracked.InputTag("hcalDigis")
process.zdctimeanal.eventDataPset.zdcDigiLabel=cms.untracked.InputTag("hcalDigis")
EOF143
    else
echo Assume no digis in input.
fi    

if (( ${#DOTREE}>0 ))
    then
    echo DOTREE=${DOTREE}
cat >> ${CFGFILE}<<EOF150
process.hbtimeanal.doTree=cms.untracked.bool(${DOTREE})
process.hetimeanal.doTree=cms.untracked.bool(${DOTREE})
process.hftimeanal.doTree=cms.untracked.bool(${DOTREE})
process.hotimeanal.doTree=cms.untracked.bool(${DOTREE})
process.zdctimeanal.doTree=cms.untracked.bool(${DOTREE})
EOF150
fi    

#==================================================

cat >> ${CFGFILE}<<EOF160
process.allAnals=cms.Sequence(${ANALS})
EOF160

if (( ${#MYPATH} > 0 ))
then
    MYPATH="${MYPATH}*process.allAnals"
else
    MYPATH="process.allAnals"
fi

cat >>${CFGFILE} <<EOF170
process.p = cms.Path(${MYPATH})
EOF170

if (( ${#SKIMOUTPUT} )) && (( ${#CASTOROUTPUTLOC} ))
then
    if (( ${#KEEPDROP} < 4 ))
    then
	KEEPDROP="keep *"
    fi
cat >>${CFGFILE} <<EOF180
# Output module configuration
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('${SKIMOUTPUT}'),
    # save only events passing the full path
    SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
    outputCommands = cms.untracked.vstring('${KEEPDROP}')
)
process.e = cms.EndPath(process.out)
EOF180
fi
