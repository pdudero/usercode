
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
fileNames = cms.untracked.vstring(
#    '/store/data/Run2010A/JetMETTau/RECO/v2/000/137/028/F4059C65-3E71-DF11-B32B-001D09F295A1.root')
    '/store/data/Run2010B/Jet/RAW/v1/000/149/011/927B9B64-43E1-DF11-8C5C-003048678098.root')
)
#process.load("MyEDmodules.HcalDelayTuner.jetmettau_fill1207raw_1")
#process.load("MyEDmodules.HcalDelayTuner.in_cff")

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
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.RawToDigi_Data_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = 'GR10_P_V11::All'

# Replace timecorr object
#process.tc_ascii2 = cms.ESSource("HcalTextCalibrations",
#  input = cms.VPSet(cms.PSet(
#           object = cms.string('TimeCorrs'),
#             file = cms.FileInPath('MyEDmodules/HcalDelayTuner/data/timecorr_may27.csv') )
#                    )
#)
#process.prefer=cms.ESPrefer('HcalTextCalibrations','tc_ascii2')

################################################################################
# Analyzers
process.TFileService = cms.Service("TFileService", 
   fileName = cms.string("anal.root"),
    closeFileFast = cms.untracked.bool(False)
)

from MyEDmodules.HcalDelayTuner.hbheTimeFilterSequences_cff import hbtimeanal
process.hbtimeanal = hbtimeanal.clone()
process.hbtimeanal.runDescription       = cms.untracked.string("/Jet/Run2010B-v1/RAW")
process.hbtimeanal.recHitEscaleMaxGeV   = cms.double(2000.5)
#process.hbtimeanal.hbheOutOfTimeFlagBit = cms.untracked.int32(31)
from MyEDmodules.HcalDelayTuner.hbheTimeFilterSequences_cff import hetimeanal
process.hetimeanal = hetimeanal.clone()
process.hetimeanal.runDescription       = cms.untracked.string("/Jet/Run2010B-v1/RAW")
process.hetimeanal.recHitEscaleMaxGeV   = cms.double(2000.5)
#process.hetimeanal.hbheOutOfTimeFlagBit = cms.untracked.int32(31)

process.hbtimeanal.eventDataPset.hbheDigiLabel = cms.untracked.InputTag("hcalDigis")
process.hetimeanal.eventDataPset.hbheDigiLabel = cms.untracked.InputTag("hcalDigis")

# Need conditions to convert digi ADC to fC in the analyzer

process.anal_seq = cms.Sequence(process.hbtimeanal+process.hetimeanal)

# Path and EndPath definitions
#process.out_step = cms.EndPath(process.output)

process.path  = cms.Path(process.RawToDigi*
                         process.reconstruction*
                         process.cleaning_seq*
                         process.anal_seq)
