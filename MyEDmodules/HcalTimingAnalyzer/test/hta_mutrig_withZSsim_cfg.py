import FWCore.ParameterSet.Config as cms

process = cms.Process("MyAnal")

process.load("Configuration.StandardSequences.MagneticField_38T_cff")
process.load("Configuration.StandardSequences.Geometry_cff")
process.load("Configuration.StandardSequences.RawToDigi_Data_cff")
process.load("Configuration.StandardSequences.ReconstructionCosmics_cff")

process.load("RecoJets.Configuration.CaloTowersES_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")

#--- Force ZS on hcalDigis !!! -------------
from SimCalorimetry.HcalZeroSuppressionProducers.hcalDigisRealistic_cfi import simHcalDigis
process.zsTheDigis = simHcalDigis.clone(digiLabel="hcalDigis")

process.hbhereco.digiLabel = cms.InputTag('zsTheDigis')
process.horeco.digiLabel   = cms.InputTag('zsTheDigis')
process.hfreco.digiLabel   = cms.InputTag('zsTheDigis')

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
#'/store/data/Commissioning08/Cosmics/RECO/CRAFT_ALL_V9_225-v2/0002/CA3CF466-E7F9-DD11-AF70-003048679006.root'
'/store/data/Commissioning08/Cosmics/RAW-RECO/CRAFT_ALL_V9_TrackingPointing_225-v3/0006/E45FEB6E-A0FF-DD11-B5ED-00304867C0FC.root'
#'/store/data/Commissioning08/Cosmics/RECO/CRAFT_ALL_V9_225-v2/0002/1AC595AF-BFFA-DD11-A899-0030486792B6.root'
    )
)

process.TFileService = cms.Service("TFileService",
     closeFileFast = cms.untracked.bool(True),
     fileName = cms.string('hta-mutrig-zssim-filters.root')
)

process.load("MyEDmodules.HcalTimingAnalyzer.hcalFilteredReco4CRAFT_cff")
#
process.hbherhfilt06ns.timeShiftNs = cms.double(25.0)
process.hbherhfilt10ns.timeShiftNs = cms.double(25.0)
process.hbherhfilt1ts.timeShiftNs  = cms.double(25.0)
process.hbherhfilt4ts.timeShiftNs  = cms.double(25.0)
process.horhfilt06ns.timeShiftNs   = cms.double(25.0)
process.horhfilt10ns.timeShiftNs   = cms.double(25.0)
process.horhfilt1ts.timeShiftNs    = cms.double(25.0)
process.horhfilt4ts.timeShiftNs    = cms.double(25.0)
process.hfrhfilt06ns.timeShiftNs   = cms.double(25.0)
process.hfrhfilt10ns.timeShiftNs   = cms.double(25.0)
process.hfrhfilt1ts.timeShiftNs    = cms.double(25.0)
process.hfrhfilt4ts.timeShiftNs    = cms.double(25.0)
process.hbherhfiltshp.timeShiftNs  = cms.double(25.0)
process.horhfiltshp.timeShiftNs    = cms.double(25.0)

# for unshifted 68288 run, low MET
# process.myanunfilt.hcalRecHitEscaleMaxGeV  = cms.double(150.5)
# process.myanunfiltShift.hcalRecHitEscaleMaxGeV  = cms.double(150.5)
# process.myanfilt06ns.hcalRecHitEscaleMaxGeV   = cms.double(150.5)
# process.myanfilt10ns.hcalRecHitEscaleMaxGeV  = cms.double(150.5)
# process.myanfilt1ts.hcalRecHitEscaleMaxGeV = cms.double(150.5)
# process.myanfilt4ts.hcalRecHitEscaleMaxGeV = cms.double(150.5)
process.myanunfilt.eventDataPset.hbheDigiLabel      = cms.untracked.InputTag("zsTheDigis")
process.myanunfilt.eventDataPset.hfDigiLabel        = cms.untracked.InputTag("zsTheDigis")
process.myanunfilt.eventDataPset.hoDigiLabel        = cms.untracked.InputTag("zsTheDigis")
process.myanunfiltShift.eventDataPset.hbheDigiLabel = cms.untracked.InputTag("zsTheDigis")
process.myanunfiltShift.eventDataPset.hfDigiLabel   = cms.untracked.InputTag("zsTheDigis")
process.myanunfiltShift.eventDataPset.hoDigiLabel   = cms.untracked.InputTag("zsTheDigis")
process.myanfilt06ns.eventDataPset.hbheDigiLabel    = cms.untracked.InputTag("zsTheDigis")
process.myanfilt10ns.eventDataPset.hbheDigiLabel    = cms.untracked.InputTag("zsTheDigis")
process.myanfilt1ts.eventDataPset.hbheDigiLabel     = cms.untracked.InputTag("zsTheDigis")
process.myanfilt4ts.eventDataPset.hbheDigiLabel     = cms.untracked.InputTag("zsTheDigis")
process.myanfiltshp.eventDataPset.hbheDigiLabel     = cms.untracked.InputTag("zsTheDigis")
process.myanfiltshp.eventDataPset.hfDigiLabel       = cms.untracked.InputTag("zsTheDigis")
process.myanfiltshp.eventDataPset.hoDigiLabel       = cms.untracked.InputTag("zsTheDigis")

#-----------------------------
# Hcal Conditions: from Global Conditions Tag
#-----------------------------
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.connect = 'frontier://Frontier/CMS_COND_21X_GLOBALTAG'
process.GlobalTag.globaltag = 'CRAFT_ALL_V9::All' # or any other appropriate
process.prefer("GlobalTag")

#L1 trigger validation
process.load("L1Trigger.Configuration.L1Config_cff")

# L1 GT EventSetup
process.load("L1TriggerConfig.L1GtConfigProducers.L1GtConfig_cff")
process.load("L1TriggerConfig.L1GtConfigProducers.Luminosity.startup.L1Menu_startup2_v2_Unprescaled_cff")
process.load("L1Trigger.Skimmer.l1Filter_cfi")
process.l1Filter.algorithms = cms.vstring("L1_SingleMuOpen")

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
                     process.myOtherDigis*process.zsTheDigis*process.myAbbrevReco*
#                    process.allSQfiltsWithShift)
                     process.mixedFiltsWithShift)

#process.p = cms.Path(process.l1Filter*process.hcalDigis*process.myan)

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
   
#     fileName = cms.untracked.string('hcaltimeanal-mutrig-pool.root')
# )

# process.o = cms.EndPath(process.out)
