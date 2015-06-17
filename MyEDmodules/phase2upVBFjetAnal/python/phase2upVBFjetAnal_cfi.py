import FWCore.ParameterSet.Config as cms

vbfanalloose = cms.EDAnalyzer('phase2upVBFjetAnal',
    GenParticleLabel = cms.InputTag('genParticles'),
    GenJetTruthLabel = cms.InputTag('ak5GenJets'),
    RecoJetLabel = cms.InputTag( 'ak5PFJets' ),
    benchmarkParams = cms.PSet(
        isSignalDoTruthMatch = cms.bool(False),
        maxEta = cms.double(5.0),
        minrecPt = cms.double(30.0),
        maxplotE = cms.double(1000.0),
        pfjBenchmarkDebug = cms.bool(False),                           
        deltaRMax = cms.double(0.1),
        PlotAgainstRecoQuantities = cms.bool(False),
        minDeltaEtajj   = cms.double(3.0),
        minMjjGeV       = cms.double(400.0),
        doHemisphereCut = cms.bool(False)
        )
)

vbfanaltight = cms.EDAnalyzer('phase2upVBFjetAnal',
    GenParticleLabel = cms.InputTag('genParticles'),
    GenJetTruthLabel = cms.InputTag('ak5GenJets'),
    RecoJetLabel = cms.InputTag( 'ak5PFJets' ),
    benchmarkParams = cms.PSet(
        isSignalDoTruthMatch = cms.bool(False),
        maxEta = cms.double(5.0),
        minrecPt = cms.double(50.0),
        maxplotE = cms.double(1000.0),
        pfjBenchmarkDebug = cms.bool(False),                           
        deltaRMax = cms.double(0.1),
        PlotAgainstRecoQuantities = cms.bool(False),
        minDeltaEtajj   = cms.double(4.0),
        minMjjGeV       = cms.double(600.0),
        doHemisphereCut = cms.bool(True)
        )
)
