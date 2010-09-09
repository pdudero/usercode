
import FWCore.ParameterSet.Config as cms

process = cms.Process("BEAMTIMEANAL")
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( -1 )
# test it!
#   input = cms.untracked.int32( 1000 ) 
)

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);
process.MessageLogger.cerr.FwkReport.reportEvery = 10

#process.source=cms.Source("PoolSource",
#fileNames = cms.untracked.vstring(
#    '/store/data/Run2010A/JetMETTau/RECO/v2/000/137/028/F4059C65-3E71-DF11-B32B-001D09F295A1.root')
#)
#process.load("MyEDmodules.HcalDelayTuner.jetmettau_fill1222reco_v4_10")
process.load("MyEDmodules.HcalDelayTuner.in_cff")

#from MyEDmodules.HcalDelayTuner.goodRunList_cfi import lumisToProcess
#process.source.lumisToProcess = lumisToProcess

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

################################################################################
# Define the cleaning sequence

process.oneGoodVertexFilter = cms.EDFilter("VertexSelector",
   src = cms.InputTag("offlinePrimaryVertices"),
#   cut = cms.string("!isFake && tracksSize > 3 && abs(z) <= 15 && position.Rho <= 2"),
   cut = cms.string("!isFake && ndof >= 5 && abs(z) <= 15"),
   filter = cms.bool(True)   # otherwise it won't filter the events, just produce an empty vertex collection.
)

process.noscraping = cms.EDFilter("FilterOutScraping",
                                  applyfilter = cms.untracked.bool(True),
                                  debugOn = cms.untracked.bool(False),
                                  numtrack = cms.untracked.uint32(10),
                                  thresh = cms.untracked.double(0.25)
                                  )

process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')

process.cleaning_seq = cms.Sequence(process.oneGoodVertexFilter*
                                    process.noscraping*
                                    process.HBHENoiseFilter)

################################################################################
# Configure Retimer-Reflagger-Tower-MET rereco chain

process.load('Configuration.StandardSequences.GeometryExtended_cff')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = 'GR10_P_V6::All'

# Replace timecorr object
process.tc_ascii2 = cms.ESSource("HcalTextCalibrations",
  input = cms.VPSet(cms.PSet(
           object = cms.string('TimeCorrs'),
             file = cms.FileInPath('MyEDmodules/HBHERecHitRetimer/data/timecorr_may27.txt') )
                    )
)
process.prefer=cms.ESPrefer('HcalTextCalibrations','tc_ascii2')
process.hbherecoRetimed = cms.EDProducer('HBHERecHitRetimer',
                                         input = cms.InputTag('hbhereco')
)

from JetMETAnalysis.HcalReflagging.hbherechitreflaggerJETMET_cfi import *
process.hbherecoReflagged = hbherechitreflaggerJETMET.clone()
process.hbherecoReflagged.hbheInputLabel=cms.untracked.InputTag("hbherecoRetimed")
process.hbherecoReflagged.hbheTimingFlagBit=cms.untracked.int32(31)
process.hbherecoReflagged.ignorelowest=cms.bool(False)
#process.hbherecoReflagged.debug=cms.untracked.int32(1)

# The following lines disable the default HFLongShort bits in the SeverityLevelComputer, and instead
# cause any rechits flagged by 'UserDefinedBit0' to be excluded from calotower creation
import JetMETAnalysis.HcalReflagging.RemoveAddSevLevel as RemoveAddSevLevel

process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")
process.hcalRecAlgos=RemoveAddSevLevel.AddFlag(process.hcalRecAlgos,"HBHETimingShapedCutsBits",10)

process.load("RecoJets.Configuration.CaloTowersRec_cff")
process.filtTowersTimeBit = process.towerMaker.clone(hbheInput=cms.InputTag("hbherecoReflagged"))

from RecoMET.METProducers.CaloMET_cfi import met
process.rerecomet = met.clone(src = cms.InputTag("filtTowersTimeBit"))

