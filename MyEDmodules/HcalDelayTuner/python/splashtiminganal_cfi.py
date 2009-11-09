import FWCore.ParameterSet.Config as cms

timeanal = cms.EDAnalyzer('SplashTimingAnalyzer',
                             
    eventDataPset = cms.untracked.PSet(
      hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
      verbose         = cms.untracked.bool(False)
    ),

    subdet            = cms.untracked.string("HB"),
    splashPlusZside   = cms.bool(True),
    runDescription    = cms.untracked.string(""),
    globalTimeOffset  = cms.double(0.0),
    badEventList      = cms.vint32(),

    SubdetPars = cms.PSet(
      minHitGeV          = cms.double(25),
      timeWindowMinNS    = cms.double(-100.0),
      timeWindowMaxNS    = cms.double(100.0),
      recHitEscaleMinGeV = cms.double(-0.5),
      recHitEscaleMaxGeV = cms.double(1000.5),
      recHitTscaleNbins  = cms.int32(201),
      recHitTscaleMinNs  = cms.double(-100.5),
      recHitTscaleMaxNs  = cms.double(100.5)
    ),

    bottomfeeder = cms.untracked.int32(0xbadf00d)
)
