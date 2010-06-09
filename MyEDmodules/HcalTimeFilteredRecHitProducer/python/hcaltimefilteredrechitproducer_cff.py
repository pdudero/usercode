import FWCore.ParameterSet.Config as cms

hbheRHfilter = cms.EDProducer('HcalTimeFilteredRecHitProducer',
     hbheLabel          = cms.untracked.InputTag("hbhereco"),
     detIds2Mask        = cms.vint32(),
     tsmearEnvelope     = cms.vdouble(),
     tfilterEnvelope    = cms.vdouble(),
     timeWindowCenterNs = cms.double(0.0),
     timeWindowGain     = cms.double(1.0),
     Subdetector        = cms.string("HBHE"),
     timeShiftNs        = cms.double(0.0),
     flagFilterMask     = cms.int32(0)
)

hfRHfilter = cms.EDProducer('HcalTimeFilteredRecHitProducer',
     hfLabel            = cms.untracked.InputTag("hfreco"),
     detIds2Mask        = cms.vint32(),
     tsmearEnvelope     = cms.vdouble(),
     tfilterEnvelope    = cms.vdouble(),
     timeWindowCenterNs = cms.double(0.0),
     timeWindowGain     = cms.double(1.0),
     Subdetector        = cms.string("HF"),
     timeShiftNs        = cms.double(0.0),
     flagFilterMask     = cms.int32(0)
)

hoRHfilter = cms.EDProducer('HcalTimeFilteredRecHitProducer',
     hoLabel            = cms.untracked.InputTag("horeco"),
     detIds2Mask        = cms.vint32(),
     tsmearEnvelope     = cms.vdouble(),
     tfilterEnvelope    = cms.vdouble(),
     timeWindowCenterNs = cms.double(0.0),
     timeWindowGain     = cms.double(1.0),
     Subdetector        = cms.string("HO"),
     timeShiftNs        = cms.double(0.0),
     flagFilterMask     = cms.int32(0)
)

rhProfilingPset = cms.untracked.PSet(
    thresholds      = cms.untracked.vdouble()
    )
