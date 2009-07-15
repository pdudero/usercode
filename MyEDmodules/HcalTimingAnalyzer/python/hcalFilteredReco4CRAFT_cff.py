import FWCore.ParameterSet.Config as cms


from MyEDmodules.HcalTimingAnalyzer.hcalFilteredReco_cff import *

hfrhfilt06ns.detIds2Mask = cms.vint32(-32,45,2)
hfrhfilt10ns.detIds2Mask = cms.vint32(-32,45,2)
hfrhfilt1ts.detIds2Mask  = cms.vint32(-32,45,2)
hfrhfilt4ts.detIds2Mask  = cms.vint32(-32,45,2)
