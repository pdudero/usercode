/**\class HcalTimingAnalAlgos HcalTimingAnalAlgos.cc MyEDmodules/HcalTimingAnalAlgos/src/HcalTimingAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalTimingAnalAlgos.cc,v 1.3 2009/05/06 19:45:10 dudero Exp $
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
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "MyEDmodules/HcalTimingAnalyzer/src/HcalTimingAnalAlgos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"

#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
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
HcalTimingAnalAlgos::HcalTimingAnalAlgos(const edm::ParameterSet& iConfig) :
  minHitGeVHB_(iConfig.getParameter<double>("minHitGeVHB")),
  minHitGeVHE_(iConfig.getParameter<double>("minHitGeVHE")),
  minHitGeVHO_(iConfig.getParameter<double>("minHitGeVHO")),
  minHitGeVHF1_(iConfig.getParameter<double>("minHitGeVHF1")),
  minHitGeVHF2_(iConfig.getParameter<double>("minHitGeVHF2")),
  ecalRecHitTscaleNbins_(iConfig.getParameter<int>("ecalRecHitTscaleNbins")),
  ecalRecHitTscaleMinNs_(iConfig.getParameter<double>("ecalRecHitTscaleMinNs")),
  ecalRecHitTscaleMaxNs_(iConfig.getParameter<double>("ecalRecHitTscaleMaxNs")),
  hcalRecHitTscaleNbins_(iConfig.getParameter<int>("hcalRecHitTscaleNbins")),
  hcalRecHitTscaleMinNs_(iConfig.getParameter<double>("hcalRecHitTscaleMinNs")),
  hcalRecHitTscaleMaxNs_(iConfig.getParameter<double>("hcalRecHitTscaleMaxNs")),
  hcalRecHitEscaleMinGeV_(iConfig.getParameter<double>("hcalRecHitEscaleMinGeV")),
  hcalRecHitEscaleMaxGeV_(iConfig.getParameter<double>("hcalRecHitEscaleMaxGeV")),
  simHitTscaleNbins_(iConfig.getParameter<int>("simHitTscaleNbins")),
  simHitTscaleMinNs_(iConfig.getParameter<double>("simHitTscaleMinNs")),
  simHitTscaleMaxNs_(iConfig.getParameter<double>("simHitTscaleMaxNs")),
  simHitEnergyMinGeVthreshold_(iConfig.getParameter<double>("simHitEnergyMinGeVthreshold")),
  rundescr_(iConfig.getUntrackedParameter<std::string>("runDescription",""))
{

 //now do what ever initialization is needed
  std::vector<int> v_events2anal(iConfig.getParameter<std::vector<int> >("eventNumbers"));

  for (unsigned i=0; i<v_events2anal.size(); i++) {
    s_events2anal_.insert(v_events2anal[i]);
  }

  cutNone_       = new myAnalCut(0,"cutNone");
  cutMaxHitOnly_ = new myAnalCut(1,"cutMaxHitOnly");
  cutTgtTwrOnly_ = new myAnalCut(2,"cutTgtTwrOnly");
  cutMinHitGeV_  = new myAnalCut(3,"cutMinHitGeV");
  cutAll_        = new myAnalCut(4,"cutAll");

  std::vector<int> v_tgtid = iConfig.getParameter<std::vector<int> > ("tgtTwrId");
  if ((v_tgtid.size() > 0) &&
      !convertIdNumbers(v_tgtid, tgtTwrId_))
    throw cms::Exception("Invalid detID vector");
}

//======================================================================

bool
HcalTimingAnalAlgos::convertIdNumbers(std::vector<int>& v_idnumbers,
				      HcalDetId& detId)
{
  // convert det ID numbers to valid detIds:
  if (v_idnumbers.size()!=3) {
    return false;
  }

  int ieta  = v_idnumbers[0];
  int iphi  = v_idnumbers[1];
  int depth = v_idnumbers[2];

  enum HcalSubdetector subdet;
  if      (depth == 4)       subdet = HcalOuter;
  else if (abs(ieta) <= 16)  subdet = HcalBarrel;
  else if (abs(ieta) <= 28)  subdet = HcalOuter; // gud enuf fer gubmint werk
  else if (abs(ieta) <= 41)  subdet = HcalForward;
  else return false;

  if (!HcalDetId::validDetId(subdet,ieta,iphi,depth))
    return false;

  detId = HcalDetId(subdet,ieta,iphi,depth);
  return true;
}                                   // HFtrigAnalAlgos::convertIdNumbers

//======================================================================

void HcalTimingAnalAlgos::bookPerRunHistos(const uint32_t rn)
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
  hpars1d.nbinsy = 0;

  st_avgPulse_ = "pulse" + runstrn;
  hpars1d.name   = st_avgPulse_;
  hpars1d.title  = "HBHE Average Pulse Shape " + runstrt;
  hpars1d.nbinsx = 10;
  hpars1d.minx   = -0.5;
  hpars1d.maxx   =  9.5;

  v_hpars1d.push_back(hpars1d);

  st_rhTimes_ = "RHTimes" + runstrn;
  hpars1d.name   = st_rhTimes_;
  hpars1d.title  = "HBHE RecHit Times " + runstrt + "; Rechit Time (ns)";
  hpars1d.nbinsx = hcalRecHitTscaleNbins_;
  hpars1d.minx   = hcalRecHitTscaleMinNs_;
  hpars1d.maxx   = hcalRecHitTscaleMaxNs_;

  v_hpars1d.push_back(hpars1d);

  st_rhEnergies_ = "RHEnergies" + runstrn;
  hpars1d.name   = st_rhEnergies_;
  hpars1d.title  = "HBHE RecHit Energies " + runstrt + "; Rechit Energy (GeV)";
  hpars1d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_ - hcalRecHitEscaleMinGeV_);
  hpars1d.minx   = hcalRecHitEscaleMinGeV_;
  hpars1d.maxx   = hcalRecHitEscaleMaxGeV_;

  v_hpars1d.push_back(hpars1d);

  st_shTimes_ = "SHTimes" + runstrn;
  hpars1d.name   = st_shTimes_;
  hpars1d.title  = "Hcal SimHit Times " + runstrt + "; Simhit Time (ns)";
  hpars1d.nbinsx = simHitTscaleNbins_;
  hpars1d.minx   = simHitTscaleMinNs_;
  hpars1d.maxx   = simHitTscaleMaxNs_;

  v_hpars1d.push_back(hpars1d);

  st_shEnergies_ = "SHEnergies" + runstrn;
  hpars1d.name   = st_shEnergies_;
  hpars1d.title  = "HBHE SimHit Energies " + runstrt + "; Simhit Energy * 200)";
  hpars1d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_ - hcalRecHitEscaleMinGeV_);
  hpars1d.minx   = hcalRecHitEscaleMinGeV_;
  hpars1d.maxx   = hcalRecHitEscaleMaxGeV_;

  v_hpars1d.push_back(hpars1d);

  st_caloMet_Met_ = "h_caloMet_Met" + runstrn;
  hpars1d.name   = st_caloMet_Met_;
  hpars1d.title  = "MET from CaloTowers " + runstrt + "; MET (GeV)";
  hpars1d.nbinsx = 100;
  hpars1d.minx   = 0.;
  hpars1d.maxx   = hcalRecHitEscaleMaxGeV_;

  v_hpars1d.push_back(hpars1d);

  st_caloMet_Phi_ = "h_caloMet_Phi" + runstrn;
  hpars1d.name   = st_caloMet_Phi_;
  hpars1d.title  = "MET #phi from CaloTowers " + runstrt + "; MET phi (rad)";
  hpars1d.nbinsx = 100;
  hpars1d.minx   =  -4.;
  hpars1d.maxx   =   4.;

  v_hpars1d.push_back(hpars1d);

  st_caloMet_SumEt_ = "h_caloMet_SumEt" + runstrn;
  hpars1d.name   = st_caloMet_SumEt_;
  hpars1d.title  = "SumET from CaloTowers " + runstrt + "; SumET (GeV)";
  hpars1d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_ - hcalRecHitEscaleMinGeV_);
  hpars1d.minx   = hcalRecHitEscaleMinGeV_;
  hpars1d.maxx   = hcalRecHitEscaleMaxGeV_;

  v_hpars1d.push_back(hpars1d);

  /*****************************************
   * 2-D HISTOGRAMS AFTER:                 *
   *****************************************/

  st_rhEmap_   = "h_rhEperIetaIphi" + runstrn;
  hpars2d.name   = st_rhEmap_;
  hpars2d.title  = "HBHE RecHit Energy Map " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2d.push_back(hpars2d);

  st_rhTimingVsE_ = "h_rhTimingVsE" + runstrn;
  hpars2d.name   = st_rhTimingVsE_;
  hpars2d.title  = "HBHE RecHit Timing vs. Energy " + runstrt + "; Rechit Energy (GeV); Rechit Time (ns)";
  hpars2d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_ - hcalRecHitEscaleMinGeV_);
  hpars2d.minx   = hcalRecHitEscaleMinGeV_;
  hpars2d.maxx   = hcalRecHitEscaleMaxGeV_;
  hpars2d.nbinsy = hcalRecHitTscaleNbins_;
  hpars2d.miny   = hcalRecHitTscaleMinNs_;
  hpars2d.maxy   = hcalRecHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_shTimingVsE_ = "h_shTimingVsE" + runstrn;
  hpars2d.name   = st_shTimingVsE_;
  hpars2d.title  = "Hcal SimHit Timing vs. Energy " + runstrt + "; Simhit Energy * 200; Simhit Time (ns)";
  hpars2d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_) + 1;
  hpars2d.minx   =  -0.5;
  hpars2d.maxx   = hcalRecHitEscaleMaxGeV_;
  hpars2d.nbinsy = simHitTscaleNbins_;
  hpars2d.miny   = simHitTscaleMinNs_;
  hpars2d.maxy   = simHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_ctHcalTvstwrE_ = "h_ctHcalTvstwrE" + runstrn;
  hpars2d.name   = st_ctHcalTvstwrE_;
  hpars2d.title  = "Calo Tower HCAL Timing vs. Energy " + runstrt + "; Tower Energy (GeV); HCAL Time (ns)";
  hpars2d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_ - hcalRecHitEscaleMinGeV_);
  hpars2d.minx   = 2*hcalRecHitEscaleMinGeV_;
  hpars2d.maxx   = 2*hcalRecHitEscaleMaxGeV_;
  hpars2d.nbinsy = 100;      // hcalRecHitTscaleNbins_;
  hpars2d.miny   = -35000.0; // hcalRecHitTscaleMinNs_;
  hpars2d.maxy   =  10000.0; // hcalRecHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_ctHcalTvstwrEg_ = "h_ctHcalTvstwrEgood" + runstrn;
  hpars2d.name   = st_ctHcalTvstwrEg_;
  hpars2d.title  = "Calo Tower HCAL Timing vs. Energy " + runstrt + "; Tower Energy (GeV); HCAL Time (ns)";
  hpars2d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_ - hcalRecHitEscaleMinGeV_);
  hpars2d.minx   = 2*hcalRecHitEscaleMinGeV_;
  hpars2d.maxx   = 2*hcalRecHitEscaleMaxGeV_;
  hpars2d.nbinsy = hcalRecHitTscaleNbins_;
  hpars2d.miny   = hcalRecHitTscaleMinNs_;
  hpars2d.maxy   = hcalRecHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_ctEcalTvstwrE_ = "h_ctEcalTvstwrE" + runstrn;
  hpars2d.name   = st_ctEcalTvstwrE_;
  hpars2d.title  = "Calo Tower ECAL Timing vs. Energy " + runstrt + "; Tower Energy (GeV); ECAL Time (ns)";
  hpars2d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_ - hcalRecHitEscaleMinGeV_);
  hpars2d.minx   = 2*hcalRecHitEscaleMinGeV_;
  hpars2d.maxx   = 2*hcalRecHitEscaleMaxGeV_;
  hpars2d.nbinsy = 100;      // ecalRecHitTscaleNbins_;
  hpars2d.miny   = -35000.0; // ecalRecHitTscaleMinNs_;
  hpars2d.maxy   =  10000.0; // ecalRecHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_ctEcalTvstwrEg_ = "h_ctEcalTvstwrEgood" + runstrn;
  hpars2d.name   = st_ctEcalTvstwrEg_;
  hpars2d.title  = "Calo Tower ECAL Timing vs. Energy " + runstrt + "; Tower Energy (GeV); ECAL Time (ns)";
  hpars2d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_ - hcalRecHitEscaleMinGeV_);
  hpars2d.minx   = 2*hcalRecHitEscaleMinGeV_;
  hpars2d.maxx   = 2*hcalRecHitEscaleMaxGeV_;
  hpars2d.nbinsy = ecalRecHitTscaleNbins_;
  hpars2d.miny   = ecalRecHitTscaleMinNs_;
  hpars2d.maxy   = ecalRecHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_ctEcalTvsHcalT_ = "h_ctEcalTvsHcalT" + runstrn;
  hpars2d.name   = st_ctEcalTvsHcalT_;
  hpars2d.title  = "Calo Tower ECAL Time vs. HCAL Time " + runstrt + "; HCAL Time (ns); ECAL Time (ns)";
  hpars2d.nbinsx = 100;      // hcalRecHitTscaleNbins_;
  hpars2d.minx   = -35000.0; // hcalRecHitTscaleMinNs_;
  hpars2d.maxx   =  10000.0; // hcalRecHitTscaleMaxNs_;
  hpars2d.nbinsy = 100;      // ecalRecHitTscaleNbins_;
  hpars2d.miny   = -35000.0; // ecalRecHitTscaleMinNs_;
  hpars2d.maxy   =  10000.0; // ecalRecHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  st_ctEcalTvsHcalTg_ = "h_ctEcalTvsHcalTgood" + runstrn;
  hpars2d.name   = st_ctEcalTvsHcalTg_;
  hpars2d.title  = "Calo Tower ECAL Time vs. HCAL Time " + runstrt + "; HCAL Time (ns); ECAL Time (ns)";
  hpars2d.nbinsx = hcalRecHitTscaleNbins_;
  hpars2d.minx   = hcalRecHitTscaleMinNs_;
  hpars2d.maxx   = hcalRecHitTscaleMaxNs_;
  hpars2d.nbinsy = ecalRecHitTscaleNbins_;
  hpars2d.miny   = ecalRecHitTscaleMinNs_;
  hpars2d.maxy   = ecalRecHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

