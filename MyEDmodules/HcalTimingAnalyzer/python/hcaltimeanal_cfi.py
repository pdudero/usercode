import FWCore.ParameterSet.Config as cms

myan = cms.EDAnalyzer('HcalTimingAnalyzer',
    eventNumbers    = cms.vint32(),
    hbheDigiLabel   = cms.untracked.InputTag("hcalDigis"),
    hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
    hfRechitLabel   = cms.untracked.InputTag("hfreco"),
    metLabel        = cms.untracked.InputTag("metNoHF"),
                      
    minHitGeVHB  = cms.double(0.5),
    minHitGeVHE  = cms.double(0.6),  # = avg (0.5 single width, 0.7 double)
    minHitGeVHO  = cms.double(0.5),
    minHitGeVHF1 = cms.double(1.2),
    minHitGeVHF2 = cms.double(1.8),
                      
    recHitEscaleMinGeV = cms.double(-50.0),
    recHitEscaleMaxGeV = cms.double(100.0)
)
