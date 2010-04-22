import FWCore.ParameterSet.Config as cms

hbtimeanal = cms.EDAnalyzer('BeamTimingAnalyzer',
                             
    eventDataPset = cms.untracked.PSet(
#     hbheDigiLabel   = cms.untracked.InputTag("hcalDigis"),
      hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
      vertexLabel     = cms.untracked.InputTag("offlinePrimaryVertices"),
      verbose         = cms.untracked.bool(False)
    ),
                             
    TrecoParams = cms.PSet(
      firstSample     = cms.int32(3),
      numSamples      = cms.int32(4),
      preSamples      = cms.int32(4)
    ),

    normalizeDigis     = cms.bool(False),
    doPerChannel       = cms.bool(True),
    minEvents4avgT     = cms.int32(10),
    subdet             = cms.untracked.string("HB"),
    runDescription     = cms.untracked.string(""),
    globalTimeOffset   = cms.double(0.0),
    badEventList       = cms.vint32(),
    acceptedBxNums     = cms.vint32(),
    acceptedPkTSnumbers= cms.vint32(),
    detIds2mask        = cms.vint32(),
    globalRecHitFlagMask = cms.int32(0),
    minHitGeV          = cms.double(2),
    maxHitGeV          = cms.double(9e99),
    timeWindowMinNS    = cms.double(-100.0),
    timeWindowMaxNS    = cms.double(100.0),
    recHitEscaleMinGeV = cms.double(-5.5),
    recHitEscaleMaxGeV = cms.double(500.5),
    recHitTscaleNbins  = cms.int32(201),
    recHitTscaleMinNs  = cms.double(-100.5),
    recHitTscaleMaxNs  = cms.double(100.5),
    maxEventNum2plot   = cms.int32(1),
    unravelHBatIeta    = cms.int32(17), # as in, _don't_ by default
    CorrectionsList    = cms.VPSet(),

    bottomfeeder = cms.untracked.int32(0xbadf00d)
)

hetimeanal = hbtimeanal.clone(subdet=cms.untracked.string("HE"))
hotimeanal = hbtimeanal.clone(subdet=cms.untracked.string("HO"))
hotimeanal.eventDataPset.hbheRechitLabel=cms.untracked.InputTag("")
hftimeanal = hotimeanal.clone(subdet=cms.untracked.string("HF"))
hotimeanal.eventDataPset.hoRechitLabel=cms.untracked.InputTag("horeco")
hftimeanal.eventDataPset.hfRechitLabel=cms.untracked.InputTag("hfreco")
#hotimeanal.eventDataPset.hoDigiLabel=cms.untracked.InputTag("hcalDigis")
#hftimeanal.eventDataPset.hfDigiLabel=cms.untracked.InputTag("hcalDigis")

zdctimeanal = hftimeanal.clone(subdet=cms.untracked.string("ZDC"))
zdctimeanal.eventDataPset.hfDigiLabel=cms.untracked.InputTag("")
zdctimeanal.eventDataPset.zdcDigiLabel=cms.untracked.InputTag("hcalDigis")
zdctimeanal.eventDataPset.hfRechitLabel=cms.untracked.InputTag("")
zdctimeanal.eventDataPset.zdcRechitLabel=cms.untracked.InputTag("zdcreco")
