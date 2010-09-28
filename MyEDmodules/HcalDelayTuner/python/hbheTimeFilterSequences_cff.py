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
# New fitted shape gained 2.5 (!!) + 3ns shift, filter down to 0GeV
#
hbheReflag2p5xPlus3nsDownTo0 = hbherechitreflaggerJETMET.clone()
hbheReflag2p5xPlus3nsDownTo0.timingshapedcutsParameters.win_offset = cms.double(3.0)
hbheReflag2p5xPlus3nsDownTo0.timingshapedcutsParameters.win_gain   = cms.double(2.5)
hbheReflag2p5xPlus3nsDownTo0.timingshapedcutsParameters.ignorelowest=cms.bool(False)
hbheReflag2p5xPlus3nsDownTo0.timingshapedcutsParameters.tfilterEnvelope  = cms.vdouble( 47.0 ,  5.0, 
                                                                                        51.73,  4.86,
                                                                                        54.74,  4.69,
                                                                                        57.75,  4.58,
                                                                                        60.76,  4.43,
                                                                                        64.53,  4.38,
                                                                                        69.05,  4.27,
                                                                                        74.32,  4.20,
                                                                                        81.09,  4.12,
                                                                                        90.13,  4.02,
                                                                                       102.93,  3.91,
                                                                                       124.77,  3.79,
                                                                                       250.0 ,  3.50)
                                                                                 
#--------------------------------------------------
# New fitted shape gained 2.5 (!!) + 3ns shift, stop filtering at lowest value
#
hbheReflag2p5xPlus3nsStopAt50 = hbheReflag2p5xPlus3nsDownTo0.clone()
hbheReflag2p5xPlus3nsStopAt50.timingshapedcutsParameters.ignorelowest  = cms.bool(True)
                                                                                 
#--------------------------------------------------
# Square Filter shape, no energy dependence, one TS wide
#
hbheReflagSquareFilter                  = hbherechitreflaggerJETMET.clone()
hbheReflagSquareFilter.timingshapedcutsParameters.tfilterEnvelope  = cms.vdouble(0.0, 12.5,
                                                                                 350.50, 12.5)
hbheReflagSquareFilter.timingshapedcutsParameters.win_offset       = cms.double(2.0)
                                                                                 
#--------------------------------------------------
# Square Filter shape, no energy dependence, one TS wide, stops at 50GeV
#
hbheReflagSquareFilterStopAt50          = hbherechitreflaggerJETMET.clone()
hbheReflagSquareFilterStopAt50.timingshapedcutsParameters.tfilterEnvelope  = cms.vdouble( 50.0,  12.5,
                                                                                         350.50, 12.5)
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
filtTowers2p5xPlus3nsDownTo0   = towerMaker.clone(hbheInput=cms.InputTag("hbheReflag2p5xPlus3nsDownTo0"))
filtTowers2p5xPlus3nsStopAt50  = towerMaker.clone(hbheInput=cms.InputTag("hbheReflag2p5xPlus3nsStopAt50"))
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
filtmet2p5xPlus3nsDownTo0   = met.clone(src = cms.InputTag("filtTowers2p5xPlus3nsDownTo0"))
filtmet2p5xPlus3nsStopAt50  = met.clone(src = cms.InputTag("filtTowers2p5xPlus3nsStopAt50"))
filtmetSquareFilter         = met.clone(src = cms.InputTag("filtTowersSquareFilter"))
filtmetSquareFilterStopAt50 = met.clone(src = cms.InputTag("filtTowersSquareFilterStopAt50"))

unfiltmetHcalOnly = met.clone(src = cms.InputTag("unfiltTowersHcalOnly"))
                                      
#--------------------------------------------------
# ANALYZERS

from MyEDmodules.HcalDelayTuner.beamtiminganal_cfi import *
# set global analyzer parameters here before cloning,
# so don't have to repeat for all the clones
#
hbtimeanal.runDescription       = cms.untracked.string("/JetMETTau/Run2010A-v1 PD")
hbtimeanal.ampCutsInfC          = cms.bool(False)
hbtimeanal.minHit_GeVorfC       = cms.double(0.7)
hbtimeanal.maxEventNum2plot     = cms.int32(30000000)
hbtimeanal.detIds2mask          = cms.vint32(14,31,1)
hbtimeanal.doPerChannel         = cms.bool(False)

hetimeanal.runDescription       = cms.untracked.string("/JetMETTau/Run2010A-v1 PD")
hetimeanal.ampCutsInfC          = cms.bool(False)
hetimeanal.minHit_GeVorfC       = cms.double(0.7)
hetimeanal.maxEventNum2plot     = cms.int32(30000000)
hetimeanal.doPerChannel         = cms.bool(False)

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

