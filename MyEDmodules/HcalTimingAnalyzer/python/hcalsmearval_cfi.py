import FWCore.ParameterSet.Config as cms

smearval = cms.EDAnalyzer('HcalSmearVal',
    unsmearedDataPset = cms.untracked.PSet(
      hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
      hfRechitLabel   = cms.untracked.InputTag("hfreco"),
      hoRechitLabel   = cms.untracked.InputTag("horeco"),
    ),
    smearedDataPset = cms.untracked.PSet(
      hbheRechitLabel = cms.untracked.InputTag("hbherecosmeared"),
      hfRechitLabel   = cms.untracked.InputTag("hfrecosmeared"),
      hoRechitLabel   = cms.untracked.InputTag("horecosmeared"),
    ),

    minHitGeVHB  = cms.double(0.5),
    minHitGeVHE  = cms.double(0.6),  # = avg (0.5 single width, 0.7 double)
    minHitGeVHO  = cms.double(0.5),
    minHitGeVHF1 = cms.double(1.2),
    minHitGeVHF2 = cms.double(1.8),

    recHitEscaleMinGeV = cms.double(-10.5),
    recHitEscaleMaxGeV = cms.double(400.5),

    deltaTscaleNbins   = cms.int32(21),
    deltaTscaleMinNs   = cms.double(-10.5),
    deltaTscaleMaxNs   = cms.double(10.5),

    recHitTscaleNbins  = cms.int32(201),
    recHitTscaleMinNs  = cms.double(-100.5),
    recHitTscaleMaxNs  = cms.double(100.5),
)
