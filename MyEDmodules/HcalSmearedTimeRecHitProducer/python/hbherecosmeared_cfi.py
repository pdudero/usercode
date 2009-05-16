import FWCore.ParameterSet.Config as cms

hbherecosmeared = cms.EDProducer('HcalSmearedTimeRecHitProducer',
   enableGuruParameters = cms.bool(False),
   recHitLabel          = cms.InputTag("hbhereco"),
   Subdetector          = cms.string("HBHE")
)
