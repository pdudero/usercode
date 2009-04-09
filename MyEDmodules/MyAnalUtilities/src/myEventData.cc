// -*- C++ -*-
//
// Package:    myEventData
// Class:      myEventData
// 
/**\class myEventData myEventData.cc MyEDmodules/myEventData/src/myEventData.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id$
//
//


// system include files
#include <memory>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"

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
myEventData::myEventData(const edm::ParameterSet& edPset) :
  hbheDigiLabel_(edPset.getUntrackedParameter<edm::InputTag>("hbheDigiLabel",edm::InputTag(""))),
  hbheRechitLabel_(edPset.getUntrackedParameter<edm::InputTag>("hbheRechitLabel",edm::InputTag(""))),
  hfRechitLabel_(edPset.getUntrackedParameter<edm::InputTag>("hfRechitLabel",edm::InputTag(""))),
  simHitLabel_(edPset.getUntrackedParameter<edm::InputTag>("simHitLabel",edm::InputTag(""))),
  metLabel_(edPset.getUntrackedParameter<edm::InputTag>("metLabel",edm::InputTag(""))),
  twrLabel_(edPset.getUntrackedParameter<edm::InputTag>("twrLabel",edm::InputTag("")))
{
}


//======================================================================

//
// member functions
//

// ------------ method called to for each event  ------------
void
myEventData::get(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  eventId_ = iEvent.id();
  runn_    = eventId_.run();
  evtn_    = eventId_.event();

  if (!iEvent.getByLabel(hbheDigiLabel_,hbhedigis_)) {
    edm::LogWarning("myEventData::get") <<
      "Digis not found"<< std::endl;
  }

  iEvent.getByLabel(simHitLabel_, hsimhits_);
 
  if (!hsimhits_.isValid()) {
    edm::LogWarning("myEventData::get") <<
      "Simhits not found"<< std::endl;
  }

  if (!iEvent.getByLabel(hbheRechitLabel_,hbherechits_)) {
    edm::LogWarning("myEventData::get") <<
      "Rechits not found"<< std::endl;
    return;
  }

  if (!iEvent.getByLabel(hfRechitLabel_,hfrechits_)) {
    edm::LogWarning("myEventData::get") <<
      "Rechits not found"<< std::endl;
    return;
  }

  // CaloTowers
  if (!iEvent.getByLabel(twrLabel_,towers_)) {
    edm::LogWarning("myEventData::get") <<
      "Calo Towers not found"<< std::endl;
  }

  // MET
  if (!iEvent.getByLabel(metLabel_, recmet_)) {
    edm::LogWarning("myEventData::get") <<
      "Calo MET not found"<< std::endl;
  }
}
