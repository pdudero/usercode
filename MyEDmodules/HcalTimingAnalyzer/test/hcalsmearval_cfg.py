import FWCore.ParameterSet.Config as cms


process = cms.Process("SmearVal")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.MessageLogger.cerr.default.limit = cms.untracked.int32(1000)
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(100)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

#process.load("MyEDmodules.HcalTimingAnalyzer.pgun_cfi")
#process.load("MyEDmodules.HcalTimingAnalyzer.pgun2_cfi")
process.load("MyEDmodules.HcalTimingAnalyzer.in_cff")

process.TFileService = cms.Service("TFileService",
     closeFileFast = cms.untracked.bool(True),
     fileName = cms.string('smearval-pgun2smearv2.root')
)

process.load("MyEDmodules.HcalTimingAnalyzer.hcalsmearval_cfi")
process.smearval.recHitEscaleMaxGeV = cms.double(350.5)
process.smearval.minHitGeVHB        = cms.double(1.0)
process.smearval.minHitGeVHE        = cms.double(1.0)
process.smearval.runDescription     = cms.untracked.string('PionGun')

process.p = cms.Path(process.smearval)
