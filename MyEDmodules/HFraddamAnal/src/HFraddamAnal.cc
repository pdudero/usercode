// -*- C++ -*-
//
// Package:    HFraddamAnal
// Class:      HFraddamAnal
// 
/**\class HFraddamAnal HFraddamAnal.cc MyEDmodules/HFraddamAnal/src/HFraddamAnal.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Phillip Dudero
//         Created:  Fri Jan  4 19:06:33 CST 2013
// $Id$
//
//


// system include files
#include <memory>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"

#include "../interface/HFraddamAnalAlgos.hh"

//
// class declaration
//

class HFraddamAnal : public edm::EDAnalyzer {
public:
  explicit HFraddamAnal(const edm::ParameterSet&);
  ~HFraddamAnal();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&);
  virtual void endRun(edm::Run const&, edm::EventSetup const&);
  virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
  virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
  
  // ----------member data ---------------------------

  myEventData       *eventData_;
  HFraddamAnalAlgos *algos_;

  std::set<uint32_t> s_runs_; // set of run numbers run over
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
HFraddamAnal::HFraddamAnal(const edm::ParameterSet& iConfig)

{
   //now do what ever initialization is needed

  edm::ParameterSet edPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("eventDataPset");

  eventData_ = new myEventData(edPset);
  algos_     = new HFraddamAnalAlgos(iConfig);

}


HFraddamAnal::~HFraddamAnal()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
HFraddamAnal::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;


  uint32_t runn = eventData_->runNumber();
  if (notInSet<uint32_t>(s_runs_,runn))
    s_runs_.insert(runn);

  eventData_->get(iEvent,iSetup);

  algos_->process(*eventData_);

#ifdef THIS_IS_AN_EVENT_EXAMPLE
   Handle<ExampleData> pIn;
   iEvent.getByLabel("example",pIn);
#endif
   
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
   ESHandle<SetupData> pSetup;
   iSetup.get<SetupRecord>().get(pSetup);
#endif
}


// ------------ method called once each job just before starting event loop  ------------
void 
HFraddamAnal::beginJob()
{
  algos_->beginJob();
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HFraddamAnal::endJob() 
{
  algos_->endAnal();
}

// ------------ method called when starting to processes a run  ------------
void 
HFraddamAnal::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
HFraddamAnal::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
HFraddamAnal::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
HFraddamAnal::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
HFraddamAnal::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HFraddamAnal);
