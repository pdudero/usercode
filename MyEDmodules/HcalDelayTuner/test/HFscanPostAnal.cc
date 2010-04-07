// -*- C++ -*-
//
// Package:    HFscanPostAnal
// Class:      HFscanPostAnal
// 
/**\class HFscanPostAnal HFscanPostAnal.cc MyEDmodules/HcalDelayTuner/src/HFscanPostAnal.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HFscanPostAnal.cc,v 1.1 2010/04/06 09:31:58 dudero Exp $
//
//


// system include files
#include <string>
#include <map>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalDetId/interface/HcalFrontEndId.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"
#include "CommonTools/Utils/interface/TFileDirectory.h"
#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerXML.hh"

#include "TMath.h"
#include "TFile.h"
#include "TH2I.h"
#include "TProfile2D.h"
#include "TGraph.h"

//
// class declaration
//

class HFscanPostAnal : public edm::EDAnalyzer {
public:
  explicit HFscanPostAnal(const edm::ParameterSet&);
  ~HFscanPostAnal();

private:
  //virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  void collectOptimizedDigis(TH2I *hoptset,int depth);
  void genSettingTable(TH2I *optdelays,int depth);

  void graphSettingsPerID(TProfile2D *tp,int depth,int delay);

  // ----------member data ---------------------------

  HcalDelayTunerXML *xml_;
  DelaySettings      optsettings_;
  HcalLogicalMap    *lmap_;
  bool               writeBricks_;

  std::map<int,TFile *> m_filesPerSetting_;
  TProfile2D *tphfpd1opt_;
  TProfile2D *tphfpd2opt_;
  TProfile2D *tphfmd1opt_;
  TProfile2D *tphfmd2opt_;

  TH2I       *optdel_hfpd1_;
  TH2I       *optdel_hfpd2_;
  TH2I       *optdel_hfmd1_;
  TH2I       *optdel_hfmd2_;

  TFileDirectory *graphdir_;
  std::map<std::string,TGraph *> m_graphsPerID_;

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

inline unsigned int str2int(const std::string& str) {
  return (unsigned int)strtoul(str.c_str(),NULL,0);
}

//======================================================================

using namespace std;

void genAdditionalProfiles(TH2 *tp, int depth)
{
  edm::Service<TFileService> fs;

  string name = string(tp->GetName())+"_pol";
  string title;

  TProfile2D *tppolar=fs->make<TProfile2D>(name.c_str(),tp->GetTitle(),
					   28,hftwrEdges,36,0.0,6.28);

  name  = string(tp->GetName())+"_ieta";
  title = string(tp->GetTitle())+"; i#eta";

  TProfile *tpieta= fs->make<TProfile>(name.c_str(),title.c_str(),13,28.5,41.5);

  name  = string(tp->GetName())+"_iphi";
  title = string(tp->GetTitle())+"; i#phi";
  TProfile *tpiphi= fs->make<TProfile>(name.c_str(),title.c_str(),73,-0.5,72.5);

  name = string(tp->GetName())+"_1D";
  TH1D *h1d;
  double max = tp->GetMaximum();
  if (max < 1.5) {
    title = string(tp->GetTitle())+"; TS3/(TS3+TS4)";
    h1d= fs->make<TH1D>(name.c_str(),title.c_str(),25,0.0,1.0); // ratio plot
  }
  else {
    title = string(tp->GetTitle())+"; Setting (ns)";
    h1d= fs->make<TH1D>(name.c_str(),title.c_str(),25,-0.5,24.5); // setting plot
  }

  for (int ibinx=1; ibinx<=tp->GetNbinsX(); ibinx++) {
    for (int ibiny=1; ibiny<=tp->GetNbinsY(); ibiny++) {
      double dieta   = tp->GetXaxis()->GetBinCenter(ibinx);
      double diphi   = tp->GetYaxis()->GetBinCenter(ibiny);
      if (!HcalDetId::validDetId(HcalForward,(int)dieta,(int)diphi,depth)) continue;
      double dangle  = TMath::Pi()*(diphi-1.)/36.;
      double dradius = hftwrRadii[41-abs((int)dieta)];
      double value   = tp->GetBinContent(ibinx,ibiny);
      tppolar->Fill(dradius,dangle,value);

      if (value != 0.0)    h1d->Fill(value);
      if (value != 0.0)	tpieta->Fill(fabs(dieta),value);
      if (value != 0.0)	tpiphi->Fill(diphi,value);
    }
  }
}                                               // genAdditionalProfiles

//======================================================================

void refineTimesAndDelays(TProfile2D *p2opt,
			  TProfile2D *p2new,
			  TH2I       *optdelays,
			  int         delay)
{
  for (int ibinx=1; ibinx<=p2opt->GetNbinsX(); ibinx++) {
    for (int ibiny=1; ibiny<=p2opt->GetNbinsY(); ibiny++) {
      if (!p2new->GetBinEntries(p2opt->GetBin(ibinx,ibiny))) continue;

      double newratio = p2new->GetBinContent(ibinx,ibiny);
      double curratio = p2opt->GetBinContent(ibinx,ibiny);

      if (p2opt->GetBinEntries(p2opt->GetBin(ibinx,ibiny)))
	curratio = p2opt->GetBinContent(ibinx,ibiny);
      else
	curratio = -9e99;
      if (fabs(newratio-0.5) < fabs(curratio-0.5)) {
	p2opt->SetBinContent(ibinx,ibiny,newratio);
	optdelays->SetBinContent(ibinx,ibiny,delay);
      }
    }
  } // histo bin loop
}

//======================================================================

template<class T>
inline int sign(T _x) { return (_x>=0)? 1:-1; }

void patchRing29(TH2I *tp2)
{
  int zside = sign<double>(tp2->GetXaxis()->GetBinCenter(tp2->GetBin(1,1)));
  int ibinx29 = (zside>0) ? 1 : 13;

  for (int ibiny=1; ibiny<=tp2->GetNbinsY(); ibiny++) {
    int delay = (int)tp2->GetBinContent(ibinx29+zside,ibiny);
    tp2->SetBinContent(ibinx29,ibiny,delay);
  }
}

//======================================================================

void zeroize(TH2I *key)
{
  for (int ibin=1; ibin<=key->GetNbinsX()*key->GetNbinsY(); ibin++)
    key->SetBinContent(ibin,0);
}

//
// static data member definitions
//
//======================================================================

//
// constructors and destructor
//
HFscanPostAnal::HFscanPostAnal(const edm::ParameterSet& iConfig)
{
  edm::Service<TFileService> fs;

  std::cerr << "-=-=-=-=-=HFscanPostAnal Constructor=-=-=-=-=-" << std::endl;

  writeBricks_ = iConfig.getUntrackedParameter<bool>("writeBricks",false);

  std::vector<std::string> fnamesPerSetting =
    iConfig.getParameter<std::vector<std::string> >("fileVectorOnePerSetting");

  for (uint32_t i=0; i<fnamesPerSetting.size(); i++) {
    int delay=-1;
    std::string& fname = fnamesPerSetting[i];
    size_t pos1 = fname.find_first_of("=");
    if (pos1 != string::npos) {
      size_t pos2 = fname.find_first_of(".",pos1+1);
      if (pos2 != string::npos) 
	delay = str2int(fname.substr(pos1+1,pos2-pos1-1));
    }
    if (delay==-1) {
      cerr << "Wrong format filename " << fname;
      cerr << ", need delay setting to be embedded in the name between '=' and '.'";
      exit(-1);
    }
    TFile *rootfile = new TFile(fnamesPerSetting[i].c_str());
    if (rootfile->IsZombie()) {
      cerr << "File failed to open, " << fnamesPerSetting[i] << endl;
      exit(-1);
    }
    
    m_filesPerSetting_.insert(std::pair<int,TFile *>(delay,rootfile));
  }

  optdel_hfpd1_=fs->make<TH2I>("h2d_optDelaysHFPd1","HFP depth 1 Optimal Settings",13, 28.5, 41.5,72,0.5,72.5);
  optdel_hfpd2_=fs->make<TH2I>("h2d_optDelaysHFPd2","HFP depth 2 Optimal Settings",13, 28.5, 41.5,72,0.5,72.5);
  optdel_hfmd1_=fs->make<TH2I>("h2d_optDelaysHFMd1","HFM depth 1 Optimal Settings",13,-41.5,-28.5,72,0.5,72.5);
  optdel_hfmd2_=fs->make<TH2I>("h2d_optDelaysHFMd2","HFM depth 2 Optimal Settings",13,-41.5,-28.5,72,0.5,72.5);

  zeroize(optdel_hfpd1_);
  zeroize(optdel_hfpd2_);
  zeroize(optdel_hfpd1_);
  zeroize(optdel_hfpd1_);

  graphdir_  = new TFileDirectory (fs->mkdir("SettingGraphsPerID"));
}

HFscanPostAnal::~HFscanPostAnal() {
  std::cerr << "-=-=-=-=-=HFscanPostAnal Destructor=-=-=-=-=-" << std::endl;
}

//======================================================================

TProfile2D *getProfile(TFile *rootfile,const std::string& name, int delay)
{
  TProfile2D *tp = (TProfile2D *)rootfile->Get(name.c_str());
  if (!tp) {
    cerr << "couldn't find " << name.c_str() << " in delay=" << delay << " file" << endl;
    exit(-1);
  }
  return tp;
}

//======================================================================

void
HFscanPostAnal::collectOptimizedDigis(TH2I *hoptset,int depth)
{
  edm::Service<TFileService> fs;
  TFileDirectory outdigidir = fs->mkdir("OptDigisGeVperID");

  for (int ibinx=1; ibinx<=hoptset->GetNbinsX(); ibinx++) {
    for (int ibiny=1; ibiny<=hoptset->GetNbinsY(); ibiny++) {
      double dieta   = hoptset->GetXaxis()->GetBinCenter(ibinx);
      double diphi   = hoptset->GetYaxis()->GetBinCenter(ibiny);
      if (!HcalDetId::validDetId(HcalForward,(int)dieta,(int)diphi,depth)) continue;
      stringstream detIDstr;
      detIDstr << HcalDetId(HcalForward,(int)dieta,(int)diphi,depth);

      int    optset  = (int)hoptset->GetBinContent(ibinx,ibiny);

      TProfile *hdigi=0;
      std::map<int,TFile *>::const_iterator it = m_filesPerSetting_.find(optset);

      if (it != m_filesPerSetting_.end()) {
	std::string hdigipath = "hftimeanal/HF/cut4ncHits/DigisGeVperID/"+detIDstr.str();
	hdigi = (TProfile *)it->second->Get(hdigipath.c_str());
      }

      if (!hdigi) {
	cerr << "Couldn't get DetID " << detIDstr.str() << " for " << hoptset->GetName() << endl;
	continue;
      }
      outdigidir.make<TProfile>(*hdigi);
    }
  }
}                               // HFscanPostAnal::collectOptimizedDigis

//======================================================================

void HFscanPostAnal::graphSettingsPerID(TProfile2D *tp,int depth,int delay)
{
  for (int ibinx=1; ibinx<=tp->GetNbinsX(); ibinx++) {
    for (int ibiny=1; ibiny<=tp->GetNbinsY(); ibiny++) {
      if (!tp->GetBinEntries(tp->GetBin(ibinx,ibiny))) continue;

      double ratio   = tp->GetBinContent(ibinx,ibiny);
      double dieta   = tp->GetXaxis()->GetBinCenter(ibinx);
      double diphi   = tp->GetYaxis()->GetBinCenter(ibiny);
      if (!HcalDetId::validDetId(HcalForward,(int)dieta,(int)diphi,depth)) continue;
      stringstream detIDstr;
      detIDstr << HcalDetId(HcalForward,(int)dieta,(int)diphi,depth);

      TGraph *gr;
      std::map<std::string,TGraph *>::const_iterator it = m_graphsPerID_.find(detIDstr.str());
      if (it == m_graphsPerID_.end()) {
	gr = graphdir_->make<TGraph>();
	gr->SetName(detIDstr.str().c_str());
	gr->SetTitle(detIDstr.str().c_str());
	m_graphsPerID_.insert(std::pair<std::string,TGraph *>(detIDstr.str(),gr));
      } else 
	gr = it->second;

      gr->SetPoint(gr->GetN(),delay,ratio);
    }
  }
}                                  // HFscanPostAnal::graphSettingsPerID

//======================================================================

void
HFscanPostAnal::genSettingTable(TH2I *optdelays,int depth)
{
  string fname = string(optdelays->GetName())+".csv";
  FILE *fp=fopen(fname.c_str(),"w");

  if (!fp) {
    cerr << "Failed to open " << fname.c_str() <<" for writing" << endl;
    return;
  }

  fprintf(fp,"SD\tieta\tiphi\tdpth\tRBX\tRM\tcard\tadc\tAvgT(ns)\n");
 
  for (int ibinx=1; ibinx<=optdelays->GetNbinsX(); ibinx++) {
    for (int ibiny=1; ibiny<=optdelays->GetNbinsY(); ibiny++) {
      int ieta  = (int)optdelays->GetXaxis()->GetBinCenter(ibinx);
      int iphi  = (int)optdelays->GetYaxis()->GetBinCenter(ibiny);
      int delay = (int)optdelays->GetBinContent(ibinx,ibiny);
      if (!HcalDetId::validDetId(HcalForward,ieta,iphi,depth)) continue;

      HcalDetId detID(HcalForward,ieta,iphi,depth);
      HcalFrontEndId feID = lmap_->getHcalFrontEndId(detID);
#if 0
      if (abs(ieta) == 29) // take the setting for abs(ieta)=30, same phi
	delay = (int)optdelays->GetBinContent(ibinx+sign(ieta),ibiny);
#endif
      optsettings_.insert
	  (std::pair<HcalFrontEndId,int>(feID,delay));

      fprintf(fp,"HF\t%3d\t%3d\t%3d\t%s\t%2d\t%3d\t%2d\t%2d\n",
	      detID.ieta(),detID.iphi(),detID.depth(),
	      feID.rbx().c_str(),feID.rm(),feID.qieCard(),feID.adc(),
	      delay);
    }
  }
  fclose(fp);
}                                     // HFscanPostAnal::genSettingTable

//======================================================================

// ------------ method called to for each event  ------------
void
HFscanPostAnal::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HFscanPostAnal::endJob()
{
  edm::Service<TFileService> fs;

  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());

  std::map<int,TFile *>::const_iterator it;
  for (it = m_filesPerSetting_.begin();
       it!= m_filesPerSetting_.end();it++) {
    int    delay    = it->first;
    TFile *rootfile = it->second;

    cout << "Processing delay=" << delay << endl;

    std::string st_hfpd1("hftimeanal/HF/cut4ncHits/p2d_TS34ratioProfHFPd1");
    std::string st_hfpd2("hftimeanal/HF/cut4ncHits/p2d_TS34ratioProfHFPd2");
    std::string st_hfmd1("hftimeanal/HF/cut4ncHits/p2d_TS34ratioProfHFMd1");
    std::string st_hfmd2("hftimeanal/HF/cut4ncHits/p2d_TS34ratioProfHFMd2");

    TProfile2D *tphfpd1 = getProfile(rootfile,st_hfpd1, it->first);
    TProfile2D *tphfpd2 = getProfile(rootfile,st_hfpd2, it->first);
    TProfile2D *tphfmd1 = getProfile(rootfile,st_hfmd1, it->first);
    TProfile2D *tphfmd2 = getProfile(rootfile,st_hfmd2, it->first);

    if (it == m_filesPerSetting_.begin()) {
      tphfpd1opt_=fs->make<TProfile2D>(*tphfpd1);
      tphfpd2opt_=fs->make<TProfile2D>(*tphfpd2);
      tphfmd1opt_=fs->make<TProfile2D>(*tphfmd1);
      tphfmd2opt_=fs->make<TProfile2D>(*tphfmd2);

      tphfpd1opt_->SetTitle("HFP depth 1 Optimized Ratios");
      tphfpd2opt_->SetTitle("HFP depth 2 Optimized Ratios");
      tphfmd1opt_->SetTitle("HFM depth 1 Optimized Ratios");
      tphfmd2opt_->SetTitle("HFM depth 2 Optimized Ratios");
      continue;
    }
    refineTimesAndDelays(tphfpd1opt_,tphfpd1,optdel_hfpd1_,delay);
    refineTimesAndDelays(tphfpd2opt_,tphfpd2,optdel_hfpd2_,delay);
    refineTimesAndDelays(tphfmd1opt_,tphfmd1,optdel_hfmd1_,delay);
    refineTimesAndDelays(tphfmd2opt_,tphfmd2,optdel_hfmd2_,delay);

    graphSettingsPerID(tphfpd1,1,delay);
    graphSettingsPerID(tphfpd2,2,delay);
    graphSettingsPerID(tphfmd1,1,delay);
    graphSettingsPerID(tphfmd2,2,delay);
  } // delay file loop

  // make additional profiles
  //
  genAdditionalProfiles(tphfpd1opt_,1);
  genAdditionalProfiles(tphfpd2opt_,2);
  genAdditionalProfiles(tphfmd1opt_,1);
  genAdditionalProfiles(tphfmd2opt_,2);

  patchRing29(optdel_hfpd1_);
  patchRing29(optdel_hfpd2_);
  patchRing29(optdel_hfmd1_);
  patchRing29(optdel_hfmd2_);

  genAdditionalProfiles(optdel_hfpd1_,1);
  genAdditionalProfiles(optdel_hfpd2_,2);
  genAdditionalProfiles(optdel_hfmd1_,1);
  genAdditionalProfiles(optdel_hfmd2_,2);

  collectOptimizedDigis(optdel_hfpd1_,1);
  collectOptimizedDigis(optdel_hfpd2_,2);
  collectOptimizedDigis(optdel_hfmd1_,1);
  collectOptimizedDigis(optdel_hfmd2_,2);

  // generate settings per feID.
  //
  optsettings_.clear();
  genSettingTable(optdel_hfpd1_,1);
  genSettingTable(optdel_hfpd2_,2);
  genSettingTable(optdel_hfmd1_,1);
  genSettingTable(optdel_hfmd2_,2);

  if (writeBricks_) {
    HcalDelayTunerXML *xml = new HcalDelayTunerXML();
    xml->writeDelayBricks(optsettings_);
  }

} // HFscanPostAnal::endJob

//define this as a plug-in
DEFINE_FWK_MODULE(HFscanPostAnal);
