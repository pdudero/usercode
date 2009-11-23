
// -*- C++ -*-
//
// Package:    SplashDelayTunerAlgos
// Class:      SplashDelayTunerAlgos
// 
/**\class SplashDelayTunerAlgos SplashDelayTunerAlgos.cc MyEDmodules/SplashDelayTunerAlgos/src/SplashDelayTunerAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: SplashDelayTunerAlgos.cc,v 1.4 2009/11/20 19:12:48 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>
#include <math.h> // floor

// user include files
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"
#include "MyEDmodules/HcalDelayTuner/interface/SplashDelayTunerAlgos.hh"


#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TProfile.h"
#include "TProfile2D.h"

inline string int2str(int i) {
  std::ostringstream ss;
  ss << i;
  return ss.str();
}

//
// constructors and destructor
//
SplashDelayTunerAlgos::SplashDelayTunerAlgos(const edm::ParameterSet& iConfig,
					     SplashHitTimeCorrector *timecor) :
  HcalDelayTunerAlgos(iConfig),
  timecor_(timecor),
  globalToffset_(iConfig.getParameter<double>("globalTimeOffset")),
  globalFlagMask_(iConfig.getParameter<int>("globalRecHitFlagMask")),
  rundescr_(iConfig.getUntrackedParameter<std::string>("runDescription",""))
{
  // cut string vector initialized in order
  // all cuts applied on top of the previous one
  //
  v_cuts_.push_back("cut0none");
  v_cuts_.push_back("cut1minHitGeV");
  v_cuts_.push_back("cut2bxnum");
  v_cuts_.push_back("cut3badFlags");
  v_cuts_.push_back("cut4badEvents");
#if 0
  v_cuts_.push_back("cut5tower16");
  v_cuts_.push_back("cut6aInTimeWindow");
  v_cuts_.push_back("cut6bOutOfTimeWindow");
#endif
  st_lastCut_ = "cut4badEvents";

  compileCorrections(iConfig.getParameter<vector<edm::ParameterSet> >("CorrectionsList"));

  minHitGeV_          = iConfig.getParameter<double>("minHitGeV");
#if 0
  timeWindowMinNS_    = iConfig.getParameter<double>("timeWindowMinNS");
  timeWindowMaxNS_    = iConfig.getParameter<double>("timeWindowMaxNS");
#endif
  recHitTscaleNbins_  = iConfig.getParameter<int>   ("recHitTscaleNbins");
  recHitTscaleMinNs_  = iConfig.getParameter<double>("recHitTscaleMinNs");
  recHitTscaleMaxNs_  = iConfig.getParameter<double>("recHitTscaleMaxNs");
  recHitEscaleMinGeV_ = iConfig.getParameter<double>("recHitEscaleMinGeV");
  recHitEscaleMaxGeV_ = iConfig.getParameter<double>("recHitEscaleMaxGeV");
  maxEventNum2plot_   = iConfig.getParameter<int>("maxEventNum2plot");


  std::vector<int> badEventVec =
    iConfig.getParameter<vector<int> >("badEventList");
  for (size_t i=0; i<badEventVec.size(); i++)
    badEventSet_.insert(badEventVec[i]);

  std::vector<int> acceptedBxVec =
    iConfig.getParameter<vector<int> >("acceptedBxNums");
  for (size_t i=0; i<acceptedBxVec.size(); i++)
    acceptedBxNums_.insert(acceptedBxVec[i]);

  std::vector<int> v_maskidnumbers =
    iConfig.getParameter<vector<int> >("detIds2mask");

  if (!buildMaskSet(v_maskidnumbers))
    throw cms::Exception("Invalid detID vector");

  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());

  switch(mysubdet_) {
  case HcalBarrel : mysubdetstr_ = "HB"; break;
  case HcalEndcap : mysubdetstr_ = "HE"; break;
  case HcalOuter  : mysubdetstr_ = "HO"; break;
  case HcalForward: mysubdetstr_ = "HF"; break;
  default:          mysubdetstr_ = "FU"; break;
  }

  firstEvent_ = true;
}                        // SplashDelayTunerAlgos::SplashDelayTunerAlgos

//======================================================================

//
// constructors and destructor
//
SplashDelayTunerAlgos::SplashDelayTunerAlgos(const edm::ParameterSet& iConfig) :
  HcalDelayTunerAlgos(iConfig)
{
  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());

  switch(mysubdet_) {
  case HcalBarrel : mysubdetstr_ = "HB"; break;
  case HcalEndcap : mysubdetstr_ = "HE"; break;
  case HcalOuter  : mysubdetstr_ = "HO"; break;
  case HcalForward: mysubdetstr_ = "HF"; break;
  default:          mysubdetstr_ = "FU"; break;
  }

  firstEvent_ = true;
}                        // SplashDelayTunerAlgos::SplashDelayTunerAlgos

//======================================================================

bool
SplashDelayTunerAlgos::buildMaskSet(const std::vector<int>& v_idnumbers)
{
  // convert det ID numbers to valid detIds:
  if (v_idnumbers.size()%3) {
    return false;
  }

  for (uint32_t i=0; i<v_idnumbers.size(); i+=3) {
    int ieta  = v_idnumbers[i];
    int iphi  = v_idnumbers[i+1];
    int depth = v_idnumbers[i+2];
    enum HcalSubdetector subdet;
    if      (depth == 4)       subdet = HcalOuter;
    else if (abs(ieta) <= 16)  subdet = (depth==3)? HcalEndcap:HcalBarrel;
    else if (abs(ieta) <= 29)  subdet = HcalEndcap; // gud enuf fer gubmint werk
    else return false;                    // no HF for splash
    
    if (!HcalDetId::validDetId(subdet,ieta,iphi,depth))
      return false;

    detIds2mask_.insert(HcalDetId(subdet,ieta,iphi,depth).hashed_index());
  }
  return true;
}                             // SplashDelayTunerAlgos::convertIdNumbers

//======================================================================

void
SplashDelayTunerAlgos::compileCorrections
(const std::vector<edm::ParameterSet>& corParList)
{
  for (size_t i=0; i<corParList.size(); i++) {
    edm::ParameterSet corpars = corParList[i];

    tCorrection tcor;

    if (corpars.getParameter<bool>("applyExternal")) {
      if (corpars.getParameter<bool>("applyToHits")) {
	TimesPerDetId somecors;
	HcalDelayTunerInput(corpars).getTimeCorrections(somecors);
	TimesPerDetId::const_iterator it;
	for (it=somecors.begin(); it!=somecors.end(); it++) {
	std::pair<TimesPerDetId::iterator,bool> retval;
	retval = exthitcors_.insert(*it);
	if (!retval.second)
	  (retval.first)->second += it->second;  // pile correction on top of previous one.
	}
      } else {
	// save the input object for the endJob
	tcor.applyExternal = true;
	tcor.extcorInput   = new HcalDelayTunerInput(corpars);
	corList_.push_back(tcor);
      }
    }
    else {
      tcor.correctByPhi     =corpars.getParameter<bool>("correctByPhi");
      tcor.correctByEta     =corpars.getParameter<bool>("correctByEta");
      tcor.selfSynchronize  =corpars.getParameter<bool>("selfSynchronize");
      tcor.ietamin          =corpars.getParameter<int>("ietamin");
      tcor.ietamax          =corpars.getParameter<int>("ietamax");
      tcor.iphimin          =corpars.getParameter<int>("iphimin");
      tcor.iphimax          =corpars.getParameter<int>("iphimax");
      tcor.devthresh        =corpars.getParameter<double>("devthresh");
      tcor.mirrorCorrection =corpars.getParameter<bool>("mirrorCorrection");
      corList_.push_back(tcor);
    }
  }
}


//======================================================================

void
SplashDelayTunerAlgos::add1dHisto(const std::string& name, const std::string& title,
				 int nbinsx, double minx, double maxx,
				 std::vector<myAnalHistos::HistoParams_t>& v_hpars1d)
{
  myAnalHistos::HistoParams_t hpars1d;
  hpars1d.name   = name;
  hpars1d.title  = title;
  hpars1d.nbinsx = nbinsx;
  hpars1d.minx   = minx;
  hpars1d.maxx   = maxx;
  hpars1d.nbinsy = 0;
  v_hpars1d.push_back(hpars1d);
}

//======================================================================

void
SplashDelayTunerAlgos::add2dHisto(const std::string& name, const std::string& title,
				 int nbinsx, double minx, double maxx,
				 int nbinsy, double miny, double maxy,
				 std::vector<myAnalHistos::HistoParams_t>& v_hpars2d)
{
  myAnalHistos::HistoParams_t hpars2d;
  hpars2d.name   = name;
  hpars2d.title  = title;
  hpars2d.nbinsx = nbinsx;
  hpars2d.minx   = minx;
  hpars2d.maxx   = maxx;
  hpars2d.nbinsy = nbinsy;
  hpars2d.miny   = miny;
  hpars2d.maxy   = maxy;
  v_hpars2d.push_back(hpars2d);
}

//======================================================================

myAnalHistos *
SplashDelayTunerAlgos::getHistos4cut(const std::string& cutstr)
{
  std::map<std::string,myAnalCut *>::const_iterator it = m_cuts_.find(cutstr);
  if (it == m_cuts_.end())
    throw cms::Exception("Cut not found, you numnutz! You changed the name: ") << cutstr << endl;

  return it->second->histos();
}

//======================================================================

void
SplashDelayTunerAlgos::bookHistos4allCuts(void)
{
  // Initialize the cuts for the run and add them to the global map
  m_cuts_.clear();

  for (unsigned i=0; i<v_cuts_.size(); i++)
    m_cuts_[v_cuts_[i]] = new myAnalCut(i,v_cuts_[i],mysubdetstr_);

  cout << "Booking histos for subdet = " << mysubdetstr_ << std::endl;
//edm::LogInfo(

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d; 
  std::vector<myAnalHistos::HistoParams_t> v_hpars1dprof;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d; 
  //std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof; // all in last cut only

  /*****************************************
   * 1-D HISTOGRAMS FIRST:                 *
   *****************************************/

  st_avgPulse_   = "h1d_pulse" + mysubdetstr_;
  add1dHisto(st_avgPulse_,"Average Pulse Shape, " + mysubdetstr_,10,-0.5,9.5,v_hpars1d);

  //==================== Collection sizes ====================

  st_digiColSize_ = "DigiCollectionSize" + mysubdetstr_;
  add1dHisto( st_digiColSize_, "Digi Collection Size, " + mysubdetstr_, 
	      5201,-0.5, 5200.5, v_hpars1d); // 72chan/RBX*72RBX = 5184, more than HF or HO

  st_rhColSize_ = "RechitCollectionSize" + mysubdetstr_;
  add1dHisto( st_rhColSize_, "Rechit Collection Size, " + mysubdetstr_,
	      5201,-0.5, 5200.5,v_hpars1d);  // 72chan/RBX*72RBX = 5184, more than HF or HO

  //==================== Total time/energy/flags distros ====================
	     
  st_rhUncorTimes_ = "h1d_rhUncorTimes" + mysubdetstr_;
  add1dHisto( st_rhUncorTimes_,
	      "RecHit Times (uncorrected), " + mysubdetstr_ + "; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhCorTimes_ = "h1d_rhCorTimes" + mysubdetstr_;
  add1dHisto( st_rhCorTimes_,
	      "RecHit Times (corrected), " + mysubdetstr_ + "; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhCorTimesD1_ = "h1d_rhCorTimesD1" + mysubdetstr_;
  add1dHisto( st_rhCorTimesD1_,
	      "Depth 1 RecHit Times (corrected), " + mysubdetstr_ + "; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhEnergies_ = "h1d_RHEnergies" + mysubdetstr_;
  add1dHisto( st_rhEnergies_,
	      "RecHit Energies, " + mysubdetstr_ + "; Rechit Energy (GeV)",
	      (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	      recHitEscaleMinGeV_,recHitEscaleMaxGeV_,v_hpars1d);

  st_rhFlagBits_ = "h1d_rhFlagBits" + mysubdetstr_;
  add1dHisto( st_rhFlagBits_,
	      "RecHit Quality Flag Bits, " + mysubdetstr_ + "; Flag Name",
	      20,0.5,20.5,v_hpars1d);  // see below (post-booking) for bin labeling
  
  st_rhHBHEtimingShapedCuts_ = "h1d_HBHEtimingShapedCuts" + mysubdetstr_;
  add1dHisto( st_rhHBHEtimingShapedCuts_,
	      "RecHit Timing Shaped Cuts Bits, " + mysubdetstr_ + "; Quality (0-7)",
	      10,-0.5,9.5,v_hpars1d);

  //==================== ...by Event ====================

  st_totalEperEv_ = "h1d_totalEperEvIn" + mysubdetstr_;
  add1dHisto( st_totalEperEv_,
  "#Sigma RecHit Energy Per Event in " + mysubdetstr_ + "; Event Number; Total Energy (GeV)",
	      (maxEventNum2plot_+1),-0.5,((float)maxEventNum2plot_)+0.5,
	      v_hpars1d);

  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  st_avgTperEvD1_ = "h1d_avgTperEvIn" + mysubdetstr_;
  add1dHisto( st_avgTperEvD1_,
	      "Depth 1 Averaged Time Per Event in " + mysubdetstr_ + "; Event Number; Average Time (ns)",
	      (maxEventNum2plot_+1),-0.5,((float)maxEventNum2plot_)+0.5,
	      v_hpars1dprof);

  /*****************************************
   * 2-D HISTOGRAMS AFTER:                 *
   *****************************************/

  st_rhEmap_        = "h2d_rhEperIetaIphi" + mysubdetstr_;
  add2dHisto(st_rhEmap_,
	     "RecHit Energy Map (#Sigma depths), " + mysubdetstr_ + "; i#eta; i#phi",
	     61, -30.5,  30.5, 72,   0.5,  72.5, v_hpars2d);

  st_uncorTimingVsE_ = "h2d_uncorTimingVsE" + mysubdetstr_;
  add2dHisto(st_uncorTimingVsE_,
"RecHit Timing (uncorrected) vs. Energy, "+mysubdetstr_+"; Rechit Energy (GeV); Rechit Time (ns)",
	     (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	     recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	     v_hpars2d);

  st_corTimingVsE_ = "h2d_corTimingVsE" + mysubdetstr_;
  add2dHisto(st_corTimingVsE_,
"RecHit Timing (corrected) vs. Energy, "+mysubdetstr_+"; Rechit Energy (GeV); Rechit Time (ns)",
	     (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	     recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	     v_hpars2d);

  /*****************************************
   * BOOK 'EM, DANNO...                    *
   *****************************************/

  std::map<string, myAnalCut *>::const_iterator it;
  for (it = m_cuts_.begin(); it != m_cuts_.end(); it++) {
    myAnalCut  *cut = it->second;
    myAnalHistos *myAH = cut->histos();
    myAH->book1d<TH1F>      (v_hpars1d);
    myAH->book1d<TProfile>  (v_hpars1dprof);
    myAH->book2d<TH2F>      (v_hpars2d);
    //myAH->book2d<TProfile2D>(v_hpars2dprof);

    TH1F *h1f = myAH->get<TH1F>(st_rhFlagBits_);
    if (h1f) {
      TAxis *xax = h1f->GetXaxis();
      xax->SetBinLabel(1,(mysubdet_==HcalForward)?"HFLongShort":"HBHEHpdHitMultiplicity");
      xax->SetBinLabel(2,(mysubdet_==HcalForward)?"HFDigiTime":"HBHEPulseShape");
      xax->SetBinLabel(3,"HSCP_R1R2");
      xax->SetBinLabel(4,"HSCP_FracLeader");
      xax->SetBinLabel(5,"HSCP_OuterEnergy");
      xax->SetBinLabel(6,"HSCP_ExpFit");
      xax->SetBinLabel(7,(mysubdet_==HcalForward)?"HFTimingTrust0":"N/A");
      xax->SetBinLabel(8,(mysubdet_==HcalForward)?"HFTimingTrust1":"N/A");
      xax->SetBinLabel(9, "HBHETimingShapedCuts0");
      xax->SetBinLabel(10,"HBHETimingShapedCuts1");
      xax->SetBinLabel(11,"HBHETimingShapedCuts2");
      xax->SetBinLabel(12,"N/A");
      xax->SetBinLabel(13,"N/A");
      xax->SetBinLabel(14,"N/A");
      xax->SetBinLabel(15,"N/A");
      xax->SetBinLabel(16,"N/A");
      xax->SetBinLabel(17,"Subtracted25ns");
      xax->SetBinLabel(18,"Added25ns");
      xax->SetBinLabel(19,"UncorBCNcapidMismatch");
      xax->SetBinLabel(20,"Saturation");
    }
  }
}                           // SplashDelayTunerAlgos::bookHistos4allCuts

//======================================================================

void
SplashDelayTunerAlgos::bookHistos4lastCut(void)
{
  char name[40];
  char title[128];

  //std::vector<myAnalHistos::HistoParams_t> v_hpars1d;  // in all cuts
  //std::vector<myAnalHistos::HistoParams_t> v_hpars2d;  // in all cuts
  std::vector<myAnalHistos::HistoParams_t> v_hpars1dprof;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof;

  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  //==================== ...by phi/eta/depth ====================

  if (mysubdet_ != HcalOuter) { 
    st_avgTimePerRMd1_ =  "p1d_avgTimePerRMd1" + mysubdetstr_;
    add1dHisto( st_avgTimePerRMd1_,
		"Averaged Time (Depth 1), " + mysubdetstr_ + "; iRM; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

    st_avgTimePerRMd2_ =  "p1d_avgTimePerRMd2" + mysubdetstr_;
    add1dHisto( st_avgTimePerRMd2_,
		"Averaged Time (Depth 2), " + mysubdetstr_ + "; iRM; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhid1_ =  "p1d_avgTimePerPhid1" + mysubdetstr_;
    add1dHisto( st_avgTimePerPhid1_,
		"Averaged Time (Depth 1), " + mysubdetstr_ + "; i#phi; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhid2_ =  "p1d_avgTimePerPhid2" + mysubdetstr_;
    add1dHisto( st_avgTimePerPhid2_,
		"Averaged Time (Depth 2), " + mysubdetstr_ + "; i#phi; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

    st_avgTuncPerIetad1_ =  "p1d_avgTuncPerIetad1" + mysubdetstr_;
    add1dHisto( st_avgTuncPerIetad1_,
		"Averaged Time (Depth 1), " + mysubdetstr_ + "; i#eta; Time (ns)",
		61,-30.5, 30.5,v_hpars1dprof);

    st_avgTuncPerIetad2_ =  "p1d_avgTuncPerIetad2" + mysubdetstr_;
    add1dHisto( st_avgTuncPerIetad2_,
		"Averaged Time (Depth 2), " + mysubdetstr_ + "; i#eta; Time (ns)",
		61,-30.5, 30.5,v_hpars1dprof);

    st_avgTcorPerIetad1_ =  "p1d_avgTcorPerIetad1" + mysubdetstr_;
    add1dHisto( st_avgTcorPerIetad1_,
		"Averaged Time (Depth 1), " + mysubdetstr_ + "; i#eta; Time (ns)",
		61,-30.5, 30.5,v_hpars1dprof);

    st_avgTcorPerIetad2_ =  "p1d_avgTcorPerIetad2" + mysubdetstr_;
    add1dHisto( st_avgTcorPerIetad2_,
		"Averaged Time (Depth 2), " + mysubdetstr_ + "; i#eta; Time (ns)",
		61,-30.5, 30.5,v_hpars1dprof);

    st_avgTimePerRBXd1_ =  "p1d_avgTimePerRBXd1" + mysubdetstr_;
    add1dHisto( st_avgTimePerRBXd1_,
		"Averaged Time (Depth 1), " + mysubdetstr_ + "; iRBX; Time (ns)",
		37,-18.5, 18.5,v_hpars1dprof);

    st_avgTimePerRBXd2_ =  "p1d_avgTimePerRBXd2" + mysubdetstr_;
    add1dHisto( st_avgTimePerRBXd2_,
		"Averaged Time (Depth 2), " + mysubdetstr_ + "; iRBX; Time (ns)",
		37,-18.5, 18.5,v_hpars1dprof);

    if (mysubdet_ == HcalEndcap) {
      //
      // Detail histos for the last cut only
      //
      st_avgTimePer2RMs_ =  "p1d_avgTimePer2RMs" + mysubdetstr_;
      add1dHisto( st_avgTimePer2RMs_,
		  "Averaged Time over 2RMs, HE; iRM/2; Time (ns)",
		  73,-36.5, 36.5,v_hpars1dprof);

      st_avgTimePerRMd3_ =  "p1d_avgTimePerRMd3HE";
      add1dHisto( st_avgTimePerRMd3_,
		  "Averaged Time (Depth 3), HE; iRM; Time (ns)",
		  145,-72.5, 72.5,v_hpars1dprof);

      st_avgTimePerPhid3_ =  "p1d_avgTimePerPhid3HE";
      add1dHisto( st_avgTimePerPhid3_,
		  "Averaged Time (Depth 3), HE; i#phi; Time (ns)",
		  145,-72.5, 72.5,v_hpars1dprof);

      st_avgTuncPerIetad3_ =  "p1d_avgTuncPerIetad3HE";
      add1dHisto( st_avgTuncPerIetad3_,
		  "Averaged Time (Depth 3), HE; i#eta; Time (ns)",
		  61,-30.5, 30.5,v_hpars1dprof);

      st_avgTcorPerIetad3_ =  "p1d_avgTcorPerIetad3HE";
      add1dHisto( st_avgTcorPerIetad3_,
		  "Averaged Time (Depth 3), HE; i#eta; Time (ns)",
		  61,-30.5, 30.5,v_hpars1dprof);

      st_avgTimePerRBXd3_ =  "p1d_avgTimePerRBXd3HE";
      add1dHisto( st_avgTimePerRBXd3_,
		  "Averaged Time (Depth 3), HE; iRBX; Time (ns)",
		  37,-18.5, 18.5,v_hpars1dprof);

      //
      // Make profiles of timing vs RM for individual pixels
      //
      for (int ipix=1; ipix<=19; ipix++) {
	sprintf(name,"p1d_rhTvsRM4pix%02dHE",ipix);
	sprintf(title,"Corrected RecHit Time vs RM for pixel %d, HE",ipix);
	v_st_rhTvsRMperPixHE_.push_back(string(name));
	string titlestr = string(title);
	add1dHisto(v_st_rhTvsRMperPixHE_[ipix-1],titlestr,145,-72.5,72.5,v_hpars1dprof);
      }

      //
      // Make profiles of timing vs RM and phi for individual ietas/depths
      //
      for (int ieta=18; ieta<=29; ieta++) {
	
	sprintf(name,"p1d_rhTvsRM4ieta%02dd2HEP",ieta);
	sprintf(title,"Corrected RecHit Time vs RM for i#eta=%d, depth=2 HEP",ieta);
	v_st_rhTvsRMperIetaD2HEP_.push_back(string(name));
	string titlestr = string(title);
	add1dHisto(v_st_rhTvsRMperIetaD2HEP_[ieta-18],titlestr,73,-0.5,72.5,v_hpars1dprof);
      
	sprintf(name,"p1d_rhTvsRM4ieta%02dd2HEM",ieta);
	sprintf(title,"Corrected RecHit Time vs RM for i#eta=-%d, depth=2 HEM",ieta);
	v_st_rhTvsRMperIetaD2HEM_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsRMperIetaD2HEM_[ieta-18],titlestr,73,-0.5,72.5,v_hpars1dprof);
      
	sprintf(name,"p1d_rhTvsPhi4ieta%02dd2HEP",ieta);
	sprintf(title,"Corrected RecHit Time vs i#phi for i#eta=%d, depth=2 HEP",ieta);
	v_st_rhTvsPhiperIetaD2HEP_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsPhiperIetaD2HEP_[ieta-18],titlestr,73,-0.5,72.5,v_hpars1dprof);
	
	sprintf(name,"p1d_rhTvsPhi4ieta%02dd2HEM",ieta);
	sprintf(title,"Corrected RecHit Time vs i#phi for i#eta=-%d, depth=2 HEM",ieta);
	v_st_rhTvsPhiperIetaD2HEM_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsPhiperIetaD2HEM_[ieta-18],titlestr,73,-0.5,72.5,v_hpars1dprof);

      }           // loop over ieta
    }           // if endcap

  } else {     // HO:

    st_avgTimePerRMd4_ = "p1d_avgTimePerRMd4HO";
    add1dHisto( st_avgTimePerRMd4_,
		"Averaged Time vs iRM, HO; iRM; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

  //==================== ...by Ring ====================

    st_avgTimePerPhiRing0_ = "p1d_avgTimePerPhiHORing0";
    add1dHisto( st_avgTimePerPhiRing0_,
		"Averaged Time vs i#phi (YB0), HO; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing1M_ = "p1d_avgTimePerPhiHORing1M";
    add1dHisto( st_avgTimePerPhiRing1M_,
		"Averaged Time vs i#phi (YB-1), HO; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing1P_ = "p1d_avgTimePerPhiHORing1P";
    add1dHisto( st_avgTimePerPhiRing1P_,
		"Averaged Time vs i#phi (YB+1), HO; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing2M_ = "p1d_avgTimePerPhiHORing2M";
    add1dHisto( st_avgTimePerPhiRing2M_,
		"Averaged Time vs i#phi (YB-2), HO; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing2P_ = "p1d_avgTimePerPhiHORing2P";
    add1dHisto( st_avgTimePerPhiRing2P_,
		"Averaged Time vs i#phi (YB+2), HO; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTuncPerIetad4_ = "p1d_avgTuncPerIetad4HO";
    add1dHisto( st_avgTuncPerIetad4_,
		"Averaged Time vs i#eta (Uncorrected), HO; i#eta; Time (ns)",
		61,-30.5, 30.5,v_hpars1dprof);

    st_avgTcorPerIetad4_ = "p1d_avgTcorPerIetad4HO";
    add1dHisto( st_avgTcorPerIetad4_,
		"Averaged Time vs i#eta (Corrected), HO; i#eta; Time (ns)",
		61,-30.5, 30.5,v_hpars1dprof);

    st_avgTimePerRBXd4_ =  "p1d_avgTimePerRBXd4HO",
    add1dHisto( st_avgTimePerRBXd4_,
		"Averaged Time by RBX (Depth 4), HO; iRBX; Time (ns)",
		37,-18.5, 18.5,v_hpars1dprof);
  }

  /*****************************************
   * 2-D PROFILES:                         *
   *****************************************/

  if (mysubdet_ != HcalOuter) { 
    st_rhTuncProfd1_  = "p2d_rhTuncPerIetaIphiD1" + mysubdetstr_;
    add2dHisto(st_rhTuncProfd1_,
 "Depth 1 RecHit Time Map-Profile (uncorrected), " + mysubdetstr_ + "; i#eta; i#phi",
	       61, -30.5,  30.5, 72, 0.5, 72.5,
	       v_hpars2dprof);

    st_rhTcorProfd1_  = "p2d_rhTcorPerIetaIphiD1" + mysubdetstr_;
    add2dHisto(st_rhTcorProfd1_,
 "Depth 1 RecHit Time Map-Profile (corrected), " + mysubdetstr_ + "; i#eta; i#phi",
	       61, -30.5,  30.5, 72, 0.5, 72.5,
	       v_hpars2dprof);

    st_rhTuncProfd2_  = "p2d_rhTuncPerIetaIphiD2" + mysubdetstr_;
    add2dHisto(st_rhTuncProfd2_,
 "Depth 2 RecHit Time Map-Profile (uncorrected), " + mysubdetstr_ + "; i#eta; i#phi",
	       61, -30.5,  30.5, 72,   0.5,  72.5,
	       v_hpars2dprof);

    st_rhTcorProfd2_  = "p2d_rhTcorPerIetaIphiD2" + mysubdetstr_;
    add2dHisto(st_rhTcorProfd2_,
 "Depth 2 RecHit Time Map-Profile (corrected), " + mysubdetstr_ + "; i#eta; i#phi",
	       61, -30.5,  30.5, 72, 0.5, 72.5,
	       v_hpars2dprof);

    st_rhTprofRBXd1_  = "p2d_rhTcorPerRBXD1" + mysubdetstr_;
    add2dHisto(st_rhTprofRBXd1_,
"Depth 1 RecHit Time RBX Map-Profile (corrected), " + mysubdetstr_ + "; i#eta; iRBX",
	       6, -3.0,  3.0, 18,   0.5,  18.5,
	       v_hpars2dprof);

    st_rhTprofRBXd2_  = "p2d_rhTcorPerRBXD2" + mysubdetstr_;
    add2dHisto(st_rhTprofRBXd2_,
"Depth 2 RecHit Time RBX Map-Profile (corrected), " + mysubdetstr_ + "; i#eta; iRBX",
	       6, -3.0,  3.0, 18,   0.5,  18.5,
	       v_hpars2dprof);

    if (mysubdet_ == HcalEndcap) {
      st_rhTuncProfd3_  = "p2d_rhTuncPerIetaIphiD3HE";
      add2dHisto(st_rhTuncProfd3_,
	 "Depth 3 RecHit Time Map-Profile (uncorrected), HE; i#eta; i#phi",
		 61, -30.5,  30.5, 72,   0.5,  72.5,
		 v_hpars2dprof);

      st_rhTcorProfd3_  = "p2d_rhTcorPerIetaIphiD3HE";
      add2dHisto(st_rhTcorProfd3_,
	 "Depth 3 RecHit Time Map-Profile (corrected), HE; i#eta; i#phi",
		 61, -30.5,  30.5, 72,   0.5,  72.5,
		 v_hpars2dprof);

      st_rhTprofRBXd3_  = "p2d_rhTcorPerRBXD3" + mysubdetstr_;
      add2dHisto(st_rhTprofRBXd3_,
"Depth 3 RecHit Time RBX Map-Profile (corrected), " + mysubdetstr_ + "; i#eta; iRBX",
		 6, -3.0,  3.0, 18,   0.5,  18.5,
		 v_hpars2dprof);
    }
  } else {
    st_rhTuncProfd4_  = "p2d_rhTuncPerIetaIphiD4HO";
    add2dHisto(st_rhTuncProfd4_,
	       "Depth 4 RecHit Time Map-Profile (uncorrected), HO; i#eta; i#phi",
	       61, -30.5,  30.5, 72,   0.5,  72.5,
	       v_hpars2dprof);

    st_rhTcorProfd4_  = "p2d_rhTcorPerIetaIphiD4HO";
    add2dHisto(st_rhTcorProfd4_,
	       "Depth 4 RecHit Time Map-Profile (corrected), HO; i#eta; i#phi",
	       61, -30.5,  30.5, 72,   0.5,  72.5,
	       v_hpars2dprof);

    st_rhTprofRBXd4_  = "p2d_rhTperRBXD4" + mysubdetstr_;
    add2dHisto(st_rhTprofRBXd4_,
"Depth 4 RecHit Time RBX Map-Profile (corrected), " + mysubdetstr_ + "; i#eta; iRBX",
	       6, -3.0,  3.0, 18,   0.5,  18.5,
	       v_hpars2dprof);
  }

  /*****************************************
   * BOOK 'EM, DANNO...                    *
   *****************************************/

  myAnalHistos *myAH = getHistos4cut(st_lastCut_);
  //myAH->book1d<TH1F>      (v_hpars1d);
  //myAH->book2d<TH2F>      (v_hpars2d);
  myAH->book1d<TProfile>  (v_hpars1dprof);
  myAH->book2d<TProfile2D>(v_hpars2dprof);

  TProfile2D *tp = myAH->get<TProfile2D>(st_rhTprofRBXd1_);
  if (tp) {
    TAxis *xax = tp->GetXaxis();
    xax->SetBinLabel(1,"HF-");
    xax->SetBinLabel(2,"HE-");
    xax->SetBinLabel(3,"HB-");
    xax->SetBinLabel(4,"HB+");
    xax->SetBinLabel(5,"HE+");
    xax->SetBinLabel(6,"HF+");
  }

  tp = myAH->get<TProfile2D>(st_rhTprofRBXd2_);
  if (tp) {
    TAxis *xax = tp->GetXaxis();
    xax->SetBinLabel(1,"HF-");
    xax->SetBinLabel(2,"HE-");
    xax->SetBinLabel(3,"HB-");
    xax->SetBinLabel(4,"HB+");
    xax->SetBinLabel(5,"HE+");
    xax->SetBinLabel(6,"HF+");
  }
  
  tp = myAH->get<TProfile2D>(st_rhTprofRBXd3_);
  if (tp) {
    TAxis *xax = tp->GetXaxis();
    xax->SetBinLabel(1,"HF-");
    xax->SetBinLabel(2,"HE-");
    xax->SetBinLabel(3,"HB-");
    xax->SetBinLabel(4,"HB+");
    xax->SetBinLabel(5,"HE+");
    xax->SetBinLabel(6,"HF+");
  }

  /*****************************************
   * To put in a separate directory
   ****************************************/

#if 0
  /* Book per channel histos */

  TFileDirectory *lastdir = myAH->dir();
  TFileDirectory *chdir = new TFileDirectory(lastdir->mkdir("PerChannelHistos"));

  // iterate over all channels in mysubdet_
  HcalDetIdGenerator genDetIds;
  std::vector<uint32_t> v_denseIds;
  genDetIds.appendDetIds4subdet(mysubdet_,v_denseIds);

  for (uint32_t i=0; i<v_denseIds.size(); i++) {
    uint32_t denseId = v_denseIds[i];
    HcalDetId detId  = HcalDetId::detIdFromDenseIndex(denseId);
    sprintf (name,"%s%ceta%dphi%ddpth%d",mysubdetstr_.c_str(),
	     (detId.zside()>0)?'P':'M',detId.ietaAbs(),detId.iphi(),detId.depth());
    sprintf (title, "Timing for Channel %s",name);

    TH1F *h1d = chdir->make<TH1F>(name,title,51,-12.25,12.25);
    m_perChHistos_.insert(std::pair<uint32_t,TH1F *>(denseId,h1d));
  }
#endif
}                           // SplashDelayTunerAlgos::bookHistos4lastCut

//======================================================================
inline int    sign   (double x) { return (x>=0) ? 1 : -1; }

void
SplashDelayTunerAlgos::fillHistos4cut(const std::string& cutstr)
{
  //edm::LogInfo("Filling histograms for subdet ") << mysubdetstr_ << std::endl;

  myAnalHistos *myAH =   getHistos4cut(cutstr);

  int        ieta = detID_.ieta();
  int        iphi = detID_.iphi();
  int       depth = detID_.depth();
  int       zside = detID_.zside();

  /* ALL subdet Histos: */
  myAH->fill1d<TH1F>(st_rhEnergies_,    hitenergy_);
  myAH->fill1d<TH1F>(st_rhCorTimes_,    corTime_);
  myAH->fill1d<TH1F>(st_rhUncorTimes_,  hittime_);
  myAH->fill2d<TH2F>(st_rhEmap_,        ieta,iphi,hitenergy_);
  myAH->fill2d<TH2F>(st_uncorTimingVsE_,hitenergy_,hittime_);
  myAH->fill2d<TH2F>(st_corTimingVsE_,  hitenergy_,corTime_);

  if (depth==1) {
    myAH->fill1d<TH1F> (st_rhCorTimesD1_, corTime_);
    myAH->fill1d<TProfile> (st_avgTperEvD1_, evtnum_, corTime_);
  }

  for (int ibit=0; ibit<20; ibit++) {
    int flagshift = (hitflags_>>ibit);
    if (ibit==8) {
      int timingCutQuality = flagshift & 7;
      myAH->fill1d<TH1F>(st_rhHBHEtimingShapedCuts_,timingCutQuality);
    }
    if (flagshift & 1) {
      myAH->fill1d<TH1F>(st_rhFlagBits_,ibit+1);
    }
  }

  if (cutstr == st_lastCut_) {
    // need front end id: 
    int            iRBX = atoi(((feID_.rbx()).substr(3,2)).c_str());
    int        iRMinRBX = feID_.rm();
    int            ipix = feID_.pixel();
    int             iRM = zside * ((iRBX-1)*4 + iRMinRBX);
    int     signed_iphi = zside*iphi;
    
    //cout << detId << "\t" << feID.rbx() << "\t" << feID.rbx().substr(3,2) << "\t";
    //cout << feID.rm() << " maps to RBX/RM# " << iRBX << "/" << iRM << endl;

    /* Per Depth Histos: */
    std::string rhTuncProf,rhTcorProf,rhTprofRBX;
    std::string avgRMt,avgPhiT,avgIetaTunc,avgIetaTcor,avgRBXt;
    std::string sduncorTime,sdcorTime;
    switch(depth) {
    case 1:
      rhTuncProf = st_rhTuncProfd1_;
      rhTcorProf = st_rhTcorProfd1_;
      rhTprofRBX = st_rhTprofRBXd1_;
      avgRMt     = st_avgTimePerRMd1_;
      avgPhiT    = st_avgTimePerPhid1_;
      avgIetaTunc= st_avgTuncPerIetad1_;
      avgIetaTcor= st_avgTcorPerIetad1_;
      avgRBXt    = st_avgTimePerRBXd1_;
      break;
    case 2:
      rhTuncProf = st_rhTuncProfd2_;
      rhTcorProf = st_rhTcorProfd2_;
      rhTprofRBX = st_rhTprofRBXd2_;
      avgRMt     = st_avgTimePerRMd2_;
      avgPhiT    = st_avgTimePerPhid2_;
      avgIetaTunc= st_avgTuncPerIetad2_;
      avgIetaTcor= st_avgTcorPerIetad2_;
      avgRBXt    = st_avgTimePerRBXd2_;
      break;
    case 3:
      rhTuncProf = st_rhTuncProfd3_;
      rhTcorProf = st_rhTcorProfd3_;
      rhTprofRBX = st_rhTprofRBXd3_;
      avgRMt     = st_avgTimePerRMd3_;
      avgPhiT    = st_avgTimePerPhid3_;
      avgIetaTunc= st_avgTuncPerIetad3_;
      avgIetaTcor= st_avgTcorPerIetad3_;
      avgRBXt    = st_avgTimePerRBXd3_;
      break;
    case 4:
      rhTuncProf = st_rhTuncProfd4_;
      rhTcorProf = st_rhTcorProfd4_;
      rhTprofRBX = st_rhTprofRBXd4_;
      avgRMt     = st_avgTimePerRMd4_;
      //avgPhiT    = st_avgTimePerPhid4_; // we do it by ring, see below 
      avgIetaTunc= st_avgTuncPerIetad4_;
      avgIetaTcor= st_avgTcorPerIetad4_;
      avgRBXt    = st_avgTimePerRBXd4_;
      break;
    default:
      edm::LogWarning("Invalid depth in rechit collection! detId = ") << detID_ << std::endl;
    }

    float fsubdet = 0;
    switch (mysubdet_) {
    case HcalBarrel:  fsubdet = zside*0.5; break;
    case HcalEndcap:  fsubdet = zside*1.5; break;
    case HcalForward: fsubdet = zside*2.5; break;
      //case HcalOuter:   fsubdet = zside*??
    default: break;
    }

    myAH->fill1d<TProfile>   (avgRMt,     iRM,          corTime_);
    myAH->fill1d<TProfile>   (avgIetaTunc,ieta,         hittime_);
    myAH->fill1d<TProfile>   (avgIetaTcor,ieta,         corTime_);
    myAH->fill1d<TProfile>   (avgRBXt,    zside*iRBX,   corTime_);
    myAH->fill2d<TProfile2D> (rhTuncProf, ieta, iphi,   hittime_);
    myAH->fill2d<TProfile2D> (rhTcorProf, ieta, iphi,   corTime_);
    myAH->fill2d<TProfile2D> (rhTprofRBX, fsubdet,iRBX, corTime_);

    // subdet-specific histos
    if (mysubdet_ == HcalEndcap) {
      int iRMavg2 = ((iRM-sign(iRM))/2)+sign(iRM);
      myAH->fill1d<TProfile> (st_avgTimePer2RMs_, iRMavg2, corTime_);
      myAH->fill1d<TProfile> (v_st_rhTvsRMperPixHE_[ipix-1],iRM,corTime_);

      if (depth==2) {
	const std::string& hRM =(ieta>0)?v_st_rhTvsRMperIetaD2HEP_[ieta-18]:v_st_rhTvsRMperIetaD2HEM_[-ieta-18];
	const std::string& hPhi=(ieta>0)?v_st_rhTvsPhiperIetaD2HEP_[ieta-18]:v_st_rhTvsPhiperIetaD2HEM_[-ieta-18];
	myAH->fill1d<TProfile>(hRM,iRM,corTime_);
	myAH->fill1d<TProfile>(hPhi,iphi,corTime_);
      }
    }
    if (depth == 4) {
      if      ((ieta>=-15)&&(ieta<=-11))myAH->fill1d<TProfile>(st_avgTimePerPhiRing2M_,iphi,corTime_);
      else if ((ieta>=-10)&&(ieta<= -5))myAH->fill1d<TProfile>(st_avgTimePerPhiRing1M_,iphi,corTime_);
      else if ((ieta>= -4)&&(ieta<=  4))myAH->fill1d<TProfile>(st_avgTimePerPhiRing0_, iphi,corTime_);
      else if ((ieta>=  5)&&(ieta<= 10))myAH->fill1d<TProfile>(st_avgTimePerPhiRing1P_,iphi,corTime_);
      else if ((ieta>= 11)&&(ieta<= 15))myAH->fill1d<TProfile>(st_avgTimePerPhiRing2P_,iphi,corTime_);
    } else
      myAH->fill1d<TProfile> (avgPhiT, signed_iphi, corTime_);

#if 0
    // per-channel histograms
    uint32_t denseId = detID_.denseIndex();
    std::map<uint32_t,TH1F *>::const_iterator it=m_perChHistos_.find(denseId);
    if (it==m_perChHistos_.end())
      cerr << "No histo booked for detId " << detID_ << "!!!" << endl;
    else
      it->second->Fill(corTime_);
#endif

  } // if last cut
}                               // SplashDelayTunerAlgos::fillHistos4cut

//==================================================================

template<class Digi>
void
SplashDelayTunerAlgos::fillDigiPulse(TH1F *pulseHist,
				     const Digi& frame)
{
  for (int isample = 0; isample < std::min(10,frame.size()); ++isample) {
    int rawadc = frame[isample].adc();
    pulseHist->Fill(isample,rawadc);

  } // loop over samples in digi
}

//==================================================================

template<class Digi,class RecHit>
void SplashDelayTunerAlgos::processDigisAndRecHits (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
						    const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle)
{
  const edm::SortedCollection<RecHit>& rechits = *rechithandle;

  myAnalHistos *myAH = getHistos4cut("cut0none");
  myAH->fill1d<TH1F>(st_rhColSize_,rechits.size());

  if (digihandle.isValid())
    myAH->fill1d<TH1F>(st_digiColSize_,digihandle->size());

  totalE_ = 0.0;
  unsigned idig=0;
  for (unsigned irh=0; irh < rechits.size(); ++irh, idig++) {

    const RecHit& rh  = rechits[irh];

    if (rh.id().subdet() != mysubdet_)
      continue; // HB and HE handled by separate instances of this class!

    if (inSet<int>(detIds2mask_,rh.id().hashed_index())) continue;

    hittime_   = rh.time() - globalToffset_;
    hitenergy_ = rh.energy();
    hitflags_  = rh.flags();
    detID_     = rh.id();
    feID_      = lmap_->getHcalFrontEndId(detID_);
    totalE_   += hitenergy_;

    // splashcorns sounded too weird.
    splashCor_ns_  = timecor_->getTcor4(detID_);
    corTime_ = hittime_ - splashCor_ns_;

    TimesPerDetId::const_iterator it = exthitcors_.find(detID_);
    if (it != exthitcors_.end()) {
      // external hit correction to apply to hits for this det ID
      corTime_ -= it->second;
    }

    fillHistos4cut("cut0none");
    if (digihandle.isValid() && (idig < digihandle->size())) {
      const Digi& frame = (*digihandle)[idig];
      if (frame.id() != rh.id()) {
	cerr << "WARNING: digis and rechits aren't tracking..." << endl;
      }
      fillDigiPulse(getHistos4cut("cut0none")->get<TH1F>(st_avgPulse_),frame);
    }

    if (rh.energy() > minHitGeV_) {
      fillHistos4cut("cut1minHitGeV");
      if (digihandle.isValid() && (idig < digihandle->size())) {
	const Digi& frame = (*digihandle)[idig];
	fillDigiPulse(getHistos4cut("cut1minHitGeV")->get<TH1F>(st_avgPulse_),frame);
      }
      if (inSet<int>(acceptedBxNums_,bxnum_)) {
	fillHistos4cut("cut2bxnum");
	if (digihandle.isValid() && (idig < digihandle->size())) {
	  const Digi& frame = (*digihandle)[idig];
	  fillDigiPulse(getHistos4cut("cut2bxnum")->get<TH1F>(st_avgPulse_),frame);
	}
	if (!(hitflags_ & globalFlagMask_)) {
	  fillHistos4cut("cut3badFlags");
	  if (digihandle.isValid() && (idig < digihandle->size())) {
	    const Digi& frame = (*digihandle)[idig];
	    fillDigiPulse(getHistos4cut("cut3badFlags")->get<TH1F>(st_avgPulse_),frame);
	  }
	  if (notInSet<int>(badEventSet_,evtnum_)) {
//	  if (evtnum_<1061000) {                      // for run 120042
	    fillHistos4cut("cut4badEvents");
	    if (digihandle.isValid() && (idig < digihandle->size())) {
	      const Digi& frame = (*digihandle)[idig];
	      fillDigiPulse(getHistos4cut("cut4badEvents")->get<TH1F>(st_avgPulse_),frame);
	    }
#if 0
	    if (abs(detID_.ieta()) != 16) {
	      fillHistos4cut("cut5tower16");
	      if ((corTime_ >= timeWindowMinNS_) &&
		  (corTime_ <= timeWindowMaxNS_)   ) {
		fillHistos4cut("cut6aInTimeWindow");
	      }
	      if ((corTime_ < timeWindowMinNS_) ||
		  (corTime_ > timeWindowMaxNS_)   ) {
		fillHistos4cut("cut6bOutOfTimeWindow");
	      }
	    }
#endif
	  }
	}
      }
    }
    tree_->Fill();

  } // loop over rechits

  // now that we have the total energy...

  myAH->fill1d<TH1F>(st_totalEperEv_,evtnum_,totalE_);

}                               // SplashDelayTunerAlgos::processRecHits

//======================================================================

// ------------ method called to for each event  ------------
void
SplashDelayTunerAlgos::process(const myEventData& ed)
{
  if (firstEvent_) {
    bookHistos4allCuts();
    bookHistos4lastCut();
    runnum_ = ed.runNumber();
    firstEvent_ = false;
  }

  lsnum_  = ed.lumiSection();
  bxnum_  = ed.bxNumber();
  evtnum_ = ed.evtNumber();

  switch (mysubdet_) {
  case HcalBarrel:
  case HcalEndcap:  processDigisAndRecHits<HBHEDataFrame,HBHERecHit>(ed.hbhedigis(),ed.hbherechits()); break;
  case HcalOuter:   processDigisAndRecHits<HODataFrame,HORecHit>    (ed.hodigis(),  ed.horechits());   break;
  case HcalForward: processDigisAndRecHits<HFDataFrame,HFRecHit>    (ed.hfdigis(),  ed.hfrechits());   break;
  default: break;
  }
}

//======================================================================

void
SplashDelayTunerAlgos::beginJob(const edm::EventSetup& iSetup)
{
  edm::Service<TFileService> fs;

  tree_ = fs->make<TTree>("mytree","Splash Results Tree");
  tree_->Branch("feID",         &feID_,  32000, 1);
  tree_->Branch("detID",        &detID_, 32000, 1);
  tree_->Branch("bxnum",        &bxnum_, 32000, 1);
  tree_->Branch("lsnum",        &lsnum_, 32000, 1);
  tree_->Branch("evtnum",       &evtnum_,32000, 1);
  tree_->Branch("hittime",      &hittime_);
  tree_->Branch("hitenergy",    &hitenergy_);
  tree_->Branch("hitflags",     &hitflags_);
  tree_->Branch("splashCor_ns", &splashCor_ns_);
  tree_->Branch("corhittime_ns",&corTime_);

  std::cout << "----------------------------------------"  << "\n" <<
  std::cout << "Parameters being used: "             << "\n" <<
    "subdet_           = "      << mysubdetstr_      << "\n" << 
    "globalToffset_    = "      << globalToffset_    << "\n" << 
    "globalFlagMask_   = "<<hex << globalFlagMask_   << "\n" << 
    "minHitGeV_        = "      << minHitGeV_        << "\n" << 
#if 0
    "timeWindowMinNS_  = "      << timeWindowMinNS_  << "\n" << 
    "timeWindowMaxNS_  = "      << timeWindowMaxNS_  << "\n" <<
#endif
    "badEventSet_      = "<<dec;
  
  std::set<int>::const_iterator it;
  for (it=badEventSet_.begin(); it!=badEventSet_.end(); it++)
    std::cout << *it << ",";
  std::cout << std::endl;

  std::cout << "acceptedBxNums_   = ";

  for (it=acceptedBxNums_.begin(); it!=acceptedBxNums_.end(); it++)
    std::cout << *it << ",";
  std::cout << std::endl;

  std::cout << "Corrections list  = " << std::endl;
  int itnum=0;
  std::vector<tCorrection>::const_iterator tc;
  for (tc=corList_.begin(); tc!=corList_.end(); tc++) {
    std::cout << "Iteration #" << itnum++ << ":\n" <<
    "\tapplyExternal     = " << tc->applyExternal     << "\n" <<
    "\tcorrectByPhi      = " << tc->correctByPhi      << "\n" <<
    "\tcorrectByEta      = " << tc->correctByEta      << "\n" <<
    "\tselfSynchronize   = " << tc->selfSynchronize   << "\n" <<
    "\tietamin           = " << tc->ietamin           << "\n" <<
    "\tietamax           = " << tc->ietamax           << "\n" <<
    "\tiphimin           = " << tc->iphimin           << "\n" <<
    "\tiphimax           = " << tc->iphimax           << "\n" <<
    "\tdevthresh         = " << tc->devthresh         << "\n" <<
    "\tmirrorCorrection  = " << tc->mirrorCorrection  << "\n";
  }
  std::cout << "----------------------------------------" << std::endl;
}

//======================================================================

void
SplashDelayTunerAlgos::writeCorrections(const TimesPerDetId& chtimes)
{
  char name[40];
  sprintf(name,"timeCorrs_run%d_%s.csv",runnum_,mysubdetstr_.c_str());
  FILE *fp = fopen(name,"w");

  fprintf(fp,"SD\tieta\tiphi\tdpth\tRBX\tRM\tcard\tadc\tAvgT(ns)\n");
  TimesPerDetId::const_iterator it;
  for (it=chtimes.begin(); it!=chtimes.end(); it++) {
    HcalDetId     detID = it->first;
    float       timecor = it->second;
    HcalFrontEndId feID = lmap_->getHcalFrontEndId(detID);
    fprintf(fp,"%s\t%3d\t%3d\t%3d\t%s\t%2d\t%3d\t%2d\t%6.1f\n",
	    mysubdetstr_.c_str(),detID.ieta(),detID.iphi(),detID.depth(),
	    feID.rbx().c_str(),feID.rm(),feID.qieCard(),feID.adc(),
	    timecor);
  }
}                             // SplashDelayTunerAlgos::writeCorrections

//======================================================================

void
SplashDelayTunerAlgos::projectResults(const TimesPerDetId& chtimes, int itnum)
{
  char itstr[20];
  sprintf (itstr,"Iteration%d",itnum);

  myAnalHistos   *lastAH  = getHistos4cut(st_lastCut_);
  TFileDirectory *lastdir = lastAH->dir();
  TFileDirectory *projdir = new TFileDirectory(lastdir->mkdir("ProjectedOutcomes"));
  TFileDirectory *itdir   = new TFileDirectory(projdir->mkdir(itstr));
  std::vector<TProfile2D *> v_old(4,(TProfile2D*)0);
  std::vector<TProfile2D *> v_new(4,(TProfile2D*)0);

  if (mysubdet_ != HcalOuter) {
    v_old[0] = lastAH->get<TProfile2D>(st_rhTcorProfd1_);
    v_new[0] = itdir->make<TProfile2D>(string("h2d_rhTprojPerIetaIphiD1"+mysubdetstr_).c_str(),
      string("Depth 1 RecHit Time Map-Profile (projected), "+mysubdetstr_+";i#eta;i#phi").c_str(),
				   61, -30.5,  30.5, 72, 0.5, 72.5);

    v_old[1] = lastAH->get<TProfile2D>(st_rhTcorProfd2_);
    v_new[1] = itdir->make<TProfile2D>(string("h2d_rhTprojPerIetaIphiD2"+mysubdetstr_).c_str(),
      string("Depth 2 RecHit Time Map-Profile (projected), "+mysubdetstr_+";i#eta;i#phi").c_str(),
				   61, -30.5,  30.5, 72, 0.5, 72.5);

    if (mysubdet_ == HcalEndcap) {
      v_old[2] = lastAH->get<TProfile2D>(st_rhTcorProfd3_);
      v_new[2] = itdir->make<TProfile2D>(string("h2d_rhTprojPerIetaIphiD3"+mysubdetstr_).c_str(),
      string("Depth 3 RecHit Time Map-Profile (projected), " + mysubdetstr_ + "; i#eta; i#phi").c_str(),
				     61, -30.5,  30.5, 72, 0.5, 72.5);
    }
  } else {
    v_old[3] = lastAH->get<TProfile2D>(st_rhTcorProfd4_);
    v_new[3] = itdir->make<TProfile2D>("h2d_rhTprojPerIetaIphiD4HO",
      string("Depth 4 RecHit Time Map-Profile (projected), " + mysubdetstr_ + "; i#eta; i#phi").c_str(),
				   61, -30.5,  30.5, 72, 0.5, 72.5);
  }

  TH1F *projdist  = itdir->make<TH1F>("projdist","Projected Timing Distro; Time (ns)", 81,-10.125,10.125);

  TimesPerDetId::const_iterator it;
  for (int idpth=1; idpth<=4; idpth++) {

    TProfile2D *p2dold = v_old[idpth-1]; if (!p2dold) continue;
    TProfile2D *p2dnew = v_new[idpth-1];

    for (int ibinx=1; ibinx<=p2dold->GetNbinsX(); ibinx++) {
      for (int ibiny=1; ibiny<=p2dold->GetNbinsY(); ibiny++) {

	// check this is a valid detector coordinate:
	int ieta = (int)p2dold->GetXaxis()->GetBinCenter(ibinx); if (!ieta) continue;
	int iphi = (int)p2dold->GetYaxis()->GetBinCenter(ibiny);
	if (!HcalDetId::validDetId(mysubdet_,ieta,iphi,idpth)) continue;

	float       timecor = 0.;
	HcalDetId   detId(mysubdet_,ieta,iphi,idpth);
	it = chtimes.find(detId);
	if (it !=chtimes.end()) timecor = it->second;

	int  ibin  = p2dold->GetBin(ibinx,ibiny);
	if (p2dold->GetBinEntries(ibin)) {
	  float time = p2dold->GetBinContent(ibin)-round(timecor);
	  p2dnew->Fill(ieta,iphi,time);
	  projdist->Fill(time);
	}
      }
    }
    last2dprof_ = p2dnew; //hack for now
    last1ddist_ = projdist;
  }
}                               // SplashDelayTunerAlgos::projectResults

//======================================================================

void
SplashDelayTunerAlgos::endAnal()
{
  if (mysubdet_ == HcalBarrel) {
    TProfile *tp=getHistos4cut(st_lastCut_)->get<TProfile>(st_avgTcorPerIetad1_);
    float TetaMinus1 = tp->GetBinContent(tp->GetXaxis()->FindBin(-1));
    float TetaPlus1  = tp->GetBinContent(tp->GetXaxis()->FindBin( 1));

    cout << "Global offset calculated from HB center is: ";
    cout << (TetaMinus1+TetaPlus1)/2. << endl;
  }

  TimesPerDetId chtimes;
  detChannelTimes(chtimes);
  writeCorrections(chtimes);
}                                      // SplashDelayTunerAlgos::endAnal

//======================================================================

void
SplashDelayTunerAlgos::detChanCorsByPhiDepth(const std::vector<TProfile *>& profsByPhi,
					     const tCorrection& tcor,
					     TimesPerDetId& chtimes)
{
  for (size_t idpth=1; idpth<=4; idpth++) {
    TProfile *phiprof=profsByPhi[idpth-1];
    if (!phiprof) continue;
    for (int ibin=1; ibin<=phiprof->GetNbinsX(); ibin++) {
      int iphi = (int)phiprof->GetBinCenter(ibin);
      if (!iphi) continue;          // skip bin at 0 if iphi is signed
      double avgphitime = phiprof->GetBinContent(ibin);

      if (fabs(avgphitime) > tcor.devthresh) { // really #ns away from 0.
	//
	// same correction for all channels at the ieta values specified
	//
	std::pair<TimesPerDetId::iterator,bool> retval;
	for (int ieta=tcor.ietamin; ieta<=tcor.ietamax; ieta++) {
	  if (HcalDetId::validDetId(mysubdet_,ieta,iphi,idpth)) {
	    HcalDetId detId(mysubdet_,ieta,iphi,idpth);
	    retval = chtimes.insert(std::pair<HcalDetId,float>(detId,avgphitime));
	    if (!retval.second)
	      (retval.first)->second += avgphitime;  // pile correction on top of previous one.
	  }
	}
      }
    }
  }
}                        // SplashDelayTunerAlgos::detChanCorsByPhiDepth

//======================================================================

void
SplashDelayTunerAlgos::detChanCorsByEtaDepth(const std::vector<TProfile *>& profsByEta,
					     const tCorrection& tcor,
					     TimesPerDetId& chtimes)
{
  for (size_t idpth=1; idpth<=4; idpth++) {

    TProfile *etaprof=profsByEta[idpth-1];
    if (!etaprof) continue;

    for (int ibin=1; ibin<=etaprof->GetNbinsX(); ibin++) {

      int ieta = (int)etaprof->GetBinCenter(ibin);
      if (!ieta || (ieta<tcor.ietamin) || (ieta>tcor.ietamax)) continue;

      double avgetatime = etaprof->GetBinContent(ibin);

      if (fabs(avgetatime) > tcor.devthresh) { // really #ns away from 0.

	// same correction for all channels at the iphi values specified
	//
	std::pair<TimesPerDetId::iterator,bool> retval;
	for (int iphi=tcor.iphimin; iphi<=tcor.iphimax; iphi++) {
	  if (!iphi) continue;

	  if (HcalDetId::validDetId(mysubdet_,ieta,iphi,idpth)) {
	    HcalDetId detId(mysubdet_,ieta,iphi,idpth);
	    retval = chtimes.insert(std::pair<HcalDetId,float>(detId,avgetatime));
	    if (!retval.second)
	      (retval.first)->second += avgetatime;  // pile correction on top of previous one.

	    if (tcor.mirrorCorrection) {
	      HcalDetId detId(mysubdet_,-ieta,iphi,idpth);
	      retval = chtimes.insert(std::pair<HcalDetId,float>(detId,avgetatime));
	      if (!retval.second)
		(retval.first)->second += avgetatime;  // pile correction on top of previous one.
	    }
	  }
	}
      }
    }
  }
}                        // SplashDelayTunerAlgos::detChanCorsByEtaDepth

//======================================================================

void
SplashDelayTunerAlgos::detChanCorsByRingHO(TimesPerDetId& chtimes)
{
  myAnalHistos *lastAH = getHistos4cut(st_lastCut_);

  TProfile *hoPhiProf = lastAH->get<TProfile>(st_avgTimePerPhiRing2M_);

  for (int ibin=1; ibin<=hoPhiProf->GetNbinsX(); ibin++) {
    if (!hoPhiProf->GetBinEntries(ibin)) continue;
    int    iphi = (int)hoPhiProf->GetBinCenter(ibin);
    float avgphitime = hoPhiProf->GetBinContent(ibin);
    for (int ieta=-15; ieta<=-11; ieta++) {  // same correction for all channels at this ieta
      HcalDetId detID(mysubdet_,ieta,iphi,4);
      chtimes.insert(std::pair<HcalDetId,float>(detID,avgphitime));
    }
  }

  hoPhiProf = lastAH->get<TProfile>(st_avgTimePerPhiRing1M_);
  for (int ibin=1; ibin<=hoPhiProf->GetNbinsX(); ibin++) {
    if (!hoPhiProf->GetBinEntries(ibin)) continue;
    int    iphi = (int)hoPhiProf->GetBinCenter(ibin);
    float avgphitime = hoPhiProf->GetBinContent(ibin);
    for (int ieta=-10; ieta<=-5; ieta++) {  // same correction for all channels at this ieta
      HcalDetId detID(mysubdet_,ieta,iphi,4);
      chtimes.insert(std::pair<HcalDetId,float>(detID,avgphitime));
    }
  }

  hoPhiProf = lastAH->get<TProfile>(st_avgTimePerPhiRing0_);
  for (int ibin=1; ibin<=hoPhiProf->GetNbinsX(); ibin++) {
    if (!hoPhiProf->GetBinEntries(ibin)) continue;
    int    iphi = (int)hoPhiProf->GetBinCenter(ibin);
    float avgphitime = hoPhiProf->GetBinContent(ibin);
    for (int ieta=-4; ieta<=4; ieta++) {  // same correction for all channels at this ieta
      if (!ieta) continue;
      HcalDetId detID(mysubdet_,ieta,iphi,4);
      chtimes.insert(std::pair<HcalDetId,float>(detID,avgphitime));
    }
  }

  hoPhiProf = lastAH->get<TProfile>(st_avgTimePerPhiRing1P_);
  for (int ibin=1; ibin<=hoPhiProf->GetNbinsX(); ibin++) {
    if (!hoPhiProf->GetBinEntries(ibin)) continue;
    int    iphi = (int)hoPhiProf->GetBinCenter(ibin);
    float avgphitime = hoPhiProf->GetBinContent(ibin);
    for (int ieta=5; ieta<=10; ieta++) {  // same correction for all channels at this ieta
      HcalDetId detID(mysubdet_,ieta,iphi,4);
      chtimes.insert(std::pair<HcalDetId,float>(detID,avgphitime));
    }
  }

  hoPhiProf = lastAH->get<TProfile>(st_avgTimePerPhiRing2P_);
  for (int ibin=1; ibin<=hoPhiProf->GetNbinsX(); ibin++) {
    if (!hoPhiProf->GetBinEntries(ibin)) continue;
    int    iphi = (int)hoPhiProf->GetBinCenter(ibin);
    float avgphitime = hoPhiProf->GetBinContent(ibin);
    for (int ieta=11; ieta<=15; ieta++) {  // same correction for all channels at this ieta
      HcalDetId detID(mysubdet_,ieta,iphi,4);
      chtimes.insert(std::pair<HcalDetId,float>(detID,avgphitime));
    }
  }

}                          // SplashDelayTunerAlgos::detChanCorsByRingHO

//======================================================================

void
SplashDelayTunerAlgos::detChanCorsIndividually (const std::vector<TProfile2D *>& profs2d,
						const tCorrection& tcor,
						TimesPerDetId& chtimes)
{
  myAnalHistos *lastAH = getHistos4cut(st_lastCut_);
  TH1F *h1all=  (last1ddist_)? last1ddist_ : lastAH->get<TH1F>(st_rhCorTimes_);
  if (!h1all) 
    throw cms::Exception("Couldn't find histogram!") << st_rhCorTimes_ << endl;

  float histmean = h1all->GetMean();
  float histRMS  = h1all->GetRMS();

  float hival = histmean + 2.*histRMS;
  float loval = histmean - 2.*histRMS;
  h1all->Fit("gaus","Q","",loval,hival);

  TF1 *fit  = h1all->GetFunction("gaus");
  float fitmean  = fit->GetParameter(1);
  float fitsigma = fit->GetParameter(2);

  printf("Hist Mean, RMS   = %6.2f,%6.2f\n", histmean,histRMS);
  printf("Fit  Mean, Sigma = %6.2f,%6.2f\n", fitmean,fitsigma);
  printf("SD\tieta\tiphi\tdpth\tRBX\tRM\tcard\tadc\tpixel\tnent\tAvgT(ns)  #sigma dev.\n");

  for (size_t idpth=1; idpth<=4; idpth++) {

    TProfile2D *prof2d=profs2d[idpth-1];
    if (!prof2d) continue;

    for (int ibinx=1; ibinx<=prof2d->GetNbinsX(); ibinx++) {
      for (int ibiny=1; ibiny<=prof2d->GetNbinsY(); ibiny++) {
	// check this is a valid detector coordinate:
	int ieta = (int)prof2d->GetXaxis()->GetBinCenter(ibinx); if (!ieta) continue;
	int iphi = (int)prof2d->GetYaxis()->GetBinCenter(ibiny);
	if (!HcalDetId::validDetId(mysubdet_,ieta,iphi,idpth)) continue;
	if ((ieta<tcor.ietamin) || (ieta>tcor.ietamax) ||
	    (iphi<tcor.iphimin) || (iphi>tcor.iphimax)   ) continue;

	// Check if the individual channel is misaligned.
	//
	int        ibin = prof2d->GetBin(ibinx,ibiny);
	double  avgtime = prof2d->GetBinContent(ibin);
	int    nentries = (int)prof2d->GetBinEntries(ibin);
	float deviation = (avgtime-fitmean)/fitsigma;

	if (!nentries) continue;

	if ( (tcor.selfSynchronize&&(fabs(deviation)>tcor.devthresh)) ||
	    (!tcor.selfSynchronize&&(fabs(avgtime)  >tcor.devthresh))){/* really #ns away
									   from system 0.*/
	  HcalDetId detID(mysubdet_,ieta,iphi,idpth);
	  HcalFrontEndId feID = lmap_->getHcalFrontEndId(detID);

	  std::pair<TimesPerDetId::iterator,bool> retval;
	  retval = chtimes.insert(std::pair<HcalDetId,float>(detID,avgtime));
	  if (!retval.second)
	    (retval.first)->second += avgtime;  // pile correction on top of previous one.

	  printf("%s\t%3d\t%3d\t%3d\t%s\t%2d\t%3d\t%2d\t%4d\t%4d\t%6.1f\t%8.2f\n",
		 mysubdetstr_.c_str(),detID.ieta(),detID.iphi(),detID.depth(),
		 feID.rbx().c_str(),feID.rm(),feID.qieCard(),feID.adc(),feID.pixel(),
		 nentries,avgtime,deviation);
	}
      }
    }
  }  
}                      // SplashDelayTunerAlgos::detChanCorsIndividually

//======================================================================

void
SplashDelayTunerAlgos::applyExternalCorrections(TimesPerDetId& extcors,
						TimesPerDetId& chtimes)
{
  TimesPerDetId::const_iterator it;
  for (it=extcors.begin(); it!=extcors.end(); it++) {
    std::pair<TimesPerDetId::iterator,bool> retval;
    retval = chtimes.insert(*it);
    if (!retval.second)
      (retval.first)->second += it->second;  // pile correction on top of previous one.
  }
}                     // SplashDelayTunerAlgos::applyExternalCorrections

//======================================================================

void
SplashDelayTunerAlgos::processCorrection(const tCorrection& tcor,
					 TimesPerDetId& chtimes)
{
  myAnalHistos *lastAH = getHistos4cut(st_lastCut_);
  std::vector<TProfile *> profs(4,(TProfile*)0);

  if (tcor.applyExternal) {
    TimesPerDetId extcors;
    tcor.extcorInput->getTimeCorrections(extcors);
    applyExternalCorrections(extcors,chtimes);
    return;
  }
  // else...

  if (tcor.correctByPhi) {
    profs[0]=lastAH->get<TProfile>(st_avgTimePerPhid1_);
    profs[1]=lastAH->get<TProfile>(st_avgTimePerPhid2_);

    switch (mysubdet_) {
    case HcalEndcap:
      profs[2]=(lastAH->get<TProfile>(st_avgTimePerPhid3_));
      break; 
    case HcalOuter:
      detChanCorsByRingHO(chtimes);
      break;
    default: break;
    }
    detChanCorsByPhiDepth(profs, tcor, chtimes);
  }
  else if (tcor.correctByEta)  {
    profs[0] = lastAH->get<TProfile>(st_avgTcorPerIetad1_);
    profs[1] = lastAH->get<TProfile>(st_avgTcorPerIetad2_);
    if (mysubdet_==HcalEndcap)
      profs[2]= lastAH->get<TProfile>(st_avgTcorPerIetad3_);
    detChanCorsByEtaDepth(profs, tcor, chtimes);
  }
  else {
    std::vector<TProfile2D *> profs2d(4,(TProfile2D *)0);
    switch (mysubdet_) {
    case HcalEndcap:
      profs2d[2] = lastAH->get<TProfile2D>(st_rhTcorProfd3_); // break; 
    case HcalForward:
    case HcalBarrel:
      profs2d[0] = lastAH->get<TProfile2D>(st_rhTcorProfd1_);
      profs2d[1] = lastAH->get<TProfile2D>(st_rhTcorProfd2_);
      break;
    case HcalOuter:
      profs2d[3] = (last2dprof_)?last2dprof_:lastAH->get<TProfile2D>(st_rhTcorProfd4_); break;
    default: break;
    }
    detChanCorsIndividually(profs2d,tcor,chtimes);
  }
}                            // SplashDelayTunerAlgos::processCorrection

//======================================================================

void
SplashDelayTunerAlgos::detChannelTimes(TimesPerDetId& chtimes)
{
  last2dprof_=(TProfile2D*)0;
  last1ddist_=(TH1F*)0;
  for (size_t i=0; i<corList_.size(); i++) {
    tCorrection tcor = corList_[i];
    processCorrection(tcor,chtimes);
    projectResults(chtimes, i);
  }
}
