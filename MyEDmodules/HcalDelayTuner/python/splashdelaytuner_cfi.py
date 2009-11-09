import FWCore.ParameterSet.Config as cms

tunedelays = cms.EDAnalyzer('SplashDelayTuner',
                            
    writeBricks  = cms.untracked.bool(False),
    subdet       = cms.untracked.string("ALL"),
    rootFilename = cms.untracked.string(""),
    hbRMprofileHistos = cms.untracked.vstring(),
    heRMprofileHisto = cms.untracked.string(""),
    hoRMprofileHisto = cms.untracked.string(""),
    oldSettingFilenames = cms.untracked.vstring(""),
                            
    bottomfeeder = cms.untracked.int32(0xbadf00d)
)
