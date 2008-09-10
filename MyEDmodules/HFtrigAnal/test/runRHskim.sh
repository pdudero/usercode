#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    if (( ${#HCALDAQ_SW_LOC} > 3  && ${#HCAL_CMSSW_RELEASE} > 3 ))
    then
	pushd $HCALDAQ_SW_LOC/src/$HCAL_CMSSW_RELEASE/src >/dev/null
	eval `scramv1 runtime -sh`
	popd >/dev/null
    fi
fi

if (( ${#LOCALRT} < 4 ))
then
    echo Please setup your runtime environment!
    exit
fi

ARG1=$1
OUTPUTFILE=run${1}-rhskim.root
#INCLUDEFILE=`printf "MyEDmodules.HFtrigAnal.run%dfiles_cfi" ${ARG1}`
INCLUDEFILE=`printf "MyEDmodules.HFtrigAnal.run%dfirst255files_cfi" ${ARG1}`


if [[ -e ./skim_setup.rc ]] 
then
    source ./skim_setup.rc
fi

if (( ${#EVENTLIMIT} == 0 )) 
then
    EVENTLIMIT="-1";
fi

#### common head part of Config File
### create the file
CFGFILE=/tmp/runCMSSWskim_${USER}_${ARG1}.py
cat > ${CFGFILE}<<EOF
import FWCore.ParameterSet.Config as cms

process = cms.Process("RHSKIM")
process.load("FWCore.MessageLogger.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(${EVENTLIMIT})
)

process.MessageLogger.cerr.INFO.limit = 500
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.load("${INCLUDEFILE}")

process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")

process.load("RecoLocalCalo.HcalRecProducers.HcalSimpleReconstructor_hf_cfi")

process.rhFilt = cms.EDFilter("HcalRecHitFilter",
  hbheRechitLabel         = cms.untracked.InputTag("hbhereco"),
  hfRechitLabel           = cms.untracked.InputTag("hfreco"),
  hbRechitThresholdGeV    = cms.double(10000000.0),
  heRechitThresholdGeV    = cms.double(10000000.0),
  hfRechitThresholdGeV    = cms.double(4.0),
  totalRechitThresholdGeV = cms.double(20.0),
  totalRechitFilterActive = cms.bool(True)
)

process.out = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('p')
    ),
    outputCommands = cms.untracked.vstring('drop *', 
     'keep *_hcalDigis_*_*',
     'keep *_hfreco_*_*'),

    fileName = cms.untracked.string('file:${OUTPUTFILE}')
)

process.hcalConditions = cms.ESSource("PoolDBESSource",
    DBParameters = cms.PSet(
        messageLevel = cms.untracked.int32(0)
    ),
    timetype = cms.string('runnumber'),
    toGet = cms.VPSet(cms.PSet(
        record = cms.string('HcalPedestalsRcd'),
        tag = cms.string('hcal_pedestals_fC_v3_mc')
    ), 
        cms.PSet(
            record = cms.string('HcalPedestalWidthsRcd'),
            tag = cms.string('hcal_widths_fC_v3')
        ), 
        cms.PSet(
            record = cms.string('HcalElectronicsMapRcd'),
            tag = cms.string('official_emap_v5_080208')
        ), 
        cms.PSet(
            record = cms.string('HcalGainsRcd'),
            tag = cms.string('hcal_gains_v2_cosmics_magoff')
        ), 
        cms.PSet(
            record = cms.string('HcalQIEDataRcd'),
            tag = cms.string('qie_normalmode_v3')
        ), 
        cms.PSet(
            record = cms.string('HcalRespCorrsRcd'),
            tag = cms.string('hcal_respcorr_trivial_mc')
        )),
    connect = cms.string('frontier://Frontier/CMS_COND_20X_HCAL'),
    siteLocalConfig = cms.untracked.bool(False)
)

process.HcalDbProducer = cms.ESProducer("HcalDbProducer")

process.es_hardcode = cms.ESSource("HcalHardcodeCalibrations",
    toGet = cms.untracked.vstring('GainWidths', 
        'channelQuality', 
        'ZSThresholds')
)

EOF

if [[ "$MODE" == "GRECO" ]] 
then
cat >> ${CFGFILE}<<EOF999
process.p = cms.Path(process.trigFilt)
EOF999
elif [[ "$MODE" == "GRAW" ]] 
then
cat >> ${CFGFILE}<<EOF999
process.p = cms.Path(process.hcalDigis*process.hfreco*process.rhFilt)
process.o = cms.EndPath(process.out)
EOF999
fi

# run cmsRun
cmsRun ${CFGFILE}
