#ifndef  _MYEDMODULESSPLASHHITTIMECORRECTOR
#define  _MYEDMODULESSPLASHHITTIMECORRECTOR

// -*- C++ -*-
//
// Package:    SplashHitTimeCorrector
// Class:      SplashHitTimeCorrector
// 
/**\class SplashHitTimeCorrector SplashHitTimeCorrector.cc MyEDmodules/SplashHitTimeCorrector/src/SplashHitTimeCorrector.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: SplashHitTimeCorrector.hh,v 1.1 2009/11/09 00:58:34 dudero Exp $
//
//


// system include files

// user include files

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "PhysicsTools/UtilAlgos/interface/TFileDirectory.h"

#include "TProfile2D.h"

//
// class declaration
//

class SplashHitTimeCorrector {
public:
  SplashHitTimeCorrector(bool plusZsideSplash);
  ~SplashHitTimeCorrector() {}

  void  init         (const edm::EventSetup& iSetup);
  float correctTime4 (const HcalDetId& id);

private:
  void   bookHistos      (void);

  /*****************************************
   * ALGORITHMS PROVIDED BY JORDAN DAMGOV
   *****************************************/
  GlobalPoint lookupTwrCtr4(const HcalDetId& id); // returns tower center for id
  float    calcRawTcorrect4(const HcalDetId& id); /* returns time difference between
						     IP and collimator in ns */

  // ----------member data ---------------------------

  // Parameters:
  bool             splashPlusZside_;

  float                globalOffset_; // calculated for barrel center
  const CaloGeometry  *geo_;          // to find centers of towers
  TFileDirectory      *rootdir_;
  std::vector<TProfile *> v_profiles_; // for histogramming the corrections
  std::map<int,float>  m_corrs_;   // indexed by a combination of ieta and depth
};

#endif // _MYEDMODULESSPLASHHITTIMECORRECTOR
