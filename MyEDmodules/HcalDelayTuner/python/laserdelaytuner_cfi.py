import FWCore.ParameterSet.Config as cms

hbdelayser = cms.EDAnalyzer('LaserTimingAnalyzer',
    subdet            = cms.untracked.string("HB"),
    runDescription    = cms.untracked.string(""),
    globalTimeOffset  = cms.double(0.0),
    globalRecHitFlagMask = cms.int32(0),
    minHitGeV          = cms.double(25),
    timeWindowMinNS    = cms.double(-100.0),
    timeWindowMaxNS    = cms.double(100.0),
    recHitEscaleMinGeV = cms.double(-0.5),
    recHitEscaleMaxGeV = cms.double(400.5),
    recHitTscaleNbins  = cms.int32(201),
    recHitTscaleMinNs  = cms.double(-100.5),
    recHitTscaleMaxNs  = cms.double(100.5),
    maxEventNum2plot   = cms.int32(1000),
    CorrectionsList    = cms.VPSet(),
    detIds2mask        = cms.vint32(),
    normalizeDigis     = cms.bool(False),
    doPerChannel       = cms.bool(True),
                            
    eventDataPset = cms.untracked.PSet(
      fedRawDataLabel = cms.untracked.InputTag(""),
      tbTrigDataLabel = cms.untracked.InputTag("tbunpack"),
      laserDigiLabel  = cms.untracked.InputTag("hcalLaserReco"),
      hbheDigiLabel   = cms.untracked.InputTag("hcalDigis"),
      hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
      hfRechitLabel   = cms.untracked.InputTag(""),
      hoRechitLabel   = cms.untracked.InputTag(""),
      verbose         = cms.untracked.bool(False)
    ),
                             
    TrecoParams = cms.PSet(
      firstSample     = cms.int32(3),
      numSamples      = cms.int32(4),
      preSamples      = cms.int32(4)
    ),
                             
    TDCpars = cms.untracked.PSet(
      TDCCutCenter = cms.untracked.double(1142),
      TDCCutWindow = cms.untracked.double(6),
      CorrectedTimeModCeiling = cms.untracked.int32(9999),
      TimeModCeiling = cms.untracked.int32(9999)
    ),

    bottomfeeder = cms.untracked.int32(0xbadf00d)
)

hedelayser = hbdelayser.clone(subdet=cms.untracked.string("HE"))

hodelayser = hbdelayser.clone(subdet=cms.untracked.string("HO"))
hodelayser.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("") # blank this label for HO,HF
hfdelayser = hodelayser.clone(subdet=cms.untracked.string("HF"))

hodelayser.eventDataPset.hoRechitLabel=cms.untracked.InputTag("horeco")
#hodelayser.eventDataPset.hoDigiLabel=cms.untracked.InputTag("hcalDigis")

hfdelayser.eventDataPset.hfRechitLabel=cms.untracked.InputTag("hfreco")
hfdelayser.eventDataPset.hfDigiLabel=cms.untracked.InputTag("hcalDigis")
