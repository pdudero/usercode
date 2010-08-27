import FWCore.ParameterSet.Config as cms

tunedelays = cms.EDAnalyzer('HcalDelayTuner',
                            
    writeBricks         = cms.untracked.bool(False),
    subdet              = cms.untracked.string("ALL"),
    timecorrFilenames   = cms.untracked.vstring(),
    timecorrScanFmt     = cms.untracked.string("%s %d %d %d %d"),
    fileNames           = cms.untracked.vstring(),
    settingScanFmt      = cms.untracked.string("%s %d %d %d %d"),
    clipAtLimits        = cms.untracked.bool(False),
    overwriteDuplicates_= cms.untracked.bool(False),

    bottomfeeder = cms.untracked.int32(0xbadf00d)
)
