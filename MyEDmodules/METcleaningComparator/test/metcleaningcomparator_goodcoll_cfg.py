import FWCore.ParameterSet.Config as cms

version = 2   # version 1 = default (loose), version 2 = (medium), version 3 = (tight)
# VERSION 2 is the currently recommended version, as of 22 April 2010.

process = cms.Process("METCLEAN")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("metcleaningcomparison.root"),
    closeFileFast = cms.untracked.bool(False)
)

#process.maxEvents = cms.untracked.PSet( output = cms.untracked.int32(10) )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

#######################################################################
# Configure input source

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
#       'file:myfile.root'
    '/store/data/Commissioning10/MinimumBias/RAW-RECO/Apr20Skim_GOODCOLL-v1/0185/F0FC8AD4-6A4E-DF11-969F-0026189438F4.root'
    )
)

#######################################################################
# Configure trigger requirements (thanks Dinko)

# PhysicsDeclared filter
process.load('HLTrigger.special.hltPhysicsDeclared_cfi')
process.hltPhysicsDeclared.L1GtReadoutRecordTag = 'gtDigis'

# BPTX & BSC triggers filter
process.load('L1TriggerConfig.L1GtConfigProducers.L1GtTriggerMaskTechTrigConfig_cff')
process.load('HLTrigger/HLTfilters/hltLevel1GTSeed_cfi')
process.hltLevel1GTSeed.L1TechTriggerSeeding = cms.bool(True)
process.hltLevel1GTSeed.L1SeedsLogicalExpression = cms.string('0 AND (40 OR 41) AND NOT (36 OR 37 OR 38 OR 39) AND NOT ((42 AND NOT 43) OR (43 AND NOT 42))')

# Primary vertex filter
process.primaryVertexFilter = cms.EDFilter("VertexSelector",
    src = cms.InputTag("offlinePrimaryVertices"),
    cut = cms.string("!isFake && ndof > 4 && abs(z) <= 15 && position.Rho <= 2"),
    filter = cms.bool(True)
)

# Scraping filter
process.scrapingVeto = cms.EDFilter("FilterOutScraping",
    applyfilter = cms.untracked.bool(True),
    debugOn = cms.untracked.bool(False),
    numtrack = cms.untracked.uint32(10),
    thresh = cms.untracked.double(0.25)
)

process.trigseq = cms.Sequence(
    process.hltPhysicsDeclared*
    process.hltLevel1GTSeed*
    process.primaryVertexFilter*
    process.scrapingVeto
)

#######################################################################
# Configure reflagger HBHE cleaning:

# Standard configurations
process.load('Configuration/StandardSequences/Services_cff')
process.load('Configuration/StandardSequences/GeometryExtended_cff')
process.load('Configuration/StandardSequences/MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration/StandardSequences/Reconstruction_cff')
process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')
process.load('Configuration/EventContent/EventContent_cff')

process.GlobalTag.globaltag ='GR10_P_V4::All'

import JetMETAnalysis.HcalReflagging.RemoveAddSevLevel as RemoveAddSevLevel
process.hcalRecAlgos=RemoveAddSevLevel.AddFlag(process.hcalRecAlgos,"UserDefinedBit0",10)

# HBHE RecHit reflagger
process.load("JetMETAnalysis/HcalReflagging/hbherechitreflaggerJETMET_cfi")
process.hbherecoReflagged = process.hbherechitreflaggerJETMET.clone()
process.hbherecoReflagged.debug=0

# Set energy threshold for identifying noise
#process.hbherecoReflagged.Ethresh=0.5
# Set number of channels/hpd that must be above threshold in order for the HPD to be marked noisy
#process.hbherecoReflagged.Nhits=14
# Turn this on to check # of hits per RBX, rather than per HPD
#process.hbherecoReflagged.RBXflag=False

# Use the reflagged HBHE RecHits to make the CaloTowers
process.towerMaker.hbheInput = "hbherecoReflagged"
process.towerMakerWithHO.hbheInput = "hbherecoReflagged"

#######################################################################
# Configure reflagger HF cleaning:

import JetMETAnalysis.HcalReflagging.RemoveAddSevLevel as RemoveAddSevLevel
process.hcalRecAlgos=RemoveAddSevLevel.RemoveFlag(process.hcalRecAlgos,"HFLongShort")

# Set energy threshold for identifying noise
#process.hbherecoReflagged.Ethresh=0.5
# Set number of channels/hpd that must be above threshold in order for the HPD to be marked noisy
#process.hbherecoReflagged.Nhits=14
# Turn this on to check # of hits per RBX, rather than per HPD
#process.hbherecoReflagged.RBXflag=False

# Use the reflagged HF RecHits to make the CaloTowers
process.towerMaker.hfInput = "hfrecoReflagged"
process.towerMakerWithHO.hfInput = "hfrecoReflagged"

# HF RecHit reflagger
process.load("JetMETAnalysis/HcalReflagging/HFrechitreflaggerJETMET_cff")
if version==1:
    process.hfrecoReflagged = process.HFrechitreflaggerJETMETv1.clone()
elif version==2:
    process.hfrecoReflagged = process.HFrechitreflaggerJETMETv2.clone()
elif version==3:
    process.hfrecoReflagged = process.HFrechitreflaggerJETMETv3.clone()
elif version==4:
    process.hfrecoReflagged = process.HFrechitreflaggerJETMETv4.clone()

