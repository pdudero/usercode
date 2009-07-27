// -*- C++ -*-
//
// Package:    LaserDelayTuner
// Class:      LaserDelayTuner
// 
/**\class LaserDelayTuner LaserDelayTuner.cc MyEDmodules/LaserDelayTuner/src/LaserDelayTuner.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserDelayTuner.cc,v 1.5 2009/04/09 21:57:19 dudero Exp $
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
#include "MyEDmodules/LaserDelayTuner/src/LaserDelayTunerTDCalgos.hh"
#include "MyEDmodules/LaserDelayTuner/src/LaserDelayTunerAlgos.hh"
#include "EventFilter/HcalRawToDigi/interface/HcalDCCHeader.h"

//
// class declaration
//


class LaserDelayTuner : public edm::EDAnalyzer {
public:
  explicit LaserDelayTuner(const edm::ParameterSet&);
  ~LaserDelayTuner();

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  // ----------member data ---------------------------

  myEventData             *eventData_;
  LaserDelayTunerTDCalgos *tdcalgo_;

  LaserDelayTunerAlgos *hbalgo_;
  LaserDelayTunerAlgos *healgo_;
  LaserDelayTunerAlgos *hfalgo_;
  LaserDelayTunerAlgos *hoalgo_;

  bool                     firstEvent_;
  int                      nlaserEv_;

  edm::ParameterSet HBpars_;
  edm::ParameterSet HEpars_;
  edm::ParameterSet HOpars_;
  edm::ParameterSet HFpars_;

  std::set<uint32_t> s_runs_; // set of run numbers run over
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
LaserDelayTuner::LaserDelayTuner(const edm::ParameterSet& iConfig)
{
  std::cerr << "-=-=-=-=-=LaserDelayTuner Constructor=-=-=-=-=-" << std::endl;

  edm::ParameterSet edPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("eventDataPset");
  edm::ParameterSet TDCpars =
    iConfig.getUntrackedParameter<edm::ParameterSet>("TDCpars");

  HBpars_ = iConfig.getParameter<edm::ParameterSet>("HBpars");
  HEpars_ = iConfig.getParameter<edm::ParameterSet>("HEpars");
  HOpars_ = iConfig.getParameter<edm::ParameterSet>("HOpars");
  HFpars_ = iConfig.getParameter<edm::ParameterSet>("HFpars");

  eventData_ = new myEventData(edPset);
  tdcalgo_   = new LaserDelayTunerTDCalgos(TDCpars);
}

LaserDelayTuner::~LaserDelayTuner() {
  std::cerr << "-=-=-=-=-=LaserDelayTuner Destructor=-=-=-=-=-" << std::endl;
}


//======================================================================

//
// member functions
//
//======================================================================

// ------------ method called to for each event  ------------
void
LaserDelayTuner::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  uint32_t runn = eventData_->runNumber();
  if (notInSet<uint32_t>(s_runs_,runn))
    s_runs_.insert(runn);

  eventData_->get(iEvent,iSetup);

  bool isLocalHCALrun = eventData_->hcaltbtrigdata().isValid();
  bool isLaserEvent   = false;
  if (isLocalHCALrun) {
    tdcalgo_->process(*eventData_);
    if (eventData_->hcaltbtrigdata()->wasLaserTrigger()) isLaserEvent = true;
  }

  if (firstEvent_) {
    if (eventData_->hbherechits().isValid()) {
      hbalgo_ = new LaserDelayTunerAlgos(HcalBarrel,HBpars_,tdcalgo_);
      healgo_ = new LaserDelayTunerAlgos(HcalEndcap,HEpars_,tdcalgo_);
    }
    if (eventData_->hfrechits().isValid())
      hfalgo_ = new LaserDelayTunerAlgos(HcalForward,HFpars_,tdcalgo_);
    if (eventData_->horechits().isValid())
      hoalgo_ = new LaserDelayTunerAlgos(HcalOuter,HOpars_,tdcalgo_);
    firstEvent_ = false;

    if (isLocalHCALrun) 
      cout << "Local TB trigger data detected\n" << endl;
  }

  // Abort Gap laser 
  if ((!isLocalHCALrun || !isLaserEvent) &&
      eventData_->fedrawdata().isValid())
  {
    //checking FEDs for calibration information
    for (int i=FEDNumbering::getHcalFEDIds().first;i<=FEDNumbering::getHcalFEDIds().second; i++) {
      const FEDRawData& fedData = eventData_->fedrawdata()->FEDData(i) ;
      if ( fedData.size() < 24 ) continue ;
      int value = ((const HcalDCCHeader*)(fedData.data()))->getCalibType() ;
      if(value==hc_HBHEHPD || value==hc_HOHPD || value==hc_HFPMT){ isLaserEvent=true; break;} 
    }
  }   

  if(!isLaserEvent) return;
  else
    nlaserEv_++;
  
  if (eventData_->hbherechits().isValid()) { hbalgo_->process(*eventData_);
                                             healgo_->process(*eventData_); }
  if (eventData_->hfrechits().isValid())     hfalgo_->process(*eventData_);
  if (eventData_->horechits().isValid())     hoalgo_->process(*eventData_);
}

// ------------ method called once each job just before starting event loop  ------------
void 
LaserDelayTuner::beginJob(const edm::EventSetup& es)
{
  firstEvent_ = true;
  tdcalgo_->beginJob(es);
  nlaserEv_ = 0;
  hbalgo_   = 0;
  healgo_   = 0;
  hfalgo_   = 0;
  hoalgo_   = 0;
}

// ------------ method called once each job just after ending the event loop  ------------
void 
LaserDelayTuner::endJob() {
  cout << nlaserEv_ << " laser events processed" << endl;

  if (hbalgo_) hbalgo_->endJob();
  if (healgo_) healgo_->endJob();
  if (hfalgo_) hfalgo_->endJob();
  if (hoalgo_) hoalgo_->endJob();
}

//define this as a plug-in
DEFINE_FWK_MODULE(LaserDelayTuner);
