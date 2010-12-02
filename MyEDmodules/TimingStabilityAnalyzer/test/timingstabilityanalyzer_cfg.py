import FWCore.ParameterSet.Config as cms

process = cms.Process("TSA")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
#    '/store/data/Run2010B/Jet/RECO/PromptReco-v2/000/148/864/8893AA77-09E1-DF11-8C84-0030487A3C92.root'
    '/store/data/Run2010B/Jet/RECO/PromptReco-v2/000/146/807/423DE393-0CCB-DF11-9470-003048F1110E.root'
    )
)

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

process.TFileService = cms.Service("TFileService", 
   fileName = cms.string("tsa.root"),
   closeFileFast = cms.untracked.bool(False)
)


process.tsa = cms.EDAnalyzer('TimingStabilityAnalyzer',
     eventDataPset = cms.untracked.PSet(
      hbheRechitLabel = cms.untracked.InputTag("hbhereco"),
      verbose         = cms.untracked.bool(False)
      ),

     HBdetIds2mask   = cms.vint32(99,54,99,
                                  -13,7,1,
                                  14,31,1),
     HEdetIds2mask   = cms.vint32(#-99,51,99,
                                  #-99,52,99,
                                  #-99,53,99,
                                  #-99,54,99,
                                  -19,43,2)
)


process.p = cms.Path(process.cleaning_seq*process.tsa)
