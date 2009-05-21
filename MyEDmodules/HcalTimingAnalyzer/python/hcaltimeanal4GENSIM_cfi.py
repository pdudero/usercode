import FWCore.ParameterSet.Config as cms

from MyEDmodules.HcalTimingAnalyzer.hcaltimeanal_cfi import *

myan.eventDataPset.simHitLabel   = cms.untracked.InputTag("g4SimHits","HcalHits")
myan.simHitTscaleNbins           = cms.int32(501)
myan.simHitTscaleMinNs           = cms.double(-10.5)
myan.simHitTscaleMaxNs           = cms.double(40.5)
myan.simHitEnergyMinGeVthreshold = cms.double(0.5)