#if 0
  st_ctWeightTvsE_ = "h_ctWeightTvsE" + runstrn;
  hpars2d.name   = st_ctWeightTvsE_;
  hpars2d.title  = "Calo Tower E-Weighted Time vs. Energy " + runstrt + "; Tower Energy (GeV); E-Weighted Time (ns)";
  hpars2d.nbinsx = (uint32_t)(hcalRecHitEscaleMaxGeV_ - hcalRecHitEscaleMinGeV_);
  hpars2d.minx   = 2*hcalRecHitEscaleMinGeV_;
  hpars2d.maxx   = 2*hcalRecHitEscaleMaxGeV_;
  hpars2d.nbinsy = hcalRecHitTscaleNbins_;
  hpars2d.miny   = hcalRecHitTscaleMinNs_;
  hpars2d.maxy   = hcalRecHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);
#endif

  /*****************************************
   * 2-D PROFILES:                         *
   *****************************************/

  st_rhTprofd1_  = "h_rhTperIetaIphiDepth1Prof" + runstrn;
  hpars2d.name   = st_rhTprofd1_;
  hpars2d.title  = "HBEF (Depth 1) RecHit Time Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_rhTprofd2_  = "h_rhTperIetaIphiDepth2Prof" + runstrn;
  hpars2d.name   = st_rhTprofd2_;
  hpars2d.title  = "HBEF (Depth 2) RecHit Time Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_rhTprofd3_  = "h_rhTperIetaIphiDepth3Prof" + runstrn;
  hpars2d.name   = st_rhTprofd3_;
  hpars2d.title  = "HBEF (Depth 3) RecHit Time Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_rhTprofd4_  = "h_rhTperIetaIphiDepth4Prof" + runstrn;
  hpars2d.name   = st_rhTprofd4_;
  hpars2d.title  = "HBEF (Depth 4) RecHit Time Map-Profile " + runstrt + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2dprof.push_back(hpars2d);

  st_ctTprof_    = "h_ctTperIetaIphiProf" + runstrn;
  hpars2d.name   = st_ctTprof_;
  hpars2d.title  = "Calo Tower Weighted Time Map-Profile " + runstrt + "; ieta; iphi";
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
  cutMinHitGeV_->histos()->book1d<TH1D>(v_hpars1d);
  cutMaxHitOnly_->histos()->book1d<TH1D>(v_hpars1d);
