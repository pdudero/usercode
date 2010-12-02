// -*- C++ -*-
//
// Package:    TimingStabilityAnalyzer
// Class:      TimingStabilityAnalyzer
// 
/**\class TimingStabilityAnalyzer TimingStabilityAnalyzer.cc MyEDmodules/TimingStabilityAnalyzer/src/TimingStabilityAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Phil Dudero
//         Created:  Fri Jun 25 09:40:22 CDT 2010
// $Id$
//
//


// system include files
#include <memory>
#include <map>
#include <set>
#include <strstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CommonTools/Utils/interface/TFileDirectory.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/HcalRecHit/interface/HBHERecHit.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"

#include "TH2F.h"

inline string int2str(int i) {
  std::ostringstream ss;
  ss << i;
  return ss.str();
}

//
// class declaration
//

class TimingStabilityAnalyzer : public edm::EDAnalyzer {
public:
  explicit TimingStabilityAnalyzer(const edm::ParameterSet&);
  ~TimingStabilityAnalyzer() {}


private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void bookHistosForRun(int runnumber);
  int lookupRBX(const HcalDetId& detId);
  bool buildMaskSet(const HcalSubdetector subdet,
		    const std::vector<int>& v_idnumbers);

      // ----------member data ---------------------------
  myEventData  *eventData_;
  std::set<int>  detIds2mask_;

  std::set<uint32_t> runnumbers_;
  std::map<uint32_t,TH2F *> m_h2dTimingPerRun_ [2][2];  // one each for HB,HE,minus,plus

  HcalLogicalMap *lmap_;

};


//
// static data member definitions
//

//
// constructors and destructor
//
TimingStabilityAnalyzer::TimingStabilityAnalyzer(const edm::ParameterSet& iConfig)

{
  edm::ParameterSet edPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("eventDataPset");
  eventData_ = new myEventData(edPset);

  std::vector<int> v_HBmaskidnumbers =
    iConfig.getParameter<vector<int> >("HBdetIds2mask");

  std::vector<int> v_HEmaskidnumbers =
    iConfig.getParameter<vector<int> >("HEdetIds2mask");

  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());

  TH1::SetDefaultSumw2();

  if (!buildMaskSet(HcalBarrel,v_HBmaskidnumbers))
    throw cms::Exception("Invalid HB detID vector");

  if (!buildMaskSet(HcalEndcap,v_HEmaskidnumbers))
    throw cms::Exception("Invalid HE detID vector");

  if (detIds2mask_.size()) {
    cout << "Masked channels:"<<endl;
    set<int>::const_iterator it;
    for (it=detIds2mask_.begin(); it!=detIds2mask_.end(); it++)
      cout << HcalDetId::detIdFromDenseIndex(*it) << ", ";
    cout << endl;
  }

}

//
// member functions
//
void
TimingStabilityAnalyzer::bookHistosForRun(int runnumber)
{
  edm::Service<TFileService> fs;
  TH2F *h2f;

  string name  = "run"+int2str(runnumber)+"HBM";
  string title = "HBM Timing for Run Number , "+int2str(runnumber)+"; RBX#; Hit Time (ns)";
  h2f          = fs->make <TH2F> (name.c_str(),title.c_str(),18,0.5,18.5,401,-100.25,100.25);
  m_h2dTimingPerRun_[0][0].insert(std::pair<int,TH2F *>(runnumber,h2f));

  name  = "run"+int2str(runnumber)+"HBP";
  title = "HBP Timing for Run Number , "+int2str(runnumber)+"; RBX#; Hit Time (ns)";
  h2f   = fs->make <TH2F> (name.c_str(),title.c_str(),18,0.5,18.5,401,-100.25,100.25);
  m_h2dTimingPerRun_[0][1].insert(std::pair<int,TH2F *>(runnumber,h2f));

  name  = "run"+int2str(runnumber)+"HEM";
  title = "HEM Timing for Run Number , "+int2str(runnumber)+"; RBX#; Hit Time (ns)";
  h2f   = fs->make <TH2F> (name.c_str(),title.c_str(),18,0.5,18.5,401,-100.25,100.25);
  m_h2dTimingPerRun_[1][0].insert(std::pair<int,TH2F *>(runnumber,h2f));

  name  = "run"+int2str(runnumber)+"HEP";
  title = "HEP Timing for Run Number , "+int2str(runnumber)+"; RBX#; Hit Time (ns)";
  h2f   = fs->make <TH2F> (name.c_str(),title.c_str(),18,0.5,18.5,401,-100.25,100.25);
  m_h2dTimingPerRun_[1][1].insert(std::pair<int,TH2F *>(runnumber,h2f));
}

int
TimingStabilityAnalyzer::lookupRBX(const HcalDetId& detId)
{
  HcalFrontEndId feID = lmap_->getHcalFrontEndId(detId);

  return ((feID.rbxIndex() % 18)+1);
}

//======================================================================

bool
TimingStabilityAnalyzer::buildMaskSet(const HcalSubdetector subdet,
				      const std::vector<int>& v_idnumbers)
{
  // convert det ID numbers to valid detIds:
  if (v_idnumbers.size()%3) {
    return false;
  }

  // 99 = wildcard
  for (uint32_t i=0; i<v_idnumbers.size(); i+=3) {
    int ieta    = v_idnumbers[i];
    int iphi    = v_idnumbers[i+1];
    int depth   = v_idnumbers[i+2];
    int minieta = ieta;
    int maxieta = ieta;
    int miniphi = iphi;
    int maxiphi = iphi;
    int mindpth = depth;
    int maxdpth = depth;
    int phiskip = 1;

    switch(subdet) {
    case HcalBarrel:
      if (depth==99)   { mindpth=1;   maxdpth=2;  }
      if (ieta==99)    { minieta=1;   maxieta=16; } else
	if (ieta==-99) { minieta=-16; maxieta=-1; }
      if (iphi==99)    { miniphi=1;   maxiphi=72; }
      break;
    case HcalEndcap:
      if (depth==99)   { mindpth=1;   maxdpth=3;  }
      if (ieta==99)    { minieta=16;  maxieta=29; } else
	if (ieta==-99) { minieta=-29; maxieta=-16;}
      if (iphi==99)    { miniphi=1;   maxiphi=72; }
      break;
    case HcalForward:
      if (depth==99)   { mindpth=1;   maxdpth=2;  }
      if (ieta==99)    { minieta=29;  maxieta=41; } else
	if (ieta==-99) { minieta=-41; maxieta=-29;}
      if (iphi==99)    { miniphi=1;   maxiphi=71; phiskip=2; }
      break;
    default:  return false;
    }

    for (int depth=mindpth; depth<=maxdpth; depth++) {
      for (int ieta=minieta; ieta<=maxieta; ieta++) {
	for (int iphi=miniphi; iphi<=maxiphi; iphi+=phiskip) {
	  if (!HcalDetId::validDetId(subdet,ieta,iphi,depth)) continue;
	  detIds2mask_.insert(HcalDetId(subdet,ieta,iphi,depth).hashed_index());
	}
      }
    }
  }

  // see if all the entries were invalid
  if (v_idnumbers.size() && !detIds2mask_.size()) return false;

  return true;
}                                   // HcalTimingAnalAlgos::buildMaskSet

// ------------ method called to for each event  ------------
void
TimingStabilityAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  eventData_->get(iEvent,iSetup);

  uint32_t runn = eventData_->runNumber();

  if (notInSet<uint32_t>(runnumbers_,runn)) {
    cout << "Run number " << runn << endl;
    bookHistosForRun(runn);
    runnumbers_.insert(runn);
  }

  const edm::SortedCollection<HBHERecHit>& rechits = *(eventData_->hbherechits());

  for (unsigned irh=0; irh < rechits.size(); ++irh) {
    const HBHERecHit& rh  = rechits[irh];
    if (rh.energy() > 20.0) {
      const HcalDetId detId = rh.id();
      if (inSet<int>(detIds2mask_,detId.hashed_index())) continue;
      int rbxnum = lookupRBX(detId);
      int zsdi  = (detId.zside()==-1) ? 0 : 1;
      int sbdi  = (int)detId.subdet()-1; // HB=0, HE=1
      m_h2dTimingPerRun_[sbdi][zsdi][runn]->Fill(rbxnum, rh.time());
    }
  }
}

//define this as a plug-in
DEFINE_FWK_MODULE(TimingStabilityAnalyzer);
