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
// $Id: DelaySettingPlotter.cc,v 1.6 2010/02/18 21:06:12 dudero Exp $
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
  void plotHB(const DelaySettings& settings,TProfile2D*mapd1,TProfile2D*mapd2,TH1D*dist);
  void plotHE(const DelaySettings& settings,TProfile2D*mapd1,TProfile2D*mapd2,TProfile2D*mapd3,TH1D*dist);
  void plotHO(const DelaySettings& settings,TProfile2D*mapd4,TH1D*dist);
  void plotHF(const DelaySettings& settings,
	      TProfile2D*mapHFPd1,TProfile2D*mapHFPd2,
	      TProfile2D*mapHFMd1,TProfile2D*mapHFMd2,TH1D*dist);
  void printStatsPerRBX(const DelaySettings& settings);

  // ----------member data ---------------------------

  HcalDelayTunerInput   *inpold_;
  HcalDelayTunerInput   *inpnew_;
  HcalLogicalMap        *lmap_;
};

//
// constants, enums and typedefs
//

static const double hftwrRadii[] = { // in meters
  1.570-1.423,  // 41
  1.570-1.385,  // 40
  1.570-1.350,  // 39
  1.570-1.308,  // 38
  1.570-1.258,  // 37
  1.570-1.198,  // 36
  1.570-1.127,  // 35
  1.570-1.042,  // 34
  1.570-0.941,  // 33
  1.570-0.821,  // 32
  1.570-0.677,  // 31
  1.570-0.505,  // 30
  1.570-0.344   // 29
};

static const double hftwrEdges[] = {  // in meters
-(1.570- 0.207),
-(1.570-(0.505+0.344)/2.), // 29
-(1.570-(0.677+0.505)/2.), // 30
-(1.570-(0.821+0.677)/2.), // 31
-(1.570-(0.941+0.821)/2.), // 32
-(1.570-(1.042+0.941)/2.), // 33
-(1.570-(1.127+1.042)/2.), // 34
-(1.570-(1.198+1.127)/2.), // 35
-(1.570-(1.258+1.198)/2.), // 36
-(1.570-(1.308+1.258)/2.), // 37
-(1.570-(1.350+1.308)/2.), // 38
-(1.570-(1.385+1.350)/2.), // 39
-(1.570-(1.423+1.385)/2.), // 40
-(1.570-1.445),            // 41
  0.,                      // ieta
  1.570-1.445,             // 41
  1.570-(1.423+1.385)/2.,  // 40
  1.570-(1.385+1.350)/2.,  // 39
  1.570-(1.350+1.308)/2.,  // 38
  1.570-(1.308+1.258)/2.,  // 37
  1.570-(1.258+1.198)/2.,  // 36
  1.570-(1.198+1.127)/2.,  // 35
  1.570-(1.127+1.042)/2.,  // 34
  1.570-(1.042+0.941)/2.,  // 33
  1.570-(0.941+0.821)/2.,  // 32
  1.570-(0.821+0.677)/2.,  // 31
  1.570-(0.677+0.505)/2.,  // 30
  1.570-(0.505+0.344)/2.,  // 29
  1.570- 0.207
};

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
static const int BADVAL=-999;
static const int maxsetting=26;

int
DelaySettingPlotter::getSetting4(const DelaySettings& settings,
				 HcalSubdetector subdet, 
				 int ieta, int iphi, int depth)
{
  int setting = BADVAL;

  if (!HcalDetId::validDetId(subdet,ieta,iphi,depth)) return setting;

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
  if ((setting >= maxsetting) || ((setting < 0) && (setting != BADVAL))) {
    cerr << "WARNING!!! setting = " << setting << " for detId " << detID << endl;
  }
  return setting;
}

//======================================================================

