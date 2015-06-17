# test file for PFDQM Validation
# performs a Jet and MET Validations (PF vs Gen and PF vs Calo)
# creates an EDM file with histograms filled with PFCandidate data,
# present in the PFJetMonitor and PFMETMonitor classes in DQMOffline/PFTau
# package for matched PFCandidates. Matching histograms (delta pt etc)
# are also available. 

import FWCore.ParameterSet.Config as cms
process = cms.Process("Anal")

doJEC=True
debugPrintout=False
isSignal=False
doTightVBF=False

from RecoJets.Configuration.RecoPFJets_cff import ak5PFJets

#------------------------
# Message Logger Settings
#------------------------
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

#--------------------------------------
# Event Source & # of Events to process
#---------------------------------------
process.source = cms.Source("PoolSource",
 fileNames = cms.untracked.vstring(
#        'file:junk.root'
        '/store/mc/Upg2023SHCAL14DR/GluGluToHToZZTo4L_em_M-125_14TeV-powheg-pythia6/GEN-SIM-RECO/PU140bx25_PH2_1K_FB_V4-v1/00000/FEF6124E-973B-E411-93C8-002618943886.root'
        )
                            )

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
    allowUnscheduled = cms.untracked.bool(True) 
)

#process.load("vbf_h2inv_140PU_shcal_reco_cfi_001")

process.maxEvents = cms.untracked.PSet(
                      input = cms.untracked.int32(-1)
)

process.load("TrackingTools/TransientTrack/TransientTrackBuilder_cfi")
process.load('Configuration.Geometry.GeometryExtended2023SHCalNoTaperReco_cff')
process.load('Configuration.Geometry.GeometryExtended2023SHCalNoTaper_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_PostLS1_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = 'PH2_1K_FB_V7::All'

############## "Classic" PAT Muons and Electrons ########################
# (made from all reco muons, and all gsf electrons, respectively)
### DEFINITION OF THE PFBRECO+PAT SEQUENCES ##########
# load the PAT config
process.load("PhysicsTools.PatAlgos.patSequences_cff")
from PhysicsTools.PatAlgos.tools.coreTools import *
from PhysicsTools.PatAlgos.tools.pfTools import *

# Configure PAT to use PFBRECO instead of AOD sources
# this function will modify the PAT sequences.
# usePF2PAT(process,runPF2PAT=True, jetAlgo="AK4", runOnMC=True, postfix="AK4",
#           jetCorrections=('AK4PF', ['L1FastJet','L2Relative','L3Absolute','L2L3Residual']))

# # use PFIsolation
# process.muIsoSequence = setupPFMuonIso(process, 'muons', 'PFIso')
# adaptPFIsoMuons( process, applyPostfix(process,"patMuons",""), 'PFIso')


process.selectedIDMuons = cms.EDFilter(
    "PATMuonSelector",
    src = cms.InputTag("selectedPatMuons"),
    cut = cms.string(
        "pt > 10 && isGlobalMuon && isTrackerMuon && globalTrack().normalizedChi2 < 10 &&" +
        "globalTrack().hitPattern().numberOfValidTrackerHits > 10 && " +
        "globalTrack().hitPattern().numberOfValidPixelHits > 0 && " +
        "globalTrack().hitPattern().numberOfValidMuonHits > 0 && " +
        "abs(dB) < 0.2 && numberOfMatches > 1 && abs(eta) < 2.4" )
    )
# Isolated muons: standard isolation; can be changed to rho-corrected one
process.selectedIsoMuons = cms.EDFilter(
    "PATMuonSelector",
    src = cms.InputTag("selectedIDMuons"),
    # cut = cms.string('userFloat("absCombIsoPUCorrected") < 0.15*pt')
    cut = cms.string("trackIso + caloIso < 0.15 * pt")
)

process.myLazyMuonSeq = cms.Sequence(process.selectedPatMuons
                                     + process.selectedIDMuons)
#                                     + process.selectedIsoMuons)

process.load("PhysicsTools.PatAlgos.selectionLayer1.leptonCountFilter_cfi")

process.count1Muon  = process.countPatLeptons.clone(countElectrons = False, muonSource = "selectedIDMuons", minNumber = 1)
process.count2Muons = process.countPatLeptons.clone(countElectrons = False, muonSource = "selectedIDMuons", minNumber = 2)

if doJEC:
    process.load("JetCorrection_cff")

    era = "PhaseII_Shashlik140PU"
#    era = "PhaseI_140PU_V2"
#process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
#process.GlobalTag.globaltag = ""
    from CondCore.DBCommon.CondDBSetup_cfi import *
    process.jec = cms.ESSource("PoolDBESSource",CondDBSetup,
                               connect =cms.string('sqlite_file:'+era+'.db'),
                               toGet =cms.VPSet(
            cms.PSet(record = cms.string("JetCorrectionsRecord"),
                     tag    = cms.string("JetCorrectorParametersCollection_"+era+"_AK4PF"),
                     label  = cms.untracked.string("AK4PF")),
            cms.PSet(record = cms.string("JetCorrectionsRecord"),
                     tag    = cms.string("JetCorrectorParametersCollection_"+era+"_AK4PFchs"),
                     label  = cms.untracked.string("AK4PFchs")),
            )
                                                           )
    process.es_prefer_jec = cms.ESPrefer("PoolDBESSource","jec")

process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("vbfjetanal.root")
)

# apparently no CHS pf collection available?
process.ak25PFJets = ak5PFJets.clone( rParam = 0.25 )

