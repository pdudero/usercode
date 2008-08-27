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
#OUTPUTFILE=$2
OUTPUTFILE=run${1}-anal.root

if [[ -e ./anal_setup.rc ]] 
then
    source ./anal_setup.rc
fi

if (( ${#EVENTLIMIT} == 0 )) 
then
    EVENTLIMIT="-1";
fi

#### common head part of Config File
### create the file
CFGFILE=/tmp/runCMSSWReco_${USER}.py
cat > ${CFGFILE}<<EOF
import FWCore.ParameterSet.Config as cms

process = cms.Process("HFTRIGANAL")
process.load("FWCore.MessageLogger.MessageLogger_cfi")

process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")

process.load("RecoLocalCalo.HcalRecProducers.HcalSimpleReconstructor_hf_cfi")

process.TFileService = cms.Service("TFileService",
    closeFileFast = cms.untracked.bool(True),
    fileName = cms.string('${OUTPUTFILE}')
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(${EVENTLIMIT})
)

process.MessageLogger.cerr.INFO.limit = 500
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.hfreco.firstSample  = 1
process.hfreco.samplesToAdd = 2

EOF

### Mode-dependent part

if [[ "$MODE" == "LOCAL" ]]
then

# ARG1 determines the file selection mode
#
if [[ "${ARG1}" == *[[:alpha:]]* ]]
then
    # Filename mode
    FILE=$ARG1
else
    # Run Number mode
    FILE=`printf "${FORMAT}" ${ARG1}`
fi

echo $FILE

cat >> ${CFGFILE}<<EOF
process.source = cms.Source("HcalTBSource",
    streams = cms.untracked.vstring('HCAL_DCC718', 
        'HCAL_DCC719', 
        'HCAL_DCC720', 
        'HCAL_DCC721', 
        'HCAL_DCC722', 
        'HCAL_DCC723'),
    fileNames = cms.untracked.vstring('file:${FILE}')
)
EOF
elif [[ "$MODE" == "GLOBAL" ]] 
    then
cat >> ${CFGFILE}<<EOF
process.load("MyEDmodules.HFtrigAnal.test.globalFiles2Anal_cfi")

process.trigFilt = cms.EDFilter("HcalTechTriggerFilt",
    gtDigiLabel     = cms.untracked.InputTag("hltGtDigis"),
    techTriggerBits = cms.vint32(25)
)
EOF
else
  echo Unknown mode '$MODE'
  exit
fi    

#### common tail part of Config File
cat >> ${CFGFILE}<<EOF99

# analysis

process.trigAnal  = cms.EDAnalyzer("HFtrigAnal",
    hfDigiLabel   = cms.untracked.InputTag("hcalDigis"),
    hfrechitLabel = cms.untracked.InputTag("hfreco"),

    lutFileName = cms.untracked.string('inputLUTcoder_CRUZET_part4_v3_1_HFonly.csv'),
    verbosity = cms.untracked.bool(False),

    digiSampleWindowMin = cms.int32(0),
    digiSampleWindowMax = cms.int32(9),

    digiSpectrumNbins  = cms.untracked.int32(100),
    digiSpectrumMinADC = cms.untracked.double(-0.5),
    digiSpectrumMaxADC = cms.untracked.double(499.5),

    minGeVperRecHit            = cms.double(6.0),
    totalEthresh4eventPlotting = cms.double(2000.0),

    # min/max event numbers to plot detailed RH energies
    eventNumberMax     = cms.int32(480),
    eventNumberMin     = cms.int32(470),

    rhTotalEnergyNbins  = cms.untracked.int32(100),
    rhTotalEnergyMinGeV = cms.untracked.double(0.0),
    rhTotalEnergyMaxGeV = cms.untracked.double(8000.0)
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
EOF99

if [[ "$MODE" == "LOCAL" ]]
then
cat >> ${CFGFILE}<<EOF999
process.p = cms.Path(process.hcalDigis*process.hfreco*process.trigAnal)
EOF999
elif [[ "$MODE" == "GLOBAL" ]] 
then
cat >> ${CFGFILE}<<EOF9999
process.p = cms.Path(process.trigFilt*process.hcalDigis*process.hfreco*process.trigAnal)
EOF9999
fi

# run cmsRun
cmsRun ${CFGFILE}
