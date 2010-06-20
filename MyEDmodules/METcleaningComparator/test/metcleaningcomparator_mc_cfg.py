import FWCore.ParameterSet.Config as cms

isMC = True
version = 4   # version 1 = default (loose), version 2 = (medium), version 3 = (tight)
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
#'/store/mc/Spring10/TTbar/GEN-SIM-RECO/START3X_V26_S09-v1/0094/FEA115B1-AF4E-DF11-ACA5-0017A4770418.root'
'/store/mc/Spring10/QCDDiJet_Pt3000to3500/GEN-SIM-RECO/START3X_V26_S09-v1/0044/1C6E8669-5149-DF11-9B14-0017A4770C14.root'
    )
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

process.GlobalTag.globaltag ='START3X_V26::All'

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
    if (isMC==False):
        process.hfrecoReflagged = process.HFrechitreflaggerJETMETv4.clone()
    else:
        process.hfrecoReflagged = process.HFrechitreflaggerJETMETv2.clone()

# Path and EndPath definitions
process.reflaggerPath = cms.Path(process.hbherecoReflagged*
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

process.jetidpath = cms.Path(process.tcMetClean)

#######################################################################
# Configure HCAL event filter cleaning

process.load('CommonTools/RecoAlgos/HBHENoiseFilterResultProducer_cfi')
process.evfiltpath = cms.Path(process.HBHENoiseFilterResultProducer)
#process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')
#process.evfiltpath = cms.Path(process.HBHENoiseFilter)

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
                                 evfiltPathName    = cms.untracked.string("evfiltpath"),
                                 hbheFlagBit       = cms.untracked.int32(31),
                                 hfFlagBit         = cms.untracked.int32(31),
# bit 31 is UserDefinedBit0; this duplicates the setting inside hbherechitreflaggerJETMET_cfi.py
                                 minMET4plotGeV    = cms.untracked.double(-100),
                                 maxMET4plotGeV    = cms.untracked.double(1000),
                                 mindMET4plotGeV   = cms.untracked.double(-200),
                                 maxdMET4plotGeV   = cms.untracked.double(200),
                                 minSumET4plotGeV  = cms.untracked.double(-100),
                                 maxSumET4plotGeV  = cms.untracked.double(5000),
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
