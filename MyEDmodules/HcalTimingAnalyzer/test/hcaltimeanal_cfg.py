import FWCore.ParameterSet.Config as cms


process = cms.Process("MyAnal")

process.load("Configuration.StandardSequences.Geometry_cff")
process.load("RecoJets.Configuration.CaloTowersES_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")

process.MessageLogger.cerr.default.limit = cms.untracked.int32(1000)
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(100)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

process.load("MyEDmodules.HcalTimingAnalyzer.in_cff")

process.TFileService = cms.Service("TFileService",
     closeFileFast = cms.untracked.bool(True),
     fileName = cms.string('hcaltimeanal-out.root')
)

process.load("MyEDmodules.HcalTimingAnalyzer.hcalFilteredReco_cff")
#process.myanunfilt.eventDataPset.hbheRechitLabel   = cms.untracked.InputTag("hbhesmear")
process.myanunfilt.recHitEscaleMaxGeV = cms.double(350.5)
process.myanunfilt.recHitTscaleNbins  = cms.int32(201)
process.myanunfilt.recHitTscaleMinNs  = cms.double(-10.5)
process.myanunfilt.recHitTscaleMaxNs  = cms.double(40.5)
process.myanunfilt.minHitGeVHB        = cms.double(1.0)
process.myanunfilt.minHitGeVHE        = cms.double(1.0)
process.myanunfilt.tgtTwrId           = cms.vint32(1,1,1)
process.myanunfilt.runDescription     = cms.untracked.string('PionGun')

#-----------------------------
# Hcal Conditions: from Global Conditions Tag
#-----------------------------
process.load("Configuration.StandardSequences.FakeConditions_cff")
#process.GlobalTag.connect = 'frontier://Frontier/CMS_COND_21X_GLOBALTAG'
#process.GlobalTag.globaltag = 'CRAFT_V4P::All' # or any other appropriate
#process.prefer("GlobalTag")

#process.p = cms.Path(process.hbhesmear*process.myanunfilt)
process.p = cms.Path(process.myanunfilt)