void
DelaySettingPlotter::plotHB(const DelaySettings& settings,
			    TProfile2D *mapd1, TProfile2D *mapd2, TH1D *dist)
{
  cout <<"plotHB" << endl;
  int setting;
  DelaySettings::const_iterator it;
  for (int iphi=1; iphi <= 72; iphi++) {
    for (int ieta=-16; ieta<=16; ieta++) {
      if (!ieta) continue;
      setting = getSetting4(settings,HcalBarrel,ieta,iphi,1);      // depth 1
      if (setting != BADVAL) { mapd1->Fill(ieta,iphi,setting); dist->Fill(setting); }
    }
    for (int ieta=15; ieta<=16; ieta++) {
      setting = getSetting4(settings,HcalBarrel,ieta,iphi,2);      // depth 2, plus side
      if (setting != BADVAL) { mapd2->Fill(ieta,iphi,setting); dist->Fill(setting); }

      setting = getSetting4(settings,HcalBarrel,-ieta,iphi,2);     // depth 2, minus side
      if (setting != BADVAL) { mapd2->Fill(-ieta,iphi,setting); dist->Fill(setting); }
    }
  }
}                                         // DelaySettingPlotter::plotHB

//======================================================================

void
DelaySettingPlotter::plotHE(const DelaySettings& settings,
			    TProfile2D *mapd1, TProfile2D *mapd2, TProfile2D *mapd3, TH1D *dist)
{
  cout <<"plotHE" << endl;
  int setting;
  DelaySettings::const_iterator it;
  for (int iphi=1; iphi <= 72; iphi++) {
    for (int ieta=17; ieta<=29; ieta++) {
      setting = getSetting4(settings,HcalEndcap,-ieta,iphi,1);      // depth 1, minus side
      if (setting != BADVAL) { mapd1->Fill(-ieta,iphi,setting); dist->Fill(setting); }

      setting = getSetting4(settings,HcalEndcap, ieta,iphi,1);      // depth 1, plus side
      if (setting != BADVAL) { mapd1->Fill(ieta,iphi,setting); dist->Fill(setting); }

    }
    for (int ieta=18; ieta<=29; ieta++) {
      setting = getSetting4(settings,HcalEndcap, ieta,iphi,2);      // depth 2, plus side
      if (setting != BADVAL) { mapd2->Fill(ieta,iphi,setting); dist->Fill(setting); }

      setting = getSetting4(settings,HcalEndcap,-ieta,iphi,2);      // depth 2, minus side
      if (setting != BADVAL) { mapd2->Fill(-ieta,iphi,setting); dist->Fill(setting); }
    }

    // Depth 3, ietas 16,27-29
    setting = getSetting4(settings,HcalEndcap, 16,iphi,3);
    if (setting != BADVAL) { mapd3->Fill( 16,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap,-16,iphi,3);
    if (setting != BADVAL) { mapd3->Fill(-16,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap, 27,iphi,3);
    if (setting != BADVAL) { mapd3->Fill( 27,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap,-27,iphi,3);
    if (setting != BADVAL) { mapd3->Fill(-27,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap, 28,iphi,3);
    if (setting != BADVAL) { mapd3->Fill( 28,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap,-28,iphi,3);
    if (setting != BADVAL) { mapd3->Fill(-28,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap, 29,iphi,3);
    if (setting != BADVAL) { mapd3->Fill( 29,iphi,setting); dist->Fill(setting); }

    setting = getSetting4(settings,HcalEndcap,-29,iphi,3);
    if (setting != BADVAL) { mapd3->Fill(-29,iphi,setting); dist->Fill(setting); }

  } // phi loop
}                                         // DelaySettingPlotter::plotHE

//======================================================================

void
DelaySettingPlotter::plotHO(const DelaySettings& settings,
			    TProfile2D *mapd4, TH1D *dist)
{
  cout <<"plotHO" << endl;
  int setting;
  DelaySettings::const_iterator it;
  for (int iphi=1; iphi <= 72; iphi++) {
    for (int ieta=-15; ieta<=15; ieta++) {
      if (!ieta) continue;
      setting = getSetting4(settings,HcalOuter,ieta,iphi,4);
      if (setting != BADVAL) { mapd4->Fill(ieta,iphi,setting); dist->Fill(setting); }
    }
  }
}                                         // DelaySettingPlotter::plotHO

//======================================================================

void
DelaySettingPlotter::printStatsPerRBX(const DelaySettings& settings)
{
  std::string rbx("");
  const std::string filename("rbxsettingstats.csv");
  float sumsetting = 0.0;
  int   minsetting = INT_MAX;
  int   maxsetting = INT_MIN;
  int   numboxes   = 0;
  FILE *fp = fopen(filename.c_str(),"w");
  if (!fp) {
    cerr << "Couldn't open outputfile " << filename << " for writing" << endl;
    return;
  }

  DelaySettings::const_iterator it;
  for (it=settings.begin(); it!=settings.end(); it++) {
    HcalFrontEndId feID = it->first;
    int setting         = it->second;
    if (rbx != feID.rbx()) {
      if (rbx.size()) {
	// finished with an RBX, print statistics
	fprintf (fp,"%s\t%d\t%d\t%5.1f\n",
		rbx.c_str(),minsetting,maxsetting,sumsetting/(float)numboxes);
      }
      rbx = feID.rbx();
      sumsetting = 0.0;
      numboxes   = 0;
      minsetting = INT_MAX;
      maxsetting = INT_MIN;
    }
    sumsetting += (float)setting;
    if (minsetting > setting) minsetting = setting;
    if (maxsetting < setting) maxsetting = setting;
    numboxes++;
  }

  if (numboxes) {
    // the last box
    fprintf (fp,"%s\t%d\t%d\t%5.1f\n",
	    rbx.c_str(),minsetting,maxsetting,sumsetting/(float)numboxes);
  }

  cout << "Wrote outputfile " << filename<< endl;

}                               // DelaySettingPlotter::printStatsPerRBX

//======================================================================

void
DelaySettingPlotter::plotHF(const DelaySettings& settings,
			    TProfile2D *mapHFPd1, TProfile2D *mapHFPd2,
			    TProfile2D *mapHFMd1, TProfile2D *mapHFMd2,
			    TH1D *dist)
{
  cout <<"plotHF" << endl;
  int settingd1, settingd2;
  DelaySettings::const_iterator it;
  for (int iphi=1; iphi <= 71; iphi+=2) {
    for (int ieta=29; ieta<=41; ieta++) {
      double angle  = TMath::Pi()*(iphi-1)/36.;
      double radius = hftwrRadii[41-ieta];
      // Plus side
      settingd1 = getSetting4(settings,HcalForward,ieta,iphi,1);
      settingd2 = getSetting4(settings,HcalForward,ieta,iphi,2);
      if (settingd1 != BADVAL) { mapHFPd1->Fill(radius,angle,settingd1); dist->Fill(settingd1); }
      if (settingd2 != BADVAL) { mapHFPd2->Fill(radius,angle,settingd2); dist->Fill(settingd2); }

      // Minus side
      settingd1 = getSetting4(settings,HcalForward,-ieta,iphi,1);
      settingd2 = getSetting4(settings,HcalForward,-ieta,iphi,2);
      if (settingd1 != BADVAL) { mapHFMd1->Fill(radius,angle,settingd1); dist->Fill(settingd1); }
      if (settingd2 != BADVAL) { mapHFMd2->Fill(radius,angle,settingd2); dist->Fill(settingd2); }
    }
  }
}                                         // DelaySettingPlotter::plotHF

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
  inpold_->getSamplingDelays(oldsettings);  inpnew_->getSamplingDelays(newsettings);

  TProfile2D*oldmapd1=fs->make<TProfile2D>("oldmapd1","Old Delay Settings, Depth 1;i#eta;i#phi", 83,-41.5,41.5, 72,0.5,72.5);
  TProfile2D*oldmapd2=fs->make<TProfile2D>("oldmapd2","Old Delay Settings, Depth 2;i#eta;i#phi", 83,-41.5,41.5, 72,0.5,72.5);
  TProfile2D*oldmapd3=fs->make<TProfile2D>("oldmapd3","Old Delay Settings, Depth 3;i#eta;i#phi", 83,-41.5,41.5, 72,0.5,72.5);
  TProfile2D*oldmapd4=fs->make<TProfile2D>("oldmapd4","Old Delay Settings, Depth 4;i#eta;i#phi", 83,-41.5,41.5, 72,0.5,72.5);
  TH1D*olddist =fs->make<TH1D>("olddist","Old DelaySettings Distro; Delay Setting (ns)", 25,-0.5,24.5);

  // variable bin delay maps for HF
  TProfile2D*oldmapHFPd1=fs->make<TProfile2D>("oldmapHFPd1","RecHit Timing, HFP Depth 1; radius; iphi",28,hftwrEdges,36,0.0,6.28);
  TProfile2D*oldmapHFPd2=fs->make<TProfile2D>("oldmapHFPd2","RecHit Timing, HFP Depth 2; radius; iphi",28,hftwrEdges,36,0.0,6.28);
  TProfile2D*oldmapHFMd1=fs->make<TProfile2D>("oldmapHFMd1","RecHit Timing, HFM Depth 1; radius; iphi",28,hftwrEdges,36,0.0,6.28);
  TProfile2D*oldmapHFMd2=fs->make<TProfile2D>("oldmapHFMd2","RecHit Timing, HFM Depth 2; radius; iphi",28,hftwrEdges,36,0.0,6.28);

  plotHB(oldsettings,oldmapd1,oldmapd2,olddist);
  plotHE(oldsettings,oldmapd1,oldmapd2,oldmapd3,olddist);
  plotHO(oldsettings,oldmapd4,olddist);
  plotHF(oldsettings,oldmapHFPd1,oldmapHFPd2,oldmapHFMd1,oldmapHFMd2,olddist);

  TProfile2D*newmapd1=fs->make<TProfile2D>("newmapd1","New Delay Settings, Depth 1;i#eta;i#phi", 83,-41.5,41.5, 72,0.5,72.5);
  TProfile2D*newmapd2=fs->make<TProfile2D>("newmapd2","New Delay Settings, Depth 2;i#eta;i#phi", 83,-41.5,41.5, 72,0.5,72.5);
  TProfile2D*newmapd3=fs->make<TProfile2D>("newmapd3","New Delay Settings, Depth 3;i#eta;i#phi", 83,-41.5,41.5, 72,0.5,72.5);
  TProfile2D*newmapd4=fs->make<TProfile2D>("newmapd4","New Delay Settings, Depth 4;i#eta;i#phi", 83,-41.5,41.5, 72,0.5,72.5);
  TH1D*newdist =fs->make<TH1D>("newdist","New DelaySettings Distro; Delay Setting (ns)", 25,-0.5,24.5);

  // variable bin delay maps for HF
  TProfile2D*newmapHFPd1=fs->make<TProfile2D>("newmapHFPd1","RecHit Timing, HFP Depth 1; radius; iphi",28,hftwrEdges,36,0.0,6.28);
  TProfile2D*newmapHFPd2=fs->make<TProfile2D>("newmapHFPd2","RecHit Timing, HFP Depth 2; radius; iphi",28,hftwrEdges,36,0.0,6.28);
  TProfile2D*newmapHFMd1=fs->make<TProfile2D>("newmapHFMd1","RecHit Timing, HFM Depth 1; radius; iphi",28,hftwrEdges,36,0.0,6.28);
  TProfile2D*newmapHFMd2=fs->make<TProfile2D>("newmapHFMd2","RecHit Timing, HFM Depth 2; radius; iphi",28,hftwrEdges,36,0.0,6.28);

  plotHB(newsettings,newmapd1,newmapd2,newdist);
  plotHE(newsettings,newmapd1,newmapd2,newmapd3,newdist);
  plotHO(newsettings,newmapd4,newdist);
  plotHF(newsettings,newmapHFPd1,newmapHFPd2,newmapHFMd1,newmapHFMd2,newdist);

  printStatsPerRBX(newsettings);
}

//define this as a plug-in
DEFINE_FWK_MODULE(DelaySettingPlotter);
