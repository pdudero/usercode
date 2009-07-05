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
// $Id: HcalRecHitFilter.cc,v 1.2 2009/04/26 23:31:02 dudero Exp $
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
  double                   timeShiftNs_; /* time to shift rechit times by
					    before applying filter window */

  std::vector<std::pair<double,double> >  tsmearEnvelope_;
  std::vector<std::pair<double,double> >  tfilterEnvelope_;

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
  maxRecoTimeNs_(iConfig.getParameter<double>("maxRecoTimeNs")),
  timeShiftNs_(iConfig.getParameter<double>("timeShiftNs"))
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

  std::vector<double> v_tsmearEnv;

  v_tsmearEnv  = iConfig.getParameter<std::vector<double> > ("smearEnvelope");

  if (v_tsmearEnv.size()) {
    if (v_tsmearEnv.size() & 1)
      throw cms::Exception("Invalid tsmearEnvelope vector");
  
    rand_ = new TRandom3();

    for (uint32_t i = 0; i<v_tsmearEnv.size(); i+=2) {
      double energy     = v_tsmearEnv[i];
      double tsmearsigma = v_tsmearEnv[i+1];
      if (tsmearsigma <= 0.0)
	throw cms::Exception("nonpositive tsmearEnvelope sigma encountered");
    
      tsmearEnvelope_.push_back(std::pair<double,double>(energy,tsmearsigma));
    }

    //sort in order of increasing energy
    std::sort(tsmearEnvelope_.begin(),
	      tsmearEnvelope_.end(),
	      comparePair1<std::pair<double,double> >());

    dumpEnvelope();
  }
}                                  // HcalRecHitFilter::HcalRecHitFilter

//======================================================================

void
HcalRecHitFilter::dumpEnvelope(void) const
{
  std::cout << "Smear Envelope:" << std::endl;
  std::cout << "GeV\tSigma" << std::endl;
  for (uint32_t i=0; i<tsmearEnvelope_.size(); i++) {
    std::cout << tsmearEnvelope_[i].first << "\t" << tsmearEnvelope_[i].second << std::endl;
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
}                                  // HcalRecHitFilter::convertIdNumbers

//======================================================================

double
HcalRecHitFilter::smearedTime(const HBHERecHit& unsmearedRH) const
{
  double unsmearedTime = unsmearedRH.time();
  double energy        = unsmearedRH.energy();
  double tsmearsigma;
  uint32_t i=0;

  for (i=0; i<tsmearEnvelope_.size(); i++)
    if (tsmearEnvelope_[i].first > energy)
	break;

  // Smearing starts at the first energy listed in the envelope.
  if (!i) return unsmearedTime;
  // No interpolation after the last energy in the envelope!
  else if (i == tsmearEnvelope_.size())
    tsmearsigma    = tsmearEnvelope_[i-1].second;
  else {
    double energy1 = tsmearEnvelope_[i-1].first;
    double sigma1  = tsmearEnvelope_[i-1].second;
    double energy2 = tsmearEnvelope_[i].first;
    double sigma2  = tsmearEnvelope_[i].second;

    tsmearsigma = sigma1 + ((sigma2-sigma1)*(energy-energy1)/(energy2-energy1));

    //char s[80];
    //sprintf (s,"(%6.1f,%6.3f) (%6.1f,%6.3f) %6.1f %6.3f\n",energy1,sigma1,energy2,sigma2,energy,tsmearsigma);
    //std::cout << s;
  }

  double smearedTime = unsmearedTime + rand_->Gaus(0.0,tsmearsigma);

  //std::cout << "Before: " << unsmearedTime << "; After: " << smearedTime << std::endl;

  return (smearedTime);
}                                       // HcalRecHitFilter::smearedTime

//======================================================================

// ------------ method called to produce the data  ------------
void
HcalRecHitFilter::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  bool doSmear = (tsmearEnvelope_.size() > 0);

  if (subdet_ == HcalBarrel || subdet_ == HcalEndcap) {
    Handle<HBHERecHitCollection> hbhein;
    if (iEvent.getByLabel(hbherecotag_,hbhein)) {
      std::auto_ptr<HBHERecHitCollection> hbheout(new HBHERecHitCollection);

      for (uint32_t i=0; i < hbhein->size(); i++) {
	const HBHERecHit& rh= (*hbhein)[i];
	double rhtime = rh.time() - timeShiftNs_;

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
	double rhtime = rh.time() - timeShiftNs_;
	if (!maskedId(rh.id()) &&
	    inTime(rhtime))
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
	double rhtime = rh.time() - timeShiftNs_;
	if (!maskedId(rh.id()) &&
	    inTime(rhtime))
	  hfout->push_back(rh);
      }
      iEvent.put(hfout);
    }
  }
}                                        // HcalRecHitFilter::produce


// ------------ method called once each job just before starting event loop  ------------
void 
HcalRecHitFilter::beginJob(const edm::EventSetup&)
{
  //  edm::LogInfo("Parameters being used: ")   <<
  std::cout << "-----------------------"  << "\n" <<
  std::cout << "Parameters being used: "  << "\n" <<
    "hbherecotag_   : " << hbherecotag_   << "\n" <<
    "hfrecotag_     : " << hfrecotag_     << "\n" <<
    "horecotag_     : " << horecotag_     << "\n" <<
    "minRecoTimeNs_ = " << minRecoTimeNs_ << "\n" <<
    "maxRecoTimeNs_ = " << maxRecoTimeNs_ << "\n" <<
    "subdet_        : " << subdet_        << "\n" <<
    "timeShiftNs_   = " << timeShiftNs_   << "\n";
  
  for (uint32_t i=0; i<detIds2mask_.size(); i++)
    //edm::LogInfo("Masking ") << detIds2mask_[i] << std::endl;
    std::cout << "Masking " << detIds2mask_[i] << std::endl;
  std::cout << "-----------------------"  << std::endl;
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalRecHitFilter::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalRecHitFilter);
