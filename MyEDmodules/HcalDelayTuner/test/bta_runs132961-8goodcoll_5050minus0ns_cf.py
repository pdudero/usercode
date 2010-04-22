
import FWCore.ParameterSet.Config as cms

process = cms.Process("BTAHFJIT")
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( -1 )
# test it!
#   input = cms.untracked.int32( 100 ) 
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
    fileName = cms.string("bta_runs132961-8goodcoll_vejitter.root"),
    closeFileFast = cms.untracked.bool(False)
)
process.load("MyEDmodules.HcalDelayTuner.runs132961-8goodcollfiles_cfi")
process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange('132961:237-132961:MAX')
process.load("L1TriggerConfig.L1GtConfigProducers.L1GtTriggerMaskTechTrigConfig_cff")
process.load("HLTrigger.HLTfilters.hltLevel1GTSeed_cfi")
process.hltLevel1GTSeed.L1TechTriggerSeeding = cms.bool(True)
process.hltLevel1GTSeed.L1SeedsLogicalExpression = cms.string("0 AND (40 OR 41) AND NOT (36 OR 37 OR 38 OR 39)")
process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")
process.load("MyEDmodules.HcalDelayTuner.beamtiminganal_cfi")
process.hftimeanal.runDescription       = cms.untracked.string("2010 HF scan, BSC trigger")
process.hftimeanal.globalRecHitFlagMask = cms.int32(0xC0002)
process.hftimeanal.badEventList         = cms.vint32()
process.hftimeanal.acceptedBxNums       = cms.vint32(1)
process.hftimeanal.acceptedPkTSnumbers  = cms.vint32(3,4)
process.hftimeanal.maxEventNum2plot     = cms.int32(30000000)
process.hftimeanal.globalTimeOffset     = cms.double(0.0)
process.hftimeanal.minHitGeV            = cms.double(5.)
process.hftimeanal.recHitEscaleMaxGeV   = cms.double(500.5)
process.hftimeanal.splitByEventRange    = cms.untracked.bool(True)
process.hftimeanal.eventDataPset.hfRechitLabel = cms.untracked.InputTag("hfreco","","BTAHFJIT")
process.load("RecoLocalCalo.HcalRecProducers.HcalHitReconstructor_hf_cfi")
process.hfreco.firstSample=cms.int32(2)
process.hfreco.samplesToAdd=cms.int32(4)
from RecoLocalCalo.HcalRecAlgos.hcalrechitreflagger_cfi import *
process.hfrecoReflagged = hcalrechitReflagger.clone()
process.hfrecoReflagged.hf_Algo3test = False  # S9/S1 algorithm (current default)
process.hfrecoReflagged.hf_Algo2test = True # PET algorithm
# Turns thresholding of PMT hits way down, makes PMT reflagger more aggressive about ID-ing PMT hits
#process.hfrecoReflagged.hf_PET_params.PET_EnergyThreshLong=cms.untracked.vdouble([5.])
#process.hfrecoReflagged.hf_PET_params.PET_EnergyThreshShort=cms.untracked.vdouble([5.])
process.hftimeanal.eventDataPset.hfRechitLabel = cms.untracked.InputTag("hfrecoReflagged")
process.hfrecoReflagged.hfInputLabel = cms.untracked.InputTag("hfreco","","BTAHFJIT")
process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")
process.load("CalibCalorimetry.HcalPlugins.Hcal_Conditions_forGlobalTag_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cfi")
process.GlobalTag.globaltag = 'GR10_P_V4::All'
process.myreco=cms.Sequence(process.hfreco)
process.oneGoodVertexFilter = cms.EDFilter("VertexSelector",
   src = cms.InputTag("offlinePrimaryVertices"),
#   cut = cms.string("!isFake && tracksSize > 3 && abs(z) <= 15 && position.Rho <= 2"),
   cut = cms.string("!isFake && ndof >= 5 && abs(z) <= 15"),
   filter = cms.bool(True)   # otherwise it won't filter the events, just produce an empty vertex collection.
)

process.noscraping = cms.EDFilter("FilterOutScraping",
                                applyfilter = cms.untracked.bool(True),
                                debugOn = cms.untracked.bool(False),
                                numtrack = cms.untracked.uint32(10),
                                thresh = cms.untracked.double(0.25)
                                )
# Need conditions to convert digi ADC to fC in the analyzer
process.hbtimeanal.eventDataPset.hbheDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hetimeanal.eventDataPset.hbheDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hftimeanal.eventDataPset.hfDigiLabel=cms.untracked.InputTag("hcalDigis")
process.hotimeanal.eventDataPset.hoDigiLabel=cms.untracked.InputTag("hcalDigis")
process.zdctimeanal.eventDataPset.zdcDigiLabel=cms.untracked.InputTag("hcalDigis")
process.allAnals=cms.Sequence(process.hftimeanal)
process.p = cms.Path(process.hltLevel1GTSeed*process.hcalDigis*process.myreco*process.hfrecoReflagged*process.noscraping*process.oneGoodVertexFilter*process.allAnals)
