import FWCore.ParameterSet.Config as cms


from MyEDmodules.HcalTimingAnalyzer.hcalFilteredReco_cff import *

hfrhfilt06ns.detIds2Mask = cms.vint32(-32,45,2)
hfrhfilt10ns.detIds2Mask = cms.vint32(-32,45,2)
hfrhfilt1ts.detIds2Mask  = cms.vint32(-32,45,2)
hfrhfilt4ts.detIds2Mask  = cms.vint32(-32,45,2)

#--------------------------------------------------
# Shifted filters -> feed into towers, MET, etc.
# faster than feeding hbherhshift in, can do them in parallel
#
hbherhfiltshp.timeShiftNs = cms.double(25.0)
horhfiltshp.timeShiftNs   = cms.double(25.0)

#--------------------------------------------------
# Shifted "un"filters -> have to be all set up from scratch

unfiltTowersShift            = towerMaker.clone()
unfiltTowersShift.hbheInput  = cms.InputTag("hbherhshift")
unfiltTowersShift.hoInput    = cms.InputTag("horhshift")
unfiltTowersShift.hfInput    = cms.InputTag("hfrhshift")
unfiltmetNoHFshift           = metNoHF.clone(src = "unfiltTowersShift")
myanunfiltShift              = myan.clone()
myanunfiltShift.eventDataPset.hbheRechitLabel  = cms.untracked.InputTag("hbherhshift")
myanunfiltShift.eventDataPset.hoRechitLabel    = cms.untracked.InputTag("horhshift")
myanunfiltShift.eventDataPset.hfRechitLabel    = cms.untracked.InputTag("hfrhshift")

rhshift     = cms.Sequence(hbherhshift+horhshift+hfrhshift)
unfiltShift = cms.Sequence(rhshift*
                           unfiltTowersShift*
                           unfiltmetNoHFshift*
                           myanunfiltShift)

timeFiltSeqShifted = cms.Sequence(unfiltShift*filtshp)

hbherhfiltshp.rhProfilingPset = cms.untracked.PSet(
    thresholds = cms.untracked.vdouble(4.0,7.5,10.0,15.0))
