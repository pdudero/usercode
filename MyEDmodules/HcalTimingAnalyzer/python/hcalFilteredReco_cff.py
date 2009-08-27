import FWCore.ParameterSet.Config as cms

#--------------------------------------------------
# RECHIT FILTERS

from MyEDmodules.HcalTimingAnalyzer.hcalRecHitFiltersTS4_cff import *

#--------------------------------------------------
# RERECO TOWERS

from RecoJets.Configuration.RecoCaloTowersGR_cff import towerMaker
filtTowers06ns  = towerMaker.clone()
filtTowers06ns.hbheInput  = cms.InputTag("hbherhfilt06ns")
filtTowers06ns.hoInput    = cms.InputTag("horhfilt06ns")
filtTowers06ns.hfInput    = cms.InputTag("hfrhfilt06ns")

filtTowers10ns = towerMaker.clone()
filtTowers10ns.hbheInput = cms.InputTag("hbherhfilt10ns")
filtTowers10ns.hoInput   = cms.InputTag("horhfilt10ns")
filtTowers10ns.hfInput   = cms.InputTag("hfrhfilt10ns")

filtTowers1ts  = towerMaker.clone()
filtTowers1ts.hbheInput  = cms.InputTag("hbherhfilt1ts")
filtTowers1ts.hoInput    = cms.InputTag("horhfilt1ts")
filtTowers1ts.hfInput    = cms.InputTag("hfrhfilt1ts")

filtTowers4ts  = towerMaker.clone()
filtTowers4ts.hbheInput  = cms.InputTag("hbherhfilt4ts")
filtTowers4ts.hoInput    = cms.InputTag("horhfilt4ts")
filtTowers4ts.hfInput    = cms.InputTag("hfrhfilt4ts")

filtTowersshp            = towerMaker.clone()
filtTowersshp.hbheInput  = cms.InputTag("hbherhfiltshp")
filtTowersshp.hoInput    = cms.InputTag("horhfiltshp")

filtTowersNoiseBit           = towerMaker.clone()
filtTowersNoiseBit.hbheInput = cms.InputTag("hbherhfiltNoiseBit")

filtTowersTimeBit            = towerMaker.clone()
filtTowersTimeBit.hbheInput  = cms.InputTag("hbherhfiltTimeBit")

unfiltTowersShift = towerMaker.clone()
unfiltTowersShift.hbheInput  = cms.InputTag("hbherhshift")
unfiltTowersShift.hoInput    = cms.InputTag("horhshift")
unfiltTowersShift.hfInput    = cms.InputTag("hfrhshift")

#--------------------------------------------------
# RERECO MET

from RecoMET.METProducers.CaloMET_cfi import metNoHF
filtmetNoHF06ns    = metNoHF.clone(src = cms.InputTag("filtTowers06ns"))
filtmetNoHF10ns    = metNoHF.clone(src = cms.InputTag("filtTowers10ns"))
filtmetNoHF1ts     = metNoHF.clone(src = cms.InputTag("filtTowers1ts"))
filtmetNoHF4ts     = metNoHF.clone(src = cms.InputTag("filtTowers4ts"))
filtmetNoHFshp     = metNoHF.clone(src = cms.InputTag("filtTowersshp"))
filtmetNoHFnoise   = metNoHF.clone(src = cms.InputTag("filtTowersNoiseBit"))
filtmetNoHFtime    = metNoHF.clone(src = cms.InputTag("filtTowersTimeBit"))
unfiltmetNoHFshift = metNoHF.clone(src = cms.InputTag("unfiltTowersShift"))

#--------------------------------------------------
# ANALYZERS:

from MyEDmodules.HcalTimingAnalyzer.hcaltimeanal_cfi import *
myanunfilt   = myan.clone()
myanunfilt.eventDataPset.hbheRechitLabel   = cms.untracked.InputTag("hbhereco")

myanunfiltShift   = myan.clone()
myanunfiltShift.eventDataPset.hbheRechitLabel   = cms.untracked.InputTag("hbherhshift")

myanfiltshp  = myan.clone()
myanfiltshp.eventDataPset.hbheRechitLabel  = cms.untracked.InputTag("hbherhfiltshp")
myanfiltshp.eventDataPset.hoRechitLabel    = cms.untracked.InputTag("horhfiltshp")
myanfiltshp.eventDataPset.twrLabel         = cms.untracked.InputTag("filtTowersshp")
myanfiltshp.eventDataPset.metLabel         = cms.untracked.InputTag("filtmetNoHFshp")

myanfilt06ns  = myan.clone()
myanfilt06ns.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfilt06ns")
myanfilt06ns.eventDataPset.hoRechitLabel   = cms.untracked.InputTag("horhfilt06ns")
myanfilt06ns.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("hfrhfilt06ns")
myanfilt06ns.eventDataPset.twrLabel        = cms.untracked.InputTag("filtTowers06ns")
myanfilt06ns.eventDataPset.metLabel        = cms.untracked.InputTag("filtmetNoHF06ns")

