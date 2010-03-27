#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    echo Please set up your runtime environment!
    exit
else
    echo "LOCALRT=${LOCALRT}"
fi

#if [[ $# > 0 ]]
#then
#    echo Processing $1
#    source $1
#elif...
if [[ -e ./skim_setup.rc ]] 
then
    source ./skim_setup.rc
fi

#command line arg overrides RC file
if (( $# > 0 ))
then
    RUN=$1
fi

echo "Processing $RUN..."

#define files
CFGFILE=run${RUN}_myskim_cfg.py
LOGFILE=run${RUN}_myskim.log
INCLUDEFILE=run${RUN}files_cfi

if (( ${#SKIMOUTPUTFMT} ))
then
    SKIMOUTPUT=`printf "${SKIMOUTPUTFMT}" ${RUN}`
fi

if (( ${#MAXOUTPUTEVENTS} ==0  ))
then
    MAXOUTPUTEVENTS="-1"
fi

#==================================================

cat > ${CFGFILE} << EOF1
import FWCore.ParameterSet.Config as cms
process = cms.Process("MYSKIM")
process.maxEvents = cms.untracked.PSet (
    output = cms.untracked.int32(${MAXOUTPUTEVENTS})
)
process.load("MyEDmodules.HcalDelayTuner.${INCLUDEFILE}")
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);
process.MessageLogger.cerr.threshold = cms.untracked.string("INFO")
process.MessageLogger.cerr.FwkReport.reportEvery = 100
EOF1

#==================================================

if (( ${#L1TTBITS} > 0 ))
then
    cat >>${CFGFILE} <<EOF2
process.load("L1TriggerConfig.L1GtConfigProducers.L1GtTriggerMaskTechTrigConfig_cff")
process.load("HLTrigger.HLTfilters.hltLevel1GTSeed_cfi")
process.hltLevel1GTSeed.L1TechTriggerSeeding = cms.bool(True)
process.hltLevel1GTSeed.L1SeedsLogicalExpression = cms.string("${L1TTBITS}")
EOF2
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
    cat >>${CFGFILE} <<EOF3
process.load("HLTrigger.HLTfilters.hltHighLevel_cfi")
process.hltHighLevel.HLTPaths = cms.vstring(${HLTPATHS})
#process.MessageLogger.debugModules = cms.untracked.vstring("hltHighLevel")
EOF3
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
    cat >>${CFGFILE} <<EOF4
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
EOF4
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.riFilt"
    else
	MYPATH="process.riFilt"
    fi
fi

#==================================================

if [ ${DODIGIS} ]
then
    cat >>${CFGFILE} <<EOF5
process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")
process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = 'FIRSTCOLL::All'
EOF5
    if (( ${#MYPATH} > 0 ))
    then
	MYPATH="${MYPATH}*process.hcalDigis"
    else
	MYPATH="process.hcalDigis"
    fi
fi
if (( ${#MYPATH} == 0 ))
then
    echo "No Skim definition provided, aborting"
    exit
fi

cat >>${CFGFILE} <<EOF6
process.p = cms.Path(${MYPATH})

# Output module configuration
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('${SKIMOUTPUT}'),
    # save only events passing the full path
    SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
    outputCommands = cms.untracked.vstring(${KEEPDROP})
)

process.e = cms.EndPath(process.out)

EOF6

# Right now the arguments to lsfbare.perl have changed...
#
#~/private/bin/lsfcafskim.perl ${PWD} ${LOCALRT} ${CFGFILE} ${LOGFILE} ${SKIMOUTPUT} ${CASTORSKIMLOC}
~/private/bin/lsfbareskim.perl ${PWD} ${LOCALRT} ${CFGFILE} ${LOGFILE} ${SKIMOUTPUT} ${CASTORSKIMLOC}
