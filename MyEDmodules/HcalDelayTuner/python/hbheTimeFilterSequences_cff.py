import FWCore.ParameterSet.Config as cms

#--------------------------------------------------
# RECHIT FILTERS

from JetMETAnalysis.HcalReflagging.hbherechitreflaggerJETMET_cfi import *
#hbherechitreflaggerJETMET.debug = 0
hbherechitreflaggerJETMET.timingshapedcutsParameters.ignorelowest=cms.bool(False)
hbherechitreflaggerJETMET.timingshapedcutsParameters.hbheTimingFlagBit=cms.untracked.int32(8)

#--------------------------------------------------
# Standard Shaped (energy-dependent) time window
#
hbheReflagStdShape = hbherechitreflaggerJETMET.clone()

#--------------------------------------------------
# Standard Shape gained 10% + 2ns shift
#
hbheReflagGained10pctPlus2ns            = hbherechitreflaggerJETMET.clone()
hbheReflagGained10pctPlus2ns.timingshapedcutsParameters.win_offset = cms.double(2.0)
hbheReflagGained10pctPlus2ns.timingshapedcutsParameters.win_gain   = cms.double(1.1)

#--------------------------------------------------
# Standard Shape gained 15% + 2ns shift
#
hbheReflagGained15pctPlus2ns            = hbherechitreflaggerJETMET.clone()
hbheReflagGained15pctPlus2ns.timingshapedcutsParameters.win_offset = cms.double(2.0)
hbheReflagGained15pctPlus2ns.timingshapedcutsParameters.win_gain   = cms.double(1.15)

#--------------------------------------------------
# New fitted shape gained 3 (!!), filter down to 0GeV
#
hbheReflag3xNewEnvDownTo0 = hbherechitreflaggerJETMET.clone()
hbheReflag3xNewEnvDownTo0.timingshapedcutsParameters.win_offset = cms.double(0.0)  # (3.0)
hbheReflag3xNewEnvDownTo0.timingshapedcutsParameters.win_gain   = cms.double(3.0)  # (2.5)
hbheReflag3xNewEnvDownTo0.timingshapedcutsParameters.ignorelowest=cms.bool(False)
#hbheReflag3xNewEnvDownTo0.timingshapedcutsParameters.tfilterEnvelope=cms.vdouble( 48.66, -2.08, 4.28,
#                                                                                  52.67, -1.95, 4.06,
#                                                                                  56.69, -1.87, 3.86,
#                                                                                  60.70, -1.81, 3.56,
#                                                                                  65.71, -1.76, 3.25,
#                                                                                  71.73, -1.79, 3.10,
#                                                                                  78.75, -1.70, 2.97,
#                                                                                  87.78, -1.72, 2.79,
#                                                                                  99.81, -1.74, 2.59,
#                                                                                  117.8, -1.73, 2.45,
#                                                                                  149.0, -1.75, 2.23,
#                                                                                  500.0, -1.83, 2.01)

hbheReflag3xNewEnvDownTo0.timingshapedcutsParameters.tfilterEnvelope=cms.vdouble(  50.0,  -2.0,  4.25,
                                                                                   52.0,  -2.0,  4.09,
                                                                                   54.0,  -2.0,  3.95,
                                                                                   56.0,  -2.0,  3.82,
                                                                                   58.0,  -2.0,  3.71,
                                                                                   60.0,  -2.0,  3.60,
                                                                                   63.0,  -2.0,  3.46,
                                                                                   66.0,  -2.0,  3.33,
                                                                                   69.0,  -2.0,  3.22,
                                                                                   73.0,  -2.0,  3.10,
                                                                                   77.0,  -2.0,  2.99,
                                                                                   82.0,  -2.0,  2.87,
                                                                                   88.0,  -2.0,  2.75,
                                                                                   95.0,  -2.0,  2.64,
                                                                                   103.0, -2.0,  2.54,
                                                                                   113.0, -2.0,  2.44,
                                                                                   127.0, -2.0,  2.33,
                                                                                   146.0, -2.0,  2.23,
                                                                                   176.0, -2.0,  2.13,
                                                                                   250.0, -2.0,  2.00)
                                                                         
