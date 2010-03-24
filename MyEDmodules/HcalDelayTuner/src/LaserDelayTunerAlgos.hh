#ifndef  _MYEDMODULESLASERDELAYTUNERALGOS
#define  _MYEDMODULESLASERDELAYTUNERALGOS

// -*- C++ -*-
//
// Package:    LaserDelayTunerAlgos
// Class:      LaserDelayTunerAlgos
// 
/**\class LaserDelayTunerAlgos LaserDelayTunerAlgos.cc MyEDmodules/LaserDelayTunerAlgos/src/LaserDelayTunerAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserDelayTunerAlgos.hh,v 1.5 2010/03/14 22:41:34 dudero Exp $
//
//


// system include files
#include <string>
#include <vector>

// user include files
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"
#include "DataFormats/Common/interface/SortedCollection.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"

#include "MyEDmodules/HcalDelayTuner/interface/HcalDelayTunerAlgos.hh"
#include "LaserDelayTunerTDCalgos.hh"

//
// class declaration
//

class LaserDelayTunerAlgos : public HcalDelayTunerAlgos {
public:
  LaserDelayTunerAlgos(const edm::ParameterSet&);
  ~LaserDelayTunerAlgos() {}
  void process(const myEventData& ed);
  void beginJob(const edm::EventSetup&);
  void endAnal();

private:
  template<class Digi>
  void processZDCDigi             (const Digi& df);
  template<class Digi>
  void    processDigi             (const Digi& df);
  template<class RecHit, class Digi >
  void    processRecHitsAndDigis  (const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle,
				   const edm::Handle<edm::SortedCollection<Digi>   >& digihandle);

  void    detChannelTimes        (TimesPerDetId& chtimes);

  // ----------member data ---------------------------
  // Parameters:
  int                nlaserEv_;

  // The collection of names of histos per subdetector
  std::string TDCLaserFireTime_;

  LaserDelayTunerTDCalgos      *TDCalgo_;
};

#endif // _MYEDMODULESLASERDELAYTUNERALGOS
