import FWCore.ParameterSet.Config as cms

process = cms.Process("ONEELFILT")

process.load("FWCore.MessageService.MessageLogger_cfi")

#process.load("MyEDmodules.OneElectronFilter.in_cff")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10000) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring('file:in.root')
)

from RecoEgamma.ElectronIdentification.electronIdCutBased_cfi import *
import RecoEgamma.ElectronIdentification.electronIdCutBased_cfi
process.eidRobust = RecoEgamma.ElectronIdentification.electronIdCutBased_cfi.eidCutBased.clone()

process.eidRobust.src = cms.InputTag("pixelMatchGsfElectrons")

process.load("Geometry.CMSCommonData.cmsIdealGeometryXML_cff")
process.load("Geometry.CaloEventSetup.CaloGeometry_cff")
process.load("Geometry.CaloEventSetup.CaloTopology_cfi")
process.load("MyEDmodules.OneElectronFilter.oneelectronfilter_cfi")

process.p = cms.Path(process.eidRobust*process.filt)

process.out = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
                SelectEvents = cms.vstring('p')
                    ),
    fileName = cms.untracked.string('out.root')
)

process.o = cms.EndPath(process.out)