#--------------------------------------------------
# New fitted shape gained 2.5 (!!) + 3ns shift, stop filtering at lowest value
#
hbheReflag3xNewEnvStopAt50 = hbheReflag3xNewEnvDownTo0.clone()
hbheReflag3xNewEnvStopAt50.timingshapedcutsParameters.ignorelowest  = cms.bool(True)
                                                                                 
#--------------------------------------------------
# Square Filter shape, no energy dependence, one TS wide
#
hbheReflagSquareFilter                  = hbherechitreflaggerJETMET.clone()
hbheReflagSquareFilter.timingshapedcutsParameters.tfilterEnvelope  = cms.vdouble(0.0,    -12.5, 12.5,
                                                                                 350.50, -12.5, 12.5)
hbheReflagSquareFilter.timingshapedcutsParameters.win_offset       = cms.double(2.0)
                                                                                 
#--------------------------------------------------
# Square Filter shape, no energy dependence, one TS wide, stops at 50GeV
#
hbheReflagSquareFilterStopAt50          = hbherechitreflaggerJETMET.clone()
hbheReflagSquareFilterStopAt50.timingshapedcutsParameters.tfilterEnvelope  = cms.vdouble( 50.0,  -12.5, 12.5,
                                                                                         350.50, -12.5, 12.5)
hbheReflagSquareFilterStopAt50.timingshapedcutsParameters.win_offset    = cms.double(2.0)
hbheReflagSquareFilterStopAt50.timingshapedcutsParameters.ignorelowest  = cms.bool(True)

#--------------------------------------------------
# RERECO TOWERS

# The following lines cause any rechits flagged by 'HBHETimingShapedCutsBits'
# to be excluded from calotower creation
#
import JetMETAnalysis.HcalReflagging.RemoveAddSevLevel as RemoveAddSevLevel

from RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi import *
hcalRecAlgos=RemoveAddSevLevel.AddFlag(hcalRecAlgos,"HBHETimingShapedCutsBits",10)

from RecoJets.Configuration.CaloTowersRec_cff import *
filtTowersStdShape             = towerMaker.clone(hbheInput=cms.InputTag("hbheReflagStdShape"))
filtTowersGained10pctPlus2ns   = towerMaker.clone(hbheInput=cms.InputTag("hbheReflagGained10pctPlus2ns"))
filtTowersGained15pctPlus2ns   = towerMaker.clone(hbheInput=cms.InputTag("hbheReflagGained15pctPlus2ns"))
filtTowers3xNewEnvDownTo0      = towerMaker.clone(hbheInput=cms.InputTag("hbheReflag3xNewEnvDownTo0"))
filtTowers3xNewEnvStopAt50     = towerMaker.clone(hbheInput=cms.InputTag("hbheReflag3xNewEnvStopAt50"))
filtTowersSquareFilter         = towerMaker.clone(hbheInput=cms.InputTag("hbheReflagSquareFilter"))
filtTowersSquareFilterStopAt50 = towerMaker.clone(hbheInput=cms.InputTag("hbheReflagSquareFilterStopAt50"))

#Exclude ECAL energy altogether
unfiltTowersHcalOnly = towerMaker.clone()
unfiltTowersHcalOnly.EBWeight       = cms.double(0.0)
unfiltTowersHcalOnly.EEWeight       = cms.double(0.0)
unfiltTowersHcalOnly.EBWeights      = cms.vdouble(0.,0.,0.,0.,0.)
unfiltTowersHcalOnly.EEWeights      = cms.vdouble(0.,0.,0.,0.,0.)

#unfiltTowersHcalOnly.EBSumThreshold = cms.double(999999.9)
#unfiltTowersHcalOnly.EESumThreshold = cms.double(999999.9)
#unfiltTowersHcalOnly.EBThreshold    = cms.double(9999.9)
#unfiltTowersHcalOnly.EEThreshold    = cms.double(9999.9)

#--------------------------------------------------
# RERECO MET


