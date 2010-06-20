#ifndef  _MYEDMODULESLASERTIMINGANALALGOS
#define  _MYEDMODULESLASERTIMINGANALALGOS

// -*- C++ -*-
//
// Package:    LaserTimingAnalAlgos
// Class:      LaserTimingAnalAlgos
// 
/**\class LaserTimingAnalAlgos LaserTimingAnalAlgos.cc MyEDmodules/LaserTimingAnalAlgos/src/LaserTimingAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserTimingAnalAlgos.hh,v 1.10 2010/05/05 23:38:39 dudero Exp $
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
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"

#include "HcalTimingAnalAlgos.hh"
#include "LaserHitTimeCorrectorTDC.hh"

//
// class declaration
//

class LaserTimingAnalAlgos : public HcalTimingAnalAlgos {
public:
  LaserTimingAnalAlgos(const edm::ParameterSet&);
  ~LaserTimingAnalAlgos() {}
  void process(const myEventData& ed);
  void beginJob(const edm::EventSetup&,const myEventData&);
  void endAnal();

private:
  template<class Digi>
  void processZDCDigi             (const Digi& df);
  template<class Digi>
  void    processDigi             (const Digi& df);
  template<class RecHit, class Digi >
  void    processRecHitsAndDigis  (const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle,
				   const edm::Handle<edm::SortedCollection<Digi>   >& digihandle);

  // ----------member data ---------------------------
  // Parameters:
  int                nlaserEv_;
  std::vector<std::string> v_cuts_;             // vector of cut strings

  // The collection of names of histos per subdetector
  std::string                   TDCLaserFireTime_;

  TFileDirectory               *mysubdetRootDir_;

  LaserHitTimeCorrectorTDC     *TDCalgo_;
};

#endif // _MYEDMODULESLASERTIMINGANALALGOS
