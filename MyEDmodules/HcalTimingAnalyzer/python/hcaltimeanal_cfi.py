import FWCore.ParameterSet.Config as cms

myan = cms.EDAnalyzer('HcalTimingAnalyzer',
    eventDataPset = cms.untracked.PSet(
      hbheDigiLabel   = cms.untracked.InputTag("hcalDigis"),
      hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
      hfRechitLabel   = cms.untracked.InputTag("hfreco"),
      hoRechitLabel   = cms.untracked.InputTag("horeco"),
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

    hcalRecHitEscaleMinGeV = cms.double(-10.5),
    hcalRecHitEscaleMaxGeV = cms.double(450.5),

    ecalRecHitTscaleNbins  = cms.int32(401),
    ecalRecHitTscaleMinNs  = cms.double(-100.5),
    ecalRecHitTscaleMaxNs  = cms.double(100.5),

    hcalRecHitTscaleNbins  = cms.int32(401),
    hcalRecHitTscaleMinNs  = cms.double(-100.5),
    hcalRecHitTscaleMaxNs  = cms.double(100.5),

    simHitTscaleNbins  = cms.int32(501),
    simHitTscaleMinNs  = cms.double(-10.5),
    simHitTscaleMaxNs  = cms.double(40.5),

    simHitEnergyMinGeVthreshold = cms.double(0.5)
)
