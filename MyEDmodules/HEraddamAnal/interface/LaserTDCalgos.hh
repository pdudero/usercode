#ifndef  _MYEDMODULESLASERTDCALGOS
#define  _MYEDMODULESLASERTDCALGOS

// -*- C++ -*-
//
// Package:    LaserTDCalgos
// Class:      LaserTDCalgos
// 
/**\class LaserTDCalgos LaserTDCalgos.cc MyEDmodules/LaserTDCalgos/src/LaserTDCalgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserTDCalgos.hh,v 1.1 2009/07/27 15:56:53 dudero Exp $
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
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"

//
// class declaration
//

class LaserTDCalgos {
public:
  explicit LaserTDCalgos(const edm::ParameterSet&);
  ~LaserTDCalgos() {}
  void   process(const myEventData& ed);
  double modulusTheTime(const double intime);  
  inline double correctTimeForJitter(double intime) {
    return (intime - laserFireDelay_);
  }
  inline bool isWithinWindow(void) {
    return (  isWithinTDCcut_ );
  }

  inline double phase(void)  { return laserFireTime_; }

  void beginJob() ;
  void endJob() ;

private:
  void   processDigis    (const edm::Handle<HcalLaserDigi>& laserDigi);

  // ----------member data ---------------------------

  bool       eventValid_;
  bool       isWithinTDCcut_;
  double     laserFireTime_,laserFireDelay_;

  // Parameters:
  double     TDCCutCenter_;
  double     TDCCutWindow_;
  double     timeModCeiling_;
  double     correctedTimeModCeiling_;
};

#endif // _MYEDMODULESLASERTDCALGOS
