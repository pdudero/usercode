
// -*- C++ -*-
//
// Package:    BeamHitTimeCorrector
// Class:      BeamHitTimeCorrector
// 
/**\class BeamHitTimeCorrector BeamHitTimeCorrector.cc MyEDmodules/HcalDelayTuner/src/BeamHitTimeCorrector.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: BeamHitTimeCorrector.cc,v 1.5 2010/03/27 18:36:05 dudero Exp $
//
//


// system include files
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <math.h> // floor

// user include files

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "MyEDmodules/HcalDelayTuner/interface/BeamHitTimeCorrector.hh"

//
// constructors and destructor
//
BeamHitTimeCorrector::BeamHitTimeCorrector(void)
{
  edm::Service<TFileService> fs;
  rootdir_ = new TFileDirectory(fs->mkdir("HitTimeCorrections"));
  bookHistos();
}                          // BeamHitTimeCorrector::BeamHitTimeCorrector

//======================================================================

void
BeamHitTimeCorrector::bookHistos(void)
{
  h1d_vertexZ_ = rootdir_->make<TH1D>("h1d_vertexZ","Primary Vertex Z",
				      200,-100.0,100.0);
}                                                          // bookHistos

//======================================================================

float
BeamHitTimeCorrector::correctTime4(const HcalDetId& id)
{
  float thecor = 0.0;
  if (id.subdet() == HcalForward)
    thecor = -1.0*id.zside()*vertex_z_/30.; // c = 30cm/ns
  return thecor;
}                                // BeamHitTimeCorrector::correctTime4

//======================================================================

float
BeamHitTimeCorrector::correctTime4(const HcalZDCDetId& id)
{
  return (-1.0*id.zside()*vertex_z_/30.); // c = 30cm/ns
}                                // BeamHitTimeCorrector::correctTime4

//======================================================================

void
BeamHitTimeCorrector::init(const edm::Handle<reco::VertexCollection>& vertices)
{
  vertex_z_ = 0.;
  if (vertices.isValid() && vertices->size()) {
    const reco::Vertex& v = (*vertices)[0];
    if (v.isValid() && !v.isFake()) {
      vertex_z_ = v.z();
      h1d_vertexZ_->Fill(vertex_z_);
    }
  }
}                                       // BeamHitTimeCorrector::init
