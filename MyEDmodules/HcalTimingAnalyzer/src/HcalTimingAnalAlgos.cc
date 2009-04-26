/**\class HcalTimingAnalAlgos HcalTimingAnalAlgos.cc MyEDmodules/HcalTimingAnalAlgos/src/HcalTimingAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalTimingAnalAlgos.cc,v 1.1 2009/04/09 21:55:48 dudero Exp $
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

#include "MyEDmodules/HcalTimingAnalyzer/src/HcalTimingAnalAlgos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"

#include "TH1F.h"
#include "TH2F.h"

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
  recHitTscaleNbins_(iConfig.getParameter<int>("recHitTscaleNbins")),
  recHitTscaleMinNs_(iConfig.getParameter<double>("recHitTscaleMinNs")),
  recHitTscaleMaxNs_(iConfig.getParameter<double>("recHitTscaleMaxNs")),
  recHitEscaleMinGeV_(iConfig.getParameter<double>("recHitEscaleMinGeV")),
  recHitEscaleMaxGeV_(iConfig.getParameter<double>("recHitEscaleMaxGeV")),
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
  if (!convertIdNumbers(v_tgtid, tgtTwrId_))
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
  std::vector<myAnalHistos::HistoParams_t> v_hpars; 

  myAnalHistos::HistoParams_t hpars;

  stringstream o; o<<rn;

  std::string runstrn = (rn>10000) ? o.str() : rundescr_;
  std::string runstrt =
    (rn>10000) ? "[RUN:" + o.str() + "]" : "[" + rundescr_ + "]";

  /*****************************************
   * 1-D HISTOGRAMS FIRST:                 *
   *****************************************/
  hpars.nbinsy = 0;

  st_avgPulse_ = "pulse" + runstrn;
  hpars.name   = st_avgPulse_;
  hpars.title  = "HBHE Average Pulse Shape " + runstrt;
  hpars.nbinsx = 10;
  hpars.minx   = -0.5;
  hpars.maxx   =  9.5;

  v_hpars.push_back(hpars);

  st_rhTimes_ = "RHTimes" + runstrn;
  hpars.name   = st_rhTimes_;
  hpars.title  = "HBHE RecHit Times " + runstrt + "; Rechit Time (ns)";
  hpars.nbinsx = recHitTscaleNbins_;
  hpars.minx   = recHitTscaleMinNs_;
  hpars.maxx   = recHitTscaleMaxNs_;

  v_hpars.push_back(hpars);

  st_rhEnergies_ = "RHEnergies" + runstrn;
  hpars.name   = st_rhEnergies_;
  hpars.title  = "HBHE RecHit Energies " + runstrt + "; Rechit Energy (GeV)";
  hpars.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars.minx   = recHitEscaleMinGeV_;
  hpars.maxx   = recHitEscaleMaxGeV_;

  v_hpars.push_back(hpars);

  st_shTimes_ = "SHTimes" + runstrn;
  hpars.name   = st_shTimes_;
  hpars.title  = "Hcal SimHit Times " + runstrt + "; Simhit Time (ns)";
  hpars.nbinsx = simHitTscaleNbins_;
  hpars.minx   = simHitTscaleMinNs_;
  hpars.maxx   = simHitTscaleMaxNs_;

  v_hpars.push_back(hpars);

  st_shEnergies_ = "SHEnergies" + runstrn;
  hpars.name   = st_shEnergies_;
  hpars.title  = "HBHE SimHit Energies " + runstrt + "; Simhit Energy * 200)";
  hpars.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars.minx   = recHitEscaleMinGeV_;
  hpars.maxx   = recHitEscaleMaxGeV_;

  v_hpars.push_back(hpars);

  st_caloMet_Met_ = "h_caloMet_Met" + runstrn;
  hpars.name   = st_caloMet_Met_;
  hpars.title  = "MET from CaloTowers " + runstrt + "; MET (GeV)";
  hpars.nbinsx = 100;
  hpars.minx   = 0.;
  hpars.maxx   = recHitEscaleMaxGeV_;

  v_hpars.push_back(hpars);

  st_caloMet_Phi_ = "h_caloMet_Phi" + runstrn;
  hpars.name   = st_caloMet_Phi_;
  hpars.title  = "MET #phi from CaloTowers " + runstrt + "; MET phi (rad)";
  hpars.nbinsx = 100;
  hpars.minx   =  -4.;
  hpars.maxx   =   4.;

  v_hpars.push_back(hpars);

  st_caloMet_SumEt_ = "h_caloMet_SumEt" + runstrn;
  hpars.name   = st_caloMet_SumEt_;
  hpars.title  = "SumET from CaloTowers " + runstrt + "; SumET (GeV)";
  hpars.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars.minx   = recHitEscaleMinGeV_;
  hpars.maxx   = recHitEscaleMaxGeV_;

  v_hpars.push_back(hpars);

  /*****************************************
   * 2-D HISTOGRAMS AFTER:                 *
   *****************************************/

  st_rhEmap_   = "h_rhEperIetaIphi" + runstrn;
  hpars.name   = st_rhEmap_;
  hpars.title  = "HBHE RecHit Energy Map " + runstrt + "; ieta; iphi";
  hpars.nbinsx =  83;
  hpars.minx   =  -41.5;
  hpars.maxx   =   41.5;
  hpars.nbinsy =  72;
  hpars.miny   =    0.5;
  hpars.maxy   =   72.5;

  v_hpars.push_back(hpars);

  st_rhTimingVsE_ = "h_rhTimingVsE" + runstrn;
  hpars.name   = st_rhTimingVsE_;
  hpars.title  = "HBHE RecHit Timing vs. Energy " + runstrt + "; Rechit Energy (GeV); Rechit Time (ns)";
  hpars.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars.minx   = recHitEscaleMinGeV_;
  hpars.maxx   = recHitEscaleMaxGeV_;
  hpars.nbinsy = recHitTscaleNbins_;
  hpars.miny   = recHitTscaleMinNs_;
  hpars.maxy   = recHitTscaleMaxNs_;

  v_hpars.push_back(hpars);

  st_shTimingVsE_ = "h_shTimingVsE" + runstrn;
  hpars.name   = st_shTimingVsE_;
  hpars.title  = "Hcal SimHit Timing vs. Energy " + runstrt + "; Simhit Energy * 200; Simhit Time (ns)";
  hpars.nbinsx = (uint32_t)(recHitEscaleMaxGeV_) + 1;
  hpars.minx   =  -0.5;
  hpars.maxx   = recHitEscaleMaxGeV_;
  hpars.nbinsy = simHitTscaleNbins_;
  hpars.miny   = simHitTscaleMinNs_;
  hpars.maxy   = simHitTscaleMaxNs_;

  v_hpars.push_back(hpars);

  st_ctTimingVsE_ = "h_ctTimingVsE" + runstrn;
  hpars.name   = st_ctTimingVsE_;
  hpars.title  = "Calo Tower Timing vs. Energy " + runstrt + "; Tower Energy (GeV); Tower Time (ns)";
  hpars.nbinsx =  100;
  hpars.minx   = recHitEscaleMinGeV_;
  hpars.maxx   = 5*recHitEscaleMaxGeV_;
  hpars.nbinsy = recHitTscaleNbins_;
  hpars.miny   = recHitTscaleMinNs_;
  hpars.maxy   = recHitTscaleMaxNs_;

  v_hpars.push_back(hpars);

  cutNone_->histos()->book(v_hpars);
  cutMaxHitOnly_->histos()->book(v_hpars);
  cutTgtTwrOnly_->histos()->book(v_hpars);
  cutMinHitGeV_->histos()->book(v_hpars);
  cutAll_->histos()->book(v_hpars);

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

  HBHERecHit maxrh(HcalDetId::Undefined,-1e99,0.0);

  for (unsigned irh = 0; irh < ed.hbherechits()->size (); ++irh) {
    const HBHERecHit& rh = (*(ed.hbherechits()))[irh];
    HcalDetId detId = rh.id();
    double htime    = rh.time();
    double energy   = rh.energy();
    TH1F *hp = 0;
    if (detId.subdet() == HcalBarrel) hp = (detId.zside() > 0) ? h_hbpt : h_hbmt;
    if (detId.subdet() == HcalEndcap) hp = (detId.zside() > 0) ? h_hept : h_hemt;

    if (hp) hp->Fill(htime);

    cutNone_->histos()->fill1d(st_rhTimes_,htime);
    cutNone_->histos()->fill1d(st_rhEnergies_,energy);
    cutNone_->histos()->fill2d(st_rhEmap_,rh.id().ieta(),rh.id().iphi(),energy);
    cutNone_->histos()->fill2d(st_rhTimingVsE_,energy,htime);

    double minHitGeV = lookupThresh(rh.id());

    if (energy > maxrh.energy()) maxrh = rh;

    if (detId == tgtTwrId_) {
      cutTgtTwrOnly_->histos()->fill1d(st_rhTimes_,htime);
      cutTgtTwrOnly_->histos()->fill1d(st_rhEnergies_,energy);
      cutTgtTwrOnly_->histos()->fill2d(st_rhEmap_,rh.id().ieta(),rh.id().iphi(),energy);
      cutTgtTwrOnly_->histos()->fill2d(st_rhTimingVsE_,energy,htime);
    }

    if (energy > minHitGeV) {
      s_idsOverThresh.insert(rh.id());

      cutMinHitGeV_->histos()->fill1d(st_rhTimes_,htime);
      cutMinHitGeV_->histos()->fill1d(st_rhEnergies_,energy);
      cutMinHitGeV_->histos()->fill2d(st_rhEmap_,rh.id().ieta(),rh.id().iphi(),energy);
      cutMinHitGeV_->histos()->fill2d(st_rhTimingVsE_,energy,htime);
    }
  } // loop over rechits

  if (maxrh.id() != HcalDetId::Undefined) {
    double htime    = maxrh.time();
    double energy   = maxrh.energy();
    HcalDetId maxId = maxrh.id();

    cutMaxHitOnly_->histos()->fill1d(st_rhTimes_,htime);
    cutMaxHitOnly_->histos()->fill1d(st_rhEnergies_,energy);
    cutMaxHitOnly_->histos()->fill2d(st_rhEmap_,maxId.ieta(),maxId.iphi(),energy);
    cutMaxHitOnly_->histos()->fill2d(st_rhTimingVsE_,energy,htime);

    if ((energy > lookupThresh(maxId)) &&
	(maxId == tgtTwrId_)) {
      cutAll_->histos()->fill1d(st_rhTimes_,htime);
      cutAll_->histos()->fill1d(st_rhEnergies_,energy);
      cutAll_->histos()->fill2d(st_rhEmap_,maxId.ieta(),maxId.iphi(),energy);
      cutAll_->histos()->fill2d(st_rhTimingVsE_,energy,htime);
    }
  }

  for (unsigned irh = 0; irh < ed.hfrechits()->size (); ++irh) {
    const HFRecHit& rh = (*(ed.hfrechits()))[irh];
    HcalDetId detId = rh.id();

    double minHitGeV = lookupThresh(rh.id());

    if (rh.energy() > minHitGeV) {
      cutMinHitGeV_->histos()->fill2d(st_rhEmap_,rh.id().ieta(),rh.id().iphi()+rh.id().depth()-1,rh.energy());
    }

  } // loop over HF rechits

  if (ed.simhits().isValid()) {
    std::vector<PCaloHit>  simhits;
    simhits.insert(simhits.end(),ed.simhits()->begin(),ed.simhits()->end());

    for (unsigned ish = 0; ish < simhits.size (); ++ish) {
      const PCaloHit& sh = simhits[ish];

      double htime    = sh.time();
      double energy   = sh.energy() * 200;

      if (energy > simHitEnergyMinGeVthreshold_) {
	cutNone_->histos()->fill1d(st_shTimes_,htime);
	cutNone_->histos()->fill1d(st_shEnergies_,energy);
	cutNone_->histos()->fill2d(st_shTimingVsE_,energy,htime);
      }
      
    } // loop over simhits
  }

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

	cutNone_->histos()->fill1d(st_avgPulse_,isample,rawadc);

	if (inSet<HcalDetId>(s_idsOverThresh,detId))
	  cutMinHitGeV_->histos()->fill1d(st_avgPulse_,isample,rawadc);

      } // loop over samples in digi
    } // loop over digi
  } // if have digis


  if (ed.towers().isValid()) {
    for (unsigned itwr = 0; itwr < ed.towers()->size (); ++itwr) {
      const CaloTower& twr = (*(ed.towers()))[itwr];
      cutMinHitGeV_->histos()->fill2d(st_ctTimingVsE_,twr.hadEnergy(),twr.hcalTime());

    } // loop over towers
  } // if have towers

  // MET
  if (ed.recmet().isValid()) {
    const CaloMET *calomet = &(ed.recmet().product()->front());
    cutMinHitGeV_->histos()->fill1d(st_caloMet_Met_,calomet->pt());
    cutMinHitGeV_->histos()->fill1d(st_caloMet_Phi_,calomet->phi());
    cutMinHitGeV_->histos()->fill1d(st_caloMet_SumEt_,calomet->sumEt());
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

