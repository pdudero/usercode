
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
// $Id: BeamHitTimeCorrector.cc,v 1.1 2010/01/26 13:54:40 dudero Exp $
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
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "MyEDmodules/HcalDelayTuner/interface/BeamHitTimeCorrector.hh"

#include "TProfile.h"

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
}                                                          // bookHistos

//======================================================================

float
BeamHitTimeCorrector::correctTime4(const HcalDetId& id)
{
  float thecor = 0.0;
  return thecor;
}                                // BeamHitTimeCorrector::correctTime4

//======================================================================

void
BeamHitTimeCorrector::init(const edm::ParameterSet& iConfig)
{
}                                       // BeamHitTimeCorrector::init
