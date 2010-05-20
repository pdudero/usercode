import FWCore.ParameterSet.Config as cms

process = cms.Process("OWNPARTICLES")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("TCMETcleaned357.CleanedTCMETProducer.cleanedtcmetproducer_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
        '/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/596/1AA11816-DB40-DF11-84F0-003048D4774E.root',
        '/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/596/34821982-4041-DF11-BD5A-00E081791809.root',
        '/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/596/42DA2867-DC40-DF11-A3BC-00E081791777.root',
        '/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/596/60C3AB51-DB40-DF11-8FA2-0025B3E06698.root',
        '/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/596/A240F5BA-D740-DF11-92E7-003048D47742.root',
        '/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/596/C6133C47-DB40-DF11-B6C9-003048D476A4.root',
        '/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/596/E04CF9A3-DC40-DF11-B8B9-003048D45FC8.root',
        '/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/596/F424D017-D840-DF11-895A-003048D479F6.root',
        '/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/596/F4FB428A-DA40-DF11-8161-003048D4624A.root'
    ),
#    skipEvents = cms.untracked.uint32(243)                            
)

#process.myProducerLabel = cms.EDProducer('CleanedTCMETProducer'
#)

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('myOutputFile.root')
)

  
process.p = cms.Path(process.tcMetClean)

process.e = cms.EndPath(process.out)
