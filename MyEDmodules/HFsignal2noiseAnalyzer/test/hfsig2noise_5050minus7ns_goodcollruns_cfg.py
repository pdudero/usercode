
import FWCore.ParameterSet.Config as cms

process = cms.Process("HFSIG2NOISE")
process.maxEvents = cms.untracked.PSet (
#   input = cms.untracked.int32( -1 )
# test it!
   input = cms.untracked.int32( 5500 ) 
)

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("hfsig2noise.root"),
    closeFileFast = cms.untracked.bool(False)
)
process.load("MyEDmodules.HFsignal2noiseAnalyzer.run133036goodcollfiles_cfi")
#source = cms.Source("PoolSource",
#fileNames = cms.untracked.vstring('file:dummy.root')
#)
process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")
process.load("MyEDmodules.HFsignal2noiseAnalyzer.hfsignal2noiseanalyzer_cfi")
#process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")
process.load("CalibCalorimetry.HcalPlugins.Hcal_Conditions_forGlobalTag_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cfi")
process.GlobalTag.globaltag = 'GR10_P_V4::All'
# Need conditions to convert digi ADC to fC in the analyzer
process.p = cms.Path(process.hcalDigis*process.hfsig2noise)