#if 0
  cutTgtTwrOnly_->histos()->book1d<TH1D>(v_hpars1d);
  cutAll_->histos()->book1d<TH1D>(v_hpars1d);
#endif

  cutNone_->histos()->book2d<TH2D>(v_hpars2d);
  cutMinHitGeV_->histos()->book2d<TH2D>(v_hpars2d);
  cutMaxHitOnly_->histos()->book2d<TH2D>(v_hpars2d);
#if 0
  cutTgtTwrOnly_->histos()->book2d<TH2D>(v_hpars2d);
  cutAll_->histos()->book2d<TH2D>(v_hpars2d);
#endif

  cutNone_->histos()->book2d<TProfile2D>(v_hpars2dprof);
  cutMinHitGeV_->histos()->book2d<TProfile2D>(v_hpars2dprof);
  cutMaxHitOnly_->histos()->book2d<TProfile2D>(v_hpars2dprof);
#if 0
  cutTgtTwrOnly_->histos()->book2d<TProfile2D>(v_hpars2dprof);
  cutAll_->histos()->book2d<TProfile2D>(v_hpars2dprof);
#endif
}                                                    // bookPerRunHistos

//======================================================================

double HcalTimingAnalAlgos::lookupThresh(const HcalDetId& id)
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

//======================================================================

