import FWCore.ParameterSet.Config as cms

myan = cms.EDAnalyzer('HcalTimingAnalyzer',
    eventDataPset = cms.untracked.PSet(
      hbheDigiLabel   = cms.untracked.InputTag("hcalDigis"),
      hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
      hfRechitLabel   = cms.untracked.InputTag("hfreco"),
      simHitLabel     = cms.untracked.InputTag("g4SimHits","HcalHits"),
      metLabel        = cms.untracked.InputTag("metNoHF"),
      twrLabel        = cms.untracked.InputTag("towerMaker")
    ),

    tgtTwrId     = cms.vint32(),
    eventNumbers = cms.vint32(),

    minHitGeVHB  = cms.double(0.5),
    minHitGeVHE  = cms.double(0.6),  # = avg (0.5 single width, 0.7 double)
    minHitGeVHO  = cms.double(0.5),
    minHitGeVHF1 = cms.double(1.2),
    minHitGeVHF2 = cms.double(1.8),

    recHitEscaleMinGeV = cms.double(-0.5),
    recHitEscaleMaxGeV = cms.double(400.5),
    recHitTscaleNbins  = cms.int32(201),
    recHitTscaleMinNs  = cms.double(-100.5),
    recHitTscaleMaxNs  = cms.double(100.5),

    simHitTscaleNbins  = cms.int32(501),
    simHitTscaleMinNs  = cms.double(-10.5),
    simHitTscaleMaxNs  = cms.double(40.5),

    simHitEnergyMinGeVthreshold = cms.double(0.5)
)
