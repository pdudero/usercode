import FWCore.ParameterSet.Config as cms

process = cms.Process("L1MUFILT")

process.load("FWCore.MessageService.MessageLogger_cfi")

#process.load("MyEDmodules.OneElectronFilter.in_cff")

#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10000) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
'/store/data/Commissioning08/Cosmics/RECO/CRAFT_ALL_V9_225-v2/0002/CA3CF466-E7F9-DD11-AF70-003048679006.root'
    )
)

# L1 GT EventSetup
process.load("L1TriggerConfig.L1GtConfigProducers.L1GtConfig_cff")
process.load("L1TriggerConfig.L1GtConfigProducers.Luminosity.startup.L1Menu_startup2_v2_Unprescaled_cff")

process.load("L1Trigger.Skimmer.l1Filter_cfi")
process.l1Filter.algorithms = cms.vstring("L1_SingleMuOpen")

process.p = cms.Path(process.l1Filter)

process.out = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
                SelectEvents = cms.vstring('p')
                    ),
    fileName = cms.untracked.string('out.root')
)

process.o = cms.EndPath(process.out)
