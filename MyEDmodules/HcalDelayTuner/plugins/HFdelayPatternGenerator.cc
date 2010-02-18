// -*- C++ -*-
//
// Package:    HFdelayPatternGenerator
// Class:      HFdelayPatternGenerator
// 
/**\class HFdelayPatternGenerator HFdelayPatternGenerator.cc MyEDmodules/HFdelayPatternGenerator/src/HFdelayPatternGenerator.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HFdelayPatternGenerator.cc,v 1.1 2009/12/04 14:33:02 dudero Exp $
//
//


// system include files
#include <stdio.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"

#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerXML.hh"

//
// class declaration
//

class HFdelayPatternGenerator : public edm::EDAnalyzer {
public:
  explicit HFdelayPatternGenerator(const edm::ParameterSet&);
  ~HFdelayPatternGenerator();

private:
  //virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  void   genPhiPattern4HF    (void);

  // ----------member data ---------------------------

  HcalDelayTunerXML     *xml_;
  DelaySettings          newsettings_;
  HcalLogicalMap        *lmap_;
  FILE                  *tablefp_;

  bool writeBricks_;
  int  globalShiftNS_;
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
HFdelayPatternGenerator::HFdelayPatternGenerator(const edm::ParameterSet& iConfig)
{
  std::cerr << "-=-=-=-=-=HFdelayPatternGenerator Constructor=-=-=-=-=-" << std::endl;

  writeBricks_   = iConfig.getUntrackedParameter<bool>("writeBricks",false);
  globalShiftNS_ = iConfig.getUntrackedParameter<int>("globalShiftNS",0);
  std::string tableFilename
    = iConfig.getUntrackedParameter<std::string>("tableFilename","hfdelpatgen.csv");

  xml_ = new HcalDelayTunerXML();

  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());

  tablefp_ = fopen(tableFilename.c_str(),"w");
  if (!tablefp_)
    std::cerr << "WARNING: couldn't create file " << tableFilename << std::endl;
}

HFdelayPatternGenerator::~HFdelayPatternGenerator() {
  std::cerr << "-=-=-=-=-=HFdelayPatternGenerator Destructor=-=-=-=-=-" << std::endl;
}

//======================================================================

//
// member functions
//
//======================================================================

static const int maxlimit=26;

void
HFdelayPatternGenerator::genPhiPattern4HF(void)
{
  newsettings_.clear();
  DelaySettings::iterator  lastins = newsettings_.begin();

  if (tablefp_)
    fprintf(tablefp_,"#SD\tieta\tiphi\tdpth\tRBX\tRM\tcard\tadc\tSetting(ns)\n");

  for (int iphi = 1; iphi <= 71; iphi += 2) {
    int setting = (iphi/2 + globalShiftNS_) % 25;

    for (int ieta = 29; ieta<=33; ieta++)
      for (int depth = 1; depth <=2; depth++) {
	HcalDetId detIDplus (HcalForward, ieta,iphi,depth);
	HcalDetId detIDminus(HcalForward,-ieta,iphi,depth);
	HcalFrontEndId feIDplus  = lmap_->getHcalFrontEndId(detIDplus);
	HcalFrontEndId feIDminus = lmap_->getHcalFrontEndId(detIDminus);
	newsettings_.insert
	  (std::pair<HcalFrontEndId,int>(feIDplus,setting));
	newsettings_.insert
	  (std::pair<HcalFrontEndId,int>(feIDminus,setting));

	if (tablefp_) {
	  fprintf(tablefp_,"HF\t%3d\t%3d\t%3d\t%s\t%2d\t%3d\t%2d\t%2d\n",
		  detIDplus.ieta(),detIDplus.iphi(),detIDplus.depth(),
		  feIDplus.rbx().c_str(),feIDplus.rm(),feIDplus.qieCard(),
		  feIDplus.adc(),setting);
	  fprintf(tablefp_,"HF\t%3d\t%3d\t%3d\t%s\t%2d\t%3d\t%2d\t%2d\n",
		  detIDminus.ieta(),detIDminus.iphi(),detIDminus.depth(),
		  feIDminus.rbx().c_str(),feIDminus.rm(),feIDminus.qieCard(),
		  feIDminus.adc(),setting);
	}
      }
  }

    // Set 0's for all other channels
  for (int ieta = 34; ieta<=41; ieta++)
    for (int iphi = 1; iphi<=71; iphi++)
      for (int depth = 1; depth <=2; depth++) {
	HcalDetId detIDplus (HcalForward, ieta,iphi,depth);
	HcalDetId detIDminus(HcalForward,-ieta,iphi,depth);
	HcalFrontEndId feIDplus  = lmap_->getHcalFrontEndId(detIDplus);
	HcalFrontEndId feIDminus = lmap_->getHcalFrontEndId(detIDminus);
	newsettings_.insert(std::pair<HcalFrontEndId,int>(feIDplus,0));
	newsettings_.insert(std::pair<HcalFrontEndId,int>(feIDminus,0));
	fprintf(tablefp_,"HF\t%3d\t%3d\t%3d\t%s\t%2d\t%3d\t%2d\t 0\n",
		detIDplus.ieta(),detIDplus.iphi(),detIDplus.depth(),
		feIDplus.rbx().c_str(),feIDplus.rm(),feIDplus.qieCard(),
		feIDplus.adc());
	fprintf(tablefp_,"HF\t%3d\t%3d\t%3d\t%s\t%2d\t%3d\t%2d\t 0\n",
		detIDminus.ieta(),detIDminus.iphi(),detIDminus.depth(),
		feIDminus.rbx().c_str(),feIDminus.rm(),feIDminus.qieCard(),
		feIDminus.adc());
      }
}                           // HFdelayPatternGenerator::genPhiPattern4HF

//======================================================================

// ------------ method called to for each event  ------------
void
HFdelayPatternGenerator::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HFdelayPatternGenerator::endJob() {
  genPhiPattern4HF();
  if (writeBricks_)
    xml_->writeDelayBricks(newsettings_);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HFdelayPatternGenerator);
