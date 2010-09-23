// -*- C++ -*-
//
// Package:    TimeCorrDumper
// Class:      TimeCorrDumper
// 
/**\class TimeCorrDumper TimeCorrDumper.cc test/TimeCorrDumper/src/TimeCorrDumper.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// $Id: TimeCorrDumper.cc,v 1.22 2008/11/10 10:13:20 rofierzy Exp $
//
//


// system include files
#include <memory>
#include <time.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"

#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalCalibrationWidths.h"
#include "CondFormats/HcalObjects/interface/HcalQIEShape.h"
#include "CondFormats/HcalObjects/interface/HcalQIECoder.h"
#include "CondFormats/HcalObjects/interface/HcalElectronicsMap.h"

#include "CalibFormats/HcalObjects/interface/HcalText2DetIdConverter.h"
#include "DataFormats/HcalDetId/interface/HcalGenericDetId.h"
#include "DataFormats/HcalDetId/interface/HcalTrigTowerDetId.h"
#include "DataFormats/HcalDetId/interface/HcalCalibDetId.h"
#include "DataFormats/HcalDetId/interface/HcalZDCDetId.h"
#include "DataFormats/HcalDetId/interface/HcalElectronicsId.h"

#include "MyEDmodules/MyAnalUtilities/interface/HcalDetIdGenerator.hh"

static const char *subdetstr[]= {
  "EMPTY", "HB", "HE", "HO", "HF", "HT", "OTH"
};

namespace {
  std::ostream& operator<<(std::ostream& fOut, const DetId& id) {
    if (id.null ()) fOut << "NULL";
    else if (id.det() == DetId:: Calo && id.subdetId()==HcalZDCDetId::SubdetectorId) {
      fOut << HcalZDCDetId (id);
    }
    else if (id.det () != DetId::Hcal)  fOut << "Not HCAL";
    else {
      HcalSubdetector sub = HcalSubdetector (id.subdetId());
      if (sub == HcalBarrel || sub == HcalEndcap || sub == HcalOuter || sub == HcalForward)
	fOut << HcalDetId (id);
      else if (sub == HcalEmpty) fOut << "EMPTY";
      else if (sub == HcalTriggerTower) fOut << HcalTrigTowerDetId (id);
      else if (sub == HcalOther) {
	HcalOtherDetId osub (id);
	HcalOtherSubdetector odetid = osub.subdet();
	if (odetid == HcalCalibration)  fOut << HcalCalibDetId (id);
	else fOut << "Unknown subtype";
      }
      else fOut << "Unknown type";
    }
    return fOut;
  }
}

class TimeCorrDumper : public edm::EDAnalyzer {
   public:
      explicit TimeCorrDumper( const edm::ParameterSet& );
      ~TimeCorrDumper ();


      virtual void analyze( const edm::Event&, const edm::EventSetup& );
   private:
      // ----------member data ---------------------------

  std::vector<uint32_t> v_ids_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
TimeCorrDumper::TimeCorrDumper( const edm::ParameterSet& iConfig )
{
   //now do what ever initialization is needed

  HcalDetIdGenerator idgen;
  idgen.appendDetIds4subdet(HcalBarrel,v_ids_);
  idgen.appendDetIds4subdet(HcalEndcap,v_ids_);
  idgen.appendDetIds4subdet(HcalForward,v_ids_);
  idgen.appendDetIds4subdet(HcalOuter,v_ids_);
}


TimeCorrDumper::~TimeCorrDumper()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
TimeCorrDumper::analyze( const edm::Event& iEvent, const edm::EventSetup& iSetup )
{
  std::cout << "TimeCorrDumper::analyze->..." << std::endl;
  edm::ESHandle<HcalDbService> pSetup;
  iSetup.get<HcalDbRecord>().get( pSetup );

  for (size_t i=0; i<v_ids_.size(); i++) {
    if (!HcalDetId::validDenseIndex(v_ids_[i])) continue;
    HcalDetId detid = HcalDetId::detIdFromDenseIndex(v_ids_[i]);
  
    const HcalCalibrations& calibs=pSetup->getHcalCalibrations(detid);

    printf("              %3d              %2d               %d              %s   %9.5f     %X\n",
	   detid.ieta(), detid.iphi(), detid.depth(), subdetstr[detid.subdet()],
	   calibs.timecorr(), detid.rawId());

  }
}

//define this as a plug-in
DEFINE_FWK_MODULE(TimeCorrDumper);
