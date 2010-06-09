// -*- C++ -*-
//
// Package:    HcalTimeFilteredRecHitProducer
// Class:      HcalTimeFilteredRecHitProducer
// 
/**\class HcalTimeFilteredRecHitProducer HcalTimeFilteredRecHitProducer.cc MyEDmodules/HcalTimeFilteredRecHitProducer/src/HcalTimeFilteredRecHitProducer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Dudero
//         Created:  Mon Mar  2 02:37:12 CST 2009
// $Id: HcalTimeFilteredRecHitProducer.cc,v 1.7 2009/10/27 02:36:32 dudero Exp $
//
//


// system include files
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"

#include "TRandom3.h"

//
// class declaration
//

class HcalTimeFilteredRecHitProducer : public edm::EDProducer {
public:
  explicit HcalTimeFilteredRecHitProducer(const edm::ParameterSet&);
  ~HcalTimeFilteredRecHitProducer();

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
 
  bool convertIdNumbers  (std::vector<int>& v_maskidnumbers,
                          std::vector<HcalDetId>& detIds2mask);

  bool maskedId          (const HcalDetId& id);

  double smearTime   (const double energy, const double time) const;
  bool   inTime      (const CaloRecHit& rh) const;
  void   dumpEnvelope(const std::vector<std::pair<double,double> >& env,
		      const std::string& descr) const;

  std::string histname (int i,bool senseGreaterThan=true);
  std::string histtitle(int i,bool senseGreaterThan=true);

  void updateCounters(double hitenergy,
		      std::vector<int>& counts);

  template<class Hit,class Col>
  void   filterHits   (edm::Event& iEvent,
		       edm::InputTag recotag);

  // ----------member data ---------------------------

  edm::ParameterSet        rhProfilingPset_; /* "profiling" in the sense of
						determining characteristics */
  edm::InputTag            hbherecotag_;
  edm::InputTag            hfrecotag_;
  edm::InputTag            horecotag_;
  double                   timeWindowCenterNs_;
  double                   timeWindowGain_; // 0-1
  int                      subdet_;
  int                      subdetOther_;
  std::string              subdetstr_;
  std::vector<HcalDetId>   detIds2mask_;
  double                   timeShiftNs_; /* time to shift rechit times by
					    before applying filter window */

  std::vector<double>      v_profileThresholds_;
  std::vector<TH1F *>      v_allhistos_;
  std::vector<TH1F *>      v_inthistos_;
  std::vector<TH1F *>      v_outhistos_;
  //myAnalHistos            *pfHistos_;
  TRandom3                *rand_;

  int                      flagFilterMask_; // filter by rechit flag bits


  std::vector<std::pair<double,double> >  tsmearEnvelope_;
  std::vector<std::pair<double,double> >  tfilterEnvelope_;
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
HcalTimeFilteredRecHitProducer::HcalTimeFilteredRecHitProducer(const edm::ParameterSet& iConfig) :
  rhProfilingPset_(iConfig.getUntrackedParameter<edm::ParameterSet>("rhProfilingPset",edm::ParameterSet())),
  hbherecotag_(iConfig.getUntrackedParameter<edm::InputTag>("hbheLabel",(edm::InputTag)"")),
  hfrecotag_(iConfig.getUntrackedParameter<edm::InputTag>("hfLabel",(edm::InputTag)"")),
  horecotag_(iConfig.getUntrackedParameter<edm::InputTag>("hoLabel",(edm::InputTag)"")),
  timeWindowCenterNs_(iConfig.getParameter<double>("timeWindowCenterNs")),
  timeWindowGain_(iConfig.getParameter<double>("timeWindowGain")),
  timeShiftNs_(iConfig.getParameter<double>("timeShiftNs")),
  flagFilterMask_(iConfig.getParameter<int>("flagFilterMask"))
{
  //register your products

  subdetstr_=iConfig.getParameter<std::string>("Subdetector");
  if (!strcasecmp(subdetstr_.c_str(),"HBHE")) {
    subdet_=HcalBarrel;
    produces<HBHERecHitCollection>();
  } else if (!strcasecmp(subdetstr_.c_str(),"HO")) {
    subdet_=HcalOuter;
    produces<HORecHitCollection>();
  } else if (!strcasecmp(subdetstr_.c_str(),"HF")) {
    subdet_=HcalForward;
    produces<HFRecHitCollection>();
  } else if (!strcasecmp(subdetstr_.c_str(),"CALIB")) {
    subdet_=HcalOther;
    subdetOther_=HcalCalibration;
    produces<HcalCalibRecHitCollection>();
  } else {
    std::cout << "HcalTimeFilteredRecHitProducer is not associated with a specific subdetector!" << std::endl;
  }       

  //now do what ever other initialization is needed

  std::vector<int> v_maskidnumbers;
  v_maskidnumbers  = iConfig.getParameter<std::vector<int> > ("detIds2Mask");
  if (!convertIdNumbers(v_maskidnumbers, detIds2mask_))
    throw cms::Exception("Invalid detID vector");

  /***************************************************
   * PROCESS SMEARING ENVELOPE
   **************************************************/

  std::vector<double> v_tsmearEnv;
  v_tsmearEnv  = iConfig.getParameter<std::vector<double> > ("tsmearEnvelope");
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
  }

  /***************************************************
   * PROCESS FILTERING ENVELOPE
   **************************************************/

  std::vector<double> v_tfilterEnv;
  v_tfilterEnv  = iConfig.getParameter<std::vector<double> > ("tfilterEnvelope");
  if (v_tfilterEnv.size()) {
    if (v_tfilterEnv.size() & 1)
      throw cms::Exception("Invalid tfilterEnvelope vector");

    for (uint32_t i = 0; i<v_tfilterEnv.size(); i+=2) {
      double energy     = v_tfilterEnv[i];
      double tfilterlim = v_tfilterEnv[i+1];
      if (tfilterlim <= 0.0)
	throw cms::Exception("nonpositive tfilterEnvelope limit encountered");
    
      tfilterEnvelope_.push_back(std::pair<double,double>(energy,tfilterlim));
    }

    //sort in order of increasing energy
    std::sort(tfilterEnvelope_.begin(),
	      tfilterEnvelope_.end(),
	      comparePair1<std::pair<double,double> >());
  }

  /***************************************************
   * PROCESS OPTIONAL PROFILING PARAMETERS
   **************************************************/

  if (!rhProfilingPset_.empty()) {
    v_profileThresholds_ =
      rhProfilingPset_.getUntrackedParameter<std::vector<double> >("thresholds");
    std::sort(v_profileThresholds_.begin(),v_profileThresholds_.end());
    //    pfHistos_ = new myAnalHistos("recHitProfilingHistos");
  }
}                                  // HcalTimeFilteredRecHitProducer::HcalTimeFilteredRecHitProducer

