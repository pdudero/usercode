// -*- C++ -*-
//
// Package:    HcalSmearedTimeRecHitProducer
// Class:      HcalSmearedTimeRecHitProducer
// 
/**\class HcalSmearedTimeRecHitProducer HcalSmearedTimeRecHitProducer.cc MyEDmodules/HcalSmearedTimeRecHitProducer/src/HcalSmearedTimeRecHitProducer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell Dudero
//         Created:  Thu May 14 21:27:19 CEST 2009
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"

#include "TRandom3.h"

//
// class declaration
//

class HcalSmearedTimeRecHitProducer : public edm::EDProducer {
public:
  explicit HcalSmearedTimeRecHitProducer(const edm::ParameterSet&);
  ~HcalSmearedTimeRecHitProducer() {}

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  double smearedTime(const CaloRecHit *rh) const;
  void   dumpEnvelope(void) const;

  template <class Hit,class Col>
  void   processHits(edm::Event& iEvent) const;
      
      // ----------member data ---------------------------
  bool                     enableGuruParameters_;
  edm::InputTag            recHitLabel_;
  int                      subdet_;
  int                      subdetOther_;

  std::vector<std::pair<double,double> >  smearEnvelope_;

  TRandom3                *rand_;
};

template <class T>
class comparePair1 {
public:
  bool operator()(const T& h1,
                  const T& h2) const {
    return (h1.first < h2.first);
  }
};

//
// constructors and destructor
//
HcalSmearedTimeRecHitProducer::HcalSmearedTimeRecHitProducer(const edm::ParameterSet& iConfig) :
  enableGuruParameters_(iConfig.getParameter<bool>("enableGuruParameters")),
  recHitLabel_(iConfig.getParameter<edm::InputTag>("recHitLabel"))
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
#if 0
    // smearing N/A for Calib rechits.
  } else if (!strcasecmp(subd.c_str(),"CALIB")) {
    subdet_=HcalOther;
    subdetOther_=HcalCalibration;
    produces<HcalCalibRecHitCollection>();
#endif
  } else if (!strcasecmp(subd.c_str(),"ZDC")) {
    subdet_=HcalZDCDetId::SubdetectorId;
    produces<ZDCRecHitCollection>();
  } else {
    throw cms::Exception("Invalid HCAL subdetector: ") << subd << std::endl;
  }       

  //now do what ever other initialization is needed

  if (enableGuruParameters_) {
    std::vector<double> v_smearEnv =
      iConfig.getParameter<std::vector<double> > ("smearEnvelope");

    if (!v_smearEnv.size())
      throw cms::Exception("smearEnvelope is empty");

    if (v_smearEnv.size() & 1)
      throw cms::Exception("Invalid smearEnvelope vector");

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

    //dumpEnvelope();
  } else if (subdet_=HcalBarrel) {
    // default smear Envelope:

    smearEnvelope_.push_back(std::pair<double,double> ( 4.00, 1.700));
    smearEnvelope_.push_back(std::pair<double,double>(  6.50, 1.800));
    smearEnvelope_.push_back(std::pair<double,double>( 10.00, 1.900));
    smearEnvelope_.push_back(std::pair<double,double>( 13.00, 2.000));
    smearEnvelope_.push_back(std::pair<double,double>( 15.00, 2.100));
    smearEnvelope_.push_back(std::pair<double,double>( 20.00, 2.200));
    smearEnvelope_.push_back(std::pair<double,double>( 25.00, 1.950));
    smearEnvelope_.push_back(std::pair<double,double>( 30.00, 1.809));
    smearEnvelope_.push_back(std::pair<double,double>( 37.00, 1.664));
    smearEnvelope_.push_back(std::pair<double,double>( 44.50, 1.519));
    smearEnvelope_.push_back(std::pair<double,double>( 56.00, 1.425));
    smearEnvelope_.push_back(std::pair<double,double>( 63.50, 1.348));
    smearEnvelope_.push_back(std::pair<double,double>( 81.00, 1.192));
    smearEnvelope_.push_back(std::pair<double,double>( 88.50, 1.130));
    smearEnvelope_.push_back(std::pair<double,double>(114.50, 1.144));
    smearEnvelope_.push_back(std::pair<double,double>(175.50, 1.070));
    smearEnvelope_.push_back(std::pair<double,double>(350.00, 1.043));
    smearEnvelope_.push_back(std::pair<double,double>(99999.00, 1.043));
  }

  if (smearEnvelope_.size())
    rand_ = new TRandom3();
}

//
// member functions
//

//======================================================================

template <class Hit,class Col>
void
HcalSmearedTimeRecHitProducer::processHits(edm::Event& iEvent) const
{
  edm::Handle<Col> colin;
  if (iEvent.getByLabel(recHitLabel_,colin)) {
    std::auto_ptr<Col> colout(new Col);

    typename Col::const_iterator itr;
    for (itr = colin->begin(); itr != colin->end(); itr++) {
      colout->push_back(Hit(itr->id(),itr->energy(),smearedTime(&(*itr))));
    }
    iEvent.put(colout);
  }
}

//======================================================================

void
HcalSmearedTimeRecHitProducer::dumpEnvelope(void) const
{
  std::cout << "Smear Envelope:" << std::endl;
  std::cout << "GeV\tSigma" << std::endl;
  for (uint32_t i=0; i<smearEnvelope_.size(); i++) {
    std::cout << smearEnvelope_[i].first << "\t" << smearEnvelope_[i].second << std::endl;
  }
}

//======================================================================

double
HcalSmearedTimeRecHitProducer::smearedTime(const CaloRecHit *rh) const
{
  double unsmearedTime  = rh->time();
  double hitenergy      = rh->energy();
  double smearsigma     = 0.0;
  uint32_t i=0;

  for (i=0; i<smearEnvelope_.size(); i++)
    if (smearEnvelope_[i].first > hitenergy)
	break;

  // Smearing occurs only within the envelope definitions.
  if (!i || (i >= smearEnvelope_.size())) return unsmearedTime;
  else {
    double energy1 = smearEnvelope_[i-1].first;
    double sigma1  = smearEnvelope_[i-1].second;
    double energy2 = smearEnvelope_[i].first;
    double sigma2  = smearEnvelope_[i].second;

    if (energy2 != energy1)
      smearsigma = sigma1 + ((sigma2-sigma1)*(hitenergy-energy1)/(energy2-energy1));
    else
      smearsigma = (sigma2+sigma1)/2.;

    //char s[80];
    //sprintf (s,"(%6.1f,%6.3f) (%6.1f,%6.3f) %6.1f %6.3f\n",
    //         energy1,sigma1,energy2,sigma2,hitenergy,smearsigma);
    //std::cout << s;
  }

  double smearedTime = unsmearedTime + ((smearsigma > 0.) ?  rand_->Gaus(0.0,smearsigma) : 0);

  return (smearedTime);
}

//======================================================================

void
HcalSmearedTimeRecHitProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  if (subdet_ == HcalBarrel ||
      subdet_ == HcalEndcap) {
    processHits<HBHERecHit,HBHERecHitCollection>(iEvent);
  }
  else if (subdet_ == HcalOuter) {
    processHits<HORecHit,HORecHitCollection>(iEvent);
  }
  else if (subdet_ == HcalForward) {
    processHits<HFRecHit,HFRecHitCollection>(iEvent);
  }
  else if (subdet_ == HcalZDCDetId::SubdetectorId ) {
    processHits<ZDCRecHit,ZDCRecHitCollection>(iEvent);
  }
}

// ------------ method called once each job just before starting event loop  ------------
void 
HcalSmearedTimeRecHitProducer::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalSmearedTimeRecHitProducer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalSmearedTimeRecHitProducer);
