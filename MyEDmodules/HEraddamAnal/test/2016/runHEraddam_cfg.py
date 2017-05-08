
import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

process = cms.Process("HERADDAM")

# setup 'analysis'  options
options = VarParsing.VarParsing ('analysis')

options.register ('run',
                  999999, # default value
                  VarParsing.VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.VarParsing.varType.int,          # string, int, or float
                  "Run Number")

options.register ('year',
                  1999, # default value
                  VarParsing.VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.VarParsing.varType.int,          # string, int, or float
                  "Year")

# get and parse the command line arguments
options.parseArguments()

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
#
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageLogger.MessageLogger_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration.StandardSequences.RawToDigi_Data_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

#----------------------------
# Event Source
#-----------------------------
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( -1 )
)

process.source = cms.Source( "HcalTBSource",
   fileNames = cms.untracked.vstring( "root://cmsxrootd.fnal.gov//store/group/dpg_hcal/comm_hcal/USC/USC_%s.root"%options.run ),
   streams = cms.untracked.vstring( #HE:
'HCAL_DCC700','HCAL_DCC701','HCAL_DCC702','HCAL_DCC703','HCAL_DCC704','HCAL_DCC705',
'HCAL_DCC706','HCAL_DCC707','HCAL_DCC708','HCAL_DCC709','HCAL_DCC710','HCAL_DCC711',
'HCAL_DCC712','HCAL_DCC713','HCAL_DCC714','HCAL_DCC715','HCAL_DCC716','HCAL_DCC717',
'HCAL_Trigger','HCAL_QADCTDC'
   )
)

#process.load("CondCore.DBCommon.CondDBSetup_cfi")

process.hcal_db_producer = cms.ESProducer("HcalDbProducer",
     dump = cms.untracked.vstring(''),
     file = cms.untracked.string('')
)

# process.es_hardcode = cms.ESSource("HcalHardcodeCalibrations",
#      toGet = cms.untracked.vstring('GainWidths', 
#          'channelQuality', 
#          'ZSThresholds')
# )

from Configuration.AlCa.autoCond import autoCond
from Configuration.AlCa.GlobalTag import GlobalTag

process.es_prefer_GlobalTag = cms.ESPrefer('PoolDBESSource', 'GlobalTag')

#process.GlobalTag.globaltag = 'GR_R_53_V21::All'
#process.GlobalTag.globaltag = '80X_dataRun2_Prompt_v10'  ### Update GlobalTag as necessary; https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideFrontierConditions
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')

#process.prefer("GlobalTag")

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(10);
process.MessageLogger.cerr.FwkReport.reportEvery = 100

#-----------------------------
# Hcal Digis and RecHits
#-----------------------------

process.tbunpack = cms.EDProducer("HcalTBObjectUnpacker",
    IncludeUnmatchedHits = cms.untracked.bool(False),
    HcalTriggerFED       = cms.untracked.int32(1),
    fedRawDataCollectionTag = cms.InputTag('source')
)

#process.load("EventFilter.HcalRawToDigi.HcalRawToDigi_cfi")
process.hcalDigis = cms.EDProducer("HcalRawToDigi",
        ### Falg to enable unpacking of TTP channels(default = false)
        UnpackTTP = cms.untracked.bool(False),
        FilterDataQuality = cms.bool(False),
        InputLabel = cms.InputTag('source'),
#        HcalFirstFED = cms.untracked.int32(700),
        ComplainEmptyData = cms.untracked.bool(False),
        UnpackCalib = cms.untracked.bool(True),
        firstSample = cms.int32(0),
        lastSample = cms.int32(9),
#        silent = cms.untracked.bool(False)
)

#process.hcalDigis.FEDs = cms.untracked.vint32(700,701,702,703,704,705,706,707,708,709,710,711,712,713,714,715,716,717)

process.hcalLaserReco = cms.EDProducer( "HcalLaserReco",
    QADCTDCFED = cms.untracked.int32(8),
    fedRawDataCollectionTag = cms.InputTag('source')
)

process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("heraddamanal_run%d.root"%options.run),
    closeFileFast = cms.untracked.bool(False)
)

process.load("MyEDmodules.HEraddamAnal.heraddamanal_cfi")
process.heraddam.TDCpars.TDCCutCenter = cms.double(1075)
process.heraddam.TDCpars.TDCCutWindow = cms.double(25)
process.heraddam.TDCpars.CorrectedTimeModCeiling = cms.int32(9999)
process.heraddam.TDCpars.TimeModCeiling = cms.int32(9999)
#process.heraddam.eventDataPset.fedRawDataLabel = cms.untracked.InputTag("")
#process.heraddam.eventDataPset.verbose = cms.untracked.bool(True)
process.heraddam.rundatesfile    = cms.untracked.string("data/rundates%d.txt"%options.year)
process.heraddam.lumiprofilefile = cms.untracked.string("data/%d-delivered-perday.csv"%options.year)
process.heraddam.tdcwindowsfile=cms.untracked.string("data/perchanwin_run%d.txt"%options.run)

#LogicalMapFilename = cms.untracked.string("HCALmapHBEF_Jun.19.2008.txt")

#process.dumpfeds = cms.EDAnalyzer('DumpFEDRawDataProduct')
   
process.p = cms.Path(process.hcalDigis
#                     * process.dumpfeds
#                     * process.tbunpack
#                     * process.hcalLaserReco
                     * process.heraddam
)
 
# Output module configuration
# process.out = cms.OutputModule("PoolOutputModule",
#     fileName = cms.untracked.string('heraddam_run%d-pool.root'%options.run),
#     # save only events passing the full path
#     #SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
#     #outputCommands = cms.untracked.vstring('keep *')
# )

  
# process.e = cms.EndPath(process.out)