from RecoMET.METProducers.CaloMET_cfi import met
filtmetStdShape             = met.clone(src = cms.InputTag("filtTowersStdShape"))
filtmetGained10pctPlus2ns   = met.clone(src = cms.InputTag("filtTowersGained10pctPlus2ns"))
filtmetGained15pctPlus2ns   = met.clone(src = cms.InputTag("filtTowersGained15pctPlus2ns"))
filtmet3xNewEnvDownTo0      = met.clone(src = cms.InputTag("filtTowers3xNewEnvDownTo0"))
filtmet3xNewEnvStopAt50     = met.clone(src = cms.InputTag("filtTowers3xNewEnvStopAt50"))
filtmetSquareFilter         = met.clone(src = cms.InputTag("filtTowersSquareFilter"))
filtmetSquareFilterStopAt50 = met.clone(src = cms.InputTag("filtTowersSquareFilterStopAt50"))

unfiltmetHcalOnly = met.clone(src = cms.InputTag("unfiltTowersHcalOnly"))
                                      
#--------------------------------------------------
# ANALYZERS

from MyEDmodules.HcalDelayTuner.beamtiminganal_cfi import *
# set global analyzer parameters here before cloning,
# so don't have to repeat for all the clones
#
hbtimeanal.runDescription       = cms.untracked.string("/Jet/Run2010B-PromptReco")
hbtimeanal.ampCutsInfC          = cms.bool(False)
hbtimeanal.minHit_GeVorfC       = cms.double(0.7)
hbtimeanal.maxEventNum2plot     = cms.int32(30000000)
hbtimeanal.doPerChannel         = cms.bool(True)

hbtimeanal.detIds2mask          = cms.vint32(99,54,99,
                                             -13,7,1,
                                             14,31,1)

hetimeanal.runDescription       = cms.untracked.string("/Jet/Run2010B-PromptReco")
hetimeanal.ampCutsInfC          = cms.bool(False)
hetimeanal.minHit_GeVorfC       = cms.double(0.7)
hetimeanal.maxEventNum2plot     = cms.int32(30000000)
hetimeanal.doPerChannel         = cms.bool(True)

hetimeanal.detIds2mask          = cms.vint32(-99,51,99,
                                             -99,52,99,
                                             -99,53,99,
                                             -99,54,99,
                                             -19,43,2)

hbanunfilt   = hbtimeanal.clone()
heanunfilt   = hetimeanal.clone()
hbanunfilt.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbhereco")
heanunfilt.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbhereco")

hbanfiltStdShape = hbtimeanal.clone()
heanfiltStdShape = hetimeanal.clone()
hbanfiltStdShape.eventDataPset.hbheRechitLabel           = cms.untracked.InputTag("hbheReflagStdShape")
heanfiltStdShape.eventDataPset.hbheRechitLabel           = cms.untracked.InputTag("hbheReflagStdShape")
#
# Feeding the analyzer a calotower tag makes it perform an additional
# minimum EM confirmation cut on the hit collection.
#
hbanfiltStdShape.eventDataPset.twrLabel                  = cms.untracked.InputTag("towerMaker")
heanfiltStdShape.eventDataPset.twrLabel                  = cms.untracked.InputTag("towerMaker")

hbanfiltGained10pctPlus2ns = hbtimeanal.clone()
heanfiltGained10pctPlus2ns = hetimeanal.clone()
hbanfiltGained10pctPlus2ns.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheReflagGained10pctPlus2ns")
heanfiltGained10pctPlus2ns.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheReflagGained10pctPlus2ns")

hbanfiltGained15pctPlus2ns = hbtimeanal.clone()
heanfiltGained15pctPlus2ns = hetimeanal.clone()
hbanfiltGained15pctPlus2ns.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheReflagGained15pctPlus2ns")
heanfiltGained15pctPlus2ns.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheReflagGained15pctPlus2ns")

