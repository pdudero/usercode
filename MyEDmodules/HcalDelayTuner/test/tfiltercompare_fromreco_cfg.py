
import FWCore.ParameterSet.Config as cms

process = cms.Process("BEAMTIMEANAL")
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( -1 )
# test it!
#   input = cms.untracked.int32( 1 ) 
)

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.source=cms.Source("PoolSource",
fileNames = cms.untracked.vstring(  # 'in.root')
    '/store/data/Run2010A/JetMETTau/RECO/v4/000/140/160/0CA7C000-1F90-DF11-B17C-003048F11C5C.root')
)

#process.load("in_cff")

#from RunFiles.GoodRunList.goodRunList_cfi import lumisToProcess
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
process.HBHENoiseFilter.maxRBXEMF = cms.double(0.01)

################################################################################
# Configure Retimer-Reflagger-Tower-MET rereco chain

process.load('Configuration.StandardSequences.GeometryExtended_cff')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = 'GR_R_36X_V12::All'

# Replace timecorr object
process.tc_ascii2 = cms.ESSource("HcalTextCalibrations",
  input = cms.VPSet(cms.PSet(
           object = cms.string('TimeCorrs'),
             file = cms.FileInPath('MyEDmodules/HcalDelayTuner/data/timecorr_4retimer_aug20_withHF.csv') )
                    )
)
process.prefer=cms.ESPrefer('HcalTextCalibrations','tc_ascii2')
process.hbherecoRetimed = cms.EDProducer('HBHERecHitRetimer',
                                         input = cms.InputTag('hbhereco')
)

process.load("MyEDmodules.HcalDelayTuner.hbheTimeFilterSequences_cff")
process.hbheReflagStdShape.hbheInputLabel          =cms.untracked.InputTag("hbherecoRetimed")
process.hbheReflagGained10pctPlus2ns.hbheInputLabel=cms.untracked.InputTag("hbherecoRetimed")
process.hbheReflagGained15pctPlus2ns.hbheInputLabel=cms.untracked.InputTag("hbherecoRetimed")
process.hbheReflagSquareFilter.hbheInputLabel      =cms.untracked.InputTag("hbherecoRetimed")

################################################################################
# Analyzers
process.TFileService = cms.Service("TFileService", 
   fileName = cms.string("anal.root"),
    closeFileFast = cms.untracked.bool(False)
)

# Need conditions to convert digi ADC to fC in the analyzer

process.p = cms.Path(process.cleaning_seq*
                     process.hbherecoRetimed*
                     process.allFilterSeqs)
