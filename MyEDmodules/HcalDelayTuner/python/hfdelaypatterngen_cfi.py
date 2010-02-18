import FWCore.ParameterSet.Config as cms

hfdelaypatgen = cms.EDAnalyzer('HFdelayPatternGenerator',
    globalShiftNS = cms.untracked.int32(0),
    tableFilename = cms.untracked.string("hfdelpatgen.csv"),
    writeBricks   = cms.untracked.bool(False),
    bottomfeeder  = cms.untracked.int32(0xbadf00d)
)
