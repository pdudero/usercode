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
// $Id: HcalTimingAnalAlgos.hh,v 1.5 2009/05/21 09:52:41 dudero Exp $
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

  bool       doSimHits_;
  bool       doHBHEdigis_;

  // Names of histos
  std::string st_shTimes_,  st_shEnergies_;
  std::string st_rhTimes_,  st_rhEnergies_;
  std::string st_avgPulse_;
  std::string st_hbhedigiColSize_;
  std::string st_hbheRHColSize_;
  std::string st_caloMet_Met_, st_caloMet_Phi_, st_caloMet_SumEt_;

  std::string st_rhEmap_;
  std::string st_rhTprofd1_;
  std::string st_rhTprofd2_;
  std::string st_rhTprofd3_;
  std::string st_rhTprofd4_;
  std::string st_hbheTimingVsE_;
  std::string st_hfTimingVsE_;
  std::string st_hoTimingVsE_;
  std::string st_shTimingVsE_;
  std::string st_ctHcalTvstwrE_;
  std::string st_ctEcalTvstwrE_;
  std::string st_ctEcalTvsHcalT_;
  std::string st_ctHcalTvstwrEg_;
  std::string st_ctEcalTvstwrEg_;
  std::string st_ctEcalTvsHcalTg_;
  std::string st_ctWeightTvsE_;
  std::string st_ctTprof_;

  std::set<uint32_t> s_events2anal_;
  std::set<uint32_t> s_runs_;

  // Parameters:
  double             minHitGeVHB_;
  double             minHitGeVHE_;
  double             minHitGeVHO_;
  double             minHitGeVHF1_;
  double             minHitGeVHF2_;
  uint32_t           ecalRecHitTscaleNbins_;
  double             ecalRecHitTscaleMinNs_;
  double             ecalRecHitTscaleMaxNs_;
  uint32_t           hcalRecHitTscaleNbins_;
  double             hcalRecHitTscaleMinNs_;
  double             hcalRecHitTscaleMaxNs_;
  double             hcalRecHitEscaleMinGeV_;
  double             hcalRecHitEscaleMaxGeV_;
  uint32_t           simHitTscaleNbins_;
  double             simHitTscaleMinNs_;
  double             simHitTscaleMaxNs_;
  double             simHitEnergyMinGeVthreshold_;
  std::string        rundescr_;

  HcalDetId tgtTwrId_;

};

#endif // _MYEDMODULESHCALTIMINGANALALGOS