hbanfilt3xNewEnvDownTo0 = hbtimeanal.clone()
heanfilt3xNewEnvDownTo0 = hetimeanal.clone()
hbanfilt3xNewEnvDownTo0.eventDataPset.hbheRechitLabel    = cms.untracked.InputTag("hbheReflag3xNewEnvDownTo0")
heanfilt3xNewEnvDownTo0.eventDataPset.hbheRechitLabel    = cms.untracked.InputTag("hbheReflag3xNewEnvDownTo0")
#hbanfilt3xNewEnvDownTo0.eventDataPset.twrLabel           = cms.untracked.InputTag("towerMaker")
#heanfilt3xNewEnvDownTo0.eventDataPset.twrLabel           = cms.untracked.InputTag("towerMaker")

hbanfilt3xNewEnvStopAt50 = hbtimeanal.clone()
heanfilt3xNewEnvStopAt50 = hetimeanal.clone()
hbanfilt3xNewEnvStopAt50.eventDataPset.hbheRechitLabel   = cms.untracked.InputTag("hbheReflag3xNewEnvStopAt50")
heanfilt3xNewEnvStopAt50.eventDataPset.hbheRechitLabel   = cms.untracked.InputTag("hbheReflag3xNewEnvStopAt50")
#hbanfilt3xNewEnvStopAt50.eventDataPset.twrLabel          = cms.untracked.InputTag("towerMaker")
#heanfilt3xNewEnvStopAt50.eventDataPset.twrLabel          = cms.untracked.InputTag("towerMaker")

hbanfiltSquareFilter = hbtimeanal.clone()
heanfiltSquareFilter = hetimeanal.clone()
hbanfiltSquareFilter.eventDataPset.hbheRechitLabel       = cms.untracked.InputTag("hbheReflagSquareFilter")
heanfiltSquareFilter.eventDataPset.hbheRechitLabel       = cms.untracked.InputTag("hbheReflagSquareFilter")
#hbanfiltSquareFilter.eventDataPset.twrLabel              = cms.untracked.InputTag("towerMaker")
#heanfiltSquareFilter.eventDataPset.twrLabel              = cms.untracked.InputTag("towerMaker")

hbanfiltSquareFilterStopAt50 = hbtimeanal.clone()
heanfiltSquareFilterStopAt50 = hetimeanal.clone()
hbanfiltSquareFilterStopAt50.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheReflagSquareFilterStopAt50")
heanfiltSquareFilterStopAt50.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheReflagSquareFilterStopAt50")

#--------------------------------------------------
# MET binners

metbinUnfilt = cms.EDAnalyzer("METbinner",
  metInput          = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("met","","RECO")),
  minMET4plotGeV    = cms.untracked.double( -100.0),
  maxMET4plotGeV    = cms.untracked.double( 5000.0),
  minSumET4plotGeV  = cms.untracked.double( -100.0),
  maxSumET4plotGeV  = cms.untracked.double(10000.0),
)

metbinfiltStdShape = metbinUnfilt.clone(
    metInput = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("filtmetStdShape"))
)
metbinfiltGained10pctPlus2ns = metbinUnfilt.clone(
    metInput = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("filtmetGained10pctPlus2ns"))
)
metbinfiltGained15pctPlus2ns = metbinUnfilt.clone(
    metInput = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("filtmetGained15pctPlus2ns"))
)
metbinfilt3xNewEnvDownTo0 = metbinUnfilt.clone(
    metInput = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("filtmet3xNewEnvDownTo0"))
)
metbinfilt3xNewEnvStopAt50 = metbinUnfilt.clone(
    metInput = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("filtmet3xNewEnvStopAt50"))
)
metbinfiltSquareFilter = metbinUnfilt.clone(
    metInput = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("filtmetSquareFilter"))
)
metbinfiltSquareFilterStopAt50 = metbinUnfilt.clone(
    metInput = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("filtmetSquareFilterStopAt50"))
)


metbinUnfiltHcalOnly = metbinUnfilt.clone(
    metInput=cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("unfiltmetHcalOnly"))
    )

#--------------------------------------------------
# MET comparators

