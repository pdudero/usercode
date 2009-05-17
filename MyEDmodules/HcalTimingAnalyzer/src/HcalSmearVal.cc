// -*- C++ -*-
//
// Package:    HcalSmearVal
// Class:      HcalSmearVal
// 
/**\class HcalSmearVal HcalSmearVal.cc MyEDmodules/HcalSmearVal/src/HcalSmearVal.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalSmearVal.cc,v 1.5 2009/04/09 21:57:19 dudero Exp $
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
#include "MyEDmodules/HcalTimingAnalyzer/src/HcalSmearValAlgos.hh"

//
// class declaration
//


class HcalSmearVal : public edm::EDAnalyzer {
public:
  explicit HcalSmearVal(const edm::ParameterSet&);
  ~HcalSmearVal() {}

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  // ----------member data ---------------------------
  myEventData         *unsmearedHits_;
  myEventData         *smearedHits_;
  HcalSmearValAlgos   *algo_;
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
HcalSmearVal::HcalSmearVal(const edm::ParameterSet& iConfig)
{
  edm::ParameterSet edUnsmearedPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("unsmearedDataPset");
  edm::ParameterSet edSmearedPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("smearedDataPset");
  unsmearedHits_ = new myEventData(edUnsmearedPset);
  smearedHits_   = new myEventData(edSmearedPset);
  algo_          = new HcalSmearValAlgos(iConfig);
}


//======================================================================

//
// member functions
//
//======================================================================

// ------------ method called to for each event  ------------
void
HcalSmearVal::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  unsmearedHits_->get(iEvent,iSetup);
  smearedHits_->get(iEvent,iSetup);
  algo_->process(*unsmearedHits_,*smearedHits_);
}

// ------------ method called once each job just before starting event loop  ------------
void 
HcalSmearVal::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalSmearVal::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalSmearVal);
