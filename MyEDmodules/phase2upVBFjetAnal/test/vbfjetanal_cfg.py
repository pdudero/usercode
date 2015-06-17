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
        'file:junk.root'
        )
                            )

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

#process.load("vbf_h2inv_140PU_shcal_reco_cfi_001")

process.maxEvents = cms.untracked.PSet(
                      input = cms.untracked.int32(-1)
)

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

if doTightVBF:
    process.vbfanal = vbfanaltight.clone()
else:
    process.vbfanal = vbfanalloose.clone()

process.vbfanal.benchmarkParams.isSignalDoTruthMatch = cms.bool(isSignal)
process.vbfanal.benchmarkParams.pfjBenchmarkDebug = cms.bool(debugPrintout)
process.vbfanal.benchmarkParams.maxplotE = cms.double(1000.0)

process.vbfanal.RecoJetCoreLabel = cms.InputTag( 'ak25PFJets' )


if doJEC:
    process.vbfanal.RecoJetLabel = cms.InputTag( 'ak4PFchsJetsL1FastL2L3' )
    process.mypath=cms.Path(  process.ak4PFchsJetsL1FastL2L3 *
                              process.ak25PFJets * 
                              process.vbfanal )
else:
    process.vbfanal.RecoJetLabel = cms.InputTag( 'ak4PFJetsCHS' )
    process.mypath=cms.Path(  process.ak25PFJets * process.vbfanal )

process.s = cms.Schedule ( process.mypath )
