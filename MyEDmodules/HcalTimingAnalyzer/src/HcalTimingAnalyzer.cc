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
// $Id: HcalTimingAnalyzer.cc,v 1.4 2009/04/03 16:35:31 dudero Exp $
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
#include "MyEDmodules/HcalTimingAnalyzer/src/HcalTimingAnalAlgos.hh"

//
// class declaration
//


class HcalTimingAnalyzer : public edm::EDAnalyzer {
public:
  explicit HcalTimingAnalyzer(const edm::ParameterSet&);
  ~HcalTimingAnalyzer() {}

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  // ----------member data ---------------------------
  myEventData         *eventData_;
  HcalTimingAnalAlgos *algo_;
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
HcalTimingAnalyzer::HcalTimingAnalyzer(const edm::ParameterSet& iConfig)
{
  edm::ParameterSet edPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("eventDataPset");
  eventData_ = new myEventData(edPset);
  algo_      = new HcalTimingAnalAlgos(iConfig);
}


//======================================================================

//
// member functions
//
//======================================================================

// ------------ method called to for each event  ------------
void
HcalTimingAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  eventData_->get(iEvent,iSetup);
  algo_->process(*eventData_);
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
