import FWCore.ParameterSet.Config as cms

hbhesmear = cms.EDProducer('HcalRecHitFilter',
     hbheLabel     = cms.untracked.InputTag("hbhereco"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble( #Energy   ss
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
                                  350.00,  1.043),
     Subdetector   = cms.string("HBHE"),
     minRecoTimeNs = cms.double(-1e99),
     maxRecoTimeNs = cms.double(1e99)
)

hbherecofilt6 = cms.EDProducer('HcalRecHitFilter',
     hbheLabel     = cms.untracked.InputTag("hbhesmear"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HBHE"),
     minRecoTimeNs = cms.double(11.0),
     maxRecoTimeNs = cms.double(17.0)
)

horecofilt6 = cms.EDProducer('HcalRecHitFilter',
     hoLabel       = cms.untracked.InputTag("horeco"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HO"),
     minRecoTimeNs = cms.double(11.0),
     maxRecoTimeNs = cms.double(17.0)
)

hfrecofilt6 = cms.EDProducer('HcalRecHitFilter',
     hfLabel       = cms.untracked.InputTag("hfreco"),
     detIds2Mask   = cms.vint32(-32,45,2),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HF"),
     minRecoTimeNs = cms.double(11.0),
     maxRecoTimeNs = cms.double(17.0)
)

hbherecofilt10 = cms.EDProducer('HcalRecHitFilter',
     hbheLabel     = cms.untracked.InputTag("hbhesmear"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HBHE"),
     minRecoTimeNs = cms.double(9.0),
     maxRecoTimeNs = cms.double(19.0)
)

horecofilt10 = cms.EDProducer('HcalRecHitFilter',
     hoLabel       = cms.untracked.InputTag("horeco"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HO"),
     minRecoTimeNs = cms.double(9.0),
     maxRecoTimeNs = cms.double(19.0)
)

hfrecofilt10 = cms.EDProducer('HcalRecHitFilter',
     hfLabel       = cms.untracked.InputTag("hfreco"),
     detIds2Mask   = cms.vint32(-32,45,2),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HF"),
     minRecoTimeNs = cms.double(9.0),
     maxRecoTimeNs = cms.double(19.0)
)

hbherecofilt1ts = cms.EDProducer('HcalRecHitFilter',
     hbheLabel     = cms.untracked.InputTag("hbhesmear"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HBHE"),
     minRecoTimeNs = cms.double(0.0),
     maxRecoTimeNs = cms.double(25.0)
)

horecofilt1ts = cms.EDProducer('HcalRecHitFilter',
     hoLabel       = cms.untracked.InputTag("horeco"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HO"),
     minRecoTimeNs = cms.double(0.0),
     maxRecoTimeNs = cms.double(25.0)
)

hfrecofilt1ts = cms.EDProducer('HcalRecHitFilter',
     hfLabel       = cms.untracked.InputTag("hfreco"),
     detIds2Mask   = cms.vint32(-32,45,2),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HF"),
     minRecoTimeNs = cms.double(0.0),
     maxRecoTimeNs = cms.double(25.0)
)

hbherecofilt4ts = cms.EDProducer('HcalRecHitFilter',
     hbheLabel     = cms.untracked.InputTag("hbhesmear"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HBHE"),
     minRecoTimeNs = cms.double(0.0),
     maxRecoTimeNs = cms.double(100.0)
)

horecofilt4ts = cms.EDProducer('HcalRecHitFilter',
     hoLabel       = cms.untracked.InputTag("horeco"),
     detIds2Mask   = cms.vint32(),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HO"),
     minRecoTimeNs = cms.double(0.0),
     maxRecoTimeNs = cms.double(100.0)
)

hfrecofilt4ts = cms.EDProducer('HcalRecHitFilter',
     hfLabel       = cms.untracked.InputTag("hfreco"),
     detIds2Mask   = cms.vint32(-32,45,2),
     smearEnvelope = cms.vdouble(),
     Subdetector   = cms.string("HF"),
     minRecoTimeNs = cms.double(0.0),
     maxRecoTimeNs = cms.double(100.0)
)

recofilt6   = cms.Sequence(hbherecofilt6+horecofilt6+hfrecofilt6)
recofilt10  = cms.Sequence(hbherecofilt10+horecofilt10+hfrecofilt10)
recofilt1ts = cms.Sequence(hbherecofilt1ts+horecofilt1ts+hfrecofilt1ts)
recofilt4ts = cms.Sequence(hbherecofilt4ts+horecofilt4ts+hfrecofilt4ts)

#
# rereco met
#
from RecoJets.Configuration.RecoCaloTowersGR_cff import towerMaker
filtTowers6   = towerMaker.clone()
filtTowers6.hbheInput = cms.InputTag("hbherecofilt6")
filtTowers6.hoInput   = cms.InputTag("horecofilt6")
filtTowers6.hfInput   = cms.InputTag("hfrecofilt6")

filtTowers10   = towerMaker.clone()
filtTowers10.hbheInput = cms.InputTag("hbherecofilt10")
filtTowers10.hoInput   = cms.InputTag("horecofilt10")
filtTowers10.hfInput   = cms.InputTag("hfrecofilt10")

filtTowers1ts   = towerMaker.clone()
filtTowers1ts.hbheInput = cms.InputTag("hbherecofilt1ts")
filtTowers1ts.hoInput   = cms.InputTag("horecofilt1ts")
filtTowers1ts.hfInput   = cms.InputTag("hfrecofilt1ts")

filtTowers4ts   = towerMaker.clone()
filtTowers4ts.hbheInput = cms.InputTag("hbherecofilt4ts")
filtTowers4ts.hoInput   = cms.InputTag("horecofilt4ts")
filtTowers4ts.hfInput   = cms.InputTag("hfrecofilt4ts")

from RecoMET.METProducers.CaloMET_cfi import metNoHF
filtmetNoHF6   = metNoHF.clone(src = cms.InputTag("filtTowers6"))
filtmetNoHF10  = metNoHF.clone(src = cms.InputTag("filtTowers10"))
filtmetNoHF1ts = metNoHF.clone(src = cms.InputTag("filtTowers1ts"))
filtmetNoHF4ts = metNoHF.clone(src = cms.InputTag("filtTowers4ts"))

from MyEDmodules.HcalTimingAnalyzer.hcaltimeanal_cfi import *
myanraw      = myan.clone()
myanunfilt   = myan.clone()
myanunfilt.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbhesmear")

myanfilt6    = myan.clone()
myanfilt6.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecofilt6")
myanfilt6.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("hfrecofilt6")
myanfilt6.eventDataPset.twrLabel        = cms.untracked.InputTag("filtTowers6")
myanfilt6.eventDataPset.metLabel        = cms.untracked.InputTag("filtmetNoHF6")

myanfilt10    = myan.clone()
myanfilt10.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecofilt10")
myanfilt10.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("hfrecofilt10")
myanfilt10.eventDataPset.twrLabel        = cms.untracked.InputTag("filtTowers10")
myanfilt10.eventDataPset.metLabel        = cms.untracked.InputTag("filtmetNoHF10")

myanfilt1ts    = myan.clone()
myanfilt1ts.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecofilt1ts")
myanfilt1ts.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("hfrecofilt1ts")
myanfilt1ts.eventDataPset.twrLabel        = cms.untracked.InputTag("filtTowers1ts")
myanfilt1ts.eventDataPset.metLabel        = cms.untracked.InputTag("filtmetNoHF1ts")

myanfilt4ts    = myan.clone()
myanfilt4ts.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecofilt4ts")
myanfilt4ts.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("hfrecofilt4ts")
myanfilt4ts.eventDataPset.twrLabel        = cms.untracked.InputTag("filtTowers4ts")
myanfilt4ts.eventDataPset.metLabel        = cms.untracked.InputTag("filtmetNoHF4ts")

filt6  = cms.Sequence(recofilt6*
                      filtTowers6*
                      filtmetNoHF6*
                      myanfilt6)

filt10 = cms.Sequence(recofilt10*
                      filtTowers10*
                      filtmetNoHF10*
                      myanfilt10)

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
                        filt6+
                        filt10+
                        filt1ts+
                        filt4ts)
