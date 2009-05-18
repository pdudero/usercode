/**\class HcalSmearValAlgos HcalSmearValAlgos.cc MyEDmodules/HcalSmearValAlgos/src/HcalSmearValAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalSmearValAlgos.cc,v 1.1 2009/05/17 18:58:27 dudero Exp $
//
//


// system include files
#include <memory>
#include <vector>
#include <set>
#include <stdlib.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "MyEDmodules/HcalTimingAnalyzer/src/HcalSmearValAlgos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"

#include "TH1D.h"
#include "TH2D.h"
#include "TProfile2D.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//
//======================================================================

//
// constructors and destructor
//
HcalSmearValAlgos::HcalSmearValAlgos(const edm::ParameterSet& iConfig) :
  minHitGeVHB_(iConfig.getParameter<double>("minHitGeVHB")),
  minHitGeVHE_(iConfig.getParameter<double>("minHitGeVHE")),
  minHitGeVHO_(iConfig.getParameter<double>("minHitGeVHO")),
  minHitGeVHF1_(iConfig.getParameter<double>("minHitGeVHF1")),
  minHitGeVHF2_(iConfig.getParameter<double>("minHitGeVHF2")),
  deltaTscaleNbins_(iConfig.getParameter<int>("deltaTscaleNbins")),
  deltaTscaleMinNs_(iConfig.getParameter<double>("deltaTscaleMinNs")),
  deltaTscaleMaxNs_(iConfig.getParameter<double>("deltaTscaleMaxNs")),
  recHitTscaleNbins_(iConfig.getParameter<int>("recHitTscaleNbins")),
  recHitTscaleMinNs_(iConfig.getParameter<double>("recHitTscaleMinNs")),
  recHitTscaleMaxNs_(iConfig.getParameter<double>("recHitTscaleMaxNs")),
  recHitEscaleMinGeV_(iConfig.getParameter<double>("recHitEscaleMinGeV")),
  recHitEscaleMaxGeV_(iConfig.getParameter<double>("recHitEscaleMaxGeV")),
  rundescr_(iConfig.getUntrackedParameter<std::string>("runDescription",""))
{
  cutNone_  = new myAnalCut(0,"cutNone");
}

//======================================================================

void HcalSmearValAlgos::bookPerRunHistos(const uint32_t rn)
{
  std::vector<myAnalHistos::HistoParams_t> v_hpars1d; 
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d; 
  std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof;

  myAnalHistos::HistoParams_t hpars1d;
  myAnalHistos::HistoParams_t hpars2d;

  stringstream o; o<<rn;

  std::string runstrn = (rn>10000) ? o.str() : rundescr_;
  std::string runstrt =
    (rn>10000) ? "[RUN:" + o.str() + "]" : "[" + rundescr_ + "]";

  /*****************************************
   * 1-D HISTOGRAMS FIRST:                 *
   *****************************************/

  /*****************************************
   * 2-D HISTOGRAMS AFTER:                 *
   *****************************************/

  st_hbheTimingVsE_ = "h2d_hbheTimingVsE" + runstrn;
  hpars2d.name   = st_hbheTimingVsE_;
  hpars2d.title  = "HBHE RecHit Timing vs. Energy " + runstrt + "; Rechit Energy (GeV); Rechit Time (ns)";
  hpars2d.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars2d.minx   = recHitEscaleMinGeV_;
  hpars2d.maxx   = recHitEscaleMaxGeV_;
  hpars2d.nbinsy = recHitTscaleNbins_;
  hpars2d.miny   = recHitTscaleMinNs_;
  hpars2d.maxy   = recHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_hoTimingVsE_ = "h2d_hoTimingVsE" + runstrn;
  hpars2d.name   = st_hoTimingVsE_;
  hpars2d.title  = "HO RecHit Timing vs. Energy " + runstrt + "; Rechit Energy (GeV); Rechit Time (ns)";
  hpars2d.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars2d.minx   = recHitEscaleMinGeV_;
  hpars2d.maxx   = recHitEscaleMaxGeV_;
  hpars2d.nbinsy = recHitTscaleNbins_;
  hpars2d.miny   = recHitTscaleMinNs_;
  hpars2d.maxy   = recHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_hfTimingVsE_ = "h2d_hfTimingVsE" + runstrn;
  hpars2d.name   = st_hfTimingVsE_;
  hpars2d.title  = "HF RecHit Timing vs. Energy " + runstrt + "; Rechit Energy (GeV); Rechit Time (ns)";
  hpars2d.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars2d.minx   = recHitEscaleMinGeV_;
  hpars2d.maxx   = recHitEscaleMaxGeV_;
  hpars2d.nbinsy = recHitTscaleNbins_;
  hpars2d.miny   = recHitTscaleMinNs_;
  hpars2d.maxy   = recHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_hbheDeltaTvsE_  = "h2d_hbheDeltaTvsE" + runstrn;
  hpars2d.name   = st_hbheDeltaTvsE_;
  hpars2d.title  = "HBHE DeltaT (Smear-Unsmear) vs. Energy " + runstrt + "; Rechit Energy (GeV); SmearT-UnsmearT (ns)";
  hpars2d.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars2d.minx   = recHitEscaleMinGeV_;
  hpars2d.maxx   = recHitEscaleMaxGeV_;
  hpars2d.nbinsy = deltaTscaleNbins_;
  hpars2d.miny   = deltaTscaleMinNs_;
  hpars2d.maxy   = deltaTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_hoDeltaTvsE_  = "h2d_hoDeltaTvsE" + runstrn;
  hpars2d.name   = st_hoDeltaTvsE_;
  hpars2d.title  = "HO DeltaT (Smear-Unsmear) vs. Energy " + runstrt + "; Rechit Energy (GeV); SmearT-UnsmearT (ns)";
  hpars2d.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars2d.minx   = recHitEscaleMinGeV_;
  hpars2d.maxx   = recHitEscaleMaxGeV_;
  hpars2d.nbinsy = deltaTscaleNbins_;
  hpars2d.miny   = deltaTscaleMinNs_;
  hpars2d.maxy   = deltaTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_hfDeltaTvsE_  = "h2d_hfDeltaTvsE" + runstrn;
  hpars2d.name   = st_hfDeltaTvsE_;
  hpars2d.title  = "HF DeltaT (Smear-Unsmear) vs. Energy " + runstrt + "; Rechit Energy (GeV); SmearT-UnsmearT (ns)";
  hpars2d.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars2d.minx   = recHitEscaleMinGeV_;
  hpars2d.maxx   = recHitEscaleMaxGeV_;
  hpars2d.nbinsy = deltaTscaleNbins_;
  hpars2d.miny   = deltaTscaleMinNs_;
  hpars2d.maxy   = deltaTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  /*****************************************
   * 2-D PROFILES:                         *
   *****************************************/

  st_rhTprofd1_  = "p2d_rhTperIetaIphiDepth1" + runstrn;
  hpars2d.name   = st_rhTprofd1_;
  hpars2d.title  = "HBEF (Depth 1) RecHit Time Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_rhTprofd2_  = "p2d_rhTperIetaIphiDepth2" + runstrn;
  hpars2d.name   = st_rhTprofd2_;
  hpars2d.title  = "HBEF (Depth 2) RecHit Time Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_rhTprofd3_  = "p2d_rhTperIetaIphiDepth3" + runstrn;
  hpars2d.name   = st_rhTprofd3_;
  hpars2d.title  = "HE (Depth 3) RecHit Time Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_rhTprofd4_  = "p2d_rhTperIetaIphiDepth4" + runstrn;
  hpars2d.name   = st_rhTprofd4_;
  hpars2d.title  = "HO (Depth 4) RecHit Time Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_deltaTprofd1_ = "p2d_deltaTperIetaIphiDepth1" + runstrn;
  hpars2d.name     = st_deltaTprofd1_;
  hpars2d.title    = "HBEF (Depth 1) Delta T Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx   =  83;
  hpars2d.minx     =  -41.5;
  hpars2d.maxx     =   41.5;
  hpars2d.nbinsy   =  72;
  hpars2d.miny     =    0.5;
  hpars2d.maxy     =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_deltaTprofd2_  = "p2d_deltaTperIetaIphiDepth2" + runstrn;
  hpars2d.name   = st_deltaTprofd2_;
  hpars2d.title  = "HBEF (Depth 2) Delta T Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_deltaTprofd3_  = "p2d_deltaTperIetaIphiDepth3" + runstrn;
  hpars2d.name   = st_deltaTprofd3_;
  hpars2d.title  = "HE (Depth 3) Delta T Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_deltaTprofd4_  = "p2d_deltaTperIetaIphiDepth4" + runstrn;
  hpars2d.name   = st_deltaTprofd4_;
  hpars2d.title  = "HO (Depth 4) Delta T Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  /*****************************************
   * BOOK 'EM, DANNO...                    *
   *****************************************/

  cutNone_->histos()->book1d<TH1D>(v_hpars1d);
  cutNone_->histos()->book2d<TH2D>(v_hpars2d);
  cutNone_->histos()->book2d<TProfile2D>(v_hpars2dprof);

}                                                    // bookPerRunHistos

