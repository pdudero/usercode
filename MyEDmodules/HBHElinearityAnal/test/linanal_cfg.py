
import FWCore.ParameterSet.Config as cms

process = cms.Process("HBHELINANAL")
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
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1)
process.MessageLogger.cerr.FwkReport.reportEvery = 10

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = 'GR10_P_V6::All'

# Replace timecorr object
process.tc_ascii2 = cms.ESSource("HcalTextCalibrations",
  input = cms.VPSet(cms.PSet(
           object = cms.string('TimeCorrs'),
             file = cms.FileInPath('MyEDmodules/HBHERecHitRetimer/data/timecorr_may27.txt') )
                    )
)
process.prefer=cms.ESPrefer('HcalTextCalibrations','tc_ascii2')
process.hbherecoRetimed = cms.EDProducer('HBHERecHitRetimer',
                                         input = cms.InputTag('hbhereco')
)

process.source=cms.Source("PoolSource",
fileNames = cms.untracked.vstring('store.root')
)

#process.load("in_cff")

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

################################################################################
# Analyzers
process.TFileService = cms.Service("TFileService", 
   fileName = cms.string("anal.root"),
    closeFileFast = cms.untracked.bool(False)
)

process.linanal = cms.EDAnalyzer("HBHElinearityAnal",
#                                 src = cms.untracked.InputTag('hbherecoRetimed')
                                 )

#process.path  = cms.Path(process.hbherecoRetimed*process.linanal)
process.path  = cms.Path(process.linanal)
