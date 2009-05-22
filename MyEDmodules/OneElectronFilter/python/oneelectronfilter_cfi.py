import FWCore.ParameterSet.Config as cms

filt = cms.EDFilter('OneElectronFilter',
  sourceElectrons      = cms.untracked.InputTag("eidRobust"),
  elMinPtGeV           = cms.double(15.0)
)