hbanfilt2p5xPlus3nsDownTo0 = hbtimeanal.clone()
heanfilt2p5xPlus3nsDownTo0 = hetimeanal.clone()
hbanfilt2p5xPlus3nsDownTo0.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheReflag2p5xPlus3nsDownTo0")
heanfilt2p5xPlus3nsDownTo0.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheReflag2p5xPlus3nsDownTo0")
#hbanfilt2p5xPlus3nsDownTo0.eventDataPset.twrLabel        = cms.untracked.InputTag("towerMaker")
#heanfilt2p5xPlus3nsDownTo0.eventDataPset.twrLabel        = cms.untracked.InputTag("towerMaker")

hbanfilt2p5xPlus3nsStopAt50 = hbtimeanal.clone()
heanfilt2p5xPlus3nsStopAt50 = hetimeanal.clone()
hbanfilt2p5xPlus3nsStopAt50.eventDataPset.hbheRechitLabel= cms.untracked.InputTag("hbheReflag2p5xPlus3nsStopAt50")
heanfilt2p5xPlus3nsStopAt50.eventDataPset.hbheRechitLabel= cms.untracked.InputTag("hbheReflag2p5xPlus3nsStopAt50")
#hbanfilt2p5xPlus3nsStopAt50.eventDataPset.twrLabel       = cms.untracked.InputTag("towerMaker")
#heanfilt2p5xPlus3nsStopAt50.eventDataPset.twrLabel       = cms.untracked.InputTag("towerMaker")

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
metbinfilt2p5xPlus3nsDownTo0 = metbinUnfilt.clone(
    metInput = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("filtmet2p5xPlus3nsDownTo0"))
)
metbinfilt2p5xPlus3nsStopAt50 = metbinUnfilt.clone(
    metInput = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("filtmet2p5xPlus3nsStopAt50"))
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

hbheanUnfilt                  = cms.Sequence(hbanunfilt+
                                             heanunfilt+
                                             metbinUnfilt)
hbheanfiltStdShape            = cms.Sequence(hbanfiltStdShape+
                                             heanfiltStdShape+
                                             metbinfiltStdShape)
hbheanfiltGained10pctPlus2ns  = cms.Sequence(hbanfiltGained10pctPlus2ns+
                                             heanfiltGained10pctPlus2ns+
                                             metbinfiltGained10pctPlus2ns)
hbheanfiltGained15pctPlus2ns  = cms.Sequence(hbanfiltGained15pctPlus2ns+
                                             heanfiltGained15pctPlus2ns+
                                             metbinfiltGained15pctPlus2ns)
hbheanfilt2p5xPlus3nsDownTo0  = cms.Sequence(hbanfilt2p5xPlus3nsDownTo0+
                                             heanfilt2p5xPlus3nsDownTo0+
                                             metbinfilt2p5xPlus3nsDownTo0)
hbheanfilt2p5xPlus3nsStopAt50 = cms.Sequence(hbanfilt2p5xPlus3nsStopAt50+
                                             heanfilt2p5xPlus3nsStopAt50+
                                             metbinfilt2p5xPlus3nsStopAt50)
hbheanfiltSquareFilter        = cms.Sequence(hbanfiltSquareFilter+
                                             heanfiltSquareFilter+
                                             metbinfiltSquareFilter)
hbheanfiltSquareFilterStopAt50= cms.Sequence(hbanfiltSquareFilterStopAt50+
                                             heanfiltSquareFilterStopAt50+
                                             metbinfiltSquareFilterStopAt50)
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

seq2p5xPlus3nsDownTo0        = cms.Sequence(hbheReflag2p5xPlus3nsDownTo0*
                                            filtTowers2p5xPlus3nsDownTo0*
                                            filtmet2p5xPlus3nsDownTo0*
                                            hbheanfilt2p5xPlus3nsDownTo0)

seq2p5xPlus3nsStopAt50       = cms.Sequence(hbheReflag2p5xPlus3nsStopAt50*
                                            filtTowers2p5xPlus3nsStopAt50*
                                            filtmet2p5xPlus3nsStopAt50*
                                            hbheanfilt2p5xPlus3nsStopAt50)

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
#                                            seq2p5xPlus3nsDownTo0+
                                            seq2p5xPlus3nsStopAt50+
                                            seqSquareFilter+
                                            seqSquareFilterStopAt50)
