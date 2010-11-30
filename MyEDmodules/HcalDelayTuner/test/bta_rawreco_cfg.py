
import FWCore.ParameterSet.Config as cms

process = cms.Process("BEAMTIMEANAL")
process.maxEvents = cms.untracked.PSet (
   input = cms.untracked.int32( -1 )
# test it!
#   input = cms.untracked.int32( 1000 ) 
)

#-----------------------------
# Hcal Conditions: from Global Conditions Tag 
#-----------------------------
# 31X:
#
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1);
process.MessageLogger.cerr.FwkReport.reportEvery = 10

process.source=cms.Source("PoolSource",
fileNames = cms.untracked.vstring( # 'in.root')
 #    '/store/data/Run2010A/JetMETTau/RECO/v2/000/137/028/F4059C65-3E71-DF11-B32B-001D09F295A1.root')
#     '/store/data/Run2010A/JetMETTau/RAW-RECO/v6/000/139/779/227B20AD-508B-DF11-84EE-003048678DD6.root')
#'/store/data/Run2010A/JetMETTau/RAW-RECO/v6/000/141/882/7A5CBED5-409B-DF11-B5D0-001A928116BA.root')
'/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/D2A05941-2BCC-DF11-B898-003048C33984.root')
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/009E2703-2ACC-DF11-B031-003048D436BE.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/0E5F9C97-2ECC-DF11-9682-003048C692A4.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/12BF78E3-29CC-DF11-88E8-003048D4385C.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/18C11DBA-28CC-DF11-8F0C-003048D4DF80.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/1CD6D0A8-2CCC-DF11-9AFD-003048C69032.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/223A4FB6-2CCC-DF11-8BF7-003048C692A4.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/2AF3007E-2ECC-DF11-9C30-003048C6929A.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/2E31130D-2DCC-DF11-9C55-00237DE0BED6.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/2EA3E9D1-29CC-DF11-936E-003048C693E2.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/44129F9D-28CC-DF11-8021-003048D4DFA4.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/503D9CC9-2CCC-DF11-B863-002481E0E450.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/52B0CF4B-2ACC-DF11-8562-002481E0D480.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/540E5668-27CC-DF11-8ECD-003048D43656.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/54112D10-28CC-DF11-A027-003048D4DFA8.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/54E9E718-28CC-DF11-864F-003048D4385C.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/5CD3F37B-2BCC-DF11-864C-003048C6929A.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/6250F66E-2BCC-DF11-AF4F-00237DE0BED6.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/66378520-28CC-DF11-93AF-003048D436BE.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/6A24F46F-2DCC-DF11-A71C-003048D45FCE.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/6A5F4998-2ECC-DF11-A9A9-002481E0DC82.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/70AA53C5-28CC-DF11-B8A2-00237DE0BED6.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/746B0F6A-26CC-DF11-B9DD-003048C69032.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/74C154EF-28CC-DF11-A35B-003048C69032.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/74F0AE21-1DCC-DF11-9C52-002481E10B86.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/78885A67-2BCC-DF11-A5BE-0030487E4B8D.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/7ED79644-2BCC-DF11-A800-002481E0E450.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/80399450-27CC-DF11-98F4-002481E0DC82.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/80575AEC-29CC-DF11-AC6E-002481E0DC82.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/849031CF-28CC-DF11-8D0C-0030487E55B9.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/861EDD82-2ACC-DF11-A376-002481E0DC4E.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/8C355F6F-2BCC-DF11-930D-003048C69312.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/96CA978B-28CC-DF11-8E46-002481E0E450.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/96D50878-2ECC-DF11-ABB9-00237DE0BED6.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/9C05927F-2BCC-DF11-A719-002481E0DC82.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/9ECCB211-28CC-DF11-A32F-0030487E4B8D.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/A085F9F4-29CC-DF11-8570-003048D4DFA8.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/AA3FC29D-26CC-DF11-89AE-003048D4DF80.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/B0608886-28CC-DF11-9107-002481E0D480.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/B060E6A0-28CC-DF11-A36B-002481E0DC4E.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/B4F122F5-25CC-DF11-8495-0030487E4B8D.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/B6D92E71-27CC-DF11-8C19-002481E0E450.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/B88AFBF3-29CC-DF11-B707-003048D43642.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/C4273E26-2ACC-DF11-AA8A-003048C692F2.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/C4F7AA5A-27CC-DF11-83D3-0030487D7109.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/CA7AA75E-25CC-DF11-B7A5-003048D43960.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/CC96CCA2-25CC-DF11-9F29-003048D43656.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/CE70CF46-25CC-DF11-9F58-003048D4385C.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/CED11210-26CC-DF11-BF50-002481E0D480.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/D2A05941-2BCC-DF11-B898-003048C33984.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/D68CB756-27CC-DF11-A2AD-0030487E55B9.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/D80D9AA6-26CC-DF11-A4A0-003048D436B4.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/D8B20068-27CC-DF11-AE52-002481E0D480.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/DA8DD89D-28CC-DF11-B638-003048D436B4.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/EA40A36F-2DCC-DF11-A950-003048C68A84.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/F477F6A9-27CC-DF11-A5CE-003048C6903E.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/F84285C4-2CCC-DF11-BF9C-003048D4385C.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/F8441DEB-25CC-DF11-A8E3-003048D4385C.root',
# '/store/data/Run2010B/Jet/RAW-RECO//v2/000/146/807/FC49DC5C-2DCC-DF11-BA6A-003048C69312.root')
)
#process.load("MyEDmodules.HcalDelayTuner.jetmettau_fill1207reco_1")
#process.load("in_cff")

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

