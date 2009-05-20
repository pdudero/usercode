import FWCore.ParameterSet.Config as cms

#--------------------------------------------------
# RECHIT FILTERS

from MyEDmodules.HcalRecHitFilter.hcalrechitfilter_cff import *

hbhesmear = hbheRHfilter.clone()
hbhesmear.smearEnvelope = cms.vdouble( #Energy   ss
                                         4.00,  1.703,
                                         6.50,  1.685,
                                        10.00,  1.756,
                                        13.00,  1.906,
                                        14.50,  1.957,
                                        20.50,  2.142,
                                        23.50,  2.045,
                                        30.00,  1.809,
                                        37.00,  1.664,
                                        44.50,  1.519,
                                        56.00,  1.425,
                                        63.50,  1.348,
                                        81.00,  1.192,
                                        88.50,  1.130,
                                       114.50,  1.144,
                                       175.50,  1.070,
                                       350.00,  1.043)

#--------------------------------------------------

hbherecofilt6ns = hbheRHfilter.clone()
hbherecofilt6ns.hbheLabel     = cms.untracked.InputTag("hbhesmear")
hbherecofilt6ns.minRecoTimeNs = cms.double(11.0)
hbherecofilt6ns.maxRecoTimeNs = cms.double(17.0)

horecofilt6ns = hoRHfilter.clone()
horecofilt6ns.minRecoTimeNs = cms.double(11.0)
horecofilt6ns.maxRecoTimeNs = cms.double(17.0)

hfrecofilt6ns = hfRHfilter.clone()
hfrecofilt6ns.minRecoTimeNs = cms.double(11.0)
hfrecofilt6ns.maxRecoTimeNs = cms.double(17.0)

#--------------------------------------------------

hbherecofilt10ns = hbheRHfilter.clone()
hbherecofilt10ns.hbheLabel     = cms.untracked.InputTag("hbhesmear")
hbherecofilt10ns.minRecoTimeNs = cms.double(9.0)
hbherecofilt10ns.maxRecoTimeNs = cms.double(19.0)

horecofilt10ns = hoRHfilter.clone()
horecofilt10ns.minRecoTimeNs = cms.double(9.0)
horecofilt10ns.maxRecoTimeNs = cms.double(19.0)

hfrecofilt10ns = hfRHfilter.clone()
hfrecofilt10ns.minRecoTimeNs = cms.double(9.0)
hfrecofilt10ns.maxRecoTimeNs = cms.double(19.0)

#--------------------------------------------------

hbherecofilt1ts = hbheRHfilter.clone()
hbherecofilt1ts.hbheLabel     = cms.untracked.InputTag("hbhesmear")
hbherecofilt1ts.minRecoTimeNs = cms.double(0.0)
hbherecofilt1ts.maxRecoTimeNs = cms.double(25.0)

horecofilt1ts = hoRHfilter.clone()
horecofilt1ts.minRecoTimeNs = cms.double(0.0)
horecofilt1ts.maxRecoTimeNs = cms.double(25.0)

hfrecofilt1ts = hfRHfilter.clone()
hfrecofilt1ts.minRecoTimeNs = cms.double(0.0)
hfrecofilt1ts.maxRecoTimeNs = cms.double(25.0)

#--------------------------------------------------

hbherecofilt4ts = hbheRHfilter.clone()
hbherecofilt4ts.hbheLabel     = cms.untracked.InputTag("hbhesmear")
hbherecofilt4ts.minRecoTimeNs = cms.double(0.0)
hbherecofilt4ts.maxRecoTimeNs = cms.double(100.0)

horecofilt4ts = hoRHfilter.clone()
horecofilt4ts.minRecoTimeNs = cms.double(0.0)
horecofilt4ts.maxRecoTimeNs = cms.double(100.0)

hfrecofilt4ts = hfRHfilter.clone()
hfrecofilt4ts.minRecoTimeNs = cms.double(0.0)
hfrecofilt4ts.maxRecoTimeNs = cms.double(100.0)

#--------------------------------------------------
# RERECO TOWERS

from RecoJets.Configuration.RecoCaloTowersGR_cff import towerMaker
filtTowers6ns  = towerMaker.clone()
filtTowers6ns.hbheInput  = cms.InputTag("hbherecofilt6ns")
filtTowers6ns.hoInput    = cms.InputTag("horecofilt6ns")
filtTowers6ns.hfInput    = cms.InputTag("hfrecofilt6ns")

filtTowers10ns = towerMaker.clone()
filtTowers10ns.hbheInput = cms.InputTag("hbherecofilt10ns")
filtTowers10ns.hoInput   = cms.InputTag("horecofilt10ns")
filtTowers10ns.hfInput   = cms.InputTag("hfrecofilt10ns")

