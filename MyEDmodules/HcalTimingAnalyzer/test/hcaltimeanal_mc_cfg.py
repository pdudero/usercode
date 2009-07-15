import FWCore.ParameterSet.Config as cms

process = cms.Process("MyAnal")

process.load("Configuration.StandardSequences.Geometry_cff")
process.load("RecoJets.Configuration.CaloTowersES_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")

process.MessageLogger.cerr.default.limit = cms.untracked.int32(1000)
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(100)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

process.source = cms.Source("PoolSource",
   # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring( # 'myfile.root'
 '/store/mc/Summer08/WJets-madgraph/GEN-SIM-RECO/IDEAL_V11_redigi_v1/0016/FECF78F2-BBE9-DD11-8EAA-003048D15DCA.root',
                            )
)

#process.load("MyEDmodules.HcalTimingAnalyzer.in_cff")

process.TFileService = cms.Service("TFileService",
     closeFileFast = cms.untracked.bool(True),
     fileName = cms.string('hta_smearedmc-filters.root')
)

process.load("MyEDmodules.HcalTimingAnalyzer.hcalFilteredRecoMC_cff")
process.myanraw.runDescription        = cms.untracked.string('QCD')  # ('Wjets')
process.myanunfilt.runDescription     = cms.untracked.string('QCD')  # ('Wjets')
process.myanfilt06ns.runDescription   = cms.untracked.string('QCD')  # ('Wjets')
process.myanfilt10ns.runDescription   = cms.untracked.string('QCD')  # ('Wjets')
process.myanfilt1ts.runDescription    = cms.untracked.string('QCD')  # ('Wjets')
process.myanfilt4ts.runDescription    = cms.untracked.string('QCD')  # ('Wjets ')
process.myanfiltshp.runDescription    = cms.untracked.string('QCD')  # ('SusyLM1')  # ('Wjets')

process.myanraw.ecalRecHitTscaleNbins  = cms.int32(201)
process.myanraw.ecalRecHitTscaleMinNs  = cms.double(-50.5)
process.myanraw.ecalRecHitTscaleMaxNs  = cms.double(50.5)
process.myanraw.hcalRecHitTscaleNbins  = cms.int32(201)
process.myanraw.hcalRecHitTscaleMinNs  = cms.double(-20.5)
process.myanraw.hcalRecHitTscaleMaxNs  = cms.double(80.5)
process.myanraw.hcalRecHitEscaleMaxGeV = cms.double(3000.5)   # 1000 for SUSY, 3000 for QCD

process.myanunfilt.ecalRecHitTscaleNbins    = cms.int32(201)
process.myanunfilt.ecalRecHitTscaleMinNs    = cms.double(-50.5)
process.myanunfilt.ecalRecHitTscaleMaxNs    = cms.double(50.5)
process.myanunfilt.hcalRecHitTscaleNbins    = cms.int32(201)
process.myanunfilt.hcalRecHitTscaleMinNs    = cms.double(-20.5)
process.myanunfilt.hcalRecHitTscaleMaxNs    = cms.double(80.5)
process.myanunfilt.hcalRecHitEscaleMaxGeV   = cms.double(3000.5)   # 1000 for SUSY, 3000 for QCD

process.myanfilt06ns.ecalRecHitTscaleNbins  = cms.int32(201)
process.myanfilt06ns.ecalRecHitTscaleMinNs  = cms.double(-50.5)
process.myanfilt06ns.ecalRecHitTscaleMaxNs  = cms.double(50.5)
process.myanfilt06ns.hcalRecHitTscaleNbins  = cms.int32(201)
process.myanfilt06ns.hcalRecHitTscaleMinNs  = cms.double(-20.5)
process.myanfilt06ns.hcalRecHitTscaleMaxNs  = cms.double(80.5)
process.myanfilt06ns.hcalRecHitEscaleMaxGeV = cms.double(3000.5)   # 1000 for SUSY, 3000 for QCD

