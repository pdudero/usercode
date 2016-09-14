
// -*- C++ -*-
//
// Package:    LaserTDCAlgos
// Class:      LaserTDCAlgos
// 
/**\class LaserTDCalgos LaserTDCalgos.cc MyEDmodules/HFraddamAnal/src/LaserTDCalgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserTDCalgos.cc,v 1.1 2009/07/27 15:56:53 dudero Exp $
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

#include "../interface/LaserTDCalgos.hh"

#include "TH1D.h"

//
// constructors and destructor
//
LaserTDCalgos::LaserTDCalgos(const edm::ParameterSet& iConfig)
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
}
				     
//======================================================================

void
LaserTDCalgos::processDigis(const edm::Handle<HcalLaserDigi>& laserDigi)
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

  isWithinTDCcut_ =
    ( laserFireDelay_ >= -TDCCutWindow_ ) &&
    ( laserFireDelay_ <=  TDCCutWindow_ );
}

//======================================================================

double
LaserTDCalgos::modulusTheTime(const double intime)
{
  double modtime = intime;
  if( intime > timeModCeiling_ )
    modtime -= 25; // one time sample.

  return modtime;
}

//======================================================================

// ------------ method called to for each event  ------------
void
LaserTDCalgos::process(const myEventData& ed)
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
LaserTDCalgos::beginJob()
{
  std::cerr << "-=-=-=-=-=-= LaserTDCalgos::beginJob =-=-=-=-=-=-" << std::endl;
}

// ------------ method called once each job just after ending the event loop  ------------
void 
LaserTDCalgos::endJob() {
}
