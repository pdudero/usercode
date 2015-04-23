import FWCore.ParameterSet.Config as cms
from RecoJets.Configuration.RecoJets_cff import *
from RecoJets.Configuration.RecoPFJets_cff import *

from JetMETCorrections.Configuration.JetCorrectionProducersAllAlgos_cff import *
from JetMETCorrections.Configuration.JetCorrectionServicesAllAlgos_cff import *
ak4PFL1Fastjet   = ak5PFL1Fastjet.clone( algorithm = 'AK4PF')
ak4PFL2Relative   = ak5PFL2Relative.clone  ( algorithm = 'AK4PF' )
ak4PFL3Absolute   = ak5PFL3Absolute.clone  ( algorithm = 'AK4PF' )
ak4PFL2L3 = cms.ESProducer(
    'JetCorrectionESChain',
    correctors = cms.vstring('ak4PFL2Relative','ak4PFL3Absolute'),
    useCondDB = cms.untracked.bool(True)
    )
ak4PFL1FastL2L3 = ak4PFL2L3.clone()
ak4PFL1FastL2L3.correctors.insert(0,'ak4PFL1Fastjet')

#These are only needed if you want to recluster the jets
ak4PFJetsL1Fast = cms.EDProducer(
                  'PFJetCorrectionProducer',
                  src = cms.InputTag('ak4PFJets'),
                  correctors = cms.vstring('ak4PFL1Fastjet')
)
ak4PFJetsL2L3   = ak5PFJetsL2L3.clone(src = 'ak4PFJets', correctors = ['ak4PFL2L3'])
ak4PFJetsL1FastL2L3 = ak5PFJetsL2L3.clone(src = 'ak4PFJets', correctors = ['ak4PFL1FastL2L3'])

ak4PFchsL1Fastjet   = cms.ESProducer(
    'L1FastjetCorrectionESProducer',
    era         = cms.string('Summer11'),
    level       = cms.string('L1FastJet'),
    algorithm   = cms.string('AK4PFchs'),
    section     = cms.string(''),
    srcRho      = cms.InputTag('kt6PFJets','rho'),
    useCondDB = cms.untracked.bool(True)
    )
ak4PFchsJetsL1Fast =  cms.EDProducer(
    'PFJetCorrectionProducer',
    src         = cms.InputTag('ak4PFchsJets'),
    correctors  = cms.vstring('ak4PFchsL1Fastjet')
    )
ak4PFchsL2Relative   =  ak5PFL2Relative.clone( algorithm = 'AK4PFchs' )
ak4PFchsL3Absolute   =  ak5PFL3Absolute.clone  ( algorithm = 'AK4PFchs' )
ak4PFchsL2L3 = cms.ESProducer(
    'JetCorrectionESChain',
    correctors = cms.vstring('ak4PFchsL2Relative','ak4PFchsL3Absolute'),
    useCondDB = cms.untracked.bool(True)
    )
ak4PFchsL1FastL2L3 = ak4PFchsL2L3.clone()
ak4PFchsL1FastL2L3.correctors.insert(0,'ak4PFchsL1Fastjet')

#These are only needed if you want to recluster the jets
ak4PFchsJetsL1Fast = cms.EDProducer(
                  'PFJetCorrectionProducer',
                  src = cms.InputTag('ak4PFchsJets'),
                  correctors = cms.vstring('ak4PFchsL1Fastjet')
)
ak4PFchsJetsL2L3   = ak5PFJetsL2L3.clone(src = 'ak4PFchsJets', correctors = ['ak4PFchsL2L3'])
ak4PFchsJetsL1FastL2L3 = ak5PFJetsL2L3.clone(src = 'ak4PFJetsCHS', correctors = ['ak4PFchsL1FastL2L3'])

########################################

ak4PUPPIL2L3 = cms.ESProducer(
   'JetCorrectionESChain',
   correctors = cms.vstring('ak4PUPPIL2Relative','ak4PUPPIL3Absolute')
   )
ak4PUPPIL1FastL2L3 = ak4PUPPIL2L3.clone()
ak4PUPPIL1FastL2L3.correctors.insert(0,'ak4PUPPIL1Fastjet')
ak4PUPPIJetsL1Fast = cms.EDProducer(
   'PFJetCorrectionProducer',
   src         = cms.InputTag('ak4PUPPIJets'),
   correctors  = cms.vstring('ak4PUPPIL1Fastjet')
   )

ak4PUPPIJetsL2L3 = cms.EDProducer('PFJetCorrectionProducer',
   src         = cms.InputTag('ak4PUPPIJets'),
   correctors  = cms.vstring('ak4PUPPIL2L3')
   )
