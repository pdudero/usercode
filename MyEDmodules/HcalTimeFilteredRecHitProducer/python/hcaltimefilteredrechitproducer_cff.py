import FWCore.ParameterSet.Config as cms

hbheRHfilter = cms.EDProducer('HcalTimeFilteredRecHitProducer',

# for generating histograms that profile the number of hits in/out
# of time per event as a function of energy thresholds in this array:
#                              
#rhProfilingPset = cms.untracked.PSet(
#    thresholds      = cms.untracked.vdouble()
#    )

     hbheLabel          = cms.untracked.InputTag("hbhereco"),
     detIds2Mask        = cms.vint32(),
     tfilterEnvelope    = cms.vdouble(),
     timeWindowCenterNs = cms.double(0.0),
     timeWindowGain     = cms.double(1.0),
     Subdetector        = cms.string("HBHE"),
     flagFilterMask     = cms.int32(0)
)

hfRHfilter = cms.EDProducer('HcalTimeFilteredRecHitProducer',
#rhProfilingPset = cms.untracked.PSet(
#    thresholds      = cms.untracked.vdouble()
#    )
     hfLabel            = cms.untracked.InputTag("hfreco"),
     detIds2Mask        = cms.vint32(),
     tfilterEnvelope    = cms.vdouble(),
     timeWindowCenterNs = cms.double(0.0),
     timeWindowGain     = cms.double(1.0),
     Subdetector        = cms.string("HF"),
     flagFilterMask     = cms.int32(0)
)

hoRHfilter = cms.EDProducer('HcalTimeFilteredRecHitProducer',
#rhProfilingPset = cms.untracked.PSet(
#    thresholds      = cms.untracked.vdouble()
#    )
     hoLabel            = cms.untracked.InputTag("horeco"),
     detIds2Mask        = cms.vint32(),
     tfilterEnvelope    = cms.vdouble(),
     timeWindowCenterNs = cms.double(0.0),
     timeWindowGain     = cms.double(1.0),
     Subdetector        = cms.string("HO"),
     flagFilterMask     = cms.int32(0)
)
