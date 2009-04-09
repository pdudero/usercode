#ifndef  _MYEDMODULESHCALTIMINGANALALGOS
#define  _MYEDMODULESHCALTIMINGANALALGOS

// -*- C++ -*-
//
// Package:    HcalTimingAnalAlgos
// Class:      HcalTimingAnalAlgos
// 
/**\class HcalTimingAnalAlgos HcalTimingAnalAlgos.cc MyEDmodules/HcalTimingAnalAlgos/src/HcalTimingAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalTimingAnalAlgos.cc,v 1.4 2009/04/03 16:35:31 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"

//
// class declaration
//


class HcalTimingAnalAlgos {
public:
  explicit HcalTimingAnalAlgos(const edm::ParameterSet&);
  ~HcalTimingAnalAlgos() {}
  void process(const myEventData& eventData);
  void beginJob(const edm::EventSetup&) ;
  void endJob() ;

private:
  double lookupThresh(const HcalDetId& id);
  void   bookPerRunHistos(const uint32_t rn);
  bool   convertIdNumbers(std::vector<int>& v_idnumbers,
			  HcalDetId& detId);

  // ----------member data ---------------------------

  myAnalCut *cutNone_;
  myAnalCut *cutMaxHitOnly_;
  myAnalCut *cutTgtTwrOnly_;
  myAnalCut *cutMinHitGeV_;
  myAnalCut *cutAll_;

  // Names of histos
  std::string st_shTimes_,  st_shEnergies_;
  std::string st_rhTimes_,  st_rhEnergies_;
  std::string st_avgPulse_;
  std::string st_caloMet_Met_, st_caloMet_Phi_, st_caloMet_SumEt_;

  std::string st_rhEmap_;
  std::string st_rhTimingVsE_;
  std::string st_shTimingVsE_;
  std::string st_ctTimingVsE_;

  std::set<uint32_t> s_events2anal_;
  std::set<uint32_t> s_runs_;

  // Parameters:
  double             minHitGeVHB_;
  double             minHitGeVHE_;
  double             minHitGeVHO_;
  double             minHitGeVHF1_;
  double             minHitGeVHF2_;
  double             recHitEscaleMinGeV_;
  double             recHitEscaleMaxGeV_;
  std::string rundescr_;

  HcalDetId tgtTwrId_;

};

#endif // _MYEDMODULESHCALTIMINGANALALGOS
