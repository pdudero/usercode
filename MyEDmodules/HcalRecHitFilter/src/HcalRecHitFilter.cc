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
// $Id$
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
      
  // ----------member data ---------------------------

  double                   minRecoTimeNs_;
  double                   maxRecoTimeNs_;
  int                      subdet_;
  int                      subdetOther_;
  std::vector<HcalDetId>   detIds2mask_;
};

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
HcalRecHitFilter::HcalRecHitFilter(const edm::ParameterSet& iConfig) :
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
}


//======================================================================

bool HcalRecHitFilter::maskedId(const HcalDetId& id)
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

// ------------ method called to produce the data  ------------
void
HcalRecHitFilter::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  if (subdet_ == HcalBarrel || subdet_ == HcalEndcap) {
    Handle<HBHERecHitCollection> hbhein;
    if (iEvent.getByLabel("hbhereco",hbhein)) {
      std::auto_ptr<HBHERecHitCollection> hbheout(new HBHERecHitCollection);

      for (uint32_t i=0; i < hbhein->size(); i++) {
	const HBHERecHit& rh= (*hbhein)[i];
	if (!maskedId(rh.id()) &&
	    inTime(rh.time()))
	  hbheout->push_back(rh);
      }
      iEvent.put(hbheout);
    }
  }
  else if (subdet_ == HcalOuter) {
    Handle<HORecHitCollection> hoin;
    if (iEvent.getByLabel("horeco",hoin)) {
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
    if (iEvent.getByLabel("hfreco",hfin)) {
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
