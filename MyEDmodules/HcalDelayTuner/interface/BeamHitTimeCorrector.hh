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
// $Id: BeamHitTimeCorrector.hh,v 1.1 2010/01/26 13:59:52 dudero Exp $
//
//


// system include files

// user include files

#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "PhysicsTools/UtilAlgos/interface/TFileDirectory.h"

//
// class declaration
//

class BeamHitTimeCorrector {
public:
  BeamHitTimeCorrector();
  ~BeamHitTimeCorrector() {}

  void  init         (const edm::ParameterSet& iConfig);
  float correctTime4 (const HcalDetId& id);

private:
  void   bookHistos      (void);

  // ----------member data ---------------------------

  TFileDirectory      *rootdir_;
};

#endif // _MYEDMODULESBEAMHITTIMECORRECTOR
