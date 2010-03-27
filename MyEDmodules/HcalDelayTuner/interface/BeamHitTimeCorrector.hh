#ifndef  _MYEDMODULESBEAMHITTIMECORRECTOR
#define  _MYEDMODULESBEAMHITTIMECORRECTOR

// -*- C++ -*-
//
// Package:    BeamHitTimeCorrector
// Class:      BeamHitTimeCorrector
// 
/**\class BeamHitTimeCorrector BeamHitTimeCorrector.cc MyEDmodules/HcaldDelayTuner/src/BeamHitTimeCorrector.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: BeamHitTimeCorrector.hh,v 1.4 2010/03/26 15:51:29 dudero Exp $
//
//


// system include files

// user include files

#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/HcalDetId/interface/HcalZDCDetId.h"
#include "CommonTools/Utils/interface/TFileDirectory.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "TH1D.h"

//
// class declaration
//

class BeamHitTimeCorrector {
public:
  BeamHitTimeCorrector();
  ~BeamHitTimeCorrector() {}

  void  init(const edm::Handle<reco::VertexCollection>& vertices);

  // returns a subtractive corrector:
  float correctTime4 (const HcalDetId&    id);
  float correctTime4 (const HcalZDCDetId& id);

private:
  void   bookHistos      (void);

  // ----------member data ---------------------------

  TFileDirectory      *rootdir_;
  double               vertex_z_;
  TH1D                *h1d_vertexZ_;
};

#endif // _MYEDMODULESBEAMHITTIMECORRECTOR
