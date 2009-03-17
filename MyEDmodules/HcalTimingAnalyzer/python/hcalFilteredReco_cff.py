import FWCore.ParameterSet.Config as cms

hbherecofilt6 = cms.EDProducer('HcalRecHitFilter',
     detIds2Mask = cms.vint32(),
     Subdetector = cms.string("HBHE"),
     minRecoTimeNs = cms.double(9.5),
     maxRecoTimeNs = cms.double(15.5)
)

horecofilt6 = cms.EDProducer('HcalRecHitFilter',
     detIds2Mask = cms.vint32(),
     Subdetector = cms.string("HO"),
     minRecoTimeNs = cms.double(9.5),
     maxRecoTimeNs = cms.double(15.5)
)

hfrecofilt6 = cms.EDProducer('HcalRecHitFilter',
     detIds2Mask = cms.vint32(-32,45,2),
     Subdetector = cms.string("HF"),
     minRecoTimeNs = cms.double(9.5),
     maxRecoTimeNs = cms.double(15.5)
)

hbherecofilt10 = cms.EDProducer('HcalRecHitFilter',
     detIds2Mask = cms.vint32(),
     Subdetector = cms.string("HBHE"),
     minRecoTimeNs = cms.double(7.5),
     maxRecoTimeNs = cms.double(17.5)
)

horecofilt10 = cms.EDProducer('HcalRecHitFilter',
     detIds2Mask = cms.vint32(),
     Subdetector = cms.string("HO"),
     minRecoTimeNs = cms.double(7.5),
     maxRecoTimeNs = cms.double(17.5)
)

hfrecofilt10 = cms.EDProducer('HcalRecHitFilter',
     detIds2Mask = cms.vint32(-32,45,2),
     Subdetector = cms.string("HF"),
     minRecoTimeNs = cms.double(7.5),
     maxRecoTimeNs = cms.double(17.5)
)

hbherecofilt25 = cms.EDProducer('HcalRecHitFilter',
     detIds2Mask = cms.vint32(),
     Subdetector = cms.string("HBHE"),
     minRecoTimeNs = cms.double(0.0),
     maxRecoTimeNs = cms.double(25.0)
)

horecofilt25 = cms.EDProducer('HcalRecHitFilter',
     detIds2Mask = cms.vint32(),
     Subdetector = cms.string("HO"),
     minRecoTimeNs = cms.double(0.0),
     maxRecoTimeNs = cms.double(25.0)
)

hfrecofilt25 = cms.EDProducer('HcalRecHitFilter',
     detIds2Mask = cms.vint32(-32,45,2),
     Subdetector = cms.string("HF"),
     minRecoTimeNs = cms.double(0.0),
     maxRecoTimeNs = cms.double(25.0)
)

recofilt6  = cms.Sequence(hbherecofilt6+horecofilt6+hfrecofilt6)
recofilt10 = cms.Sequence(hbherecofilt10+horecofilt10+hfrecofilt10)
recofilt25 = cms.Sequence(hbherecofilt25+horecofilt25+hfrecofilt25)

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

filtTowers25   = towerMaker.clone()
filtTowers25.hbheInput = cms.InputTag("hbherecofilt25")
filtTowers25.hoInput   = cms.InputTag("horecofilt25")
filtTowers25.hfInput   = cms.InputTag("hfrecofilt25")

from RecoMET.METProducers.CaloMET_cfi import metNoHF
filtmetNoHF6   = metNoHF.clone(src = cms.InputTag("filtTowers6"))
filtmetNoHF10  = metNoHF.clone(src = cms.InputTag("filtTowers10"))
filtmetNoHF25  = metNoHF.clone(src = cms.InputTag("filtTowers25"))

from MyEDmodules.HcalTimingAnalyzer.hcaltimeanal_cfi import *
myanunfilt   = myan.clone()
myanfilt6    = myan.clone()
myanfilt6.hbheRechitLabel = cms.untracked.InputTag("hbherecofilt6")
myanfilt6.hfRechitLabel   = cms.untracked.InputTag("hfrecofilt6")
myanfilt6.metLabel        = cms.untracked.InputTag("filtmetNoHF6")

myanfilt10    = myan.clone()
myanfilt10.hbheRechitLabel = cms.untracked.InputTag("hbherecofilt10")
myanfilt10.hfRechitLabel   = cms.untracked.InputTag("hfrecofilt10")
myanfilt10.metLabel        = cms.untracked.InputTag("filtmetNoHF10")

myanfilt25    = myan.clone()
myanfilt25.hbheRechitLabel = cms.untracked.InputTag("hbherecofilt25")
myanfilt25.hfRechitLabel   = cms.untracked.InputTag("hfrecofilt25")
myanfilt25.metLabel        = cms.untracked.InputTag("filtmetNoHF25")

filt6  = cms.Sequence(recofilt6*
                      filtTowers6*
                      filtmetNoHF6*
                      myanfilt6)

filt10 = cms.Sequence(recofilt10*
                      filtTowers10*
                      filtmetNoHF10*
                      myanfilt10)

filt25 = cms.Sequence(recofilt25*
                      filtTowers25*
                      filtmetNoHF25*
                      myanfilt25)

