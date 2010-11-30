
import FWCore.ParameterSet.Config as cms

process = cms.Process("BTA")
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

NOISEFILE="HighPfMETlowtcMET"
#NOISEFILE="HightcMETlowPFMET"
INFILE='file:/uscms/home/stjohn/public_html/'+NOISEFILE+'.root'
ANALFILE=NOISEFILE+"-anal.root"
POOLFILE=NOISEFILE+"himetevents_reco.root"

process.source=cms.Source("PoolSource",
fileNames = cms.untracked.vstring(
#    '/store/data/Run2010A/JetMETTau/RECO/v2/000/137/028/F4059C65-3E71-DF11-B32B-001D09F295A1.root'
#     '/store/data/Commissioning10/MinimumBias/RECO/Jun14thReReco_v1/0062/A8E20604-227B-DF11-A16E-00261894389F.root'
    INFILE
))

#process.load("MyEDmodules.HcalDelayTuner.jetmettau_postMDreco_v4_100")
#process.load("MyEDmodules.HcalDelayTuner.in_cff")

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

################################################################################
# Define the "dirtying" sequence

process.load("L1TriggerConfig.L1GtConfigProducers.L1GtTriggerMaskTechTrigConfig_cff")
process.load("HLTrigger.HLTfilters.hltLevel1GTSeed_cfi")
process.hltLevel1GTSeed.L1TechTriggerSeeding = cms.bool(True)
process.hltLevel1GTSeed.L1SeedsLogicalExpression = cms.string("NOT (0 OR 40 OR 41)")

# process.oneGoodVertexFilter = cms.EDFilter("VertexSelector",
#    src = cms.InputTag("offlinePrimaryVertices"),
# #   cut = cms.string("!isFake && tracksSize > 3 && abs(z) <= 15 && position.Rho <= 2"),
#    cut = cms.string("!isFake && ndof >= 5 && abs(z) <= 15"),
#    filter = cms.bool(True)   # otherwise it won't filter the events, just produce an empty vertex collection.
# )

# process.noscraping = cms.EDFilter("FilterOutScraping",
#                                   applyfilter = cms.untracked.bool(True),
#                                   debugOn = cms.untracked.bool(False),
#                                   numtrack = cms.untracked.uint32(10),
#                                   thresh = cms.untracked.double(0.25)
#                                   )

#process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')
#process.dirtying_seq = cms.Sequence(!process.HBHENoiseFilter)
process.dirtying_seq = cms.Sequence(process.hltLevel1GTSeed)
#process.dirtying_seq = cms.Sequence(process.hltLevel1GTSeed*process.HBHENoiseFilter)

################################################################################
# Configure Retimer-Reflagger-Tower-MET rereco chain

process.load('Configuration.StandardSequences.GeometryExtended_cff')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = 'GR_R_36X_V12::All'

# Replace timecorr object
#process.tc_ascii2 = cms.ESSource("HcalTextCalibrations",
#  input = cms.VPSet(cms.PSet(
#           object = cms.string('TimeCorrs'),
#             file = cms.FileInPath('MyEDmodules/HcalDelayTuner/data/timecorr_4retimer_aug20_withHF.csv') )
#                    )
#)
#process.prefer=cms.ESPrefer('HcalTextCalibrations','tc_ascii2')
#process.hbherecoRetimed = cms.EDProducer('HBHERecHitRetimer',
#                                         input = cms.InputTag('hbhereco')
#)

process.load("MyEDmodules.HcalDelayTuner.hbheTimeFilterSequences_cff")
#process.hbheReflagStdShape.hbheInputLabel           =cms.untracked.InputTag("hbherecoRetimed")
#process.hbheReflagSquareFilter.hbheInputLabel        =cms.untracked.InputTag("hbherecoRetimed")
#process.hbheReflagSquareFilterStopAt50.hbheInputLabel=cms.untracked.InputTag("hbherecoRetimed")

#process.unfiltTowersHcalOnly.hbheInput=cms.InputTag("hbherecoRetimed")

#Exclude ECAL energy altogether
#process.filtTowersStdShape.EBSumThreshold = cms.double(999999.9)
#process.filtTowersStdShape.EESumThreshold = cms.double(999999.9)
#process.filtTowersStdShape.EBThreshold    = cms.double(9999.9)
#process.filtTowersStdShape.EEThreshold    = cms.double(9999.9)

process.filtTowersStdShape.EBWeight        = cms.double(0.0)
process.filtTowersStdShape.EEWeight        = cms.double(0.0)
process.filtTowersStdShape.EBWeights       = cms.vdouble(0.,0.,0.,0.,0.)
process.filtTowersStdShape.EEWeights       = cms.vdouble(0.,0.,0.,0.,0.)

process.filtTowersSquareFilter.EBWeight            = cms.double(0.0)
process.filtTowersSquareFilter.EEWeight            = cms.double(0.0)
process.filtTowersSquareFilter.EBWeights           = cms.vdouble(0.,0.,0.,0.,0.)
process.filtTowersSquareFilter.EEWeights           = cms.vdouble(0.,0.,0.,0.,0.)

process.filtTowersSquareFilterStopAt50.EBWeight    = cms.double(0.0)
process.filtTowersSquareFilterStopAt50.EEWeight    = cms.double(0.0)
process.filtTowersSquareFilterStopAt50.EBWeights   = cms.vdouble(0.,0.,0.,0.,0.)
process.filtTowersSquareFilterStopAt50.EEWeights   = cms.vdouble(0.,0.,0.,0.,0.)

################################################################################

# Clone Reco Met
process.CloneWhichMet = cms.EDProducer("CaloMETShallowCloneProducer",
    src = cms.InputTag("filtmetStdShape")
)

# Select Reco Met
process.SelectWhichMet = cms.EDFilter("PtMinCandSelector",
      src = cms.InputTag("CloneWhichMet"),
    ptMin = cms.double(100.0),
   filter = cms.bool(True)   # otherwise it won't filter the events!
)

process.himetfilterseq = cms.Sequence(process.CloneWhichMet*
                                      process.SelectWhichMet)

################################################################################
# Analyzers
process.TFileService = cms.Service("TFileService", 
   fileName = cms.string(ANALFILE),
    closeFileFast = cms.untracked.bool(False)
)

process.binners = cms.Path(#process.dirtying_seq*
#                           process.hbherecoRetimed*
                           process.allFilterSeqs)

process.himetfilterseq = cms.Sequence(process.CloneWhichMet*
                                      process.SelectWhichMet)


process.himetfilterpath = cms.Path(#process.dirtying_seq*
#                                   process.hbherecoRetimed*
                                   process.himetfilterseq)

#------------------------------
# Output module configuration
#------------------------------
#
process.load('Configuration/EventContent/EventContent_cff')
process.output = cms.OutputModule("PoolOutputModule",
    process.RECOEventContent,
    fileName = cms.untracked.string(POOLFILE),
    # save only events passing the full path
    SelectEvents=cms.untracked.PSet(SelectEvents=cms.vstring('himetfilterpath')),
                                  
#    outputCommands = cms.untracked.vstring("drop *", "keep *_*_*_BEAMTIMEANAL")
)

process.out_step = cms.EndPath(process.output)

process.schedule = cms.Schedule(process.binners,
                                process.himetfilterpath,
                                process.out_step)
