import FWCore.ParameterSet.Config as cms

from MyEDmodules.HcalTimingAnalyzer.hcalFilteredReco_cff import *

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

hbherecofilt6ns.hbheLabel  = cms.untracked.InputTag("hbhesmear")
hbherecofilt10ns.hbheLabel = cms.untracked.InputTag("hbhesmear")
hbherecofilt1ts.hbheLabel  = cms.untracked.InputTag("hbhesmear")
hbherecofilt4ts.hbheLabel  = cms.untracked.InputTag("hbhesmear")

myanraw = myan.clone()
myanunfilt.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbhesmear")

allMCfilts = cms.Sequence(myanraw+allfilts)