process.rereco_seq = cms.Sequence(process.hbherecoRetimed*
                                  process.hbherecoReflagged*
                                  process.filtTowersTimeBit*
                                  process.rerecomet)

################################################################################
# Analyzers
process.TFileService = cms.Service("TFileService", 
   fileName = cms.string("anal.root"),
    closeFileFast = cms.untracked.bool(False)
)

from MyEDmodules.HcalDelayTuner.beamtiminganal_cfi import *
process.hbantfilt = hbtimeanal.clone()
process.hbantfilt.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecoReflagged")
process.hbantfilt.runDescription       = cms.untracked.string("/JetMETTau/Run2010A-v1 PD")
process.hbantfilt.ampCutsInfC          = cms.bool(False)
process.hbantfilt.minHit_GeVorfC       = cms.double(0.7)
process.hbantfilt.recHitEscaleMaxGeV   = cms.double(800.5)
process.hbantfilt.maxEventNum2plot     = cms.int32(30000000)

process.hbantfilt.detIds2mask          = cms.vint32(14,31,1)

process.heantfilt = hetimeanal.clone()
process.heantfilt.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecoReflagged")
process.heantfilt.runDescription       = cms.untracked.string("/JetMETTau/Run2010A-v1 PD")
process.heantfilt.ampCutsInfC          = cms.bool(False)
process.heantfilt.minHit_GeVorfC       = cms.double(0.8)
process.heantfilt.recHitEscaleMaxGeV   = cms.double(800.5)
process.heantfilt.maxEventNum2plot     = cms.int32(30000000)

process.metunfilt = cms.EDAnalyzer("METbinner",
  metInput          = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("met","","RECO")),
  minMET4plotGeV    = cms.untracked.double( -100.0),
  maxMET4plotGeV    = cms.untracked.double( 5000.0),
  minSumET4plotGeV  = cms.untracked.double( -100.0),
  maxSumET4plotGeV  = cms.untracked.double(10000.0),
)

process.mettfilt = cms.EDAnalyzer("METbinner",
  metInput          = cms.untracked.PSet(caloMETlabel=cms.untracked.InputTag("rerecomet","","BEAMTIMEANAL")),
  minMET4plotGeV    = cms.untracked.double( -100.0),
  maxMET4plotGeV    = cms.untracked.double( 5000.0),
  minSumET4plotGeV  = cms.untracked.double( -100.0),
  maxSumET4plotGeV  = cms.untracked.double(10000.0),
)

# Need conditions to convert digi ADC to fC in the analyzer

process.anal_seq     = cms.Sequence(process.hbantfilt+process.heantfilt+process.mettfilt)

process.unfiltpath = cms.Path(process.cleaning_seq*process.metunfilt)
process.tfiltpath  = cms.Path(process.cleaning_seq*process.rereco_seq*process.anal_seq)

# Clone Reco Met
process.CloneRecoMet = cms.EDProducer("CaloMETShallowCloneProducer",
    src = cms.InputTag("met")
)

# Select Reco Met
process.SelectRecoMet = cms.EDFilter("PtMinCandSelector",
      src = cms.InputTag("CloneRecoMet"),
    ptMin = cms.double(100.0),
   filter = cms.bool(True)   # otherwise it won't filter the events!
)

process.himetfilterpath = cms.Path(process.cleaning_seq*
                                   process.CloneRecoMet*
                                   process.SelectRecoMet)

#------------------------------
# Output module configuration
#------------------------------
#
process.output = cms.OutputModule("PoolOutputModule",
   fileName = cms.untracked.string("/home/grad/dudero/data/pool.root"),
    # save only events passing the full path
#    SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('himetfilterpath') ),
   outputCommands = cms.untracked.vstring("keep *")
)

process.out_step = cms.EndPath(process.output)

process.schedule = cms.Schedule(process.unfiltpath
                               ,process.tfiltpath
#                              ,process.himetfilterpath
#                              ,process.out_step
                                )
