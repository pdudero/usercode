# test file for PFDQM Validation
# performs a Jet and MET Validations (PF vs Gen and PF vs Calo)
# creates an EDM file with histograms filled with PFCandidate data,
# present in the PFJetMonitor and PFMETMonitor classes in DQMOffline/PFTau
# package for matched PFCandidates. Matching histograms (delta pt etc)
# are also available. 

import FWCore.ParameterSet.Config as cms
process = cms.Process("Anal")

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
process.maxEvents = cms.untracked.PSet(
                      input = cms.untracked.int32(-1)
)

process.gpfilt = cms.EDFilter('GenParticleFilter',
                              verbose = cms.untracked.bool(True),
                              GenParticleLabel = cms.InputTag('genParticles'),
                              excludeIDlist = cms.vint32(-15,-11,11,15)
                              )

process.p =cms.Path( process.gpfilt )
