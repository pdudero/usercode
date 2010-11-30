import FWCore.ParameterSet.Config as cms

#--------------------------------------------------
# RECHIT FILTERS

from JetMETAnalysis.HcalReflagging.hbherechitreflaggerJETMET_cfi import *
#hbherechitreflaggerJETMET.debug = 0
hbherechitreflaggerJETMET.timingshapedcutsParameters.hbheTimingFlagBit=cms.untracked.int32(8)

#--------------------------------------------------
# Standard Shaped (energy-dependent) time window
#
hbheReflagStdShape = hbherechitreflaggerJETMET.clone()
                                                                                 
#--------------------------------------------------
# Square Filter shape, no energy dependence, one TS wide
#
hbheReflagSquareFilter                  = hbherechitreflaggerJETMET.clone()
hbheReflagSquareFilter.timingshapedcutsParameters.tfilterEnvelope  = cms.vdouble(0.0,    -12.5, 12.5,
                                                                                 350.50, -12.5, 12.5)
hbheReflagSquareFilter.timingshapedcutsParameters.win_offset       = cms.double(2.0)
hbheReflagSquareFilter.timingshapedcutsParameters.ignorelowest=cms.bool(False)
                                                                                 
#--------------------------------------------------
# Square Filter shape, no energy dependence, one TS wide, stops at 50GeV
#
hbheReflagSquareFilterStopAt50          = hbherechitreflaggerJETMET.clone()
hbheReflagSquareFilterStopAt50.timingshapedcutsParameters.tfilterEnvelope  = cms.vdouble( 50.0,  -12.5, 12.5,
                                                                                         350.50, -12.5, 12.5)
hbheReflagSquareFilterStopAt50.timingshapedcutsParameters.win_offset    = cms.double(2.0)
hbheReflagSquareFilterStopAt50.timingshapedcutsParameters.ignorelowest  = cms.bool(True)

#--------------------------------------------------
# DEFLAGGER, for clearing all flags from prompt reconstruction!
#
hbheDEflagger = hbherechitreflaggerJETMET.clone()
hbheDEflagger.timingshapedcutsParameters.tfilterEnvelope  = cms.vdouble( 0.0,  -1e99, 1e99,
                                                                         1e99, -1e99, 1e99 )
hbheDEflagger.timingshapedcutsParameters.win_gain      = cms.double(1.0)
hbheDEflagger.timingshapedcutsParameters.win_offset    = cms.double(0.0)
hbheDEflagger.timingshapedcutsParameters.ignorelowest  = cms.bool(False)
hbheDEflagger.timingshapedcutsParameters.ignorehighest = cms.bool(False)

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
filtTowersSquareFilter         = towerMaker.clone(hbheInput=cms.InputTag("hbheReflagSquareFilter"))
filtTowersSquareFilterStopAt50 = towerMaker.clone(hbheInput=cms.InputTag("hbheReflagSquareFilterStopAt50"))

#Exclude ECAL energy altogether
unfiltTowersHcalOnly = towerMaker.clone(hbheInput=cms.InputTag("hbheDEflagger"))
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
hbanunfilt.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheDEflagger")
heanunfilt.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbheDEflagger")

hbanfiltStdShape = hbtimeanal.clone()
heanfiltStdShape = hetimeanal.clone()
hbanfiltStdShape.eventDataPset.hbheRechitLabel           = cms.untracked.InputTag("hbheReflagStdShape")
heanfiltStdShape.eventDataPset.hbheRechitLabel           = cms.untracked.InputTag("hbheReflagStdShape")
#
# Feeding the analyzer a calotower tag makes it perform an additional
# minimum EM confirmation cut on the hit collection.
#
#hbanfiltStdShape.eventDataPset.twrLabel                  = cms.untracked.InputTag("towerMaker")
#heanfiltStdShape.eventDataPset.twrLabel                  = cms.untracked.InputTag("towerMaker")

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

# Assumes no time filtering in standard reco
#
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
    hbheRechitLabel = cms.untracked.InputTag("hbheDEflagger"),
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
hbheanfiltSquareFilter        = cms.Sequence(hbanfiltSquareFilter+
                                             heanfiltSquareFilter+
                                             metcmpfiltSquareFilter)
hbheanfiltSquareFilterStopAt50= cms.Sequence(hbanfiltSquareFilterStopAt50+
                                             heanfiltSquareFilterStopAt50+
                                             metcmpfiltSquareFilterStopAt50)
hbheanUnfiltHcalOnly          = cms.Sequence(metbinUnfiltHcalOnly)

#--------------------------------------------------
# SEQUENCES

seqUnfilt                    = cms.Sequence(hbheDEflagger*
                                            hbheanUnfilt)

seqStdShape                  = cms.Sequence(hbheReflagStdShape*
                                            filtTowersStdShape*
                                            filtmetStdShape*
                                            hbheanfiltStdShape)

seqSquareFilter              = cms.Sequence(hbheReflagSquareFilter*
                                            filtTowersSquareFilter*
                                            filtmetSquareFilter*
                                            hbheanfiltSquareFilter)

seqSquareFilterStopAt50      = cms.Sequence(hbheReflagSquareFilterStopAt50*
                                            filtTowersSquareFilterStopAt50*
                                            filtmetSquareFilterStopAt50*
                                            hbheanfiltSquareFilterStopAt50)

seqHcalOnly                  = cms.Sequence(hbheDEflagger*
                                            unfiltTowersHcalOnly*
                                            unfiltmetHcalOnly*
                                            hbheanUnfiltHcalOnly)

allFilterSeqs                = cms.Sequence(seqUnfilt+
                                            seqHcalOnly+
#                                            seqSquareFilter+
#                                            seqSquareFilterStopAt50+
                                            seqStdShape)
