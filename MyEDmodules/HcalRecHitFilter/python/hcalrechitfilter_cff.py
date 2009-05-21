import FWCore.ParameterSet.Config as cms

hbheRHfilter = cms.EDProducer('HcalRecHitFilter',
     hbheLabel     = cms.untracked.InputTag("hbhereco"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HBHE"),
     minRecoTimeNs = cms.double(-1e99),
     maxRecoTimeNs = cms.double(1e99)
)

hfRHfilter = cms.EDProducer('HcalRecHitFilter',
     hfLabel       = cms.untracked.InputTag("hfreco"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HF"),
     minRecoTimeNs = cms.double(-1e99),
     maxRecoTimeNs = cms.double(1e99)
)

hoRHfilter = cms.EDProducer('HcalRecHitFilter',
     hoLabel       = cms.untracked.InputTag("horeco"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HO"),
     minRecoTimeNs = cms.double(-1e99),
     maxRecoTimeNs = cms.double(1e99)
)