myanfilt10ns = myan.clone()
myanfilt10ns.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfilt10ns")
myanfilt10ns.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("hfrhfilt10ns")
myanfilt10ns.eventDataPset.hoRechitLabel   = cms.untracked.InputTag("horhfilt10ns")
myanfilt10ns.eventDataPset.twrLabel        = cms.untracked.InputTag("filtTowers10ns")
myanfilt10ns.eventDataPset.metLabel        = cms.untracked.InputTag("filtmetNoHF10ns")

myanfilt1ts  = myan.clone()
myanfilt1ts.eventDataPset.hbheRechitLabel  = cms.untracked.InputTag("hbherhfilt1ts")
myanfilt1ts.eventDataPset.hoRechitLabel    = cms.untracked.InputTag("horhfilt1ts")
myanfilt1ts.eventDataPset.hfRechitLabel    = cms.untracked.InputTag("hfrhfilt1ts")
myanfilt1ts.eventDataPset.twrLabel         = cms.untracked.InputTag("filtTowers1ts")
myanfilt1ts.eventDataPset.metLabel         = cms.untracked.InputTag("filtmetNoHF1ts")

myanfilt4ts  = myan.clone()
myanfilt4ts.eventDataPset.hbheRechitLabel  = cms.untracked.InputTag("hbherhfilt4ts")
myanfilt4ts.eventDataPset.hoRechitLabel    = cms.untracked.InputTag("horhfilt4ts")
myanfilt4ts.eventDataPset.hfRechitLabel    = cms.untracked.InputTag("hfrhfilt4ts")
myanfilt4ts.eventDataPset.twrLabel         = cms.untracked.InputTag("filtTowers4ts")
myanfilt4ts.eventDataPset.metLabel         = cms.untracked.InputTag("filtmetNoHF4ts")

myanfiltNoise = myan.clone()
myanfiltNoise.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfiltNoiseBit")
myanfiltNoise.eventDataPset.twrLabel        = cms.untracked.InputTag("filtTowersNoiseBit")
myanfiltNoise.eventDataPset.metLabel        = cms.untracked.InputTag("filtmetNoHFnoise")

myanfiltTime = myan.clone()
myanfiltTime.eventDataPset.hbheRechitLabel  = cms.untracked.InputTag("hbherhfiltTimeBit")
myanfiltTime.eventDataPset.twrLabel         = cms.untracked.InputTag("filtTowersTimeBit")
myanfiltTime.eventDataPset.metLabel         = cms.untracked.InputTag("filtmetNoHFtime")

#--------------------------------------------------
# SEQUENCES:

rhfiltshp  = cms.Sequence(hbherhfiltshp+horhfiltshp)
rhshift    = cms.Sequence(hbherhshift+horhshift+hfrhshift)

rhfilt06ns = cms.Sequence(hbherhfilt06ns+horhfilt06ns+hfrhfilt06ns)
rhfilt10ns = cms.Sequence(hbherhfilt10ns+horhfilt10ns+hfrhfilt10ns)
rhfilt1ts  = cms.Sequence(hbherhfilt1ts+horhfilt1ts+hfrhfilt1ts)
rhfilt4ts  = cms.Sequence(hbherhfilt4ts+horhfilt4ts+hfrhfilt4ts)

filt06ns = cms.Sequence(rhfilt06ns*
                        filtTowers06ns*
                        filtmetNoHF06ns*
                        myanfilt06ns)

filt10ns = cms.Sequence(rhfilt10ns*
                        filtTowers10ns*
                        filtmetNoHF10ns*
                        myanfilt10ns)

filt1ts = cms.Sequence(rhfilt1ts*
                       filtTowers1ts*
                       filtmetNoHF1ts*
                       myanfilt1ts)

filt4ts = cms.Sequence(rhfilt4ts*
                       filtTowers4ts*
                       filtmetNoHF4ts*
                       myanfilt4ts)

unfiltShift = cms.Sequence(rhshift*
                           unfiltTowersShift*
                           unfiltmetNoHFshift*
                           myanunfiltShift)

filtshp = cms.Sequence(rhfiltshp*
                       filtTowersshp*
                       filtmetNoHFshp*
                       myanfiltshp)

filtnoise = cms.Sequence(hbherhfiltNoiseBit*
                         filtTowersNoiseBit*
                         filtmetNoHFnoise*
                         myanfiltNoise)

filttime = cms.Sequence(hbherhfiltTimeBit*
                        filtTowersTimeBit*
                        filtmetNoHFtime*
                        myanfiltTime)

allSQfilts = cms.Sequence(myanunfilt+
                          filt06ns+
                          filt10ns+
                          filt1ts+
                          filt4ts)

allSQfiltsWithShift = cms.Sequence(myanunfilt+
                                   unfiltShift+
                                   filt06ns+
                                   filt10ns+
                                   filt1ts+
                                   filt4ts)

#
# combination of square filters (>=1ts)
# and a shaped filter (<1ts)
#
timeFiltSeq = cms.Sequence(filtshp+
                           filt1ts+
                           filt4ts)

timeFiltsWithShift = cms.Sequence(unfiltShift+
                                  filtshp+
                                  filt1ts+
                                  filt4ts)

flagFiltSeq  = cms.Sequence(filtnoise+filttime)
