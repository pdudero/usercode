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

if [[ -e ./anal_setup.rc ]] 
then
    source ./anal_setup.rc
fi

ARG1=$1
#OUTPUTFILE=$2
OUTPUTFILE=`printf "${OUTPUTFILEFMT}" ${ARG1} ${ARG1}`
INCLUDEFILE=`printf "${FORMAT}" ${ARG1}`

if (( ${#EVENTLIMIT} == 0 )) 
then
    EVENTLIMIT="-1";
fi

#### common head part of Config File
### create the file
CFGFILE=/tmp/runCMSSWReco_${USER}_$$.py
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
process.hfreco.samplesToAdd = 8

EOF

### Mode-dependent part

if [[ "${FILEMODE}" == "LOCAL" ]]
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

if [[ "${SOURCE}" == "HCAL" ]]
then
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
elif [[ "${SOURCE}" == "POOL" ]]
then
cat >> ${CFGFILE}<<EOF
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:${FILE}')
)
EOF
fi

elif [[ "${FILEMODE}" == "GLOBAL" ]] 
    then
cat >> ${CFGFILE}<<EOF
process.load("${INCLUDEFILE}")
EOF
fi

if [[ "${MODE}" == "RAW" ]]
    then
cat >> ${CFGFILE}<<EOF
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
elif [[ "$MODE" == "RECO" ]]
    then
echo Assume reco products in input.
else
  echo Unknown mode "${MODE}"
  exit
fi    

#### common tail part of Config File
cat >> ${CFGFILE}<<EOF99

# analysis

process.trigAnal  = cms.EDAnalyzer("HFtrigAnal",
    hfDigiLabel   = cms.untracked.InputTag("hcalDigis"),
    hfrechitLabel = cms.untracked.InputTag("hfreco"),

    lutFileName = cms.untracked.string('${LUTFILENAME}'),
    verbosity = cms.untracked.bool(False),

    digiSampleWindowMin = cms.int32(0),
    digiSampleWindowMax = cms.int32(9),

    adcTrigThreshold   = cms.int32(12),

    detIds2Mask        = cms.vint32(-31,25,1,
                                     35,67,1,
                                     30,67,2,
                                     32,67,2,
                                     36,67,2),

    #validBxNumbers     = cms.vint32(2618,2619,2620,2621,2622,2623,2624),
    validBxNumbers     = cms.vint32(900,901,904),

    digiSpectrumNbins  = cms.untracked.int32(100),
    digiSpectrumMinADC = cms.untracked.double(-0.5),
    digiSpectrumMaxADC = cms.untracked.double(499.5),

    minGeVperRecHit            = cms.double(10.0),
    totalEthresh4eventPlotting = cms.double(2000.0),

    # min/max event numbers to plot detailed RH energies
    eventNumberMax     = cms.int32(480),
    eventNumberMin     = cms.int32(470),

    rhTotalEnergyNbins  = cms.untracked.int32(110),
    rhTotalEnergyMinGeV = cms.untracked.double(0.0),
    rhTotalEnergyMaxGeV = cms.untracked.double(11000.0),

    nWedgesPlotNbins  = cms.untracked.int32(21),
    nWedgesPlotMin    = cms.untracked.double(-0.5),
    nWedgesPlotMax    = cms.untracked.double(20.5),

    lumiSegPlotNbins  = cms.untracked.int32(160),
    lumiSegPlotMin    = cms.untracked.double(-0.5),
    lumiSegPlotMax    = cms.untracked.double(159.5)
)
EOF99

if [[ "$MODE" == "RAW" ]]
then
cat >> ${CFGFILE}<<EOF999
process.p = cms.Path(process.hcalDigis*process.hfreco*process.trigAnal)
EOF999
elif [[ "$MODE" == "RECO" ]] 
then
cat >> ${CFGFILE}<<EOF999
process.p = cms.Path(process.trigAnal)
EOF999
fi

# run cmsRun
# cmsRun ${CFGFILE}
callgrind --instr-atstart=no cmsRun ${CFGFILE}
