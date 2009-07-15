import FWCore.ParameterSet.Config as cms

# shift the HBHE rechit times down by a time sample's worth.

from MyEDmodules.HcalRecHitFilter.hcalrechitfilter_cff import *

hbherhfilt06ns = hbheRHfilter.clone()
hbherhfilt06ns.timeShiftNs         = cms.double(0.0)
hbherhfilt06ns.tfilterEnvelope     = cms.vdouble(0.0,3.0,
                                                1e99,3.0)
hbherhfilt06ns.timeWindowCenterNs_ = cms.double(14.0)
hbherhfilt06ns.timeWindowGain      = cms.double(1.0)

horhfilt06ns = hoRHfilter.clone()
horhfilt06ns.timeShiftNs           = cms.double(0.0)
horhfilt06ns.tfilterEnvelope       = cms.vdouble(0.0,3.0,
                                                 1e99,3.0)
horhfilt06ns.timeWindowCenterNs_   = cms.double(14.0)
horhfilt06ns.timeWindowGain        = cms.double(1.0)

hfrhfilt06ns = hfRHfilter.clone()
hfrhfilt06ns.timeShiftNs           = cms.double(0.0)
hfrhfilt06ns.tfilterEnvelope       = cms.vdouble(0.0,3.0,
                                                 1e99,3.0)
hfrhfilt06ns.timeWindowCenterNs_   = cms.double(14.0)
hfrhfilt06ns.timeWindowGain        = cms.double(1.0)

#--------------------------------------------------

hbherhfilt10ns = hbheRHfilter.clone()
hbherhfilt10ns.timeShiftNs         = cms.double(0.0)
hbherhfilt10ns.tfilterEnvelope     = cms.vdouble(0.0,5.0,
                                                1e99,5.0)
hbherhfilt10ns.timeWindowCenterNs_ = cms.double(14.0)
hbherhfilt10ns.timeWindowGain      = cms.double(1.0)

horhfilt10ns = hoRHfilter.clone()
horhfilt10ns.timeShiftNs           = cms.double(0.0)
horhfilt10ns.tfilterEnvelope       = cms.vdouble(0.0,5.0,
                                                 1e99,5.0)
horhfilt10ns.timeWindowCenterNs_   = cms.double(14.0)
horhfilt10ns.timeWindowGain        = cms.double(1.0)

hfrhfilt10ns = hfRHfilter.clone()
hfrhfilt10ns.timeShiftNs           = cms.double(0.0)
hfrhfilt10ns.tfilterEnvelope       = cms.vdouble(0.0,5.0,
                                                 1e99,5.0)
hfrhfilt10ns.timeWindowCenterNs_   = cms.double(14.0)
hfrhfilt10ns.timeWindowGain        = cms.double(1.0)

#--------------------------------------------------

hbherhfilt1ts = hbheRHfilter.clone()
hbherhfilt1ts.timeShiftNs         = cms.double(0.0)
hbherhfilt1ts.tfilterEnvelope     = cms.vdouble(0.0,12.5,
                                                1e99,12.5)
hbherhfilt1ts.timeWindowCenterNs_ = cms.double(12.5)
hbherhfilt1ts.timeWindowGain      = cms.double(1.0)

horhfilt1ts = hoRHfilter.clone()
horhfilt1ts.timeShiftNs           = cms.double(0.0)
horhfilt1ts.tfilterEnvelope       = cms.vdouble(0.0,12.5,
                                                 1e99,12.5)
horhfilt1ts.timeWindowCenterNs_   = cms.double(12.5)
horhfilt1ts.timeWindowGain        = cms.double(1.0)

hfrhfilt1ts = hfRHfilter.clone()
hfrhfilt1ts.timeShiftNs           = cms.double(0.0)
hfrhfilt1ts.tfilterEnvelope       = cms.vdouble(0.0,12.5,
                                                 1e99,12.5)
hfrhfilt1ts.timeWindowCenterNs_   = cms.double(12.5)
hfrhfilt1ts.timeWindowGain        = cms.double(1.0)

#--------------------------------------------------

hbherhfilt4ts = hbheRHfilter.clone()
hbherhfilt4ts.timeShiftNs         = cms.double(0.0)
hbherhfilt4ts.tfilterEnvelope     = cms.vdouble(0.0,50.0,
                                                1e99,50.0)
hbherhfilt4ts.timeWindowCenterNs_ = cms.double(50.0)
hbherhfilt4ts.timeWindowGain      = cms.double(1.0)

horhfilt4ts = hoRHfilter.clone()
horhfilt4ts.timeShiftNs           = cms.double(0.0)
horhfilt4ts.tfilterEnvelope       = cms.vdouble(0.0,50.0,
                                                 1e99,50.0)
horhfilt4ts.timeWindowCenterNs_   = cms.double(50.0)
horhfilt4ts.timeWindowGain        = cms.double(1.0)

hfrhfilt4ts = hfRHfilter.clone()
hfrhfilt4ts.timeShiftNs           = cms.double(0.0)
hfrhfilt4ts.tfilterEnvelope       = cms.vdouble(0.0,50.0,
                                                 1e99,50.0)
hfrhfilt4ts.timeWindowCenterNs_   = cms.double(50.0)
hfrhfilt4ts.timeWindowGain        = cms.double(1.0)

#--------------------------------------------------
# Shaped (energy-dependent) time windows (not HF)
#
hbherhfiltshp = hbheRHfilter.clone()
hbherhfiltshp.timeShiftNs         = cms.double(0.0)
hbherhfiltshp.timeWindowCenterNs_ = cms.double(14.0)
hbherhfiltshp.timeWindowGain      = cms.double(2.0)
hbherhfiltshp.tfilterEnvelope     = cms.vdouble(  4.00,6.02,
                                                 13.00,4.39,
                                                 23.50,4.41,
                                                 37.00,3.69,
                                                 56.00,3.15,
                                                 81.00,2.82,
                                                114.50,2.72,
                                                175.50,2.69,
                                                350.50,2.57)

horhfiltshp = hoRHfilter.clone()
horhfiltshp.timeShiftNs         = cms.double(0.0)
horhfiltshp.timeWindowCenterNs_ = cms.double(14.0)
horhfiltshp.timeWindowGain      = cms.double(2.0)
horhfiltshp.tfilterEnvelope     = cms.vdouble(  4.00,6.02,
                                               13.00,4.39,
                                               23.50,4.41,
                                               37.00,3.69,
                                               56.00,3.15,
                                               81.00,2.82,
                                              114.50,2.72,
                                              175.50,2.69,
                                              350.50,2.57)

#--------------------------------------------------
# In case ya want'em...unfiltered shifted rechits

hbherhshift = hbheRHfilter.clone(timeShiftNs = cms.double(25.0))
hfrhshift   = hfRHfilter.clone(timeShiftNs = cms.double(25.0))
horhshift   = hoRHfilter.clone(timeShiftNs = cms.double(25.0))
