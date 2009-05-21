import FWCore.ParameterSet.Config as cms

from MyEDmodules.HcalTimingAnalyzer.hcaltimeanal_cfi import *
myan.eventDataPset.hbheDigiLabel   = cms.untracked.InputTag("hcalDigis")
