// -*- C++ -*-
//
// Package:    SplashDelayTuner
// Class:      SplashDelayTuner
// 
/**\class SplashDelayTuner SplashDelayTuner.cc MyEDmodules/HcalDelayTuner/src/SplashDelayTuner.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: SplashDelayTuner.cc,v 1.1 2009/07/27 15:56:53 dudero Exp $
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
#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerInput.hh"
#include "MyEDmodules/HcalDelayTuner/interface/SplashDelayTunerAlgos.hh"

//
// class declaration
//

class SplashDelayTuner : public edm::EDAnalyzer {
public:
  explicit SplashDelayTuner(const edm::ParameterSet&);
  ~SplashDelayTuner();

private:
  //virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  // ----------member data ---------------------------

  SplashDelayTunerAlgos *algo_;
  HcalDelayTunerInput *inp_;
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
SplashDelayTuner::SplashDelayTuner(const edm::ParameterSet& iConfig)
{
  std::cerr << "-=-=-=-=-=SplashDelayTuner Constructor=-=-=-=-=-" << std::endl;

  algo_ = new SplashDelayTunerAlgos(iConfig);
  inp_  = new HcalDelayTunerInput(iConfig);
}

SplashDelayTuner::~SplashDelayTuner() {
  std::cerr << "-=-=-=-=-=SplashDelayTuner Destructor=-=-=-=-=-" << std::endl;
}

//======================================================================

//
// member functions
//
//======================================================================

//======================================================================

// ------------ method called to for each event  ------------
void
SplashDelayTuner::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
SplashDelayTuner::endJob() {
  DelaySettings oldsettings;
  inp_->getSamplingDelays(oldsettings);
  algo_->endJob(oldsettings);
}

//define this as a plug-in
DEFINE_FWK_MODULE(SplashDelayTuner);
