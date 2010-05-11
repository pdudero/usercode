import FWCore.ParameterSet.Config as cms

hfsig2noise = cms.EDAnalyzer('HFsignal2noiseAnalyzer',
    eventDataPset = cms.untracked.PSet(
    hfDigiLabel   = cms.untracked.InputTag("hcalDigis"),
    )
)