from MyEDmodules.HcalDelayTuner.goodRunList_cfi import lumisToProcess
process.source.lumisToProcess = lumisToProcess

################################################################################
# Define the cleaning sequence

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

process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')

process.cleaning_seq = cms.Sequence(process.noscraping*
                                    process.oneGoodVertexFilter*
                                    process.HBHENoiseFilter)

################################################################################
# Configure Retimer-Reflagger-Tower-MET rereco chain

process.load('Configuration.StandardSequences.GeometryExtended_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('EventFilter.HcalRawToDigi.HcalRawToDigi_cfi')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = 'GR10_P_V11::All'

# Replace timecorr object
# process.tc_ascii2 = cms.ESSource("HcalTextCalibrations",
#   input = cms.VPSet(cms.PSet(
#            object = cms.string('TimeCorrs'),
# #             file = cms.FileInPath('MyEDmodules/HBHERecHitRetimer/data/timecorr_may27.txt') )
#              file = cms.FileInPath('MyEDmodules/HcalDelayTuner/data/timecorr_may27.csv') )
#                     )
# )
# process.prefer=cms.ESPrefer('HcalTextCalibrations','tc_ascii2')
# process.hbherecoRetimed = cms.EDProducer('HBHERecHitRetimer',
#                                          input = cms.InputTag('hbhereco')
# )

# from JetMETAnalysis.HcalReflagging.hbherechitreflaggerJETMET_cfi import *
# process.hbherecoReflagged = hbherechitreflaggerJETMET.clone()
# process.hbherecoReflagged.hbheInputLabel=cms.untracked.InputTag("hbherecoRetimed")
# process.hbherecoReflagged.ignorelowest=cms.bool(False)
#process.hbherecoReflagged.debug=cms.untracked.int32(1)

# from JetMETAnalysis.HcalReflagging.HFrechitreflaggerJETMET_cff import *
# process.hfrecoReflagged = hfrechitreflaggerJETMET.clone()
#process.hfrecoReflagged.PETstat.flagsToSkip =string.atoi('10',2)
#process.hfrecoReflagged.S8S1stat.flagsToSkip=string.atoi('10010',2)
#process.hfrecoReflagged.S9S1stat.flagsToSkip=string.atoi('11010',2)
#
# set  flag 4 (HFPET  -- also sets HFLongShort),
# then flag 3 (HFS8S1 -- also sets HFLongShort),
# then flag 0 (HFLongShort -- set directly via S9S1)
#process.hfrecoReflagged.FlagsToSet=(4,3,0)

process.load("RecoJets.Configuration.CaloTowersES_cfi")
# process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")

# process.rereco_seq = cms.Sequence(process.hbherecoRetimed*
#                                   process.hbherecoReflagged*
#                                   process.hfrecoReflagged)

################################################################################
# Analyzers
process.TFileService = cms.Service("TFileService", 
   fileName = cms.string("anal.root"),
    closeFileFast = cms.untracked.bool(False)
)

from MyEDmodules.HcalDelayTuner.beamtiminganal_cfi import *
process.hbtimeanal = hbtimeanal.clone()
process.hbtimeanal.runDescription       = cms.untracked.string("Run 146807")
process.hbtimeanal.ampCutsInfC          = cms.bool(False)
process.hbtimeanal.minHit_GeVorfC       = cms.double(0.7)
process.hbtimeanal.minHit_GeVorfC_perChan= cms.untracked.double(20.0)
process.hbtimeanal.maxEventNum2plot     = cms.int32(30000000)
process.hetimeanal = hetimeanal.clone()
process.hetimeanal.runDescription       = cms.untracked.string("Run 146807")
process.hetimeanal.ampCutsInfC          = cms.bool(False)
process.hetimeanal.minHit_GeVorfC       = cms.double(0.8)
process.hetimeanal.minHit_GeVorfC_perChan= cms.untracked.double(20.0)
process.hetimeanal.maxEventNum2plot     = cms.int32(30000000)

#process.hbtimeanal.hbheOutOfTimeFlagBit = cms.untracked.int32(-1)

process.hotimeanal = hotimeanal.clone()
process.hotimeanal.runDescription       = cms.untracked.string("Run 146807")
process.hotimeanal.ampCutsInfC          = cms.bool(False)
process.hotimeanal.minHit_GeVorfC       = cms.double(3.5)
process.hotimeanal.recHitEscaleMaxGeV   = cms.double(200.5)
process.hotimeanal.maxEventNum2plot     = cms.int32(30000000)

process.hftimeanal = hftimeanal.clone()
process.hftimeanal.runDescription        = cms.untracked.string("Run 146807")
process.hftimeanal.ampCutsInfC           = cms.bool(False)
process.hftimeanal.minHit_GeVorfC        = cms.double(0.8)
process.hftimeanal.minHit_GeVorfC_perChan= cms.untracked.double(40.0)
process.hftimeanal.recHitEscaleMaxGeV    = cms.double(5000.5)
process.hftimeanal.maxEventNum2plot      = cms.int32(30000000)

# process.hbtimeanal.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecoReflagged")
# process.hetimeanal.eventDataPset.hbheRechitLabel = cms.untracked.InputTag("hbherecoReflagged")
# process.hftimeanal.eventDataPset.hfRechitLabel   = cms.untracked.InputTag("hfrecoReflagged")
process.hbtimeanal.eventDataPset.hbheDigiLabel   = cms.untracked.InputTag("hcalDigis")
process.hetimeanal.eventDataPset.hbheDigiLabel   = cms.untracked.InputTag("hcalDigis")
process.hotimeanal.eventDataPset.hoDigiLabel     = cms.untracked.InputTag("hcalDigis")
process.hftimeanal.eventDataPset.hfDigiLabel     = cms.untracked.InputTag("hcalDigis")

# Need conditions to convert digi ADC to fC in the analyzer

process.anal_seq     = cms.Sequence(process.hbtimeanal+
                                    process.hetimeanal)
#                                    process.hotimeanal+
#                                    process.hftimeanal)

process.path  = cms.Path(process.cleaning_seq*
                         process.hcalDigis*
#                         process.rereco_seq*
                         process.anal_seq)
