#ifndef _MYEDMODULESMYANALUTILITIES
#define _MYEDMODULESMYANALUTILITIES

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
#include <vector>

// user include files
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"
#include "DataFormats/CaloTowers/interface/CaloTowerCollection.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"

//
// class declaration
//

class myEventData {
public:
  myEventData(const edm::ParameterSet&);
  ~myEventData() {}

  void get(const edm::Event&, const edm::EventSetup&);

  inline edm::Handle<HBHEDigiCollection>     hbhedigis(void)   const { return hbhedigis_;   }
  inline edm::Handle<HBHERecHitCollection>   hbherechits(void) const { return hbherechits_; }
  inline edm::Handle<HFRecHitCollection>     hfrechits(void)   const { return hfrechits_;   }
  inline edm::Handle<CaloTowerCollection>    towers(void)      const { return towers_;      }
  inline edm::Handle<CaloMETCollection>      recmet(void)      const { return recmet_;      }
  inline edm::Handle<edm::PCaloHitContainer> simhits(void)     const { return hsimhits_;    }

  inline uint32_t                            runNumber(void)   const { return runn_; }
  inline uint32_t                            evtNumber(void)   const { return evtn_; }

private:

  // ----------member data ---------------------------
  edm::InputTag      hbheDigiLabel_;
  edm::InputTag      hbheRechitLabel_;
  edm::InputTag      hfRechitLabel_;
  edm::InputTag      simHitLabel_;
  edm::InputTag      metLabel_;
  edm::InputTag      twrLabel_;

  edm::EventID eventId_;
  uint32_t runn_;
  uint32_t evtn_;

  edm::Handle<edm::PCaloHitContainer> hsimhits_;
  edm::Handle<HBHEDigiCollection>     hbhedigis_;
  edm::Handle<HBHERecHitCollection>   hbherechits_;
  edm::Handle<HFRecHitCollection>     hfrechits_;
  edm::Handle<CaloTowerCollection>    towers_;
  edm::Handle<CaloMETCollection>      recmet_;

  bool havedigis_;
  bool havetowers_;
  bool havemet_;
  bool havesimhits_;

};

#endif // _MYEDMODULESMYANALUTILITIES
