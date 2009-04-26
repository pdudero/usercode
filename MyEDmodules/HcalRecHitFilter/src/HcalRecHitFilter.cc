// -*- C++ -*-
//
// Package:    HcalRecHitFilter
// Class:      HcalRecHitFilter
// 
/**\class HcalRecHitFilter HcalRecHitFilter.cc MyEDmodules/HcalRecHitFilter/src/HcalRecHitFilter.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Dudero
//         Created:  Mon Mar  2 02:37:12 CST 2009
// $Id: HcalRecHitFilter.cc,v 1.1 2009/04/19 13:50:51 dudero Exp $
//
//


// system include files
#include <memory>
#include <vector>
#include <string>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"

#include "TRandom3.h"

//
// class declaration
//

class HcalRecHitFilter : public edm::EDProducer {
public:
  explicit HcalRecHitFilter(const edm::ParameterSet&);
  ~HcalRecHitFilter();

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
 
  bool convertIdNumbers  (std::vector<int>& v_maskidnumbers,
                          std::vector<HcalDetId>& detIds2mask);

  bool maskedId          (const HcalDetId& id);


  inline bool inTime(double hittime) {
    return ((hittime >= minRecoTimeNs_) &&
	    (hittime <= maxRecoTimeNs_)   );
  }
  double smearedTime(const HBHERecHit& unsmearedRH) const;
  void   dumpEnvelope(void) const;
      
  // ----------member data ---------------------------

  edm::InputTag            hbherecotag_;
  edm::InputTag            hfrecotag_;
  edm::InputTag            horecotag_;
  double                   minRecoTimeNs_;
  double                   maxRecoTimeNs_;
  int                      subdet_;
  int                      subdetOther_;
  std::vector<HcalDetId>   detIds2mask_;

  std::vector<std::pair<double,double> >  smearEnvelope_;

  TRandom3                *rand_;
};

//
// constants, enums and typedefs
//

template <class T>
class comparePair1 {
public:
  bool operator()(const T& h1,
                  const T& h2) const {
    return (h1.first < h2.first);
  }
};


//
// static data member definitions
//

//
// constructors and destructor
//
HcalRecHitFilter::HcalRecHitFilter(const edm::ParameterSet& iConfig) :
  hbherecotag_(iConfig.getUntrackedParameter<edm::InputTag>("hbheLabel",(edm::InputTag)"")),
  hfrecotag_(iConfig.getUntrackedParameter<edm::InputTag>("hfLabel",(edm::InputTag)"")),
  horecotag_(iConfig.getUntrackedParameter<edm::InputTag>("hoLabel",(edm::InputTag)"")),
  minRecoTimeNs_(iConfig.getParameter<double>("minRecoTimeNs")),
  maxRecoTimeNs_(iConfig.getParameter<double>("maxRecoTimeNs"))
{
  //register your products

  std::string subd=iConfig.getParameter<std::string>("Subdetector");
  if (!strcasecmp(subd.c_str(),"HBHE")) {
    subdet_=HcalBarrel;
    produces<HBHERecHitCollection>();
  } else if (!strcasecmp(subd.c_str(),"HO")) {
    subdet_=HcalOuter;
    produces<HORecHitCollection>();
  } else if (!strcasecmp(subd.c_str(),"HF")) {
    subdet_=HcalForward;
    produces<HFRecHitCollection>();
  } else if (!strcasecmp(subd.c_str(),"CALIB")) {
    subdet_=HcalOther;
    subdetOther_=HcalCalibration;
    produces<HcalCalibRecHitCollection>();
  } else {
    std::cout << "HcalRecHitFilter is not associated with a specific subdetector!" << std::endl;
  }       

  //now do what ever other initialization is needed

  std::vector<int> v_maskidnumbers;
  v_maskidnumbers  = iConfig.getParameter<std::vector<int> > ("detIds2Mask");
  if (!convertIdNumbers(v_maskidnumbers, detIds2mask_))
    throw cms::Exception("Invalid detID vector");
  
  for (uint32_t i=0; i<detIds2mask_.size(); i++)
    std::cout << "Masking " << detIds2mask_[i] << std::endl;

  std::vector<double> v_smearEnv;

  v_smearEnv  = iConfig.getParameter<std::vector<double> > ("smearEnvelope");

  if (v_smearEnv.size()) {
    if (v_smearEnv.size() & 1)
      throw cms::Exception("Invalid smearEnvelope vector");
  
    rand_ = new TRandom3();

    for (uint32_t i = 0; i<v_smearEnv.size(); i+=2) {
      double energy     = v_smearEnv[i];
      double smearsigma = v_smearEnv[i+1];
      if (smearsigma <= 0.0)
	throw cms::Exception("nonpositive smearEnvelope sigma encountered");
    
      smearEnvelope_.push_back(std::pair<double,double>(energy,smearsigma));
    }

    //sort in order of increasing energy
    std::sort(smearEnvelope_.begin(),
	      smearEnvelope_.end(),
	      comparePair1<std::pair<double,double> >());

    dumpEnvelope();
  }
}

//======================================================================

void
HcalRecHitFilter::dumpEnvelope(void) const
{
  std::cout << "Smear Envelope:" << std::endl;
  std::cout << "GeV\tSigma" << std::endl;
  for (uint32_t i=0; i<smearEnvelope_.size(); i++) {
    std::cout << smearEnvelope_[i].first << "\t" << smearEnvelope_[i].second << std::endl;
  }
}

//======================================================================

bool
HcalRecHitFilter::maskedId(const HcalDetId& id)
{
  for (uint32_t i=0; i<detIds2mask_.size(); i++)
    if (id == detIds2mask_[i]) return true;
  return false;
}

HcalRecHitFilter::~HcalRecHitFilter()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//
//======================================================================

bool
HcalRecHitFilter::convertIdNumbers(std::vector<int>& v_maskidnumbers,
                                  std::vector<HcalDetId>& detIds2mask)
{
  // convert det ID numbers to valid detIds:
  if (v_maskidnumbers.size()%3) {
    return false;
  }
  for (uint32_t i=0; i<v_maskidnumbers.size(); i+=3) {
    int ieta  = v_maskidnumbers[i];
    int iphi  = v_maskidnumbers[i+1];
    int depth = v_maskidnumbers[i+2];
    if ((abs(ieta) <  1) || (abs(ieta) > 41)) return false;
    if (    (iphi  <  0) ||    (iphi   > 71)) return false;
    if (   (depth  <  1) &&    (depth  >  4)) return false;
    HcalDetId id(HcalForward,ieta,iphi,depth);
    detIds2mask.push_back(id);
  }
  return true;
}                             // HcalRecHitFilter::convertIdNumbers

//======================================================================

double
HcalRecHitFilter::smearedTime(const HBHERecHit& unsmearedRH) const
{
  double unsmearedTime = unsmearedRH.time();
  double energy        = unsmearedRH.energy();
  double smearsigma;
  uint32_t i=0;

  for (i=0; i<smearEnvelope_.size(); i++)
    if (smearEnvelope_[i].first > energy)
	break;

  // Smearing starts at the first energy listed in the envelope.
  if (!i) return unsmearedTime;
  // No interpolation after the last energy in the envelope!
  else if (i == smearEnvelope_.size())
    smearsigma = smearEnvelope_[i-1].second;
  else {
    double energy1 = smearEnvelope_[i-1].first;
    double sigma1  = smearEnvelope_[i-1].second;
    double energy2 = smearEnvelope_[i].first;
    double sigma2  = smearEnvelope_[i].second;

    smearsigma = sigma1 + ((sigma2-sigma1)*(energy-energy1)/(energy2-energy1));

    //char s[80];
    //sprintf (s,"(%6.1f,%6.3f) (%6.1f,%6.3f) %6.1f %6.3f\n",energy1,sigma1,energy2,sigma2,energy,smearsigma);
    //std::cout << s;
  }

  double smearedTime = unsmearedTime + rand_->Gaus(0.0,smearsigma);

  //std::cout << "Before: " << unsmearedTime << "; After: " << smearedTime << std::endl;

  return (smearedTime);
}

//======================================================================

// ------------ method called to produce the data  ------------
void
HcalRecHitFilter::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  bool doSmear = (smearEnvelope_.size() > 0);

  if (subdet_ == HcalBarrel || subdet_ == HcalEndcap) {
    Handle<HBHERecHitCollection> hbhein;
    if (iEvent.getByLabel(hbherecotag_,hbhein)) {
      std::auto_ptr<HBHERecHitCollection> hbheout(new HBHERecHitCollection);

      for (uint32_t i=0; i < hbhein->size(); i++) {
	const HBHERecHit& rh= (*hbhein)[i];
	double rhtime = rh.time();

	if (doSmear)
	  rhtime = smearedTime(rh);

	if (!maskedId(rh.id()) &&
	    inTime(rhtime))
	  hbheout->push_back(HBHERecHit(rh.id(),rh.energy(),rhtime));
      }
      iEvent.put(hbheout);
    }
  }
  else if (subdet_ == HcalOuter) {
    Handle<HORecHitCollection> hoin;
    if (iEvent.getByLabel(horecotag_,hoin)) {
      std::auto_ptr<HORecHitCollection> hoout(new HORecHitCollection);

      for (uint32_t i=0; i < hoin->size(); i++) {
	const HORecHit& rh= (*hoin)[i];
	if (!maskedId(rh.id()) &&
	    inTime(rh.time()))
	  hoout->push_back(rh);
      }
      iEvent.put(hoout);
    }
  }
  else if (subdet_ == HcalForward) {
    Handle<HFRecHitCollection> hfin;
    if (iEvent.getByLabel(hfrecotag_,hfin)) {
      std::auto_ptr<HFRecHitCollection> hfout(new HFRecHitCollection);

      for (uint32_t i=0; i < hfin->size(); i++) {
	const HFRecHit& rh= (*hfin)[i];
	if (!maskedId(rh.id()) &&
	    inTime(rh.time()))
	  hfout->push_back(rh);
      }
      iEvent.put(hfout);
    }
  }
}


// ------------ method called once each job just before starting event loop  ------------
void 
HcalRecHitFilter::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalRecHitFilter::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalRecHitFilter);
