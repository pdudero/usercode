import FWCore.ParameterSet.Config as cms

plotdelays = cms.EDAnalyzer('DelaySettingPlotter',
    oldSettingParameters = cms.PSet(
       fileNames = cms.untracked.vstring("")
    ),
    newSettingParameters = cms.PSet(
      fileNames = cms.untracked.vstring("")
    ),
    bottomfeeder = cms.untracked.int32(0xbadf00d)
)
