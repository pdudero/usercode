import FWCore.ParameterSet.Config as cms


process = cms.Process("MyAnal")

process.load("Configuration.StandardSequences.Geometry_cff")
process.load("RecoJets.Configuration.CaloTowersES_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to useru
    fileNames = cms.untracked.vstring(
#'/store/data/Commissioning08/Cosmics/RECO/CRAFT_ALL_V9_225-v2/0002/CA3CF466-E7F9-DD11-AF70-003048679006.root'
#'/store/data/Commissioning08/Cosmics/RAW-RECO/CRAFT_ALL_V9_TrackingPointing_225-v3/0006/E45FEB6E-A0FF-DD11-B5ED-00304867C0FC.root'
'/store/data/Commissioning08/Cosmics/RECO/CRAFT_ALL_V9_225-v2/0002/1AC595AF-BFFA-DD11-A899-0030486792B6.root'
    )
)

process.TFileService = cms.Service("TFileService",
     closeFileFast = cms.untracked.bool(True),
     fileName = cms.string('hcalanal-jettrig.root')
)

process.load("MyEDmodules.HcalTimingAnalyzer.hcalFilteredReco_cff")

#-----------------------------
# Hcal Conditions: from Global Conditions Tag
#-----------------------------
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.connect = 'frontier://Frontier/CMS_COND_21X_GLOBALTAG'
process.GlobalTag.globaltag = 'CRAFT_V4P::All' # or any other appropriate
process.prefer("GlobalTag")

# L1 GT EventSetup
process.load("L1TriggerConfig.L1GtConfigProducers.L1GtConfig_cff")
process.load("L1TriggerConfig.L1GtConfigProducers.Luminosity.startup.L1Menu_startup2_v2_Unprescaled_cff")
process.load("L1Trigger.Skimmer.l1Filter_cfi")
#process.l1Filter.algorithms = cms.vstring("L1_SingleMuOpen")
#process.l1Filter.algorithms = cms.vstring("L1_SingleEG5_00001")
process.l1Filter.algorithms = cms.vstring("L1_SingleJet10_0001")

#process.p = cms.Path(process.l1Filter*process.hcalDigis*process.myan)
process.p = cms.Path(process.l1Filter*
                     process.myanunfilt+
                     process.filt6+
                     process.filt10)

process.out = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('p')
    ),
    outputCommands = cms.untracked.vstring(
        "drop *",
        "keep *_*_*_MyAnal",
        "keep recoCaloMETs_*_*_*",
        "keep *_hbhereco_*_*",
        "keep *_hfreco_*_*",
        "keep *_horeco_*_*",
        "keep *_towerMaker_*_*"
    ),
   
    fileName = cms.untracked.string('hcaltimeanal-jettrig-pool.root')
)

process.o = cms.EndPath(process.out)
