// -*- C++ -*-
//
// Package:    SplashTimingAnalyzer
// Class:      SplashTimingAnalyzer
// 
/**\class SplashTimingAnalyzer SplashTimingAnalyzer.cc MyEDmodules/HcalDelayTuner/plugins/SplashTimingAnalyzer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: SplashTimingAnalyzer.cc,v 1.1 2009/11/09 00:59:05 dudero Exp $
//
//


// system include files

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"

#include "MyEDmodules/HcalDelayTuner/interface/HcalDelayTunerAlgos.hh"
#include "MyEDmodules/HcalDelayTuner/interface/SplashDelayTunerAlgos.hh"
#include "MyEDmodules/HcalDelayTuner/interface/SplashHitTimeCorrector.hh"

//
// class declaration
//

class SplashTimingAnalyzer : public edm::EDAnalyzer {
public:
  explicit SplashTimingAnalyzer(const edm::ParameterSet&);
  ~SplashTimingAnalyzer();

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  // ----------member data ---------------------------

  myEventData            *eventData_;
  SplashDelayTunerAlgos  *algo_;
  SplashHitTimeCorrector *timecor_;
  std::set<uint32_t>      s_runs_; // set of run numbers run over
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
SplashTimingAnalyzer::SplashTimingAnalyzer(const edm::ParameterSet& iConfig)
{
  std::cerr << "-=-=SplashTimingAnalyzer Constructor=-=-" << std::endl;

  edm::ParameterSet edPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("eventDataPset");

  eventData_ = new myEventData(edPset);
  timecor_   = new
    SplashHitTimeCorrector(iConfig.getUntrackedParameter<bool>("splashPlusZside"));

  algo_ = new SplashDelayTunerAlgos(iConfig,timecor_);
}

SplashTimingAnalyzer::~SplashTimingAnalyzer() {
  std::cerr << "-=-=SplashTimingAnalyzer Destructor=-=-" << std::endl;
}

//======================================================================

//
// member functions
//
//======================================================================

// ------------ method called to for each event  ------------
void
SplashTimingAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  eventData_->get(iEvent,iSetup);

  uint32_t runn = eventData_->runNumber();
  if (notInSet<uint32_t>(s_runs_,runn))
    s_runs_.insert(runn);

  algo_->process(*eventData_);
}

// ------------ method called once each job just before starting event loop  ------------
void 
SplashTimingAnalyzer::beginJob(const edm::EventSetup& es)
{
  timecor_->init(es);
  algo_->beginJob(es);
}

// ------------ method called once each job just after ending the event loop  ------------
void 
SplashTimingAnalyzer::endJob() {
  algo_->endJob();
}

//define this as a plug-in
DEFINE_FWK_MODULE(SplashTimingAnalyzer);