//======================================================================

void
HcalTimeFilteredRecHitProducer::dumpEnvelope(const std::vector<std::pair<double,double> >& env,
			       const std::string& descr) const
{
  if (!env.size()) std::cout << descr << " is empty" << std::endl;
  else {
    std::cout << descr << ":" << std::endl;
    std::cout << "E (GeV)\tT (ns)" << std::endl;
    for (uint32_t i=0; i<env.size(); i++) {
      std::cout << env[i].first << "\t" << env[i].second << std::endl;
    }
  }
}

//======================================================================

bool
HcalTimeFilteredRecHitProducer::maskedId(const HcalDetId& id)
{
  for (uint32_t i=0; i<detIds2mask_.size(); i++)
    if (id == detIds2mask_[i]) return true;
  return false;
}

HcalTimeFilteredRecHitProducer::~HcalTimeFilteredRecHitProducer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//
//======================================================================

bool
HcalTimeFilteredRecHitProducer::convertIdNumbers(std::vector<int>& v_maskidnumbers,
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
}                                  // HcalTimeFilteredRecHitProducer::convertIdNumbers

//======================================================================

double
HcalTimeFilteredRecHitProducer::smearTime(const double energy, const double unsmearedTime) const
{
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
}                                       // HcalTimeFilteredRecHitProducer::smearedTime

//======================================================================

