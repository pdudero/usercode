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
// $Id: myEventData.cc,v 1.1 2009/04/09 22:12:04 dudero Exp $
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
  hbheDigiTag_(edPset.getUntrackedParameter<edm::InputTag>("hbheDigiLabel",edm::InputTag(""))),
  hbheRechitTag_(edPset.getUntrackedParameter<edm::InputTag>("hbheRechitLabel",edm::InputTag(""))),
  hfRechitTag_(edPset.getUntrackedParameter<edm::InputTag>("hfRechitLabel",edm::InputTag(""))),
  hoRechitTag_(edPset.getUntrackedParameter<edm::InputTag>("hoRechitLabel",edm::InputTag(""))),
  simHitTag_(edPset.getUntrackedParameter<edm::InputTag>("simHitLabel",edm::InputTag(""))),
  metTag_(edPset.getUntrackedParameter<edm::InputTag>("metLabel",edm::InputTag(""))),
  twrTag_(edPset.getUntrackedParameter<edm::InputTag>("twrLabel",edm::InputTag(""))),
  verbose_(edPset.getUntrackedParameter<bool>("verbose",false))
{
  if (verbose_) {
    cout << "hbheDigiTag_   = " << hbheDigiTag_   << endl;
    cout << "hbheRechitTag_ = " << hbheRechitTag_ << endl;
    cout << "hfRechitTag_   = " << hfRechitTag_   << endl;
    cout << "hoRechitTag_   = " << hoRechitTag_   << endl;
    cout << "simHitTag_     = " << simHitTag_     << endl;
    cout << "metTag_        = " << metTag_        << endl;
    cout << "twrTag_        = " << twrTag_        << endl;
  }
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

  if (hbheDigiTag_.label().size())
    if(!iEvent.getByLabel(hbheDigiTag_,hbhedigis_)) {
      edm::LogWarning("myEventData::get") <<
	"Digis not found, "<< hbheDigiTag_ << std::endl;
    } else if (verbose_) 
      edm::LogInfo("myEventData::get") << "Got HBHE digis "<< hbheDigiTag_ << std::endl;

  if (simHitTag_.label().size())
    if (!iEvent.getByLabel(simHitTag_, hsimhits_)) {
      edm::LogWarning("myEventData::get") <<
	"Simhits not found, " << simHitTag_ << std::endl;
    } else if (verbose_) 
      edm::LogInfo("myEventData::get") << "Got sim hits " << simHitTag_ << std::endl;

  if (hbheRechitTag_.label().size())
    if (!iEvent.getByLabel(hbheRechitTag_,hbherechits_)) {
      edm::LogWarning("myEventData::get") <<
	"HBHE Rechits not found, " << hbheRechitTag_  << std::endl;
      return;
    } else if (verbose_) 
      edm::LogInfo("myEventData::get") << "Got HBHE Rechits " << hbheRechitTag_ << std::endl;

  if (hfRechitTag_.label().size())
    if (!iEvent.getByLabel(hfRechitTag_,hfrechits_)) {
      edm::LogWarning("myEventData::get") <<
	"HF Rechits not found, " << hfRechitTag_ << std::endl;
      return;
    } else if (verbose_) 
      edm::LogInfo("myEventData::get") << "Got HF Rechits " << hfRechitTag_ << std::endl;
  
  if (hoRechitTag_.label().size())
    if (!iEvent.getByLabel(hoRechitTag_,horechits_)) {
      edm::LogWarning("myEventData::get") <<
	"HO Rechits not found, " << hoRechitTag_ << std::endl;
      return;
    } else if (verbose_) 
      edm::LogInfo("myEventData::get") << "Got HO Rechits " << hoRechitTag_ << std::endl;

  // CaloTowers
  if (twrTag_.label().size())
    if (!iEvent.getByLabel(twrTag_,towers_)) {
      edm::LogWarning("myEventData::get") <<
	"Calo Towers not found, " << twrTag_ << std::endl;
    } else if (verbose_) 
      edm::LogInfo("myEventData::get") << "Got Calo Towers " << twrTag_ << std::endl;
  
  // MET
  if (metTag_.label().size())
    if (!iEvent.getByLabel(metTag_, recmet_)) {
      edm::LogWarning("myEventData::get") <<
	"Calo MET not found, " << metTag_ << std::endl;
    } else if (verbose_) 
      edm::LogInfo("myEventData::get") << "Got Calo MET " << metTag_ << std::endl;
}