process.gpfilt = cms.EDFilter('GenParticleFilter',
                              GenParticleLabel = cms.InputTag('genParticles'),
                              excludeIDlist = cms.vint32(-15,-11,11,15)
                              )

from MyEDmodules.phase2upVBFjetAnal.phase2upVBFjetAnal_cfi import *

process.vbfanaltight = vbfanaltight.clone()
process.vbfanalloose = vbfanalloose.clone()

process.vbfanaltight.benchmarkParams.isSignalDoTruthMatch = cms.bool(isSignal)
process.vbfanaltight.benchmarkParams.pfjBenchmarkDebug = cms.bool(debugPrintout)
process.vbfanaltight.RecoJetCoreLabel = cms.InputTag( 'ak25PFJets' )

process.vbfanalloose.benchmarkParams.isSignalDoTruthMatch = cms.bool(isSignal)
process.vbfanalloose.benchmarkParams.pfjBenchmarkDebug = cms.bool(debugPrintout)
process.vbfanalloose.RecoJetCoreLabel = cms.InputTag( 'ak25PFJets' )

process.vbfanaltight1muon  = process.vbfanaltight.clone()
process.vbfanaltight2muons = process.vbfanaltight.clone()

process.vbfanalloose1muon  = process.vbfanalloose.clone()
process.vbfanalloose2muons = process.vbfanalloose.clone()


if doJEC:
    process.vbfanalloose.RecoJetLabel       = cms.InputTag( 'ak4PFchsJetsL1FastL2L3' )
    process.vbfanalloose1muon.RecoJetLabel  = cms.InputTag( 'ak4PFchsJetsL1FastL2L3' )
    process.vbfanalloose2muons.RecoJetLabel = cms.InputTag( 'ak4PFchsJetsL1FastL2L3' )
    process.vbfanaltight.RecoJetLabel       = cms.InputTag( 'ak4PFchsJetsL1FastL2L3' )
    process.vbfanaltight1muon.RecoJetLabel  = cms.InputTag( 'ak4PFchsJetsL1FastL2L3' )
    process.vbfanaltight2muons.RecoJetLabel = cms.InputTag( 'ak4PFchsJetsL1FastL2L3' )
    process.loosenomuonpath=cms.Path(  process.ak4PFchsJetsL1FastL2L3 *
                                       process.ak25PFJets * 
                                       process.vbfanalloose )
    process.looseonemuonpath=cms.Path( process.myLazyMuonSeq *
                                       process.count1Muon *
                                       process.ak4PFchsJetsL1FastL2L3 *
                                       process.ak25PFJets * 
                                       process.vbfanalloose1muon )
    process.loosetwomuonpath=cms.Path( process.myLazyMuonSeq *
                                       process.count2Muons *
                                       process.ak4PFchsJetsL1FastL2L3 *
                                       process.ak25PFJets * 
                                       process.vbfanalloose2muons )
    process.tightnomuonpath=cms.Path(  process.ak4PFchsJetsL1FastL2L3 *
                                       process.ak25PFJets * 
                                       process.vbfanaltight )
    process.tightonemuonpath=cms.Path( process.myLazyMuonSeq *
                                       process.count1Muon *
                                       process.ak4PFchsJetsL1FastL2L3 *
                                       process.ak25PFJets * 
                                       process.vbfanaltight1muon )
    process.tighttwomuonpath=cms.Path( process.myLazyMuonSeq *
                                       process.count2Muons *
                                       process.ak4PFchsJetsL1FastL2L3 *
                                       process.ak25PFJets * 
                                       process.vbfanaltight2muons )
else:
    process.vbfanalloose.RecoJetLabel       = cms.InputTag( 'ak4PFchsJets' )
    process.vbfanalloose1muon.RecoJetLabel  = cms.InputTag( 'ak4PFchsJets' )
    process.vbfanalloose2muons.RecoJetLabel = cms.InputTag( 'ak4PFchsJets' )
    process.vbfanaltight.RecoJetLabel       = cms.InputTag( 'ak4PFchsJets' )
    process.vbfanaltight1muon.RecoJetLabel  = cms.InputTag( 'ak4PFchsJets' )
    process.vbfanaltight2muons.RecoJetLabel = cms.InputTag( 'ak4PFchsJets' )
    process.loosenomuonpath=cms.Path(  process.ak25PFJets * 
                                       process.vbfanalloose )
    process.looseonemuonpath=cms.Path( process.myLazyMuonSeq *
                                       process.count1Muon *
                                       process.ak25PFJets * 
                                       process.vbfanalloose1muon )
    process.loosetwomuonpath=cms.Path( process.myLazyMuonSeq *
                                       process.count2Muons *
                                       process.ak25PFJets * 
                                       process.vbfanalloose2muons )
    process.tightnomuonpath=cms.Path(  process.ak25PFJets * 
                                       process.vbfanaltight )
    process.tightonemuonpath=cms.Path( process.myLazyMuonSeq *
                                       process.count1Muon *
                                       process.ak25PFJets * 
                                       process.vbfanaltight1muon )
    process.tighttwomuonpath=cms.Path( process.myLazyMuonSeq *
                                       process.count2Muons *
                                       process.ak25PFJets * 
                                       process.vbfanaltight2muons )


process.s = cms.Schedule ( process.loosenomuonpath
                           , process.looseonemuonpath
                           , process.loosetwomuonpath
                           , process.tightnomuonpath
                           , process.tightonemuonpath
                           , process.tighttwomuonpath
                           )