metcmpfiltStdShape = cms.EDAnalyzer('METcomparator',
                                 triggerResults = cms.untracked.PSet(
    trgResultsLabel = cms.untracked.InputTag("TriggerResults","","BTA"),
#    verbose         = cms.untracked.bool(True)
    ),
                                    dirtyInput = cms.untracked.PSet(
    hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
    caloMETlabel    = cms.untracked.InputTag("met","","RECO"),
#    verbose         = cms.untracked.bool(True)
    ),
                                    reflagFltrdOutput = cms.untracked.PSet(
    hbheRechitLabel = cms.untracked.InputTag("hbheReflagStdShape"),
    caloMETlabel    = cms.untracked.InputTag("filtmetStdShape","","BTA"),
#    verbose         = cms.untracked.bool(True)
    ),
                                 evfiltPathName   = cms.untracked.string("evfiltpath"),
                                 minMET4plotGeV    = cms.untracked.double(-100),
                                 maxMET4plotGeV    = cms.untracked.double(5000),
                                 mindMET4plotGeV   = cms.untracked.double(-2000),
                                 maxdMET4plotGeV   = cms.untracked.double(2000),
                                 minSumET4plotGeV  = cms.untracked.double(-100),
                                 maxSumET4plotGeV  = cms.untracked.double(10000),
                                 mindSumET4plotGeV = cms.untracked.double(-500),
                                 maxdSumET4plotGeV = cms.untracked.double(2000)
)

metcmpfiltGained10pctPlus2ns = metcmpfiltStdShape.clone()
metcmpfiltGained10pctPlus2ns.reflagFltrdOutput.hbheRechitLabel=cms.untracked.InputTag("hbheReflagGained10pctPlus2ns")
metcmpfiltGained10pctPlus2ns.reflagFltrdOutput.caloMETlabel=cms.untracked.InputTag("filtmetGained10pctPlus2ns")

metcmpfiltGained15pctPlus2ns = metcmpfiltStdShape.clone()
metcmpfiltGained15pctPlus2ns.reflagFltrdOutput.hbheRechitLabel=cms.untracked.InputTag("hbheReflagGained15pctPlus2ns")
metcmpfiltGained15pctPlus2ns.reflagFltrdOutput.caloMETlabel=cms.untracked.InputTag("filtmetGained15pctPlus2ns")

metcmpfilt3xNewEnvDownTo0 = metcmpfiltStdShape.clone()
metcmpfilt3xNewEnvDownTo0.reflagFltrdOutput.hbheRechitLabel=cms.untracked.InputTag("hbheReflag3xNewEnvDownTo0")
metcmpfilt3xNewEnvDownTo0.reflagFltrdOutput.caloMETlabel=cms.untracked.InputTag("filtmet3xNewEnvDownTo0")

metcmpfilt3xNewEnvStopAt50 = metcmpfiltStdShape.clone()
metcmpfilt3xNewEnvStopAt50.reflagFltrdOutput.hbheRechitLabel=cms.untracked.InputTag("hbheReflag3xNewEnvStopAt50")
metcmpfilt3xNewEnvStopAt50.reflagFltrdOutput.caloMETlabel=cms.untracked.InputTag("filtmet3xNewEnvStopAt50")

metcmpfiltSquareFilter = metcmpfiltStdShape.clone()
metcmpfiltSquareFilter.reflagFltrdOutput.hbheRechitLabel=cms.untracked.InputTag("hbheReflagSquareFilter")
metcmpfiltSquareFilter.reflagFltrdOutput.caloMETlabel=cms.untracked.InputTag("filtmetSquareFilter")

metcmpfiltSquareFilterStopAt50 = metcmpfiltStdShape.clone()
metcmpfiltSquareFilterStopAt50.reflagFltrdOutput.hbheRechitLabel=cms.untracked.InputTag("hbheReflagSquareFilterStopAt50")
metcmpfiltSquareFilterStopAt50.reflagFltrdOutput.caloMETlabel=cms.untracked.InputTag("filtmetSquareFilterStopAt50")

#--------------------------------------------------

hbheanUnfilt                  = cms.Sequence(hbanunfilt+
                                             heanunfilt+
                                             metbinUnfilt)
