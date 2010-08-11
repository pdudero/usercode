import FWCore.ParameterSet.Config as cms

from RecoLocalCalo.Configuration.RecoLocalCalo_cff import *
from RecoTracker.Configuration.RecoTracker_cff import *
from TrackingTools.Configuration.TrackingTools_cff import *
from RecoMET.METProducers.hcalnoiseinfoproducer_cfi import *
# Global  reco
from RecoEcal.Configuration.RecoEcal_cff import *
from RecoJets.Configuration.CaloTowersRec_cff import *
from RecoVertex.Configuration.RecoVertex_cff import *
from RecoVertex.BeamSpotProducer.BeamSpot_cff import *
from RecoPixelVertexing.Configuration.RecoPixelVertexing_cff import *
#local reconstruction
from RecoLocalTracker.Configuration.RecoLocalTracker_cff import *


localreco = cms.Sequence(trackerlocalreco+calolocalreco)

globalreco = cms.Sequence(offlineBeamSpot+
                          recopixelvertexing*
                          ckftracks+
                          ecalClusters+
                          caloTowersRec*
                          vertexreco*
                          hcalnoise)

# "Export" Section
mypartialreco = cms.Sequence(localreco*globalreco)
