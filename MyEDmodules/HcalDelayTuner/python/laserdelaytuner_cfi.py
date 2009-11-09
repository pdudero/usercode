import FWCore.ParameterSet.Config as cms

tundelayser = cms.EDAnalyzer('LaserDelayTuner',
                             
    eventDataPset = cms.untracked.PSet(
      fedRawDataLabel = cms.untracked.InputTag("source"),
      laserDigiLabel  = cms.untracked.InputTag("hcalLaserReco"),
      hbheDigiLabel   = cms.untracked.InputTag("hcalDigis"),
      hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
      hfRechitLabel   = cms.untracked.InputTag("hfreco"),
      hoRechitLabel   = cms.untracked.InputTag("horeco"),
      verbose         = cms.untracked.bool(False)
    ),
                             
    TDCpars = cms.untracked.PSet(
      TDCCutCenter = cms.untracked.double(1142),
      TDCCutWindow = cms.untracked.double(6),
      CorrectedTimeModCeiling = cms.untracked.int32(9999),
      TimeModCeiling = cms.untracked.int32(9999)
    ),

    HBpars = cms.PSet(
      minHitGeV          = cms.double(25), # from Luis' program
      recHitEscaleMinGeV = cms.double(-0.5),
      recHitEscaleMaxGeV = cms.double(400.5),
      recHitTscaleNbins  = cms.int32(201),
      recHitTscaleMinNs  = cms.double(-100.5),
      recHitTscaleMaxNs  = cms.double(100.5)
    ),

    HEpars = cms.PSet(
      minHitGeV          = cms.double(25), # from Luis' program
      recHitEscaleMinGeV = cms.double(-0.5),
      recHitEscaleMaxGeV = cms.double(400.5),
      recHitTscaleNbins  = cms.int32(201),
      recHitTscaleMinNs  = cms.double(-100.5),
      recHitTscaleMaxNs  = cms.double(100.5)
    ),

    HOpars = cms.PSet(
      minHitGeV          = cms.double(25), # from Luis' program
      recHitEscaleMinGeV = cms.double(-0.5),
      recHitEscaleMaxGeV = cms.double(400.5),
      recHitTscaleNbins  = cms.int32(201),
      recHitTscaleMinNs  = cms.double(-100.5),
      recHitTscaleMaxNs  = cms.double(100.5)
    ),
    HFpars = cms.PSet(
      minHitGeV          = cms.double(25), # from Luis' program
      recHitEscaleMinGeV = cms.double(-0.5),
      recHitEscaleMaxGeV = cms.double(400.5),
      recHitTscaleNbins  = cms.int32(201),
      recHitTscaleMinNs  = cms.double(-100.5),
      recHitTscaleMaxNs  = cms.double(100.5)
    ),

    bottomfeeder = cms.untracked.int32(0xbadf00d)
)
