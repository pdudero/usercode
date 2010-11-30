
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

process.source=cms.Source("PoolSource",
fileNames = cms.untracked.vstring(
#    '/store/data/Run2010A/JetMETTau/RECO/v2/000/137/028/F4059C65-3E71-DF11-B32B-001D09F295A1.root'
    '/store/data/Run2010B/Jet/RECO/PromptReco-v2/000/148/864/8893AA77-09E1-DF11-8C84-0030487A3C92.root'
    )
)
#process.load("MyEDmodules.HcalDelayTuner.jetmettau_fill1222reco_v4_10")
#process.load("MyEDmodules.HcalDelayTuner.in_cff")

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
#process.GlobalTag.globaltag = 'GR_R_36X_V12::All'
process.GlobalTag.globaltag = 'GR10_P_V11::All'

# Replace timecorr object
# process.tc_ascii2 = cms.ESSource("HcalTextCalibrations",
#   input = cms.VPSet(cms.PSet(
#            object = cms.string('TimeCorrs'),
#              file = cms.FileInPath('MyEDmodules/HcalDelayTuner/data/timecorr_4retimer_aug20_withHF.csv') )
#                     )
# )
# process.prefer=cms.ESPrefer('HcalTextCalibrations','tc_ascii2')
# process.hbherecoRetimed = cms.EDProducer('HBHERecHitRetimer',
#                                          input = cms.InputTag('hbhereco')
# )

process.load("MyEDmodules.HcalDelayTuner.hbheTimeFilterSequences_cff")
#process.hbheReflagStdShape.hbheInputLabel           =cms.untracked.InputTag("hbherecoRetimed")
#process.hbheReflagGained10pctPlus2ns.hbheInputLabel =cms.untracked.InputTag("hbherecoRetimed")
#process.hbheReflagGained15pctPlus2ns.hbheInputLabel =cms.untracked.InputTag("hbherecoRetimed")
#process.hbheReflag3xNewEnvDownTo0.hbheInputLabel    =cms.untracked.InputTag("hbherecoRetimed")
#process.hbheReflag3xNewEnvStopAt50.hbheInputLabel   =cms.untracked.InputTag("hbherecoRetimed")
#process.hbheReflagSquareFilter.hbheInputLabel        =cms.untracked.InputTag("hbherecoRetimed")
#process.hbheReflagSquareFilterStopAt50.hbheInputLabel=cms.untracked.InputTag("hbherecoRetimed")

#process.unfiltTowersHcalOnly.hbheInput=cms.InputTag("hbherecoRetimed")

#Exclude ECAL energy altogether
#process.filtTowers3xNewEnvStopAt50.EBSumThreshold = cms.double(999999.9)
#process.filtTowers3xNewEnvStopAt50.EESumThreshold = cms.double(999999.9)
#process.filtTowers3xNewEnvStopAt50.EBThreshold    = cms.double(9999.9)
#process.filtTowers3xNewEnvStopAt50.EEThreshold    = cms.double(9999.9)

process.filtTowersStdShape.EBWeight               = cms.double(0.0)
process.filtTowersStdShape.EEWeight               = cms.double(0.0)
process.filtTowersStdShape.EBWeights              = cms.vdouble(0.,0.,0.,0.,0.)
process.filtTowersStdShape.EEWeights              = cms.vdouble(0.,0.,0.,0.,0.)

process.filtTowersSquareFilter.EBWeight           = cms.double(0.0)
process.filtTowersSquareFilter.EEWeight           = cms.double(0.0)
process.filtTowersSquareFilter.EBWeights          = cms.vdouble(0.,0.,0.,0.,0.)
process.filtTowersSquareFilter.EEWeights          = cms.vdouble(0.,0.,0.,0.,0.)

process.filtTowersSquareFilterStopAt50.EBWeight   = cms.double(0.0)
process.filtTowersSquareFilterStopAt50.EEWeight   = cms.double(0.0)
process.filtTowersSquareFilterStopAt50.EBWeights  = cms.vdouble(0.,0.,0.,0.,0.)
process.filtTowersSquareFilterStopAt50.EEWeights  = cms.vdouble(0.,0.,0.,0.,0.)

#
# compare apples to apples!
#
process.metcmpfiltStdShape.dirtyInput.caloMETlabel=cms.untracked.InputTag("unfiltmetHcalOnly")

################################################################################

# Clone Reco Met
# process.CloneWhichMet = cms.EDProducer("CaloMETShallowCloneProducer",
#     src = cms.InputTag("filtmet3xNewEnvStopAt50","","BTA")
# )

# # Select Reco Met
# process.SelectWhichMet = cms.EDFilter("PtMinCandSelector",
#       src = cms.InputTag("CloneWhichMet"),
#     ptMin = cms.double(500.0),
#    filter = cms.bool(True)   # otherwise it won't filter the events!
# )

# process.himetfilterseq = cms.Sequence(process.CloneWhichMet*
#                                       process.SelectWhichMet)

################################################################################
# Analyzers
process.TFileService = cms.Service("TFileService", 
   fileName = cms.string("anal.root"),
    closeFileFast = cms.untracked.bool(False)
)

process.binners = cms.Sequence(process.cleaning_seq*
                               #process.hbherecoRetimed*
                               process.allFilterSeqs)

#process.himetfilterseq = cms.Sequence(process.CloneWhichMet*
#                                      process.SelectWhichMet)


#process.himetfilterpath = cms.Path(process.cleaning_seq*
##                                   process.hbherecoRetimed*
#                                   process.himetfilterseq)

#------------------------------
# Output module configuration
#------------------------------
#
# process.output = cms.OutputModule("PoolOutputModule",
#     fileName = cms.untracked.string('himetevents_reco.root'),
#     # save only events passing the full path
#     SelectEvents=cms.untracked.PSet(SelectEvents=cms.vstring('himetfilterpath')),
                                  
#     outputCommands = cms.untracked.vstring("drop *", "keep *_*_*_BTA")
# )

process.out_step = cms.EndPath(process.binners) # *process.output)

process.schedule = cms.Schedule(#process.himetfilterpath,
                                process.out_step)
