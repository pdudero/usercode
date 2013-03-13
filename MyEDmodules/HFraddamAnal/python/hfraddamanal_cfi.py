import FWCore.ParameterSet.Config as cms

hfraddam = cms.EDAnalyzer('HFraddamAnal',
                             
    eventDataPset = cms.untracked.PSet(
      fedRawDataLabel = cms.untracked.InputTag("source"),
      tbTrigDataLabel = cms.untracked.InputTag("tbunpack"),
      laserDigiLabel  = cms.untracked.InputTag("hcalLaserReco"),
      hfDigiLabel     = cms.untracked.InputTag("hcalDigis"),
      hcalibDigiLabel = cms.untracked.InputTag("hcalDigis"),
      verbose         = cms.untracked.bool(False)
    ),
                             
    TDCpars = cms.untracked.PSet(
      TDCCutCenter = cms.untracked.double(1075),
      TDCCutWindow = cms.untracked.double(25),
      CorrectedTimeModCeiling = cms.untracked.int32(9999),
      TimeModCeiling = cms.untracked.int32(9999)
    ),

    ampCutsInfC        = cms.bool(True),
    minHit_GeVorfC     = cms.double(0),
    maxHit_GeVorfC     = cms.double(9e99),
    doPerChannel       = cms.bool(True),
    doTree             = cms.untracked.bool(True),

    hfraddamchannels = cms.vint32(-30,35,1,  -30,71,1,  -32,15,1,  -32,51,1,
                                  -34,35,1,  -34,71,1,  -36,15,1,  -36,51,1,
                                  -38,35,1,  -38,71,1,  -40,15,1,  -40,51,1,
                                  -41,35,1,  -41,71,1,

                                  -30,15,2,  -30,51,2,  -32,35,2,  -32,71,2,
                                  -34,15,2,  -34,51,2,  -36,35,2,  -36,71,2,
                                  -38,15,2,  -38,51,2,  -40,35,2,  -40,71,2,
                                  -41,15,2,  -41,51,2,

                                  30,21,1,    30,57,1,   32, 1,1,   32,37,1,
                                  34,21,1,    34,57,1,   36, 1,1,   36,37,1,
                                  38,21,1,    38,57,1,                        40,35,1,   40,71,1,
                                  
                                  41,19,1,    41,55,1,

                                  30, 1,2,    30,37,2,   32,21,2,   32,57,2,
                                  34, 1,2,    34,37,2,   36,21,2,   36,57,2,
                                  38, 1,2,    38,37,2,                        40,19,2,   40,55,2,
                                  
                                  41,35,2,    41,71,2
    ),
    tdcwindowsfile = cms.untracked.string("perchanwindows.txt"),
    rundatesfile   = cms.untracked.string("../data/rundates2012.txt"),
    s2overs1meansfile = cms.untracked.string("../data/s2overs1meansperchan.txt"),
    lumiprofilefile   = cms.untracked.string("../data/2012-delivered-perday.csv"),
                          
    bottomfeeder = cms.untracked.int32(0xbadf00d)
)
