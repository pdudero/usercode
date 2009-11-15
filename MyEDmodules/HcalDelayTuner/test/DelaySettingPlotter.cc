// -*- C++ -*-
//
// Package:    DelaySettingPlotter
// Class:      DelaySettingPlotter
// 
/**\class DelaySettingPlotter DelaySettingPlotter.cc MyEDmodules/HcalDelayTuner/src/DelaySettingPlotter.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: DelaySettingPlotter.cc,v 1.1 2009/11/09 00:59:05 dudero Exp $
//
//


// system include files

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalDetId/interface/HcalFrontEndId.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"
#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"

#include "MyEDmodules/HcalDelayTuner/interface/HcalDelayTunerAlgos.hh"
#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerInput.hh"
#include "MyEDmodules/HcalDelayTuner/interface/SplashDelayTunerAlgos.hh"

//
// class declaration
//

class DelaySettingPlotter : public edm::EDAnalyzer {
public:
  explicit DelaySettingPlotter(const edm::ParameterSet&);
  ~DelaySettingPlotter();

private:
  //virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  int getSetting4(const DelaySettings& settings,
		  HcalSubdetector subdet,int ieta, int iphi, int depth);
  void plotHB(const DelaySettings& settings, TH2D *mapd1, TH2D *mapd2, TH1D *dist);
  void plotHE(const DelaySettings& settings, TH2D *mapd1, TH2D *mapd2, TH2D *mapd3, TH1D *dist);
  void plotHO(const DelaySettings& settings, TH2D *mapd4, TH1D *dist);

  // ----------member data ---------------------------

  HcalDelayTunerInput   *inpold_;
  HcalDelayTunerInput   *inpnew_;
  HcalLogicalMap        *lmap_;
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
DelaySettingPlotter::DelaySettingPlotter(const edm::ParameterSet& iConfig)
{
  std::cerr << "-=-=-=-=-=DelaySettingPlotter Constructor=-=-=-=-=-" << std::endl;

  edm::ParameterSet oldpars = iConfig.getParameter<edm::ParameterSet>("oldSettingParameters");
  edm::ParameterSet newpars = iConfig.getParameter<edm::ParameterSet>("newSettingParameters");

  inpold_  = new HcalDelayTunerInput(oldpars);
  inpnew_  = new HcalDelayTunerInput(newpars);
}

DelaySettingPlotter::~DelaySettingPlotter() {
  std::cerr << "-=-=-=-=-=DelaySettingPlotter Destructor=-=-=-=-=-" << std::endl;
}

//======================================================================

//
// member functions
//
//======================================================================

int
DelaySettingPlotter::getSetting4(const DelaySettings& settings,
				 HcalSubdetector subdet, 
				 int ieta, int iphi, int depth)
{
  int setting = -1;

  // no direct inverse mapping, have to do it the hard way!

  HcalDetId detID(subdet, ieta, iphi, depth);
  try {
    HcalFrontEndId feID = lmap_->getHcalFrontEndId(detID);
    HcalFrontEndId srchID(feID.rbx(),feID.rm(),0,1,0,feID.qieCard(),feID.adc());
    DelaySettings::const_iterator it = settings.find(srchID);
    if (it != settings.end())
      setting = it->second;
  }catch (...) {
  }
  return setting;
}

//======================================================================

void
DelaySettingPlotter::plotHB(const DelaySettings& settings,
			    TH2D *mapd1, TH2D *mapd2, TH1D *dist)
{
  cout <<"plotHB" << endl;
  int setting;
  DelaySettings::const_iterator it;
  for (int iphi=0; iphi < 72; iphi++) {
    for (int ieta=-16; ieta<=16; ieta++) {
      if (!ieta) continue;
      setting = getSetting4(settings,HcalBarrel,ieta,iphi,1);      // depth 1
      if (setting >=0) { mapd1->Fill(ieta,iphi,setting); dist->Fill(setting); }
    }
    for (int ieta=15; ieta<=16; ieta++) {
      setting = getSetting4(settings,HcalBarrel,ieta,iphi,2);      // depth 2, plus side
      if (setting >=0) { mapd2->Fill(ieta,iphi,setting); dist->Fill(setting); }

      setting = getSetting4(settings,HcalBarrel,-ieta,iphi,2);     // depth 2, minus side
      if (setting >=0) { mapd2->Fill(-ieta,iphi,setting); dist->Fill(setting); }
    }
  }
}

//======================================================================

void
DelaySettingPlotter::plotHE(const DelaySettings& settings,
			    TH2D *mapd1, TH2D *mapd2, TH2D *mapd3, TH1D *dist)
{
  cout <<"plotHE" << endl;
  int setting;
  DelaySettings::const_iterator it;
  for (int iphi=0; iphi < 72; iphi++) {
    for (int ieta=17; ieta<=29; ieta++) {
      setting = getSetting4(settings,HcalEndcap,-ieta,iphi,1);      // depth 1, minus side
      if (setting >=0) { mapd1->Fill(-ieta,iphi,setting); dist->Fill(setting); }

      setting = getSetting4(settings,HcalEndcap, ieta,iphi,1);      // depth 1, plus side
      if (setting >=0) { mapd1->Fill(ieta,iphi,setting); dist->Fill(setting); }

    }
    for (int ieta=18; ieta<29; ieta++) {
      setting = getSetting4(settings,HcalEndcap, ieta,iphi,2);      // depth 2, plus side
      if (setting >=0) { mapd2->Fill(ieta,iphi,setting); dist->Fill(setting); }

      setting = getSetting4(settings,HcalEndcap,-ieta,iphi,2);      // depth 2, minus side
      if (setting >=0) { mapd2->Fill(-ieta,iphi,setting); dist->Fill(setting); }
    }

    // Depth 3, ietas 16,27-29
    setting = getSetting4(settings,HcalEndcap, 16,iphi,3);
    if (setting >=0) { mapd3->Fill( 16,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap,-16,iphi,3);
    if (setting >=0) { mapd3->Fill(-16,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap, 27,iphi,3);
    if (setting >=0) { mapd3->Fill( 27,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap,-27,iphi,3);
    if (setting >=0) { mapd3->Fill(-27,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap, 28,iphi,3);
    if (setting >=0) { mapd3->Fill( 28,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap,-28,iphi,3);
    if (setting >=0) { mapd3->Fill(-28,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap, 29,iphi,3);
    if (setting >=0) { mapd3->Fill( 29,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap,-29,iphi,3);
    if (setting >=0) { mapd3->Fill(-29,iphi,setting); dist->Fill(setting); }

  } // phi loop
}

//======================================================================

void
DelaySettingPlotter::plotHO(const DelaySettings& settings,
			    TH2D *mapd4, TH1D *dist)
{
  cout <<"plotHO" << endl;
  int setting;
  DelaySettings::const_iterator it;
  for (int iphi=0; iphi < 72; iphi++) {
    for (int ieta=-15; ieta<=15; ieta++) {
      if (!ieta) continue;
      setting = getSetting4(settings,HcalOuter,ieta,iphi,4);
      if (setting >=0) { mapd4->Fill(ieta,iphi,setting); dist->Fill(setting); }
    }
  }
}

//======================================================================

// ------------ method called to for each event  ------------
void
DelaySettingPlotter::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
DelaySettingPlotter::endJob()
{
  edm::Service<TFileService> fs;

  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());
  
  DelaySettings oldsettings, newsettings;
  inpold_->getSamplingDelays(oldsettings);
  inpnew_->getSamplingDelays(newsettings);

  TH2D *oldmapd1 = fs->make<TH2D>("oldmapd1","Old Delay Settings, Depth 1;i#eta;i#phi", 61,-30.5,30.5, 72,0.5,72.5);
  TH2D *oldmapd2 = fs->make<TH2D>("oldmapd2","Old Delay Settings, Depth 2;i#eta;i#phi", 61,-30.5,30.5, 72,0.5,72.5);
  TH2D *oldmapd3 = fs->make<TH2D>("oldmapd3","Old Delay Settings, Depth 3;i#eta;i#phi", 61,-30.5,30.5, 72,0.5,72.5);
  TH2D *oldmapd4 = fs->make<TH2D>("oldmapd4","Old Delay Settings, Depth 4;i#eta;i#phi", 61,-30.5,30.5, 72,0.5,72.5);
  TH1D *olddist  = fs->make<TH1D>("olddist","Old DelaySettings Distro; Delay Setting (ns)", 25,-0.5,24.5);
  plotHB(oldsettings,oldmapd1,oldmapd2,olddist);
  plotHE(oldsettings,oldmapd1,oldmapd2,oldmapd3,olddist);
  plotHO(oldsettings,oldmapd4,olddist);
  TH2D *newmapd1 = fs->make<TH2D>("newmapd1","New Delay Settings, Depth 1;i#eta;i#phi", 61,-30.5,30.5, 72,0.5,72.5);
  TH2D *newmapd2 = fs->make<TH2D>("newmapd2","New Delay Settings, Depth 2;i#eta;i#phi", 61,-30.5,30.5, 72,0.5,72.5);
  TH2D *newmapd3 = fs->make<TH2D>("newmapd3","New Delay Settings, Depth 3;i#eta;i#phi", 61,-30.5,30.5, 72,0.5,72.5);
  TH2D *newmapd4 = fs->make<TH2D>("newmapd4","New Delay Settings, Depth 4;i#eta;i#phi", 61,-30.5,30.5, 72,0.5,72.5);
  TH1D *newdist  = fs->make<TH1D>("newdist","New DelaySettings Distro; Delay Setting (ns)", 25,-0.5,24.5);
  plotHB(newsettings,newmapd1,newmapd2,newdist);
  plotHE(newsettings,newmapd1,newmapd2,newmapd3,newdist);
  plotHO(newsettings,newmapd4,newdist);
}

//define this as a plug-in
DEFINE_FWK_MODULE(DelaySettingPlotter);
