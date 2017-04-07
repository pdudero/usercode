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
process.MessageLogger.cerr.FwkReport.reportEvery = 1
#--------------------------------------
# Event Source & # of Events to process
#---------------------------------------
process.source = cms.Source("PoolSource",
                   fileNames = cms.untracked.vstring()
                 )
process.maxEvents = cms.untracked.PSet(
                      input = cms.untracked.int32(-1)
                    )

process.TFileService = cms.Service("TFileService",
	fileName = cms.string('resumweights.root')
)

process.load("MyEDmodules.WWptResummation.WWptResummation_cfi")

process.WWptResummationAnal.pars.centralFile = cms.string('central8tev.dat')
process.WWptResummationAnal.pars.QupFile = cms.string('resumscaleQup8tev.dat')
process.WWptResummationAnal.pars.QdnFile = cms.string('resumscaleQdn8tev.dat')
process.WWptResummationAnal.pars.RupFile = cms.string('renormscaleRup8tev.dat')
process.WWptResummationAnal.pars.RdnFile = cms.string('renormscaleRdn8tev.dat')

# "62" for Powheg2
#
#process.WWptResummationAnal.pars.finalParticleStatus = cms.int32(62)

process.p =cms.Path(
    process.WWptResummationAnal
    )

#--------------------------------------
# List File names here
#---------------------------------------
#process.PoolSource.fileNames = [
## '/store/user/pdudero/WW_To_LNuQQ_8TeV_powheg2_pythia6/WW_To_LNuQQ_8TeV_powheg2_pythia6/c68646355e1365536efbdf7dc4feb95c/WW_To_LnuQQ_CT10_8TeV_powheg2_pythia6_GEN_100_1_evj.root',
#'/store/user/pdudero/WWToLnuQQ_CT10_8TeV_Powheg2_Pythia8_Tune4C_Tauolapp_GENSIM/WWToLnuQQ_CT10_8TeV_Powheg2_Pythia8_Tune4C_Tauolapp_GENSIM/ac8d04934314f73a0a36d81ac67fa779/WWToLnuQQ_CT10_8TeV_Powheg2_Pythia8_Tune4C_Tauolapp_GENSIM_1_1_CNE.root'
#]
