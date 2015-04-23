# test file for PFDQM Validation
# performs a Jet and MET Validations (PF vs Gen and PF vs Calo)
# creates an EDM file with histograms filled with PFCandidate data,
# present in the PFJetMonitor and PFMETMonitor classes in DQMOffline/PFTau
# package for matched PFCandidates. Matching histograms (delta pt etc)
# are also available. 

import FWCore.ParameterSet.Config as cms
process = cms.Process("Anal")

doJEC=False
debugPrintout=False
#isSignal=False
isSignal=False

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
        '/store/mc/TP2023SHCALDR/GJet_Pt-15to3000_Tune4C_14TeV_pythia8/GEN-SIM-RECO/SHCALJan23_PU140BX25_PH2_1K_FB_V6-v1/00000/FE2777A8-AAA6-E411-BD08-002618943908.root'
        )
                            )

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

#process.load("vbf_h2inv_140PU_shcal_reco_cfi_001")

process.maxEvents = cms.untracked.PSet(
                      input = cms.untracked.int32(-1)
)

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

from Dead.Puppi.Puppi_PhaseII_cff_v2 import *
process.load('Dead.Puppi.Puppi_PhaseII_cff_v2')

#sequence = cms.Sequence(sequence * )


process.load("PhysicsTools.PatAlgos.selectionLayer1.leptonCountFilter_cfi")

process.count2Muons = process.countPatLeptons.clone(countElectrons = False, muonSource = "selectedIsoMuons", minNumber = 2)
process.count4Muons = process.countPatLeptons.clone(countElectrons = False, muonSource = "selectedIsoMuons", minNumber = 4)

process.load("JetCorrection_cff")
process.ak4PUPPIJets = ak5PFJets.clone( rParam=0.4, src = cms.InputTag('puppi','Puppi'))

if doJEC:
    process.ak4PUPPIL1Fastjet = cms.ESProducer('L1FastjetCorrectionESProducer',
                                               era         = cms.string('Summer11'),
                                               level       = cms.string('L1FastJet'),
                                               algorithm   = cms.string('AK4PUPPI'),
                                               section     = cms.string(''),
                                               srcRho      = cms.InputTag('kt6PFJets','rho'),
                                               useCondDB = cms.untracked.bool(True)
                                               )

    era = "PhaseII_Shashlik140PU"
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
process.ak20PUPPIJets = process.ak4PUPPIJets.clone( rParam = 0.20 )
process.ak25PUPPIJets = process.ak4PUPPIJets.clone( rParam = 0.25 )

process.gpfilt = cms.EDFilter('GenParticleFilter',
                              GenParticleLabel = cms.InputTag('genParticles'),
                              excludeIDlist = cms.vint32(-15,-11,11,15)
                              )

from MyEDmodules.phase2upVBFjetAnal.phase2upVBFjetAnal_cfi import vbfanalloose

process.vbfanal20cores = vbfanalloose.clone()
process.vbfanal20cores.benchmarkParams.isSignalDoTruthMatch = cms.bool(isSignal)
process.vbfanal20cores.benchmarkParams.pfjBenchmarkDebug = cms.bool(debugPrintout)
process.vbfanal20cores.RecoJetCoreLabel = cms.InputTag( 'ak20PUPPIJets' )


# if doJEC:
#     process.vbfanal20cores.RecoJetLabel = cms.InputTag( 'ak4PUPPIJetsL2L#' )
# else:
#     process.vbfanal20cores.RecoJetLabel = cms.InputTag( 'ak4PUPPIJets' )

process.vbfanal25cores = process.vbfanal20cores.clone( RecoJetLabel = 'ak25PUPPIJets' )

if doJEC:
    process.nomuon20corespath=cms.Path(  puppi * process.ak4PUPPIJets *
                                         process.ak20PFJets * process.vbfanal20cores )
    process.nomuon25corespath=cms.Path(  puppi * process.ak4PUPPIJets *
                                         process.ak25PFJets * process.vbfanal25cores )
else:
    process.nomuon20corespath=cms.Path(  puppi * process.ak20PUPPIJets * process.vbfanal20cores )
    process.nomuon25corespath=cms.Path(  puppi * process.ak25PUPPIJets * process.vbfanal25cores )

# process.twomuonpath=cms.Path( process.selectedIDMuons *
#                               process.selectedIsoMuons *
#                               process.count2Muons *
#                               process.ak4PFchsJetsL1FastL2L3 *
#                               process.ak15PFJets * process.vbfanal )
# process.fourmuonpath=cms.Path(process.selectedIDMuons *
#                               process.selectedIsoMuons *
#                               process.count4Muons *
#                               process.ak4PFchsJetsL1FastL2L3 *
#                               process.ak15PFJets * process.vbfanal )

process.s = cms.Schedule ( process.nomuon20corespath,
                           process.nomuon25corespath
#                          , process.twomuonpath
#                          , process.fourmuonpath
                           )