# Path and EndPath definitions
process.reflaggerPath = cms.Path(process.trigseq*
                                 process.hbherecoReflagged*
                                 process.hfrecoReflagged*
                                 process.caloTowersRec*
                                 (process.recoJets*process.recoJetIds+process.recoTrackJets)*
                                 process.recoJetAssociations*
                                 process.metreco) # re-reco jets and met

#process.rereco_step = cms.Path(process.towerMaker*process.ak5CaloJets*process.met) # a simpler use case

#######################################################################
# Configure "jetID-based cleaning" (Frank's/Avi's cleaning)

process.load("TCMETcleaned357.CleanedTCMETProducer.cleanedtcmetproducer_cfi")
process.tcMetClean.tcmetInputTag     = cms.InputTag("met","","RECO")  # recoCaloMETs_met__RECO
process.tcMetClean.useHFcorrection   = cms.bool(True)
process.tcMetClean.useECALcorrection = cms.bool(False)
process.tcMetClean.useHCALcorrection = cms.bool(True)
process.tcMetClean.alias             = cms.string("TCcleanedCaloMET")

process.jetidpath = cms.Path(process.trigseq*process.tcMetClean)

#######################################################################
# Configure HCAL event filter cleaning

process.load('CommonTools/RecoAlgos/HBHENoiseFilterResultProducer_cfi')
process.evfiltpath = cms.Path(process.trigseq*process.HBHENoiseFilterResultProducer)
#process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')
#process.evfiltpath = cms.Path(process.trigseq*process.HBHENoiseFilter)

# If you would like to also add a filter which cuts on the EMF of an RBX,
# we recommend that you add the line:
#process.HBHENoiseFilter.maxRBXEMF = cms.double(0.01)

#######################################################################
# Configure Endpath

# Output definition
process.output = cms.OutputModule(
    "PoolOutputModule",
    splitLevel = cms.untracked.int32(0),
#   outputCommands = process.RECOEventContent.outputCommands,
#    fileName = cms.untracked.string('/uscmst1b_scratch/lpc1/3DayLifetime/pdudero/output_file.root'),
    fileName = cms.untracked.string('pooloutput.root'),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('RECO'),
        filterName = cms.untracked.string('')
    )
)

process.output.outputCommands = cms.untracked.vstring("drop *",
                                                      "keep CaloTowersSorted_*_*_*",
                                                      "keep *_TriggerResults_*_*",
                                                      "keep *_HBHENoiseFilterResultProducer_*_*",
#                                                      "keep *_HBHENoiseFilter_*_*",
                                                      "keep *_hbhereco_*_*",
                                                      "keep *_hbherecoReflagged_*_*",
                                                      "keep *_hfreco_*_*",
                                                      "keep *_hfrecoReflagged_*_*",
                                                      "keep recoCaloMETs_*_*_*",
                                                      "keep recoMETs_*_*_*")
                                            
#new rechit collection name is:  HBHERecHitsSorted_hbherecoReflagged__METCLEAN

#######################################################################
# Configure Analyzer

process.compare = cms.EDAnalyzer('METcleaningComparator',
                                 triggerResults = cms.untracked.PSet(
    trgResultsLabel = cms.untracked.InputTag("TriggerResults","","METCLEAN"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 dirtyInput = cms.untracked.PSet(
    hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
    hfRechitLabel   = cms.untracked.InputTag("hfreco"),
    caloMETlabel    = cms.untracked.InputTag("met","","RECO"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 jetidCleanOutput = cms.untracked.PSet(
    recoMETlabel    = cms.untracked.InputTag("tcMetClean","","METCLEAN"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 reflagCleanOutput = cms.untracked.PSet(
    hbheRechitLabel = cms.untracked.InputTag("hbherecoReflagged"),
    hfRechitLabel   = cms.untracked.InputTag("hfrecoReflagged"),
    caloMETlabel    = cms.untracked.InputTag("met","","METCLEAN"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 evfiltCleanOutput = cms.untracked.PSet(
    hbheNoiseResultLabel = cms.untracked.InputTag("HBHENoiseFilterResultProducer","HBHENoiseFilterResult","METCLEAN"),
#    caloMETlabel    = cms.untracked.InputTag("HBHENoiseFilter","","METCLEAN"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 evfiltPathName   = cms.untracked.string("evfiltpath"),
                                 hbheFlagBit      = cms.untracked.int32(31),
                                 hfFlagBit        = cms.untracked.int32(31),
# bit 31 is UserDefinedBit0; this duplicates the setting inside hbherechitreflaggerJETMET_cfi.py
                                 minMET4plotGeV    = cms.untracked.double(-100),
                                 maxMET4plotGeV    = cms.untracked.double(1200),
                                 mindMET4plotGeV   = cms.untracked.double(-1200),
                                 maxdMET4plotGeV   = cms.untracked.double(100),
                                 minSumET4plotGeV  = cms.untracked.double(-100),
                                 maxSumET4plotGeV  = cms.untracked.double(8000),
                                 mindSumET4plotGeV = cms.untracked.double(-500),
                                 maxdSumET4plotGeV = cms.untracked.double(500)
)

#######################################################################
# Schedule definition

#process.out_step = cms.EndPath(process.compare*process.output)
process.out_step = cms.EndPath(process.compare)
#process.out_step = cms.EndPath(process.output)

process.schedule = cms.Schedule(process.jetidpath,
                                process.reflaggerPath,
                                process.evfiltpath,
                                process.out_step)