bool
HcalTimeFilteredRecHitProducer::inTime(const CaloRecHit& rh) const
{
  double rhtime  = rh.time();
  double energy  = rh.energy();
  double twinmax, twinmin;
  uint32_t i=0;

  if (!tfilterEnvelope_.size()) // no envelope, so always "in time"
    return true;

  if (energy  < tfilterEnvelope_[0].second)
    twinmax = tfilterEnvelope_[0].second;
  else {
    for (i=0; i<tfilterEnvelope_.size(); i++)
      if (tfilterEnvelope_[i].first > energy)
	break;

    if (i == tfilterEnvelope_.size())
      twinmax = tfilterEnvelope_[i-1].second;
    else {
      double energy1 = tfilterEnvelope_[i-1].first;
      double lim1    = tfilterEnvelope_[i-1].second;
      double energy2 = tfilterEnvelope_[i].first;
      double lim2    = tfilterEnvelope_[i].second;

      twinmax = lim1 + ((lim2-lim1)*(energy-energy1)/(energy2-energy1));
      
      //char s[80];
      /*sprintf (s,"(%6.1f,%6.3f) (%6.1f,%6.3f) %6.1f %6.3f\n",
                 energy1,lim1,energy2,lim2,energy,twinmax); */
      //std::cout << s;
    }
  }

  twinmin = timeWindowCenterNs_ - (twinmax*timeWindowGain_);
  twinmax = timeWindowCenterNs_ + (twinmax*timeWindowGain_);

  //std::cout << "Time Window: " << twinmin << "-" << twinmax << std::endl;

  return ((rhtime > twinmin) && (rhtime < twinmax));
}                                            // HcalTimeFilteredRecHitProducer::inTime

//======================================================================

std::string
HcalTimeFilteredRecHitProducer::histname(int i,bool senseGreaterThan)
{
  std::ostringstream namss;
  namss << "h1d_"<<subdetstr_<<"nRecHits"<<(senseGreaterThan?"Over":"Under")<<"Thresh"<<i;
  return namss.str();
}

std::string HcalTimeFilteredRecHitProducer::histtitle(int i,bool senseGreaterThan)
{
  std::ostringstream titss;
  double thresh = v_profileThresholds_[i];
  titss << subdetstr_<<" # RecHits " << (senseGreaterThan?"> ":"< ") << thresh << " GeV";
  return titss.str();
}

//======================================================================

void
HcalTimeFilteredRecHitProducer::updateCounters(double hitenergy,
				 std::vector<int>& counts) // hits above/below threshold
{
  // unroll the first iteration because most hits are here -
  // lower than the lowest threshold (assuming threshold is set reasonably
  //
  if (hitenergy < v_profileThresholds_[0]) { counts[0]++; return; }

  for (uint32_t i=0; i<v_profileThresholds_.size(); i++) {
    double thresh = v_profileThresholds_[i];
    if (hitenergy > thresh) counts[i+1]++;
    else return;
  }
}

//======================================================================

template<class Hit,class Col>
void
HcalTimeFilteredRecHitProducer::filterHits(edm::Event& iEvent,
			     edm::InputTag recotag)
{
  using namespace edm;

  bool doSmear = ((tsmearEnvelope_.size() > 0) &&
		  ((subdet_ == HcalBarrel) || 
		   (subdet_ == HcalEndcap))       );

  std::vector<int> allcounters(v_profileThresholds_.size()+1,0);
  std::vector<int> intimecounters(v_profileThresholds_.size()+1,0);
  std::vector<int> outoftimecounters(v_profileThresholds_.size()+1,0);

  Handle<Col> incol;
  if (iEvent.getByLabel(recotag,incol)) {
    std::auto_ptr<Col> outCol(new Col);

    for (uint32_t i=0; i < incol->size(); i++) {
      const Hit& inhit= (*incol)[i];
      Hit outh;

      if (maskedId(inhit.id())) continue;   // masked cells just plain don't count

#ifdef CMSSW3XX
      if (inhit.flags() & flagFilterMask_) continue; // filtering by flag bits
#endif

      Hit outhit(inhit.id(),
		 inhit.energy(),
		 inhit.time()-timeShiftNs_); // time-shifted hit (if shift != 0)

      if (doSmear)
	outhit = Hit(inhit.id(),
		     inhit.energy(),
		     smearTime(outhit.energy(),
			       outhit.time()));  // smeared hit

#ifdef CMSSW3XX
      outhit.setFlags(inhit.flags());
#endif

      if (inTime(outhit))
	outCol->push_back(outhit);               // in-time hit

      if (v_profileThresholds_.size()) {
	if (inTime(outhit)) {
	  updateCounters(outhit.energy(),intimecounters);
	} else {
	  updateCounters(outhit.energy(),outoftimecounters);
	}
	updateCounters(outhit.energy(),allcounters);
      }
    } // hit loop

    // fill any profiling histograms
    for (uint32_t i=0; i<v_allhistos_.size(); i++) {
      v_allhistos_[i]->Fill((double)allcounters[i]);
      v_inthistos_[i]->Fill((double)intimecounters[i]);
      v_outhistos_[i]->Fill((double)outoftimecounters[i]);
    }

    iEvent.put(outCol);
  }
}                                        // HcalTimeFilteredRecHitProducer::filterHits

