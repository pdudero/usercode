import FWCore.ParameterSet.Config as cms

process = cms.Process("METCLEAN")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("metcleaningcomparison.root"),
    closeFileFast = cms.untracked.bool(False)
)

#process.maxEvents = cms.untracked.PSet( output = cms.untracked.int32(100) )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

#######################################################################
# Configure input source

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
#       'file:myfile.root'
    '/store/data/Commissioning10/MinimumBias/RECO/v9/000/135/131/3698D99C-705A-DF11-9275-000423D944FC.root'
    )
)

#######################################################################
# Configure trigger requirements

#process.load("L1TriggerConfig.L1GtConfigProducers.L1GtTriggerMaskTechTrigConfig_cff")
#process.load("HLTrigger.HLTfilters.hltLevel1GTSeed_cfi")
#process.hltLevel1GTSeed.L1TechTriggerSeeding = cms.bool(True)
#process.hltLevel1GTSeed.L1SeedsLogicalExpression = cms.string("${L1TTBITS}")

process.load("HLTrigger.HLTfilters.hltHighLevel_cfi")
#
# Noise data:
#
process.jetnobptx = process.hltHighLevel.clone(HLTPaths = cms.vstring("HLT_L1Jet10U_NoBPTX"))
process.bscorbptx = process.hltHighLevel.clone(HLTPaths = cms.vstring("HLT_L1_BscMinBiasOR_BptxPlusORMinus"))

process.trigpath = cms.Path(process.jetnobptx*(~process.bscorbptx))

#######################################################################
# Configure reflagger cleaning:

# Standard configurations
process.load('Configuration/StandardSequences/Services_cff')
process.load('Configuration/StandardSequences/GeometryExtended_cff')
process.load('Configuration/StandardSequences/MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration/StandardSequences/Reconstruction_cff')
process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')
process.load('Configuration/EventContent/EventContent_cff')

import JetMETAnalysis.HcalReflagging.RemoveAddSevLevel as RemoveAddSevLevel
process.hcalRecAlgos=RemoveAddSevLevel.AddFlag(process.hcalRecAlgos,"UserDefinedBit0",10)

# HBHE RecHit reflagger
process.load("JetMETAnalysis/HcalReflagging/hbherechitreflaggerJETMET_cfi")
process.hbherecoReflagged = process.hbherechitreflaggerJETMET.clone()
process.hbherecoReflagged.debug=0

process.GlobalTag.globaltag ='GR10_P_V4::All'

# Set energy threshold for identifying noise
#process.hbherecoReflagged.Ethresh=0.5
# Set number of channels/hpd that must be above threshold in order for the HPD to be marked noisy
#process.hbherecoReflagged.Nhits=14
# Turn this on to check # of hits per RBX, rather than per HPD
#process.hbherecoReflagged.RBXflag=False

# Use the reflagged HBHE RecHits to make the CaloTowers
process.cleanTowerMaker = process.towerMaker.clone(hbheInput = "hbherecoReflagged")
process.cleanTowerMakerWithHO = process.towerMakerWithHO.clone(hbheInput = "hbherecoReflagged")

# Path and EndPath definitions
process.reflagging_step = cms.Path(process.hbherecoReflagged)
process.rereco_step = cms.Path(process.caloTowersRec*(process.recoJets*process.recoJetIds+process.recoTrackJets)*process.recoJetAssociations*process.metreco) # re-reco jets and met
#process.rereco_step = cms.Path(process.towerMaker*process.ak5CaloJets*process.met) # a simpler use case

#######################################################################
# Configure "tcMET" (Frank's/Avi's cleaning)

process.load("TCMETcleaned357.CleanedTCMETProducer.cleanedtcmetproducer_cfi")
process.tcMetClean.tcmetInputTag     = cms.InputTag("met","","RECO")  # recoCaloMETs_met__RECO
process.tcMetClean.useHFcorrection   = cms.bool(False)
process.tcMetClean.useECALcorrection = cms.bool(False)
process.tcMetClean.useHCALcorrection = cms.bool(True)
process.tcMetClean.alias             = cms.string("TCcleanedCaloMET")

process.tcmetpath = cms.Path(process.tcMetClean)

#######################################################################
# Configure HCAL event filter cleaning

process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')
process.evfiltpath = cms.Path(process.HBHENoiseFilter)

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
                                 dirtyInput = cms.untracked.PSet(
    hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
    caloMETlabel    = cms.untracked.InputTag("met","","RECO"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 tcmetCleanOutput = cms.untracked.PSet(
    recoMETlabel    = cms.untracked.InputTag("tcMetClean","","METCLEAN"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 reflagCleanOutput = cms.untracked.PSet(
    hbheRechitLabel = cms.untracked.InputTag("hbherecoReflagged"),
    caloMETlabel    = cms.untracked.InputTag("met","","METCLEAN"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 evfiltCleanOutput = cms.untracked.PSet(
    trgResultsLabel = cms.untracked.InputTag("TriggerResults","","METCLEAN"),
    caloMETlabel    = cms.untracked.InputTag("met","","METCLEAN"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 evfiltPathName = cms.untracked.string("evfiltpath"),
                                 hbheFlagBit    = cms.untracked.int32(31)
# bit 31 is UserDefinedBit0; this duplicates the setting inside hbherechitreflaggerJETMET_cfi.py
)

#######################################################################
# Schedule definition

process.out_step = cms.EndPath(process.compare*process.output)

process.schedule = cms.Schedule(process.trigpath,
                                process.tcmetpath,
                                process.reflagging_step,
                                process.rereco_step,
                                process.evfiltpath,
                                process.out_step)
