import FWCore.ParameterSet.Config as cms
readFiles = cms.untracked.vstring()
source = cms.Source ("PoolSource",fileNames=readFiles)
readFiles.extend( [
'/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/746/DC7F5700-BF42-DF11-B09B-00E08178C039.root',
'/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/746/74BAD07D-BE42-DF11-B2E1-0025B3E06510.root',
'/store/data/Commissioning10/MinimumBias/RAW-RECO/v8/000/132/746/6661023B-B942-DF11-8502-003048674052.root'
])