filtTowers1ts  = towerMaker.clone()
filtTowers1ts.hbheInput  = cms.InputTag("hbherecofilt1ts")
filtTowers1ts.hoInput    = cms.InputTag("horecofilt1ts")
filtTowers1ts.hfInput    = cms.InputTag("hfrecofilt1ts")

filtTowers4ts  = towerMaker.clone()
filtTowers4ts.hbheInput  = cms.InputTag("hbherecofilt4ts")
filtTowers4ts.hoInput    = cms.InputTag("horecofilt4ts")
filtTowers4ts.hfInput    = cms.InputTag("hfrecofilt4ts")

#--------------------------------------------------
# RERECO MET

from RecoMET.METProducers.CaloMET_cfi import metNoHF
filtmetNoHF6ns  = metNoHF.clone(src = cms.InputTag("filtTowers6ns"))
filtmetNoHF10ns = metNoHF.clone(src = cms.InputTag("filtTowers10ns"))
filtmetNoHF1ts  = metNoHF.clone(src = cms.InputTag("filtTowers1ts"))
filtmetNoHF4ts  = metNoHF.clone(src = cms.InputTag("filtTowers4ts"))

#--------------------------------------------------
# ANALYZERS:

from MyEDmodules.HcalTimingAnalyzer.hcaltimeanal_cfi import *
myanraw      = myan.clone()
myanunfilt   = myan.clone()
myanunfilt.eventDataPset.hbheRechitLabel   = cms.untracked.InputTag("hbhesmear")

myanfilt6ns  = myan.clone()
myanfilt6ns.eventDataPset.hbheRechitLabel  = cms.untracked.InputTag("hbherecofilt6ns")
myanfilt6ns.eventDataPset.hfRechitLabel    = cms.untracked.InputTag("hfrecofilt6ns")
myanfilt6ns.eventDataPset.twrLabel         = cms.untracked.InputTag("filtTowers6ns")
myanfilt6ns.eventDataPset.metLabel         = cms.untracked.InputTag("filtmetNoHF6ns")

myanfilt10ns = myan.clone()
myanfilt10ns.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecofilt10ns")
myanfilt10ns.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("hfrecofilt10ns")
myanfilt10ns.eventDataPset.twrLabel        = cms.untracked.InputTag("filtTowers10ns")
myanfilt10ns.eventDataPset.metLabel        = cms.untracked.InputTag("filtmetNoHF10ns")

myanfilt1ts  = myan.clone()
myanfilt1ts.eventDataPset.hbheRechitLabel  = cms.untracked.InputTag("hbherecofilt1ts")
myanfilt1ts.eventDataPset.hfRechitLabel    = cms.untracked.InputTag("hfrecofilt1ts")
myanfilt1ts.eventDataPset.twrLabel         = cms.untracked.InputTag("filtTowers1ts")
myanfilt1ts.eventDataPset.metLabel         = cms.untracked.InputTag("filtmetNoHF1ts")

myanfilt4ts  = myan.clone()
myanfilt4ts.eventDataPset.hbheRechitLabel  = cms.untracked.InputTag("hbherecofilt4ts")
myanfilt4ts.eventDataPset.hfRechitLabel    = cms.untracked.InputTag("hfrecofilt4ts")
myanfilt4ts.eventDataPset.twrLabel         = cms.untracked.InputTag("filtTowers4ts")
myanfilt4ts.eventDataPset.metLabel         = cms.untracked.InputTag("filtmetNoHF4ts")

#--------------------------------------------------
# SEQUENCES:

recofilt6ns  = cms.Sequence(hbherecofilt6ns+horecofilt6ns+hfrecofilt6ns)
recofilt10ns = cms.Sequence(hbherecofilt10ns+horecofilt10ns+hfrecofilt10ns)
recofilt1ts  = cms.Sequence(hbherecofilt1ts+horecofilt1ts+hfrecofilt1ts)
recofilt4ts  = cms.Sequence(hbherecofilt4ts+horecofilt4ts+hfrecofilt4ts)

filt6ns  = cms.Sequence(recofilt6ns*
                        filtTowers6ns*
                        filtmetNoHF6ns*
                        myanfilt6ns)

filt10ns = cms.Sequence(recofilt10ns*
                        filtTowers10ns*
                        filtmetNoHF10ns*
                        myanfilt10ns)

filt1ts = cms.Sequence(recofilt1ts*
                       filtTowers1ts*
                       filtmetNoHF1ts*
                       myanfilt1ts)

filt4ts = cms.Sequence(recofilt4ts*
                       filtTowers4ts*
                       filtmetNoHF4ts*
                       myanfilt4ts)

allfilts = cms.Sequence(myanraw+
                        myanunfilt+
                        filt6ns+
                        filt10ns+
                        filt1ts+
                        filt4ts)
