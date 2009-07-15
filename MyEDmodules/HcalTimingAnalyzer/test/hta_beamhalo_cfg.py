import FWCore.ParameterSet.Config as cms

process = cms.Process("MyAnal")

process.load("Configuration.StandardSequences.Geometry_cff")
process.load("RecoJets.Configuration.CaloTowersES_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")

process.MessageLogger.cerr.default.limit = cms.untracked.int32(1000)
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(100)

#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

process.source = cms.Source("PoolSource",
   # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring( # 'myfile.root'
 '/store/data/BeamCommissioning08/BeamHalo/RECO/v1/000/063/463/FE4DE6E7-2487-DD11-ADD0-000423D6AF24.root'
                            )
)

#process.load("MyEDmodules.HcalTimingAnalyzer.in_cff")

process.TFileService = cms.Service("TFileService",
     closeFileFast = cms.untracked.bool(True),
     fileName = cms.string('hta-out.root')
)

process.load("MyEDmodules.HcalTimingAnalyzer.hcalFilteredReco4CRAFT_cff")

#process.myanunfilt.ecalRecHitTscaleNbins    = cms.int32(201)
#process.myanunfilt.ecalRecHitTscaleMinNs    = cms.double(-50.5)
#process.myanunfilt.ecalRecHitTscaleMaxNs    = cms.double(50.5)
#process.myanunfilt.hcalRecHitTscaleNbins    = cms.int32(201)
#process.myanunfilt.hcalRecHitTscaleMinNs    = cms.double(-20.5)
#process.myanunfilt.hcalRecHitTscaleMaxNs    = cms.double(80.5)

#process.myanfilt06ns.ecalRecHitTscaleNbins  = cms.int32(201)
#process.myanfilt06ns.ecalRecHitTscaleMinNs  = cms.double(-50.5)
#process.myanfilt06ns.ecalRecHitTscaleMaxNs  = cms.double(50.5)
#process.myanfilt06ns.hcalRecHitTscaleNbins  = cms.int32(201)
#process.myanfilt06ns.hcalRecHitTscaleMinNs  = cms.double(-20.5)
#process.myanfilt06ns.hcalRecHitTscaleMaxNs  = cms.double(80.5)

#process.myanfilt10ns.ecalRecHitTscaleNbins  = cms.int32(201)
#process.myanfilt10ns.ecalRecHitTscaleMinNs  = cms.double(-50.5)
#process.myanfilt10ns.ecalRecHitTscaleMaxNs  = cms.double(50.5)
#process.myanfilt10ns.hcalRecHitTscaleNbins  = cms.int32(201)
#process.myanfilt10ns.hcalRecHitTscaleMinNs  = cms.double(-20.5)
#process.myanfilt10ns.hcalRecHitTscaleMaxNs  = cms.double(80.5)

#process.myanfilt1ts.ecalRecHitTscaleNbins   = cms.int32(201)
#process.myanfilt1ts.ecalRecHitTscaleMinNs   = cms.double(-50.5)
#process.myanfilt1ts.ecalRecHitTscaleMaxNs   = cms.double(50.5)
#process.myanfilt1ts.hcalRecHitTscaleNbins   = cms.int32(201)
#process.myanfilt1ts.hcalRecHitTscaleMinNs   = cms.double(-20.5)
#process.myanfilt1ts.hcalRecHitTscaleMaxNs   = cms.double(80.5)

#process.myanfilt4ts.ecalRecHitTscaleNbins   = cms.int32(201)
#process.myanfilt4ts.ecalRecHitTscaleMinNs   = cms.double(-50.5)
#process.myanfilt4ts.ecalRecHitTscaleMaxNs   = cms.double(50.5)
#process.myanfilt4ts.hcalRecHitTscaleNbins   = cms.int32(201)
#process.myanfilt4ts.hcalRecHitTscaleMinNs   = cms.double(-20.5)
#process.myanfilt4ts.hcalRecHitTscaleMaxNs   = cms.double(80.5)

#-----------------------------
# Hcal Conditions: from Global Conditions Tag
#-----------------------------
#process.load("Configuration.StandardSequences.FakeConditions_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.connect = 'frontier://Frontier/CMS_COND_21X_GLOBALTAG'
process.GlobalTag.globaltag = 'IDEAL_V11::All' # or any other appropriate
process.prefer("GlobalTag")

process.p = cms.Path(process.mixedFilts)
#process.p = cms.Path(process.allSQfilts)

#process.out = cms.OutputModule("PoolOutputModule",
#    SelectEvents = cms.untracked.PSet(
#        SelectEvents = cms.vstring('p')
#    ),
#    outputCommands = cms.untracked.vstring(
#        "drop *",
#        "keep *_*_*_MyAnal",
#        "keep recoCaloMETs_*_*_*",
#        "keep *_hbhereco_*_*",
#        "keep *_hfreco_*_*",
#        "keep *_horeco_*_*",
#        "keep *_towerMaker_*_*"
#    ),
#   
#    fileName = cms.untracked.string('hta-pool.root')
#)

#process.o = cms.EndPath(process.out)
