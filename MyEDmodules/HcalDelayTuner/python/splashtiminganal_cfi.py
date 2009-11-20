import FWCore.ParameterSet.Config as cms

hbtimeanal = cms.EDAnalyzer('SplashTimingAnalyzer',
                             
    eventDataPset = cms.untracked.PSet(
      hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
      verbose         = cms.untracked.bool(False)
    ),

    subdet            = cms.untracked.string("HB"),
    splashPlusZside   = cms.bool(True),
    runDescription    = cms.untracked.string(""),
    globalTimeOffset  = cms.double(0.0),
    badEventList      = cms.vint32(),
    acceptedBxNums    = cms.vint32(),
    detIds2mask       = cms.vint32(),
    globalRecHitFlagMask = cms.int32(0),
    minHitGeV          = cms.double(25),
    timeWindowMinNS    = cms.double(-100.0),
    timeWindowMaxNS    = cms.double(100.0),
    recHitEscaleMinGeV = cms.double(-0.5),
    recHitEscaleMaxGeV = cms.double(1000.5),
    recHitTscaleNbins  = cms.int32(201),
    recHitTscaleMinNs  = cms.double(-100.5),
    recHitTscaleMaxNs  = cms.double(100.5),
    maxEventNum2plot   = cms.int32(1),

    CorrectionsList    = cms.VPSet(),

    bottomfeeder = cms.untracked.int32(0xbadf00d)
)

hetimeanal = hbtimeanal.clone(subdet=cms.untracked.string("HE"))
hotimeanal = hbtimeanal.clone(subdet=cms.untracked.string("HO"))
hotimeanal.eventDataPset.hbheRechitLabel=cms.untracked.InputTag("")
hftimeanal = hotimeanal.clone(subdet=cms.untracked.string("HF"))
hotimeanal.eventDataPset.hoRechitLabel=cms.untracked.InputTag("horeco")
hftimeanal.eventDataPset.hfRechitLabel=cms.untracked.InputTag("hfreco")
