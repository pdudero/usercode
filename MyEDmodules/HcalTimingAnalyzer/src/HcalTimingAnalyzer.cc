// -*- C++ -*-
//
// Package:    HcalTimingAnalyzer
// Class:      HcalTimingAnalyzer
// 
/**\class HcalTimingAnalyzer HcalTimingAnalyzer.cc MyEDmodules/HcalTimingAnalyzer/src/HcalTimingAnalyzer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalTimingAnalyzer.cc,v 1.1 2009/03/17 08:52:18 dudero Exp $
//
//


// system include files
#include <memory>
#include <vector>
#include <set>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "PhysicsTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"
#include "DataFormats/CaloTowers/interface/CaloTowerCollection.h"

#include "MyEDmodules/HcalTimingAnalyzer/src/inSet.hh"

#include "TH1F.h"
#include "TH2F.h"

//
// class declaration
//


class HcalTimingAnalyzer : public edm::EDAnalyzer {
public:
  explicit HcalTimingAnalyzer(const edm::ParameterSet&);
  ~HcalTimingAnalyzer() {}

private:
  void bookPerRunHistos(const uint32_t runnum);
  TH1F *book1d(TFileDirectory & fDir,
	       const uint32_t runnum,
	       const std::string & fName,
	       const std::string & fTitle,
	       int fNbins, double fXmin, double fXmax) const;
  TH2F *book2d(TFileDirectory & fDir,
	       const uint32_t runnum,
	       const std::string & fName,
	       const std::string & fTitle,
	       int fNbinsX, double fXmin, double fXmax,
	       int fNbinsY, double fYmin, double fYmax) const;
  double lookupThresh(const HcalDetId& id);
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  // ----------member data ---------------------------
  edm::InputTag      hbheDigiLabel_;
  edm::InputTag      hbheRechitLabel_;
  edm::InputTag      hfRechitLabel_;
  edm::InputTag      metLabel_;
  edm::InputTag      twrLabel_;
  double             minHitGeVHB_;
  double             minHitGeVHE_;
  double             minHitGeVHO_;
  double             minHitGeVHF1_;
  double             minHitGeVHF2_;
  double             recHitEscaleMinGeV_;
  double             recHitEscaleMaxGeV_;
  std::set<uint32_t> s_events2anal_;
  std::set<uint32_t> s_runs_;

  TH1F *h_avgPulseAll_, *h_rhTimesAll_, *h_rhEnergiesAll_;
  TH1F *h_avgPulseFlt_, *h_rhTimesFlt_, *h_rhEnergiesFlt_;
  TH1F *h_caloMet_Met_, *h_caloMet_Phi_, *h_caloMet_SumEt_;

  TH2F *h2f_rhEmap_;
  TH2F *h2f_rhTimingVsE_;
  TH2F *h2f_ctTimingVsE_;
};

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
HcalTimingAnalyzer::HcalTimingAnalyzer(const edm::ParameterSet& iConfig) :
  hbheDigiLabel_(iConfig.getUntrackedParameter<edm::InputTag>("hbheDigiLabel")),
  hbheRechitLabel_(iConfig.getUntrackedParameter<edm::InputTag>("hbheRechitLabel")),
  hfRechitLabel_(iConfig.getUntrackedParameter<edm::InputTag>("hfRechitLabel")),
  metLabel_(iConfig.getUntrackedParameter<edm::InputTag>("metLabel")),
  twrLabel_(iConfig.getUntrackedParameter<edm::InputTag>("twrLabel")),
  minHitGeVHB_(iConfig.getParameter<double>("minHitGeVHB")),
  minHitGeVHE_(iConfig.getParameter<double>("minHitGeVHE")),
  minHitGeVHO_(iConfig.getParameter<double>("minHitGeVHO")),
  minHitGeVHF1_(iConfig.getParameter<double>("minHitGeVHF1")),
  minHitGeVHF2_(iConfig.getParameter<double>("minHitGeVHF2")),
  recHitEscaleMinGeV_(iConfig.getParameter<double>("recHitEscaleMinGeV")),
  recHitEscaleMaxGeV_(iConfig.getParameter<double>("recHitEscaleMaxGeV"))
{
  //now do what ever initialization is needed
  std::vector<int> v_events2anal(iConfig.getParameter<std::vector<int> >("eventNumbers"));

  for (unsigned i=0; i<v_events2anal.size(); i++) {
    s_events2anal_.insert(v_events2anal[i]);
  }
}


//======================================================================

//
// member functions
//
TH1F *HcalTimingAnalyzer::book1d(TFileDirectory & fDir,
			   const uint32_t runnum,
			   const std::string & fName,
			   const std::string & fTitle,
			   int fNbins, double fXmin, double fXmax) const {
  char title[1024];

  sprintf(title, "%s [RUN:%d]", fTitle.c_str(), runnum);
  std::cout << "booking 1d histogram " << title << std::endl;
  return fDir.make <TH1F> (fName.c_str(), title, fNbins, fXmin, fXmax);
}
//======================================================================

TH2F *HcalTimingAnalyzer::book2d(TFileDirectory & fDir,
			   const uint32_t runnum,
			   const std::string & fName,
			   const std::string & fTitle,
			   int fNbinsX, double fXmin, double fXmax,
			   int fNbinsY, double fYmin, double fYmax) const {
  char title[1024];

  sprintf(title, "%s [RUN:%d]", fTitle.c_str(), runnum);
  std::cout << "booking 2d histogram " << title << std::endl;
  return fDir.make <TH2F> (fName.c_str(),title,fNbinsX,fXmin,fXmax,fNbinsY,fYmin,fYmax);
}
//======================================================================

void HcalTimingAnalyzer::bookPerRunHistos(const uint32_t rn)
{
  edm::Service<TFileService> fs;

  TFileDirectory dir = fs->mkdir("Hcal");

  h_avgPulseAll_   = book1d(dir,rn,"pulse","HBHE Average Pulse Shape", 10,-0.5,9.5 );
  h_rhTimesAll_    = book1d(dir,rn,"RHTimes","HBHE RecHit Times",101,-100.5,100.5);
  h_rhEnergiesAll_ = book1d(dir,rn,"RHEnergies","HBHE RecHit Energies",
				     100,recHitEscaleMinGeV_,recHitEscaleMaxGeV_);

  h_avgPulseFlt_   = book1d(dir,rn,"pulseFlt","HBHE Average Pulse Shape, E>thresh", 10,-0.5,9.5 );
  h_rhTimesFlt_    = book1d(dir,rn,"RHTimesFlt","HBHE RecHit Times, E>thresh",101,-100.5,100.5);
  h_rhEnergiesFlt_ = book1d(dir,rn,"RHEnergiesFlt","HBHE RecHit Energies Above Threshold",
				     100,recHitEscaleMinGeV_,recHitEscaleMaxGeV_);

  h_caloMet_Met_   = book1d(dir,rn,"h_caloMet_Met", "MET from CaloTowers",100,0,recHitEscaleMaxGeV_);
  h_caloMet_Phi_   = book1d(dir,rn,"h_caloMet_Phi", "MET #phi from CaloTowers",100, -4, 4);
  h_caloMet_SumEt_ = book1d(dir,rn,"h_caloMet_SumEt", "SumET from CaloTowers",
				     100,recHitEscaleMinGeV_,recHitEscaleMaxGeV_);

  h2f_rhEmap_      = book2d(dir,rn,"h_rhEperIetaIphi", "HBHE RecHit Energy Map",
			    83, -41.5, 41.5,
			    72, 0.5,72.5);

  h2f_rhTimingVsE_ = book2d(dir,rn,"h_rhTimingVsE", "HBHE RecHit Timing vs. Energy",
			    100,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
			    101,-100.5,100.5);
  h2f_ctTimingVsE_ = book2d(dir,rn,"h_ctTimingVsE", "Calo Tower Timing vs. Energy",
			    100,recHitEscaleMinGeV_,5*recHitEscaleMaxGeV_,
			    101,-100.5,100.5);
}
//======================================================================

double HcalTimingAnalyzer::lookupThresh(const HcalDetId& id)
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
HcalTimingAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  bool havedigis  = true;
  bool havetowers = true;

  edm::EventID eventId = iEvent.id();
  uint32_t runn   = eventId.run ();
  uint32_t evtn = eventId.event ();

  if (notInSet<uint32_t>(s_runs_,runn)) {
    bookPerRunHistos(runn);
    s_runs_.insert(runn);
  }

  edm::Handle<HBHEDigiCollection> hbhedigis;
  if (!iEvent.getByLabel(hbheDigiLabel_,hbhedigis)) {
    edm::LogWarning("HcalTimingAnalyzer::analyze") <<
      "Digis not found"<< std::endl;
    havedigis = false;
  }

  edm::Handle<HBHERecHitCollection> hbherechits;
  if (!iEvent.getByLabel(hbheRechitLabel_,hbherechits)) {
    edm::LogWarning("HcalTimingAnalyzer::analyze") <<
      "Rechits not found"<< std::endl;
    return;
  }

  edm::Handle<HFRecHitCollection> hfrechits;
  if (!iEvent.getByLabel(hfRechitLabel_,hfrechits)) {
    edm::LogWarning("HcalTimingAnalyzer::analyze") <<
      "Rechits not found"<< std::endl;
    return;
  }

  // CaloTowers
  edm::Handle<CaloTowersCollection> towers;
  if (!iEvent.getByLabel(twrLabel_,towers)) {
    edm::LogWarning("HcalTimingAnalyzer::analyze") <<
      "Calo Towers not found"<< std::endl;
    havetowers = false;
  }

  // MET
  const CaloMET *calomet;
  edm::Handle < CaloMETCollection > recmet;
  iEvent.getByLabel(metLabel_, recmet);
  const CaloMETCollection *metCol = recmet.product();
  calomet = &(metCol->front());

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

  for (unsigned irh = 0; irh < hbherechits->size (); ++irh) {
    const HBHERecHit& rh = (*hbherechits)[irh];
    HcalDetId detId = rh.id();
    double time     = rh.time();
    double energy   = rh.energy();
    TH1F *hp = 0;
    if (detId.subdet() == HcalBarrel) hp = (detId.zside() > 0) ? h_hbpt : h_hbmt;
    if (detId.subdet() == HcalEndcap) hp = (detId.zside() > 0) ? h_hept : h_hemt;

    if (hp) hp->Fill(time);

    h_rhTimesAll_->Fill(time);
    h_rhEnergiesAll_->Fill(energy);

    double minHitGeV = lookupThresh(rh.id());

    if (energy > minHitGeV) {
      s_idsOverThresh.insert(rh.id());
      h_rhTimesFlt_->Fill(time);
      h_rhEnergiesFlt_->Fill(energy);
      h2f_rhEmap_->Fill(rh.id().ieta(), rh.id().iphi(),energy);

      h2f_rhTimingVsE_->Fill(energy,time);
    }

  } // loop over rechits

  for (unsigned irh = 0; irh < hfrechits->size (); ++irh) {
    const HFRecHit& rh = (*hfrechits)[irh];
    HcalDetId detId = rh.id();

    double minHitGeV = lookupThresh(rh.id());

    if (rh.energy() > minHitGeV) {
      h2f_rhEmap_->Fill(rh.id().ieta(),rh.id().iphi()+rh.id().depth()-1,rh.energy());
    }

  } // loop over HF rechits

  if (havedigis) {
    for (unsigned idig = 0; idig < hbhedigis->size (); ++idig) {
      const HBHEDataFrame& frame = (*hbhedigis)[idig];
      HcalDetId detId = frame.id();
      TH1F *hp = 0;
      if (detId.subdet() == HcalBarrel) hp = (detId.zside() > 0) ? h_hbp : h_hbm; 
      if (detId.subdet() == HcalEndcap) hp = (detId.zside() > 0) ? h_hep : h_hem; 
      
      for (int isample = 0; isample < std::min(10,frame.size()); ++isample) {
	int rawadc = frame[isample].adc();
	if (hp) hp->Fill(isample,rawadc);

	h_avgPulseAll_->Fill(isample,rawadc);

	if (inSet<HcalDetId>(s_idsOverThresh,detId))
	  h_avgPulseFlt_->Fill(isample,rawadc);

      } // loop over samples in digi
    } // loop over digi
  } // if have digis


  if (havetowers) {
    for (unsigned itwr = 0; itwr < towers->size (); ++itwr) {
      const CaloTower& twr = (*towers)[itwr];
      h2f_ctTimingVsE_->Fill(twr->hadEnergy(),towr->hcalTime());

    } // loop over towers
  } // if have towers

  // MET
  h_caloMet_Met_->Fill(calomet->pt());
  h_caloMet_Phi_->Fill(calomet->phi());
  h_caloMet_SumEt_->Fill(calomet->sumEt());


}


// ------------ method called once each job just before starting event loop  ------------
void 
HcalTimingAnalyzer::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalTimingAnalyzer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalTimingAnalyzer);