process.myanfilt10ns.ecalRecHitTscaleNbins  = cms.int32(201)
process.myanfilt10ns.ecalRecHitTscaleMinNs  = cms.double(-50.5)
process.myanfilt10ns.ecalRecHitTscaleMaxNs  = cms.double(50.5)
process.myanfilt10ns.hcalRecHitTscaleNbins  = cms.int32(201)
process.myanfilt10ns.hcalRecHitTscaleMinNs  = cms.double(-20.5)
process.myanfilt10ns.hcalRecHitTscaleMaxNs  = cms.double(80.5)
process.myanfilt10ns.hcalRecHitEscaleMaxGeV = cms.double(3000.5)   # 1000 for SUSY, 3000 for QCD

process.myanfilt1ts.ecalRecHitTscaleNbins   = cms.int32(201)
process.myanfilt1ts.ecalRecHitTscaleMinNs   = cms.double(-50.5)
process.myanfilt1ts.ecalRecHitTscaleMaxNs   = cms.double(50.5)
process.myanfilt1ts.hcalRecHitTscaleNbins   = cms.int32(201)
process.myanfilt1ts.hcalRecHitTscaleMinNs   = cms.double(-20.5)
process.myanfilt1ts.hcalRecHitTscaleMaxNs   = cms.double(80.5)
process.myanfilt1ts.hcalRecHitEscaleMaxGeV  = cms.double(3000.5)   # 1000 for SUSY, 3000 for QCD

process.myanfilt4ts.ecalRecHitTscaleNbins   = cms.int32(201)
process.myanfilt4ts.ecalRecHitTscaleMinNs   = cms.double(-50.5)
process.myanfilt4ts.ecalRecHitTscaleMaxNs   = cms.double(50.5)
process.myanfilt4ts.hcalRecHitTscaleNbins   = cms.int32(201)
process.myanfilt4ts.hcalRecHitTscaleMinNs   = cms.double(-20.5)
process.myanfilt4ts.hcalRecHitTscaleMaxNs   = cms.double(80.5)
process.myanfilt4ts.hcalRecHitEscaleMaxGeV  = cms.double(3000.5)   # 1000 for SUSY, 3000 for QCD

process.myanfiltshp.ecalRecHitTscaleNbins   = cms.int32(201)
process.myanfiltshp.ecalRecHitTscaleMinNs   = cms.double(-50.5)
process.myanfiltshp.ecalRecHitTscaleMaxNs   = cms.double(50.5)
process.myanfiltshp.hcalRecHitTscaleNbins   = cms.int32(201)
process.myanfiltshp.hcalRecHitTscaleMinNs   = cms.double(-20.5)
process.myanfiltshp.hcalRecHitTscaleMaxNs   = cms.double(80.5)
process.myanfiltshp.hcalRecHitEscaleMaxGeV  = cms.double(3000.5)   # 1000 for SUSY, 3000 for QCD

#-----------------------------
# Hcal Conditions: from Global Conditions Tag
#-----------------------------
#process.load("Configuration.StandardSequences.FakeConditions_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.connect = 'frontier://Frontier/CMS_COND_21X_GLOBALTAG'
process.GlobalTag.globaltag = 'IDEAL_V12::All' # or any other appropriate
process.prefer("GlobalTag")

process.p = cms.Path(process.hbhesmear*process.mixedMCfilts)
#process.p = cms.Path(process.hbhesmear*process.allMCSQfilts)

#process.out = cms.OutputModule("PoolOutputModule",
#    SelectEvents = cms.untracked.PSet(
#        SelectEvents = cms.vstring('p')
#    ),
#    outputCommands = cms.untracked.vstring(
#        "drop *",
#        "keep *_*_*_MyAnal",
#        "keep recoCaloMETs_*_*_*",
#        "keep *_hbhereco_*_*",
#        "keep *_hfreco_*_*",
#        "keep *_horeco_*_*",
#        "keep *_towerMaker_*_*"
#    ),
#   
#    fileName = cms.untracked.string('hta_smearedmc-pool.root')
#)

#process.o = cms.EndPath(process.out)
