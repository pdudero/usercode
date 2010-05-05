
// -*- C++ -*-
//
// Package:    LaserDelayTunerTDCAlgos
// Class:      LaserDelayTunerTDCAlgos
// 
/**\class LaserDelayTunerTDCalgos LaserDelayTunerTDCalgos.cc MyEDmodules/LaserDelayTuner/src/LaserDelayTunerTDCalgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserDelayTunerTDCalgos.cc,v 1.2 2010/04/22 03:22:59 dudero Exp $
//
//

// system include files
#include <set>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"

#include "MyEDmodules/HcalDelayTuner/src/LaserDelayTunerTDCalgos.hh"

#include "TH1D.h"

//
// constructors and destructor
//
LaserDelayTunerTDCalgos::LaserDelayTunerTDCalgos(const edm::ParameterSet& iConfig)
{
  TDCCutCenter_  = iConfig.getParameter<double>( "TDCCutCenter" );
  TDCCutWindow_ = iConfig.getParameter<double>( "TDCCutWindow" );
  std::cerr << "TDC cut set to within +/-" << TDCCutWindow_ << "ns of " << TDCCutCenter_ << "ns" << std::endl;

  correctedTimeModCeiling_     = iConfig.getParameter<int>( "CorrectedTimeModCeiling" );
  std::cerr << "Corrected Time Modulus: if > " << correctedTimeModCeiling_;
  std::cerr << "ns subtract 25 ns" << std::endl;

  timeModCeiling_     = iConfig.getParameter<int>( "TimeModCeiling" );
  std::cerr << "RecTime Modulus: if > " << timeModCeiling_;
  std::cerr << "ns subtract 25 ns" << std::endl;

  v_cuts_.push_back("cutNone");
  v_cuts_.push_back("cutTDCwindow");
}

//======================================================================

void LaserDelayTunerTDCalgos::bookHistos(void)
{
  // Initialize the cuts for the run and add them to the global map

  m_cuts_.clear();

  edm::Service<TFileService> fs;
  rootDir_ = new TFileDirectory(fs->mkdir("TDC"));

  for (unsigned i=0; i<v_cuts_.size(); i++) {
    m_cuts_[v_cuts_[i]] = new myAnalCut(i,v_cuts_[i],*rootDir_);
  }

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d; 

  myAnalHistos::HistoParams_t hpars1d;

  hpars1d.nbinsy = 0;

  st_TDCLaserFireTime_ = "TDClaserFireTime"; // Binned to .8 ns width
  hpars1d.name    = st_TDCLaserFireTime_;
  hpars1d.title   = "TDC Laser Fire Time; Time (ns); Pulses";
  hpars1d.nbinsx  = 376;
  hpars1d.minx    = 1000;
  hpars1d.maxx    = 1300;

  v_hpars1d.push_back(hpars1d);

  /*****************************************
   * BOOK 'EM, DANNO...                    *
   *****************************************/

  std::map<string, myAnalCut *>::const_iterator it;
  for (it = m_cuts_.begin(); it != m_cuts_.end(); it++) {
    edm::LogInfo("Booking for cut ") << it->first << std::endl;
    myAnalHistos  *myAH = it->second->cuthistos();
    myAH->book1d<TH1D> (v_hpars1d);
  }
}                                                          // bookHistos

//======================================================================

void
LaserDelayTunerTDCalgos::fillHistos4cut(const std::string& cutstr)
{
  myAnalHistos *myAH = m_cuts_[cutstr]->cuthistos();
  myAH->fill1d<TH1D>(st_TDCLaserFireTime_,laserFireTime_);
}
				     
//======================================================================

void
LaserDelayTunerTDCalgos::processDigis(const edm::Handle<HcalLaserDigi>& laserDigi)
{
  isWithinTDCcut_ = false;

  double tTrigger = -1, tRawOptosync = -1;
  for( uint ch = 0; ch < laserDigi->tdcHits(); ch++ ) {
    if( laserDigi->hitChannel(ch) == 2 && tTrigger     < 0 ) tTrigger     = laserDigi->hitNS(ch);
    if( laserDigi->hitChannel(ch) == 3 && tRawOptosync < 0 ) tRawOptosync = laserDigi->hitNS(ch);
  }
    
  if( tRawOptosync > 0 && tTrigger > 0 ) {
    laserFireTime_  = tRawOptosync   - tTrigger;
    laserFireDelay_ = laserFireTime_ - TDCCutCenter_;
  }
  else {
    fprintf( stderr, "-=-=-=-=-= Bad TDC data! =-=-=-=-=-\n" );
    laserFireTime_  = 0;
    laserFireDelay_ = 1e99;
  }

  fillHistos4cut("cutNone");

  isWithinTDCcut_ =
    ( laserFireDelay_ >= -TDCCutWindow_ ) &&
    ( laserFireDelay_ <=  TDCCutWindow_ );

  if (isWithinTDCcut_)
    fillHistos4cut("cutTDCwindow");
}

//======================================================================

double
LaserDelayTunerTDCalgos::modulusTheTime(const double intime)
{
  double modtime = intime;
  if( intime > timeModCeiling_ )
    modtime -= 25; // one time sample.

  return modtime;
}

//======================================================================

// ------------ method called to for each event  ------------
void
LaserDelayTunerTDCalgos::process(const myEventData& ed)
{
  if (ed.laserdigi().isValid()) {
    processDigis (ed.laserdigi());
    eventValid_ = true;
  }
  else {
    eventValid_ = false;
    std::cerr << "-=-=-=-=-=-= No Laser Digis (QADCTDC) =-=-=-=-=-=-";
    std::cerr << std::endl;
    return;
  }
}


// ------------ method called once each job just before starting event loop  ------------
void 
LaserDelayTunerTDCalgos::beginJob(const edm::EventSetup&)
{
  std::cerr << "-=-=-=-=-=-= LaserDelayTunerTDCalgos::beginJob =-=-=-=-=-=-" << std::endl;
  bookHistos();
}

// ------------ method called once each job just after ending the event loop  ------------
void 
LaserDelayTunerTDCalgos::endJob() {
}
