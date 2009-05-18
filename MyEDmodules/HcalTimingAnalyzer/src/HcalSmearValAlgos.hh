#ifndef  _MYEDMODULESHCALSMEARVALALGOS
#define  _MYEDMODULESHCALSMEARVALALGOS

// -*- C++ -*-
//
// Package:    HcalSmearValAlgos
// Class:      HcalSmearValAlgos
// 
/**\class HcalSmearValAlgos HcalSmearValAlgos.cc MyEDmodules/HcalSmearValAlgos/src/HcalSmearValAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalSmearValAlgos.hh,v 1.1 2009/05/17 18:58:27 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"

#include "TH2D.h"
#include "TProfile2D.h"

//
// class declaration
//


class HcalSmearValAlgos {
public:
  explicit HcalSmearValAlgos(const edm::ParameterSet&);
  ~HcalSmearValAlgos() {}
  void process(const myEventData& unsmeared, 
	       const myEventData& smeared);
  void beginJob(const edm::EventSetup&) ;
  void endJob() ;

private:
  void   bookPerRunHistos(const uint32_t rn);

  template <class Hit, class Col>
  void
  compareHits(typename edm::Handle<Col> unsmearedHits,
	      typename edm::Handle<Col> smearedHits);

  // ----------member data ---------------------------

  myAnalCut *cutNone_;

  // Names of histos

  std::string st_rhTprofd1_;
  std::string st_rhTprofd2_;
  std::string st_rhTprofd3_;
  std::string st_rhTprofd4_;
  std::string st_deltaTprofd1_;
  std::string st_deltaTprofd2_;
  std::string st_deltaTprofd3_;
  std::string st_deltaTprofd4_;
  std::string st_hbheTimingVsE_;
  std::string st_hfTimingVsE_;
  std::string st_hoTimingVsE_;
  std::string st_hbheDeltaTvsE_;
  std::string st_hfDeltaTvsE_;
  std::string st_hoDeltaTvsE_;

  std::set<uint32_t> s_runs_;

  // Parameters:
  double             minHitGeVHB_;
  double             minHitGeVHE_;
  double             minHitGeVHO_;
  double             minHitGeVHF1_;
  double             minHitGeVHF2_;
  uint32_t           deltaTscaleNbins_;
  double             deltaTscaleMinNs_;
  double             deltaTscaleMaxNs_;
  uint32_t           recHitTscaleNbins_;
  double             recHitTscaleMinNs_;
  double             recHitTscaleMaxNs_;
  double             recHitEscaleMinGeV_;
  double             recHitEscaleMaxGeV_;
  std::string        rundescr_;

};

//======================================================================

template <class Hit, class Col>
void
HcalSmearValAlgos::compareHits(typename edm::Handle<Col> unsmearedHits,
			       typename edm::Handle<Col> smearedHits)
{
  if (!unsmearedHits.isValid())
    edm::LogWarning("Unsmeared Rechit collection handle not valid!");
  else if (!smearedHits.isValid())
    edm::LogWarning("Smeared Rechit collection handle not valid!");
  else {
    if (smearedHits->size() != unsmearedHits->size() )
      edm::LogWarning("Rechit collection sizes not the same: ") <<
	unsmearedHits->size() << " != " << smearedHits->size() << endl;

    for (unsigned iunsm = 0, ism=0;
	 (iunsm < unsmearedHits->size()) &&
	   (ism < smearedHits->size());
	 ++iunsm, ++ism) {
      const Hit& unsmrh = (*(unsmearedHits))[iunsm];
      const Hit& smrh   = (*(smearedHits))[ism];

      if (smrh.id() != unsmrh.id())
	throw cms::Exception("the rechits aren't tracking, man!") << endl;

      HcalDetId detId = smrh.id();
      double htime    = smrh.time();
      double energy   = smrh.energy();
      double deltaT   = htime - unsmrh.time();
      int    ieta     = detId.ieta();
      int    iphi     = detId.iphi();
      int    depth    = detId.depth();

      myAnalHistos *myAH = cutNone_->histos();

      // time vs. energy by subdetector:

      switch(detId.subdet()) {
      case HcalBarrel: 
      case HcalEndcap: 
	myAH->fill2d<TH2D>(st_hbheTimingVsE_,energy,htime);
	myAH->fill2d<TH2D>(st_hbheDeltaTvsE_,energy,deltaT);
	break;
      case HcalForward: 
	myAH->fill2d<TH2D>(st_hfTimingVsE_,energy,htime);
	myAH->fill2d<TH2D>(st_hfDeltaTvsE_,energy,deltaT);
	break;
      case HcalOuter: 
	myAH->fill2d<TH2D>(st_hoTimingVsE_,energy,htime);
	myAH->fill2d<TH2D>(st_hoDeltaTvsE_,energy,deltaT);
	break;
      default:
	edm::LogWarning("subdet not covered: ") << detId << endl;
	break;
      }

      // depth-based timing profiles:

      std::string st_rhTprof, st_deltaTprof;
      switch(depth) {
      case 1: st_rhTprof = st_rhTprofd1_; st_deltaTprof = st_deltaTprofd1_; break;
      case 2: st_rhTprof = st_rhTprofd2_; st_deltaTprof = st_deltaTprofd2_; break;
      case 3: st_rhTprof = st_rhTprofd3_; st_deltaTprof = st_deltaTprofd3_; break;
      case 4: st_rhTprof = st_rhTprofd4_; st_deltaTprof = st_deltaTprofd4_; break;
      default:
	edm::LogWarning("Invalid depth in rechit collection! detId = ") << detId << std::endl;
	continue;
      }

      if (energy > 5.0) {
	myAH->fill2d<TProfile2D>(st_rhTprof,ieta,iphi,htime);
	myAH->fill2d<TProfile2D>(st_deltaTprof,ieta,iphi,deltaT);
      }

      //double minHitGeV = lookupThresh(detId);
    }
  }
}

#endif // _MYEDMODULESHCALSMEARVALALGOS
