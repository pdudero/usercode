
import FWCore.ParameterSet.Config as cms

process = cms.Process("DUALRECO")
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( -1 )
# test it!
#   input = cms.untracked.int32( 1000 ) 
)

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);
process.MessageLogger.cerr.FwkReport.reportEvery = 10

#process.source=cms.Source("PoolSource",
#fileNames = cms.untracked.vstring(
#'/store/data/Run2010A/JetMETTau/RAW/v1/000/139/980/74F06846-A08D-DF11-A817-00304879FBB2.root')
#)
#process.load("MyEDmodules.HcalDelayTuner.jetmettau_fill1207raw_1")
process.load("in_cff")
process.load("RunFiles.GoodRunList.goodRunList_cfi")

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

################################################################################
# Define the cleaning sequence

process.oneGoodVertexFilter = cms.EDFilter("VertexSelector",
   src = cms.InputTag("offlinePrimaryVertices"),
#   cut = cms.string("!isFake && tracksSize > 3 && abs(z) <= 15 && position.Rho <= 2"),
   cut = cms.string("!isFake && ndof >= 5 && abs(z) <= 15"),
   filter = cms.bool(True)   # otherwise it won't filter the events, just produce an empty vertex collection.
)

process.noscraping = cms.EDFilter("FilterOutScraping",
                                  applyfilter = cms.untracked.bool(True),
                                  debugOn = cms.untracked.bool(False),
                                  numtrack = cms.untracked.uint32(10),
                                  thresh = cms.untracked.double(0.25)
                                  )

process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')

process.cleaning_seq = cms.Sequence(process.oneGoodVertexFilter*
                                    process.noscraping*
                                    process.HBHENoiseFilter)

################################################################################
# Configure Retimer-Reflagger-Tower-MET rereco chain

process.load('Configuration.StandardSequences.GeometryExtended_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.RawToDigi_Data_cff')
process.load('MyEDmodules.HBHElinearityAnal.mypartialreco_cff')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
#process.GlobalTag.globaltag = 'GR10_P_V7::All'
process.GlobalTag.globaltag = 'GR_R_36X_V12::All'

process.myaltreco = process.hbhereco.clone()
process.myaltreco.samplesToAdd = cms.int32(5)
process.myaltreco.firstSample = cms.int32(3)

process.myreco_step = cms.Path(process.RawToDigi*
                               process.mypartialreco*
                               process.myaltreco*
                               process.cleaning_seq)

# Path and EndPath definitions

# Output definition
process.output = cms.OutputModule("PoolOutputModule",
   fileName = cms.untracked.string("pool.root"),
    # save only events passing the full path
   SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('myreco_step') ),
   outputCommands = cms.untracked.vstring("drop *",
                                          "keep *_hbhereco_*_*",
                                          "keep *_myaltreco_*_*",
                                          "keep *_hcalnoise_*_*")
)

process.out_step = cms.EndPath(process.output)

process.s  = cms.Schedule(process.myreco_step,
                          process.out_step)
