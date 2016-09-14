import FWCore.ParameterSet.Config as cms

year=2012

heraddam = cms.EDAnalyzer('HEraddamAnal',
                             
    eventDataPset = cms.untracked.PSet(
      fedRawDataLabel = cms.untracked.InputTag("source"),
      tbTrigDataLabel = cms.untracked.InputTag("tbunpack"),
      laserDigiLabel  = cms.untracked.InputTag("hcalLaserReco"),
      hbheDigiLabel   = cms.untracked.InputTag("hcalDigis"),
      hcalibDigiLabel = cms.untracked.InputTag("hcalDigis"),
      verbose         = cms.untracked.bool(False)
    ),
                             
    TDCpars = cms.untracked.PSet(
      TDCCutCenter = cms.untracked.double(1075),
      TDCCutWindow = cms.untracked.double(25),
      CorrectedTimeModCeiling = cms.untracked.int32(9999),
      TimeModCeiling = cms.untracked.int32(9999)
    ),

    year               = cms.int32(year),
    ampCutsInfC        = cms.bool(True),
    minHit_GeVorfC     = cms.double(100),
    maxHit_GeVorfC     = cms.double(1000),
    doPerChannel       = cms.bool(True),
    doTree             = cms.untracked.bool(True),

    heraddamchannels = cms.vint32(99,99,99, -99,99,99),
    tdcwindowsfile = cms.untracked.string("perchanwindows.txt"),
    rundatesfile   = cms.untracked.string("../data/rundates%d.txt"%year),
    lumiprofilefile   = cms.untracked.string("../data/%d-delivered-perday.csv"%year),
                          
    bottomfeeder = cms.untracked.int32(0xbadf00d)
)
