
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

process.source=cms.Source("PoolSource",
fileNames = cms.untracked.vstring('in.root')
 #    '/store/data/Run2010A/JetMETTau/RECO/v2/000/137/028/F4059C65-3E71-DF11-B32B-001D09F295A1.root')
#     '/store/data/Run2010A/JetMETTau/RAW-RECO/v6/000/139/779/227B20AD-508B-DF11-84EE-003048678DD6.root')
#'/store/data/Run2010A/JetMETTau/RAW-RECO/v6/000/141/882/7A5CBED5-409B-DF11-B5D0-001A928116BA.root')
)
#process.load("MyEDmodules.HcalDelayTuner.jetmettau_fill1207reco_1")
#process.load("in_cff")

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

from MyEDmodules.HcalDelayTuner.goodRunList_cfi import lumisToProcess
process.source.lumisToProcess = lumisToProcess

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

process.cleaning_seq = cms.Sequence(process.noscraping*
                                    process.oneGoodVertexFilter*
                                    process.HBHENoiseFilter)

################################################################################
# Configure Retimer-Reflagger-Tower-MET rereco chain

process.load('Configuration.StandardSequences.GeometryExtended_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('EventFilter.HcalRawToDigi.HcalRawToDigi_cfi')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = 'GR10_P_V7::All'

# Replace timecorr object
process.tc_ascii2 = cms.ESSource("HcalTextCalibrations",
  input = cms.VPSet(cms.PSet(
           object = cms.string('TimeCorrs'),
#             file = cms.FileInPath('MyEDmodules/HBHERecHitRetimer/data/timecorr_may27.txt') )
             file = cms.FileInPath('MyEDmodules/HcalDelayTuner/data/timecorr_may27.csv') )
                    )
)
process.prefer=cms.ESPrefer('HcalTextCalibrations','tc_ascii2')
process.hbherecoRetimed = cms.EDProducer('HBHERecHitRetimer',
                                         input = cms.InputTag('hbhereco')
)

from JetMETAnalysis.HcalReflagging.hbherechitreflaggerJETMET_cfi import *
process.hbherecoReflagged = hbherechitreflaggerJETMET.clone()
process.hbherecoReflagged.hbheInputLabel=cms.untracked.InputTag("hbherecoRetimed")
process.hbherecoReflagged.ignorelowest=cms.bool(False)
#process.hbherecoReflagged.debug=cms.untracked.int32(1)

from JetMETAnalysis.HcalReflagging.HFrechitreflaggerJETMET_cff import *
process.hfrecoReflagged = hfrechitreflaggerJETMET.clone()
#process.hfrecoReflagged.PETstat.flagsToSkip =string.atoi('10',2)
#process.hfrecoReflagged.S8S1stat.flagsToSkip=string.atoi('10010',2)
#process.hfrecoReflagged.S9S1stat.flagsToSkip=string.atoi('11010',2)
#
# set  flag 4 (HFPET  -- also sets HFLongShort),
# then flag 3 (HFS8S1 -- also sets HFLongShort),
# then flag 0 (HFLongShort -- set directly via S9S1)
#process.hfrecoReflagged.FlagsToSet=(4,3,0)

process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")

process.rereco_seq = cms.Sequence(process.hbherecoRetimed*
                                  process.hbherecoReflagged*
                                  process.hfrecoReflagged)

################################################################################
# Analyzers
process.TFileService = cms.Service("TFileService", 
   fileName = cms.string("anal.root"),
    closeFileFast = cms.untracked.bool(False)
)

from MyEDmodules.HcalDelayTuner.beamtiminganal_cfi import *
process.hbtimeanal = hbtimeanal.clone()
process.hbtimeanal.runDescription       = cms.untracked.string("/JetMETTau/Run2010A-v4")
process.hbtimeanal.ampCutsInfC          = cms.bool(False)
process.hbtimeanal.minHit_GeVorfC       = cms.double(0.7)
process.hbtimeanal.minHit_GeVorfC_perChan= cms.untracked.double(20.0)
process.hbtimeanal.maxEventNum2plot     = cms.int32(30000000)
process.hetimeanal = hetimeanal.clone()
process.hetimeanal.runDescription       = cms.untracked.string("/JetMETTau/Run2010A-v4")
process.hetimeanal.ampCutsInfC          = cms.bool(False)
process.hetimeanal.minHit_GeVorfC       = cms.double(0.8)
process.hetimeanal.minHit_GeVorfC_perChan= cms.untracked.double(20.0)
process.hetimeanal.maxEventNum2plot     = cms.int32(30000000)

#process.hbtimeanal.hbheOutOfTimeFlagBit = cms.untracked.int32(-1)

process.hotimeanal = hotimeanal.clone()
process.hotimeanal.runDescription       = cms.untracked.string("/JetMETTau/Run2010A-v4")
process.hotimeanal.ampCutsInfC          = cms.bool(False)
process.hotimeanal.minHit_GeVorfC       = cms.double(3.5)
process.hotimeanal.recHitEscaleMaxGeV   = cms.double(200.5)
process.hotimeanal.maxEventNum2plot     = cms.int32(30000000)

process.hftimeanal = hftimeanal.clone()
process.hftimeanal.runDescription        = cms.untracked.string("/JetMETTau/Run2010A-v4")
process.hftimeanal.ampCutsInfC           = cms.bool(False)
process.hftimeanal.minHit_GeVorfC        = cms.double(0.8)
process.hftimeanal.minHit_GeVorfC_perChan= cms.untracked.double(40.0)
process.hftimeanal.recHitEscaleMaxGeV    = cms.double(5000.5)
process.hftimeanal.maxEventNum2plot      = cms.int32(30000000)

process.hbtimeanal.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecoReflagged")
process.hetimeanal.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecoReflagged")
process.hftimeanal.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("hfrecoReflagged")
process.hbtimeanal.eventDataPset.hbheDigiLabel   = cms.untracked.InputTag("hcalDigis")
process.hetimeanal.eventDataPset.hbheDigiLabel   = cms.untracked.InputTag("hcalDigis")
process.hotimeanal.eventDataPset.hoDigiLabel     = cms.untracked.InputTag("hcalDigis")
process.hftimeanal.eventDataPset.hfDigiLabel     = cms.untracked.InputTag("hcalDigis")

# Need conditions to convert digi ADC to fC in the analyzer

process.anal_seq     = cms.Sequence(process.hbtimeanal+
                                    process.hetimeanal+
                                    process.hotimeanal+
                                    process.hftimeanal)

process.path  = cms.Path(process.cleaning_seq*
                         process.hcalDigis*
                         process.rereco_seq*
                         process.anal_seq)
