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

process.hbhereco.digiLabel = 'zsTheDigis'

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
'/store/data/Commissioning08/Cosmics/RAW/v1/000/072/011/88DF3F26-0AB7-DD11-A966-000423D6CAF2.root'
#'/store/data/Commissioning08/Cosmics/RECO/CRAFT_ALL_V9_225-v2/0002/CA3CF466-E7F9-DD11-AF70-003048679006.root'
#'/store/data/Commissioning08/Cosmics/RAW-RECO/CRAFT_ALL_V9_TrackingPointing_225-v3/0006/E45FEB6E-A0FF-DD11-B5ED-00304867C0FC.root'
#'/store/data/Commissioning08/Cosmics/RECO/CRAFT_ALL_V9_225-v2/0002/1AC595AF-BFFA-DD11-A899-0030486792B6.root'
    )
)

process.TFileService = cms.Service("TFileService",
     closeFileFast = cms.untracked.bool(True),
     fileName = cms.string('hta-egtrig_pulses.root')
)

process.load("MyEDmodules.HcalTimingAnalyzer.hcalRHfiltersCRAFTegTrig_cff")
from MyEDmodules.HcalTimingAnalyzer.hcaltimeanal_cfi import *

process.myanTS1 = myan.clone()
process.myanTS1.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfiltTS1")
process.myanTS1.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("")
process.myanTS1.eventDataPset.hoRechitLabel   = cms.untracked.InputTag("")
process.myanTS1.eventDataPset.metLabel        = cms.untracked.InputTag("")
process.myanTS1.eventDataPset.twrLabel        = cms.untracked.InputTag("")
process.myanTS1.eventDataPset.hbheDigiLabel   = cms.untracked.InputTag("zsTheDigis")
process.myanTS1.eventDataPset.verbose         = cms.untracked.bool(True)
process.myanTS1.minHitGeVHB                   = cms.double(50.0)
process.myanTS1.minHitGeVHE                   = cms.double(50.0)
process.myanTS1.hcalRecHitEscaleMinGeV        = cms.double(-0.5)
process.myanTS1.hcalRecHitEscaleMaxGeV        = cms.double(450.5)

process.myanTS2 = process.myanTS1.clone()
process.myanTS3 = process.myanTS1.clone()
process.myanTS4 = process.myanTS1.clone()
process.myanTS5 = process.myanTS1.clone()
process.myanTS6 = process.myanTS1.clone()
process.myanTS7 = process.myanTS1.clone()
process.myanTS8 = process.myanTS1.clone()

process.myanTS2.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfiltTS2")
process.myanTS3.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfiltTS3")
process.myanTS4.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfiltTS4")
process.myanTS5.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfiltTS5")
process.myanTS6.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfiltTS6")
process.myanTS7.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfiltTS7")
process.myanTS8.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherhfiltTS8")

process.allTSanals = cms.Sequence(process.myanTS1+
                                  process.myanTS2+
                                  process.myanTS3+
                                  process.myanTS4+
                                  process.myanTS5+
                                  process.myanTS6+
                                  process.myanTS7+
                                  process.myanTS8)

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
#process.l1Filter.algorithms = cms.vstring("L1_SingleMuOpen")
process.l1Filter.algorithms = cms.vstring("L1_SingleEG5_00001")

process.p = cms.Path(process.gtDigis*process.l1Filter*
                     process.hcalDigis*process.zsTheDigis*process.hbhereco*
                     process.allHBHETSfilters*process.allTSanals)

#process.p = cms.Path(process.l1Filter*process.hcalDigis*process.myan)


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
   
    fileName = cms.untracked.string('hcaltimeanal-egtrig-pool.root')
)

process.o = cms.EndPath(process.out)