hbheanfiltStdShape            = cms.Sequence(hbanfiltStdShape+
                                             heanfiltStdShape+
                                             metcmpfiltStdShape)
hbheanfiltGained10pctPlus2ns  = cms.Sequence(hbanfiltGained10pctPlus2ns+
                                             heanfiltGained10pctPlus2ns+
                                             metcmpfiltGained10pctPlus2ns)
hbheanfiltGained15pctPlus2ns  = cms.Sequence(hbanfiltGained15pctPlus2ns+
                                             heanfiltGained15pctPlus2ns+
                                             metcmpfiltGained15pctPlus2ns)
hbheanfilt3xNewEnvDownTo0     = cms.Sequence(hbanfilt3xNewEnvDownTo0+
                                             heanfilt3xNewEnvDownTo0+
                                             metcmpfilt3xNewEnvDownTo0)
hbheanfilt3xNewEnvStopAt50    = cms.Sequence(hbanfilt3xNewEnvStopAt50+
                                             heanfilt3xNewEnvStopAt50+
                                             metcmpfilt3xNewEnvStopAt50)
hbheanfiltSquareFilter        = cms.Sequence(hbanfiltSquareFilter+
                                             heanfiltSquareFilter+
                                             metcmpfiltSquareFilter)
hbheanfiltSquareFilterStopAt50= cms.Sequence(hbanfiltSquareFilterStopAt50+
                                             heanfiltSquareFilterStopAt50+
                                             metcmpfiltSquareFilterStopAt50)
hbheanUnfiltHcalOnly          = cms.Sequence(metbinUnfiltHcalOnly)

#--------------------------------------------------
# SEQUENCES


seqStdShape                  = cms.Sequence(hbheReflagStdShape*
                                            filtTowersStdShape*
                                            filtmetStdShape*
                                            hbheanfiltStdShape)

seqGained10pctPlus2ns        = cms.Sequence(hbheReflagGained10pctPlus2ns*
                                            filtTowersGained10pctPlus2ns*
                                            filtmetGained10pctPlus2ns*
                                            hbheanfiltGained10pctPlus2ns)

seqGained15pctPlus2ns        = cms.Sequence(hbheReflagGained15pctPlus2ns*
                                            filtTowersGained15pctPlus2ns*
                                            filtmetGained15pctPlus2ns*
                                            hbheanfiltGained15pctPlus2ns)

seq3xNewEnvDownTo0           = cms.Sequence(hbheReflag3xNewEnvDownTo0*
                                            filtTowers3xNewEnvDownTo0*
                                            filtmet3xNewEnvDownTo0*
                                            hbheanfilt3xNewEnvDownTo0)

seq3xNewEnvStopAt50          = cms.Sequence(hbheReflag3xNewEnvStopAt50*
                                            filtTowers3xNewEnvStopAt50*
                                            filtmet3xNewEnvStopAt50*
                                            hbheanfilt3xNewEnvStopAt50)

seqSquareFilter              = cms.Sequence(hbheReflagSquareFilter*
                                            filtTowersSquareFilter*
                                            filtmetSquareFilter*
                                            hbheanfiltSquareFilter)

seqSquareFilterStopAt50      = cms.Sequence(hbheReflagSquareFilterStopAt50*
                                            filtTowersSquareFilterStopAt50*
                                            filtmetSquareFilterStopAt50*
                                            hbheanfiltSquareFilterStopAt50)

seqHcalOnly                  = cms.Sequence(unfiltTowersHcalOnly*
                                            unfiltmetHcalOnly*
                                            hbheanUnfiltHcalOnly)

allFilterSeqs                = cms.Sequence(hbheanUnfilt+
                                            seqHcalOnly+
#                                            seqStdShape+
#                                            seqGained10pctPlus2ns+
#                                            seqGained15pctPlus2ns+
#                                            seq3xNewEnvDownTo0+
                                            seq3xNewEnvStopAt50)
#                                            seqSquareFilter+
#                                            seqSquareFilterStopAt50)
