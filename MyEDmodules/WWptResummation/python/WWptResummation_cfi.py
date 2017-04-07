import FWCore.ParameterSet.Config as cms

WWptResummationAnal = cms.EDAnalyzer("WWptResummationAnalyzer",
    InputTruthLabel = cms.InputTag('genParticles'),

    pars = cms.PSet(
        centralFile = cms.string('../data/central8tev.dat'),
        QupFile = cms.string('../data/resumscaleQup8tev.dat'),
        QdnFile = cms.string('../data/resumscaleQdn8tev.dat'),
        RupFile = cms.string('../data/renormscaleRup8tev.dat'),
        RdnFile = cms.string('../data/renormscaleRdn8tev.dat'),
        centralXsecPb = cms.double(54.397),
        QupXsecPb = cms.double(54.397),
        QdnXsecPb = cms.double(54.397),
        RupXsecPb = cms.double(53.395),
        RdnXsecPb = cms.double(55.681),
        finalParticleStatus = cms.int32(2)
        )
)