// ------------ method called to for each event  ------------
void
HcalTimingAnalAlgos::process(const myEventData& ed)
{
  using namespace edm;

  uint32_t runn = ed.runNumber();
  uint32_t evtn = ed.evtNumber();

  if (notInSet<uint32_t>(s_runs_,runn)) {
    bookPerRunHistos(runn);
    s_runs_.insert(runn);
  }

  TH1F *h_hbp = 0;
  TH1F *h_hep = 0;
  TH1F *h_hbm = 0;
  TH1F *h_hem = 0;
  TH1F *h_hbpt= 0;
  TH1F *h_hept= 0;
  TH1F *h_hbmt= 0;
  TH1F *h_hemt= 0;

  if (inSet<uint32_t>(s_events2anal_,evtn)) {
    edm::Service<TFileService> fs;
    char n[80];
    sprintf (n,"run%devent%dpulseHBP", runn,evtn);  h_hbp = fs->make<TH1F>(n,n, 10,-0.5,9.5 );
    sprintf (n,"run%devent%dpulseHEP", runn,evtn);  h_hep = fs->make<TH1F>(n,n, 10,-0.5,9.5 );
    sprintf (n,"run%devent%dpulseHBM", runn,evtn);  h_hbm = fs->make<TH1F>(n,n, 10,-0.5,9.5 );
    sprintf (n,"run%devent%dpulseHEM", runn,evtn);  h_hem = fs->make<TH1F>(n,n, 10,-0.5,9.5 );

    sprintf (n,"run%devent%dRHTimesHBP",runn,evtn); h_hbpt=fs->make<TH1F>(n,n,101,-10.5,90.5);
    sprintf (n,"run%devent%dRHTimesHEP",runn,evtn); h_hept=fs->make<TH1F>(n,n,101,-10.5,90.5);
    sprintf (n,"run%devent%dRHTimesHBM",runn,evtn); h_hbmt=fs->make<TH1F>(n,n,101,-10.5,90.5);
    sprintf (n,"run%devent%dRHTimesHEM",runn,evtn); h_hemt=fs->make<TH1F>(n,n,101,-10.5,90.5);
  }

  std::set<HcalDetId> s_idsOverThresh;

  /***************************************************
   *                    HBHE RECHITS                 *
   ***************************************************/

  //HBHERecHit maxrh(HcalDetId::Undefined,-1e99,0.0);
  HcalDetId maxId;
  double  maxenergy = -1e99;
  double  maxtime = -1e99;

  for (unsigned irh = 0; irh < ed.hbherechits()->size (); ++irh) {
    const HBHERecHit& rh = (*(ed.hbherechits()))[irh];

    //std::cout << rh.id().rawId() << std::endl;

    HcalDetId detId = rh.id();
    double htime    = rh.time();
    double energy   = rh.energy();
    int    ieta     = detId.ieta();
    int    iphi     = detId.iphi();
    int    depth    = detId.depth();

    TH1F *hp = 0;
    if (detId.subdet() == HcalBarrel) hp = (detId.zside() > 0) ? h_hbpt : h_hbmt;
    if (detId.subdet() == HcalEndcap) hp = (detId.zside() > 0) ? h_hept : h_hemt;

    if (hp) hp->Fill(htime);

    myAnalHistos *myAH = cutNone_->histos();

    myAH->fill1d<TH1D>(st_rhTimes_,htime);
    myAH->fill1d<TH1D>(st_rhEnergies_,energy);
    myAH->fill2d<TH2D>(st_rhEmap_,ieta,iphi,energy);
    myAH->fill2d<TH2D>(st_rhTimingVsE_,energy,htime);

    std::string st_rhTprof;
    switch(depth) {
    case 1: st_rhTprof = st_rhTprofd1_; break;
    case 2: st_rhTprof = st_rhTprofd2_; break;
    case 3: st_rhTprof = st_rhTprofd3_; break;
      //case 4: st_rhTprof = st_rhTprofd4_; break; // not for hbhe!
    default:
      edm::LogWarning("Invalid depth in rechit collection! detId = ") << detId << std::endl;
      continue;
    }

    if (energy > 50.0)
      myAH->fill2d<TProfile2D>(st_rhTprof,ieta,iphi,htime);

    double minHitGeV = lookupThresh(detId);

    //if (energy > maxrh.energy()) maxrh = rh;
    if (energy > maxenergy) {
      maxtime   = htime;
      maxenergy = energy;
      maxId= rh.id(); 
    }
#if 0
    if (detId == tgtTwrId_) {
      myAnalHistos *myAH = cutTgtTwrOnly_->histos();
    
      myAH->fill1d<TH1D>(st_rhTimes_,htime);
      myAH->fill1d<TH1D>(st_rhEnergies_,energy);
      myAH->fill2d<TH2D>(st_rhEmap_,rh.id().ieta(),rh.id().iphi(),energy);
      myAH->fill2d<TH2D>(st_rhTimingVsE_,energy,htime);
      if (energy > 50.0)
	myAH->fill2d<TProfile2D>(st_rhTprof,rh.id().ieta(),rh.id().iphi(),htime);
    }
#endif
    if (energy > minHitGeV) {
      s_idsOverThresh.insert(rh.id());
      myAnalHistos *myAH = cutMinHitGeV_->histos();

      myAH->fill1d<TH1D>(st_rhTimes_,htime);
      myAH->fill1d<TH1D>(st_rhEnergies_,energy);
      myAH->fill2d<TH2D>(st_rhEmap_,rh.id().ieta(),rh.id().iphi(),energy);
      myAH->fill2d<TH2D>(st_rhTimingVsE_,energy,htime);
      if (energy > 50.0)
	myAH->fill2d<TProfile2D>(st_rhTprof,rh.id().ieta(),rh.id().iphi(),htime);
    }
  } // loop over rechits

  //if (maxrh.id() != HcalDetId::Undefined) {
  if (maxenergy > 0.0) {
    myAnalHistos *myAH = cutMaxHitOnly_->histos();

    myAH->fill1d<TH1D>(st_rhTimes_,maxtime);
    myAH->fill1d<TH1D>(st_rhEnergies_,maxenergy);
    myAH->fill2d<TH2D>(st_rhEmap_,maxId.ieta(),maxId.iphi(),maxenergy);
    myAH->fill2d<TH2D>(st_rhTimingVsE_,maxenergy,maxtime);

    switch(maxId.depth()) {
    case 1: myAH->fill2d<TProfile2D>(st_rhTprofd1_,maxId.ieta(),maxId.iphi(),maxtime); break;
    case 2: myAH->fill2d<TProfile2D>(st_rhTprofd2_,maxId.ieta(),maxId.iphi(),maxtime); break;
    case 3: myAH->fill2d<TProfile2D>(st_rhTprofd3_,maxId.ieta(),maxId.iphi(),maxtime); break;
      //case 4: not for HBHE!
    default:
      edm::LogWarning("Invalid depth in rechit collection! detId = ") << maxId << std::endl;
    }

#if 0
    if ((energy > lookupThresh(maxId)) &&
	(maxId == tgtTwrId_)) {
      cutAll_->histos()->fill1d<TH1D>(st_rhTimes_,htime);
      cutAll_->histos()->fill1d<TH1D>(st_rhEnergies_,energy);
      cutAll_->histos()->fill2d<TH2D>(st_rhEmap_,maxId.ieta(),maxId.iphi(),energy);
      if (energy > 50.0)
	cutAll_->histos()->fill2d<TProfile2D>(st_rhTprof,maxId.ieta(),maxId.iphi(),htime);
    }
#endif
  }

  /***************************************************
   *                    HF   RECHITS                 *
   ***************************************************/

  for (unsigned irh = 0; irh < ed.hfrechits()->size (); ++irh) {
    const HFRecHit& rh = (*(ed.hfrechits()))[irh];
    HcalDetId detId = rh.id();

    std::string st_rhTprof;
    switch(detId.depth()) {
    case 1: st_rhTprof = st_rhTprofd1_; break;
    case 2: st_rhTprof = st_rhTprofd2_; break;
      //case 3: st_rhTprof = st_rhTprofd3_; break; // not for hf!
      //case 4: st_rhTprof = st_rhTprofd4_; break; // not for hf!
    default:
      edm::LogWarning("Invalid depth in rechit collection! detId = ") << detId << std::endl;
      continue;
    }

    double minHitGeV = lookupThresh(detId);

    if (rh.energy() > minHitGeV) {
      cutMinHitGeV_->histos()->fill2d<TH2D>(st_rhEmap_,
					    detId.ieta(),
					    detId.iphi()+detId.depth()-1,
					    rh.energy());
      if (rh.energy() > 50.0)
	cutMinHitGeV_->histos()->fill2d<TProfile2D>(st_rhTprof,
						    detId.ieta(),
						    detId.iphi()+detId.depth()-1,
						    rh.time());
    }

  } // loop over HF rechits

  /***************************************************
   *                    HO   RECHITS                 *
   ***************************************************/

  for (unsigned irh = 0; irh < ed.horechits()->size (); ++irh) {
    const HORecHit& rh = (*(ed.horechits()))[irh];
    HcalDetId detId = rh.id();

    std::string st_rhTprof;
    if (detId.depth() != 4) {
      edm::LogWarning("Invalid depth in rechit collection! detId = ") << detId << std::endl;
      continue;
    }

    double minHitGeV = lookupThresh(rh.id());

    if (rh.energy() > minHitGeV) {
      cutMinHitGeV_->histos()->fill2d<TH2D>(st_rhEmap_,
					    detId.ieta(),
					    detId.iphi(),
					    rh.energy());
      if (rh.energy() > 50.0)
	cutMinHitGeV_->histos()->fill2d<TProfile2D>(st_rhTprofd4_,
						    detId.ieta(),
						    detId.iphi(),
						    rh.time());
    }
  } // loop over HO rechits

  /***************************************************
   *                    SIMHITS                      *
   ***************************************************/

  if (ed.simhits().isValid()) {
    std::vector<PCaloHit>  simhits;
    simhits.insert(simhits.end(),ed.simhits()->begin(),ed.simhits()->end());

    for (unsigned ish = 0; ish < simhits.size (); ++ish) {
      const PCaloHit& sh = simhits[ish];

      double htime    = sh.time();
      double energy   = sh.energy() * 200;

      if (energy > simHitEnergyMinGeVthreshold_) {
	myAnalHistos *myAH = cutMinHitGeV_->histos();
	myAH->fill1d<TH1D>(st_shTimes_,htime);
	myAH->fill1d<TH1D>(st_shEnergies_,energy);
	myAH->fill2d<TH2D>(st_shTimingVsE_,energy,htime);
      }
      
    } // loop over simhits
  }

  /***************************************************
   *                    HBHE DIGIS                   *
   ***************************************************/

  if (ed.hbhedigis().isValid()) {
    for (unsigned idig = 0; idig < ed.hbhedigis()->size (); ++idig) {
      const HBHEDataFrame& frame = (*(ed.hbhedigis()))[idig];
      HcalDetId detId = frame.id();
      TH1F *hp = 0;
      if (detId.subdet() == HcalBarrel) hp = (detId.zside() > 0) ? h_hbp : h_hbm; 
      if (detId.subdet() == HcalEndcap) hp = (detId.zside() > 0) ? h_hep : h_hem; 
      
      for (int isample = 0; isample < std::min(10,frame.size()); ++isample) {
	int rawadc = frame[isample].adc();
	if (hp) hp->Fill(isample,rawadc);

	cutNone_->histos()->fill1d<TH1D>(st_avgPulse_,isample,rawadc);

	if (inSet<HcalDetId>(s_idsOverThresh,detId))
	  cutMinHitGeV_->histos()->fill1d<TH1D>(st_avgPulse_,isample,rawadc);

      } // loop over samples in digi
    } // loop over digi
  } // if have digis


  /***************************************************
   *                    CALO TOWERS                  *
   ***************************************************/

  if (ed.towers().isValid()) {
    //TTree t1("t1","BadTowers");
    CaloTower *badecal = new CaloTower();
    CaloTower *badhcal = new CaloTower();
    CaloTower *badboth = new CaloTower();

    //TBranch *badecalbr = t1.Branch<CaloTower>("BadECAL",&badecal);
    //TBranch *badhcalbr = t1.Branch<CaloTower>("BadHCAL",&badhcal);
    //TBranch *badbothbr = t1.Branch<CaloTower>("BadBoth",&badboth);

    for (unsigned itwr=0; itwr<ed.towers()->size(); ++itwr) {
      const CaloTower& twr = (*(ed.towers()))[itwr];
      double twrEnergy = twr.hadEnergy() + twr.emEnergy(); // + twr.outerEnergy();
      double ecalT     = twr.ecalTime();
      double hcalT     = twr.hcalTime();

      myAnalHistos *myAH = cutNone_->histos();
      myAH->fill2d<TH2D>(st_ctHcalTvstwrE_,twrEnergy,hcalT);
      myAH->fill2d<TH2D>(st_ctEcalTvstwrE_,twrEnergy,ecalT);
      myAH->fill2d<TH2D>(st_ctEcalTvsHcalT_,hcalT,ecalT);

      myAH = cutMinHitGeV_->histos();

      if (twr.hcalTime() > -200.0) {
	if (twrEnergy > 0.) myAH->fill2d<TH2D>(st_ctHcalTvstwrEg_,twrEnergy,hcalT);
	if (twr.ecalTime() > -200.0) {
	  if (twrEnergy > 0) {
	    myAH->fill2d<TH2D>(st_ctEcalTvsHcalTg_,hcalT,ecalT);
	    myAH->fill2d<TH2D>(st_ctEcalTvstwrEg_,twrEnergy,ecalT);
	  }
	} else {
	  *badecal = twr;
	  //badecalbr->Fill();
	}
      } else {
	if (twr.ecalTime() > -200.0) {
	  myAH->fill2d<TH2D>(st_ctEcalTvstwrEg_,twrEnergy,ecalT);
	  *badhcal = twr;
	  //badhcalbr->Fill();
	} else {
	  *badboth = twr;
	  //badbothbr->Fill();
	}
      }
#if 0
      double weightedT =  // doesn't mean much now with hcal energy mean off of zero.
	(hcalT*twr.hadEnergy() + ecalT*twr.emEnergy())/twrEnergy;
      myAH->fill2d<TH2D>(st_ctWeightTvsE_,twrEnergy,weightedT);
      myAH->fill2d<TProfile2D>(st_ctTprof_,twr.ieta(), twr.iphi(), weightedT);
#endif
    } // loop over towers

    //t1.Write("", TObject::kOverwrite);

  } // if have towers


  /***************************************************
   *                    HF   RECHITS                 *
   ***************************************************/

  // MET
  if (ed.recmet().isValid()) {
    const CaloMET *calomet = &(ed.recmet().product()->front());
    cutNone_->histos()->fill1d<TH1D>(st_caloMet_Met_,calomet->pt());
    cutNone_->histos()->fill1d<TH1D>(st_caloMet_Phi_,calomet->phi());
    cutNone_->histos()->fill1d<TH1D>(st_caloMet_SumEt_,calomet->sumEt());
  }
}


// ------------ method called once each job just before starting event loop  ------------
void 
HcalTimingAnalAlgos::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalTimingAnalAlgos::endJob() {
}

