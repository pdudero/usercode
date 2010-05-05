#ifndef  _MYEDMODULESLASERDELAYTUNERTDCALGOS
#define  _MYEDMODULESLASERDELAYTUNERTDCALGOS

// -*- C++ -*-
//
// Package:    LaserDelayTunerTDCalgos
// Class:      LaserDelayTunerTDCalgos
// 
/**\class LaserDelayTunerTDCalgos LaserDelayTunerTDCalgos.cc MyEDmodules/LaserDelayTunerTDCalgos/src/LaserDelayTunerTDCalgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserDelayTunerTDCalgos.hh,v 1.1 2009/11/09 00:57:59 dudero Exp $
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

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"

//
// class declaration
//

class LaserDelayTunerTDCalgos {
public:
  explicit LaserDelayTunerTDCalgos(const edm::ParameterSet&);
  ~LaserDelayTunerTDCalgos() {}
  void   process(const myEventData& ed);
  double modulusTheTime(const double intime);  
  inline double correctTimeForJitter(double intime) {
    return (intime - laserFireDelay_);
  }
  inline bool isWithinWindow(void) {
    return (  isWithinTDCcut_ );
  }

  void beginJob(const edm::EventSetup&) ;
  void endJob() ;

private:
  void   bookHistos      (void);
  void   fillHistos4cut  (const std::string& cutstr);
  void   processDigis    (const edm::Handle<HcalLaserDigi>& laserDigi);

  // The collection of names of histos per subdetector
  std::string TDCLaserFireTime;

  // ----------member data ---------------------------

  TFileDirectory *rootDir_;

  std::vector<std::string> v_cuts_; // vector of cut strings
  std::map<std::string, myAnalCut *> m_cuts_;

  std::string st_TDCLaserFireTime_;
  bool       eventValid_;
  bool       isWithinTDCcut_;
  double     laserFireTime_,laserFireDelay_;

  // Parameters:
  double     TDCCutCenter_;
  double     TDCCutWindow_;
  double     timeModCeiling_;
  double     correctedTimeModCeiling_;
};

#endif // _MYEDMODULESLASERDELAYTUNERTDCALGOS
