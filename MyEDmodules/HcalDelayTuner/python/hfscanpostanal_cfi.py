import FWCore.ParameterSet.Config as cms

readFiles = cms.vstring()

hfscananal = cms.EDAnalyzer('HFscanPostAnal',
    globalOffsetns = cms.int32(0),
    fileVectorOnePerSetting = readFiles
)