//======================================================================

// ------------ method called to produce the data  ------------
void
HcalTimeFilteredRecHitProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  switch (subdet_) {
  case HcalBarrel:
  case HcalEndcap:  filterHits<HBHERecHit,HBHERecHitCollection>(iEvent,hbherecotag_); break;
  case HcalOuter:   filterHits<HORecHit,HORecHitCollection>(iEvent,horecotag_); break;
  case HcalForward: filterHits<HFRecHit,HFRecHitCollection>(iEvent,hfrecotag_); break;
  default: break;
  }
}                                        // HcalTimeFilteredRecHitProducer::produce


// --- method called once each job just before starting event loop  ----
void 
HcalTimeFilteredRecHitProducer::beginJob(const edm::EventSetup&)
{
  //  edm::LogInfo("Parameters being used: ")   <<

  std::cout << "----------------------------------------"  << "\n" <<
  std::cout << "Parameters being used: "  << "\n" <<
    "hbherecotag_         : " << hbherecotag_        << "\n" <<
    "hfrecotag_           : " << hfrecotag_          << "\n" <<
    "horecotag_           : " << horecotag_          << "\n" <<
    "subdet_              : " << subdet_             << "\n" <<
    "timeWindowCenterNs_  = " << timeWindowCenterNs_ << "\n" <<
    "timeWindowGain_      = " << timeWindowGain_     << "\n" <<
    "timeShiftNs_         = " << timeShiftNs_        << "\n" <<
    "flagFilterMask_      = " << flagFilterMask_     << std::endl;
  
  for (uint32_t i=0; i<detIds2mask_.size(); i++)
    //edm::LogInfo("Masking ") << detIds2mask_[i] << std::endl;
    std::cout << "Masking " << detIds2mask_[i] << std::endl;


  dumpEnvelope(tsmearEnvelope_,  "Smear Sigma Envelope");
  dumpEnvelope(tfilterEnvelope_, "Time Filter Envelope Limits");

  /******************************
   * BOOK PROFILING HISTOS
   ******************************/
  if (v_profileThresholds_.size()) { /* currently acts also as an indicator
					of profile activities in general */
    edm::Service<TFileService> fs;
      
    v_allhistos_.push_back(fs->make<TH1F>((histname(0,false)+"All").c_str(),
					  (histtitle(0,false)+ " (Unfiltered)").c_str(),
					  101, -0.5, 5000.5));
    v_inthistos_.push_back(fs->make<TH1F>((histname(0,false)+"InTime").c_str(),
					  (histtitle(0,false)+ " (In Time)").c_str(),
					  101, -0.5, 5000.5));
    v_outhistos_.push_back(fs->make<TH1F>((histname(0,false)+"OutOfTime").c_str(),
					  (histtitle(0,false)+ " (Out Of Time)").c_str(),
					  101, -0.5, 5000.5));

    for (uint32_t i=0; i<v_profileThresholds_.size(); i++) {
      v_allhistos_.push_back(fs->make<TH1F>((histname(i+1)+"All").c_str(),
					    (histtitle(i)+ " (Unfiltered)").c_str(),
					    101, -0.5, 100.5));
      v_inthistos_.push_back(fs->make<TH1F>((histname(i+1)+"InTime").c_str(),
					    (histtitle(i)+ " (In Time)").c_str(),
					    101, -0.5, 100.5));
      v_outhistos_.push_back(fs->make<TH1F>((histname(i+1)+"OutOfTime").c_str(),
					    (histtitle(i)+ " (Out Of Time)").c_str(),
					    101, -0.5, 100.5));
    }
    //pfHistos_->book1d<TH1D>(v_hpars1d);
  }
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalTimeFilteredRecHitProducer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalTimeFilteredRecHitProducer);
