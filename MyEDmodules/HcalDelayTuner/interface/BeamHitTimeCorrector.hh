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
// $Id: BeamHitTimeCorrector.hh,v 1.2 2010/03/01 06:52:17 dudero Exp $
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

  void  init         ();
  float correctTime4 (const HcalDetId& id);

private:
  void   bookHistos      (void);

  // ----------member data ---------------------------

  TFileDirectory      *rootdir_;
};

#endif // _MYEDMODULESBEAMHITTIMECORRECTOR
