import FWCore.ParameterSet.Config as cms

from MyEDmodules.HcalTimingAnalyzer.hcalFilteredReco_cff import *

hfrecofilt6ns.detIds2Mask  = cms.vint32(-32,45,2)
hfrecofilt10ns.detIds2Mask = cms.vint32(-32,45,2)
hfrecofilt1ts.detIds2Mask  = cms.vint32(-32,45,2)
hfrecofilt4ts.detIds2Mask  = cms.vint32(-32,45,2)
