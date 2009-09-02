import FWCore.ParameterSet.Config as cms

process = cms.Process("MyAnal")

process.load("Configuration.StandardSequences.MagneticField_38T_cff")
process.load("Configuration.StandardSequences.Geometry_cff")
process.load("Configuration.StandardSequences.RawToDigi_Data_cff")
process.load("Configuration.StandardSequences.ReconstructionCosmics_cff")

process.load("RecoJets.Configuration.CaloTowersES_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")
# process.MessageLogger.cout.threshold = cms.untracked.string('DEBUG')

process.MessageLogger.cerr.default.limit = cms.untracked.int32(1000)
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(100)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )


process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to useru
    fileNames = cms.untracked.vstring(
'/store/data/CRAFT09/HcalHPDNoise/RAW/v1/000/111/045/14DDFAEF-C78A-DE11-8BEC-003048D2C0F0.root',
'/store/data/CRAFT09/HcalHPDNoise/RAW/v1/000/111/045/A0893484-CB8A-DE11-80A9-001D09F2305C.root'
#    'file:/tmp/dudero/A0893484-CB8A-DE11-80A9-001D09F2305C.root'
#'/store/data/Commissioning08/Cosmics/RECO/CRAFT_ALL_V9_225-v2/0002/CA3CF466-E7F9-DD11-AF70-003048679006.root'
#'/store/data/Commissioning08/Cosmics/RAW-RECO/CRAFT_ALL_V9_TrackingPointing_225-v3/0006/E45FEB6E-A0FF-DD11-B5ED-00304867C0FC.root'
#'/store/data/Commissioning08/Cosmics/RECO/CRAFT_ALL_V9_225-v2/0002/1AC595AF-BFFA-DD11-A899-0030486792B6.root'
    )
)

process.TFileService = cms.Service("TFileService",
     closeFileFast = cms.untracked.bool(True),
     fileName = cms.string('hta-egtrig-run111045.root')
)

process.load("MyEDmodules.HcalTimingAnalyzer.hcalFilteredReco_cff")
process.myanunfilt.eventDataPset.hbheDigiLabel  = cms.untracked.InputTag("hcalDigis")
process.myanfiltshp.eventDataPset.hbheDigiLabel = cms.untracked.InputTag("hcalDigis")
process.myanfilt1ts.eventDataPset.hbheDigiLabel = cms.untracked.InputTag("hcalDigis")
process.myanfilt4ts.eventDataPset.hbheDigiLabel = cms.untracked.InputTag("hcalDigis")

#-----------------------------
# Hcal Conditions: from Global Conditions Tag
#-----------------------------
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
#process.GlobalTag.connect = 'frontier://Frontier/CMS_COND_21X_GLOBALTAG'
#process.GlobalTag.globaltag = 'CRAFT_ALL_V9::All' # or any other appropriate
process.GlobalTag.globaltag = 'GR09_P_V1::All' # or any other appropriate
process.prefer("GlobalTag")

#L1 trigger validation
process.load("L1Trigger.Configuration.L1Config_cff")

# L1 GT EventSetup
process.load("L1TriggerConfig.L1GtConfigProducers.L1GtConfig_cff")
process.load("L1TriggerConfig.L1GtConfigProducers.Luminosity.startup.L1Menu_startup2_v2_Unprescaled_cff")
process.load("L1Trigger.Skimmer.l1Filter_cfi")
#process.l1Filter.algorithms = cms.vstring("L1_SingleMuOpen")
#process.l1Filter.algorithms = cms.vstring("L1_SingleEG5_00001")
process.l1Filter.algorithms = cms.vstring("L1_SingleEG5")

process.myOtherDigis = cms.Sequence(process.siPixelDigis+
                                    process.siStripDigis+
                                    process.ecalDigis+
                                    process.ecalPreshowerDigis+
                                    process.hcalDigis)

process.myAbbrevReco = cms.Sequence(process.trackerCosmics*
                                    process.caloCosmics*
                                    process.recoCaloTowersGR*
                                    process.metNoHF)

process.p = cms.Path(process.gtDigis*process.l1Filter*
                     process.myOtherDigis*process.myAbbrevReco*
                     process.myanunfilt*process.timeFiltSeq*process.flagFiltSeq)

# process.out = cms.OutputModule("PoolOutputModule",
#     SelectEvents = cms.untracked.PSet(
#         SelectEvents = cms.vstring('p')
#     ),
#     outputCommands = cms.untracked.vstring(
#         "drop *",
#         "keep *_*_*_MyAnal",
#         "keep recoCaloMETs_*_*_*",
#         "keep *_hbhereco_*_*",
#         "keep *_hfreco_*_*",
#         "keep *_horeco_*_*",
#         "keep *_towerMaker_*_*"
#     ),
   
#     fileName = cms.untracked.string('hta-egtrig-run111045-pool.root')
# )

# process.o = cms.EndPath(process.out)