//======================================================================
#if 0
double HcalSmearValAlgos::lookupThresh(const HcalDetId& id)
{
  double thresh;
  switch (id.subdet()) {
  case HcalBarrel: thresh = minHitGeVHB_; break;
  case HcalEndcap: thresh = minHitGeVHE_; break;
  case HcalOuter:  thresh = minHitGeVHO_; break;
  case HcalForward:
    if (id.depth() == 1)      thresh = minHitGeVHF1_;
    else if (id.depth() == 2) thresh = minHitGeVHF2_;
    else                      thresh = 0.0;
    break;
  default:         thresh = 0.0; break;
  }
  return thresh;
}
#endif

//======================================================================

// ------------ method called to for each event  ------------
void
HcalSmearValAlgos::process(const myEventData& unsmeared, 
			   const myEventData& smeared)
{
  using namespace edm;

  uint32_t runn = unsmeared.runNumber();

  if (notInSet<uint32_t>(s_runs_,runn)) {
    bookPerRunHistos(runn);
    s_runs_.insert(runn);
  }
      
  compareHits<HBHERecHit,HBHERecHitCollection>(unsmeared.hbherechits(),
					       smeared.hbherechits());
  compareHits<HFRecHit,HFRecHitCollection>(unsmeared.hfrechits(),
					   smeared.hfrechits());
  compareHits<HORecHit,HORecHitCollection>(unsmeared.horechits(),
					   smeared.horechits());
}


// ------------ method called once each job just before starting event loop  ------------
void 
HcalSmearValAlgos::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalSmearValAlgos::endJob() {
}

