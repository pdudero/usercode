
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
// $Id: SplashDelayTunerAlgos.cc,v 1.2 2009/11/10 22:46:44 dudero Exp $
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
#include "TH1D.h"
#include "TH2D.h"
#include "TF2.h"
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
  sdpars_(iConfig.getParameter<edm::ParameterSet>("SubdetPars")),
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
  v_cuts_.push_back("cut5tower16");
  //v_cuts_.push_back("cut6aInTimeWindow");
  //v_cuts_.push_back("cut6bOutOfTimeWindow");
  st_lastCut_ = "cut5tower16";

  minHitGeV_          = sdpars_.getParameter<double>("minHitGeV");
#if 0
  timeWindowMinNS_    = sdpars_.getParameter<double>("timeWindowMinNS");
  timeWindowMaxNS_    = sdpars_.getParameter<double>("timeWindowMaxNS");
#endif
  recHitTscaleNbins_  = sdpars_.getParameter<int>   ("recHitTscaleNbins");
  recHitTscaleMinNs_  = sdpars_.getParameter<double>("recHitTscaleMinNs");
  recHitTscaleMaxNs_  = sdpars_.getParameter<double>("recHitTscaleMaxNs");
  recHitEscaleMinGeV_ = sdpars_.getParameter<double>("recHitEscaleMinGeV");
  recHitEscaleMaxGeV_ = sdpars_.getParameter<double>("recHitEscaleMaxGeV");
  maxEventNum2plot_   = sdpars_.getParameter<int>("maxEventNum2plot");

  std::vector<int> badEventVec =
    iConfig.getUntrackedParameter<vector<int> >("badEventList");
  for (size_t i=0; i<badEventVec.size(); i++)
    badEventSet_.insert(badEventVec[i]);

  std::vector<int> acceptedBxVec =
    iConfig.getUntrackedParameter<vector<int> >("acceptedBxNums");
  for (size_t i=0; i<acceptedBxVec.size(); i++)
    acceptedBxNums_.insert(acceptedBxVec[i]);

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

  rootFilename_      = iConfig.getUntrackedParameter<std::string>("rootFilename");
  hbRMprofileHistos_ = iConfig.getUntrackedParameter<std::vector<std::string> >("hbRMprofileHistos");
  heRMprofileHisto_  = iConfig.getUntrackedParameter<std::string>("heRMprofileHisto");
  hoRMprofileHisto_  = iConfig.getUntrackedParameter<std::string>("hoRMprofileHisto");

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

//======================================================================

myAnalCut *
SplashDelayTunerAlgos::findCut(const std::string& cutstr)
{
  std::map<std::string,myAnalCut *>::const_iterator it = m_cuts_.find(cutstr);
  if (it == m_cuts_.end())
    throw cms::Exception("Cut not found, you numnutz! You changed the name: ") << cutstr;

  return it->second;
}

//======================================================================

void
SplashDelayTunerAlgos::bookHistos(void)
{
  // Initialize the cuts for the run and add them to the global map
  char name[40];
  char title[40];

  m_cuts_.clear();

  cout << "Booking histos for subdet = " << mysubdetstr_ << std::endl;
//edm::LogInfo(

  for (unsigned i=0; i<v_cuts_.size(); i++)
    m_cuts_[v_cuts_[i]] = new myAnalCut(i,v_cuts_[i],mysubdetstr_);

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d; 
  std::vector<myAnalHistos::HistoParams_t> v_hpars1dprof;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d; 
  std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof;

  /*****************************************
   * 1-D HISTOGRAMS FIRST:                 *
   *****************************************/

  st_avgPulse_   = "h1d_pulse" + mysubdetstr_;
  add1dHisto(st_avgPulse_,"Average Pulse Shape, " + mysubdetstr_,10,-0.5,9.5,v_hpars1d);

  st_digiColSize_ = "DigiCollectionSize" + mysubdetstr_;
  add1dHisto( st_digiColSize_, "Digi Collection Size, " + mysubdetstr_, 
	      5201,-0.5, 5200.5, v_hpars1d); // 72chan/RBX*72RBX = 5184, more than HF or HO

  st_rhColSize_ = "RechitCollectionSize" + mysubdetstr_;
  add1dHisto( st_rhColSize_, "Rechit Collection Size, " + mysubdetstr_,
	      5201,-0.5, 5200.5,v_hpars1d);  // 72chan/RBX*72RBX = 5184, more than HF or HO
	     
  st_rhUncorTimes_ = "h1d_rhUncorTimes" + mysubdetstr_;
  add1dHisto( st_rhUncorTimes_,
	      "RecHit Times (uncorrected), " + mysubdetstr_ + "; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhCorTimes_ = "h1d_rhCorTimes" + mysubdetstr_;
  add1dHisto( st_rhCorTimes_,
	      "RecHit Times (corrected), " + mysubdetstr_ + "; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhEnergies_ = "h1d_RHEnergies" + mysubdetstr_;
  add1dHisto( st_rhEnergies_,
	      "RecHit Energies, " + mysubdetstr_ + "; Rechit Energy (GeV)",
	      (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	      recHitEscaleMinGeV_,recHitEscaleMaxGeV_,v_hpars1d);

  st_totalEperEv_ = "h1d_totalEperEvIn" + mysubdetstr_;
  add1dHisto( st_totalEperEv_,
  "#Sigma RecHit Energy Per Event in " + mysubdetstr_ + "; Event Number; Total Energy (GeV)",
	      501,-0.5,500.5,
	      v_hpars1d);

  st_rhCorTimesD1_ = "h1d_rhCorTimesD1" + mysubdetstr_;
  add1dHisto( st_rhCorTimesD1_,
	      "Depth 1 RecHit Times (corrected), " + mysubdetstr_ + "; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhFlagBits_ = "h1d_rhFlagBits" + mysubdetstr_;
  add1dHisto( st_rhFlagBits_,
	      "RecHit Quality Flag Bits, " + mysubdetstr_ + "; Flag Name",
	      20,0.5,20.5,v_hpars1d);  // see below (post-booking) for bin labeling
  
  st_rhHBHEtimingShapedCuts_ = "h1d_HBHEtimingShapedCuts" + mysubdetstr_;
  add1dHisto( st_rhHBHEtimingShapedCuts_,
	      "RecHit Timing Shaped Cuts Bits, " + mysubdetstr_ + "; Quality (0-7)",
	      10,-0.5,9.5,v_hpars1d);

  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  st_avgTperEvD1_ = "h1d_avgTperEvIn" + mysubdetstr_;
  add1dHisto( st_avgTperEvD1_,
	      "Depth 1 Averaged Time Per Event in " + mysubdetstr_ + "; Event Number; Average Time (ns)",
	      (maxEventNum2plot_+1),-0.5,((float)maxEventNum2plot_)+0.5,
	      v_hpars1dprof);

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
#if 0
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
#endif
      //
      // Make profiles of timing vs RM and phi for individual ietas/depths
      //
      for (int ieta=18; ieta<=29; ieta++) {
	sprintf(name,"p1d_rhTvsRM4ieta%02d2HEP",ieta);
	sprintf(title,"Corrected RecHit Time vs RM for i#eta=%d, depth=2 HEP",ieta);
	v_st_rhTvsRMperIetaD2HEP_.push_back(string(name));
	string titlestr = string(title);
	add1dHisto(v_st_rhTvsRMperIetaD2HEP_[ieta-18],titlestr,73,-0.5,72.5,v_hpars1dprof);

	sprintf(name,"p1d_rhTvsRM4ieta%02d2HEM",ieta);
	sprintf(title,"Corrected RecHit Time vs RM for i#eta=-%d, depth=2 HEM",ieta);
	v_st_rhTvsRMperIetaD2HEM_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsRMperIetaD2HEM_[ieta-18],titlestr,73,-0.5,72.5,v_hpars1dprof);

	sprintf(name,"p1d_rhTvsPhi4ieta%02d2HEP",ieta);
	sprintf(title,"Corrected RecHit Time vs i#phi for i#eta=%d, depth=2 HEP",ieta);
	v_st_rhTvsPhiperIetaD2HEP_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsPhiperIetaD2HEP_[ieta-18],titlestr,73,-0.5,72.5,v_hpars1dprof);

	sprintf(name,"p1d_rhTvsPhi4ieta%02d2HEM",ieta);
	sprintf(title,"Corrected RecHit Time vs i#phi for i#eta=-%d, depth=2 HEM",ieta);
	v_st_rhTvsPhiperIetaD2HEM_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsPhiperIetaD2HEM_[ieta-18],titlestr,73,-0.5,72.5,v_hpars1dprof);
      }

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
    }
  } else {
    st_avgTimePerRMd4_ = "p1d_avgTimePerRMd4HO";
    add1dHisto( st_avgTimePerRMd4_,
		"Averaged Time vs iRM, HO; iRM; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing0_ = "p1d_avgTimePerPhiHORing0";
    add1dHisto( st_avgTimePerPhiRing0_,
		"Averaged Time vs i#phi (YB0), HO; i#phi; Time (ns)",
		145,0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing1M_ = "p1d_avgTimePerPhiHORing1M";
    add1dHisto( st_avgTimePerPhiRing1M_,
		"Averaged Time vs i#phi (YB-1), HO; i#phi; Time (ns)",
		145,0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing1P_ = "p1d_avgTimePerPhiHORing1P";
    add1dHisto( st_avgTimePerPhiRing1P_,
		"Averaged Time vs i#phi (YB+1), HO; i#phi; Time (ns)",
		145,0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing2M_ = "p1d_avgTimePerPhiHORing2M";
    add1dHisto( st_avgTimePerPhiRing2M_,
		"Averaged Time vs i#phi (YB-2), HO; i#phi; Time (ns)",
		145,0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing2P_ = "p1d_avgTimePerPhiHORing2P";
    add1dHisto( st_avgTimePerPhiRing2P_,
		"Averaged Time vs i#phi (YB+2), HO; i#phi; Time (ns)",
		145,0.5, 72.5,v_hpars1dprof);

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

  std::map<string, myAnalCut *>::const_iterator it;
  for (it = m_cuts_.begin(); it != m_cuts_.end(); it++) {
    myAnalCut  *cut = it->second;
    myAnalHistos *myAH = cut->histos();
    myAH->book1d<TH1D>      (v_hpars1d);
    myAH->book1d<TProfile>  (v_hpars1dprof);
    myAH->book2d<TH2D>      (v_hpars2d);
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

    TH1D *h1d = myAH->get<TH1D>(st_rhFlagBits_);
    if (h1d) {
      TAxis *xax = h1d->GetXaxis();
      xax->SetBinLabel(1,(mysubdet_==HcalForward)?"HFLongShort":"HBHEHpdHitMultiplicity");
      xax->SetBinLabel(2,(mysubdet_==HcalForward)?"HFDigiTime":"HBHEPulseShape");
      xax->SetBinLabel(3,"HSCP_R1R2");
      xax->SetBinLabel(4,"HSCP_FracLeader");
      xax->SetBinLabel(5,"HSCP_OuterEnergy");
      xax->SetBinLabel(6,"HSCP_ExpFit");
      xax->SetBinLabel(7,(mysubdet_==HcalForward)?"HFTimingTrust0":"N/A");
      xax->SetBinLabel(8,(mysubdet_==HcalForward)?"HFTimingTrust1":"N/A");
      xax->SetBinLabel(9,"HBHETimingShapedCuts0");
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
}                                                          // bookHistos

//======================================================================
inline int    sign   (double x) { return (x>=0) ? 1 : -1; }

void
SplashDelayTunerAlgos::fillHistos4cut(const std::string& cutstr)
{
  //edm::LogInfo("Filling histograms for subdet ") << mysubdetstr_ << std::endl;

  myAnalHistos *myAH =   findCut(cutstr)->histos();

  int        ieta = detID_.ieta();
  int        iphi = detID_.iphi();
  int       depth = detID_.depth();
  int       zside = detID_.zside();

  // need front end id: 
  int            iRBX = atoi(((feID_.rbx()).substr(3,2)).c_str());
  int        iRMinRBX = feID_.rm();
  int            ipix = feID_.pixel();
  int             iRM = zside * ((iRBX-1)*4 + iRMinRBX);
  int     signed_iphi = zside*iphi;

  //cout << detId << "\t" << feID.rbx() << "\t" << feID.rbx().substr(3,2) << "\t";
  //cout << feID.rm() << " maps to RBX/RM# " << iRBX << "/" << iRM << endl;

  /* ALL subdet Histos: */
  myAH->fill1d<TH1D>(st_rhEnergies_,hitenergy_);
  myAH->fill2d<TH2D>(st_rhEmap_,ieta,iphi,hitenergy_);
  myAH->fill2d<TH2D>(st_uncorTimingVsE_,hitenergy_,hittime_);
  myAH->fill2d<TH2D>(st_corTimingVsE_,hitenergy_,corTime_);

  for (int ibit=0; ibit<20; ibit++) {
    int flagshift = (hitflags_>>ibit);
    if (ibit==8) {
      int timingCutQuality = flagshift & 7;
      myAH->fill1d<TH1D>(st_rhHBHEtimingShapedCuts_,timingCutQuality);
    }
    if (flagshift & 1) {
      myAH->fill1d<TH1D>(st_rhFlagBits_,ibit+1);
    }
  }

  /* Per Depth Histos: */

  std::string rhTuncProf,rhTcorProf,rhTprofRBX;
  std::string avgRMt,avgPhiT,avgIetaTunc,avgIetaTcor,avgRBXt;
  std::string sduncorTime,sdcorTime;
  switch(depth) {
  case 1:
    myAH->fill1d<TH1D>     (st_rhCorTimesD1_, corTime_);
    myAH->fill1d<TProfile> (st_avgTperEvD1_, evtnum_, corTime_);
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

  myAH->fill1d<TH1D>       (st_rhCorTimes_,           corTime_);
  myAH->fill1d<TH1D>       (st_rhUncorTimes_,         hittime_);

  myAH->fill1d<TProfile>   (avgRMt,     iRM,          corTime_); // for passing to delay tuner
  //myAH->fill1d<TProfile>   (avgPhiT,    signed_iphi,  corTime_); // for talking to everyone else! - but not for HO, see below
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
    //myAH->fill1d<TProfile> (v_st_rhTvsRMperPixHE_[ipix-1],iRM,corTime_);

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
  } else {
    myAH->fill1d<TProfile> (avgPhiT, signed_iphi, corTime_); // for talking to everyone else!
  }

}                               // SplashDelayTunerAlgos::fillHistos4cut

//======================================================================

template<class RecHit>
void SplashDelayTunerAlgos::processRecHits (const edm::SortedCollection<RecHit>& rechits)
{
  myAnalHistos *myAH = findCut("cut0none")->histos();
  myAH->fill1d<TH1D>(st_rhColSize_,rechits.size());

  totalE_ = 0.0;
  for (unsigned irh = 0;
       irh < rechits.size();
       ++irh) {

    const RecHit& rh = rechits[irh];

    if (rh.id().subdet() != mysubdet_)
      continue; // HB and HE handled by separate instances of this class!

    hittime_   = rh.time() - globalToffset_;
    hitenergy_ = rh.energy();
    hitflags_  = rh.flags();
    detID_     = rh.id();
    feID_      = lmap_->getHcalFrontEndId(detID_);
    totalE_   += hitenergy_;

    // splashcorns sounded too weird.
    splashCor_ns_  = timecor_->getTcor4(detID_);
    corTime_ = hittime_ - splashCor_ns_;

    fillHistos4cut("cut0none");
    if (rh.energy() > minHitGeV_) {
      fillHistos4cut("cut1minHitGeV");
      if (inSet<int>(acceptedBxNums_,bxnum_)) {
	fillHistos4cut("cut2bxnum");
	if (!(hitflags_ & globalFlagMask_)) {
	  fillHistos4cut("cut3badFlags");
	  //	if (notInSet<int>(badEventSet_,evtnum_)) {
	  if (evtnum_<1061000) {
	    fillHistos4cut("cut4badEvents");
	    if (abs(detID_.ieta()) != 16) {
	      fillHistos4cut("cut5tower16");
#if 0
	      if ((corTime_ >= timeWindowMinNS_) &&
		  (corTime_ <= timeWindowMaxNS_)   ) {
		fillHistos4cut("cut6aInTimeWindow");
	      }
	      if ((corTime_ < timeWindowMinNS_) ||
		  (corTime_ > timeWindowMaxNS_)   ) {
		fillHistos4cut("cut6bOutOfTimeWindow");
	      }
#endif
	    }
	  }
	}
      }
    }
    tree_->Fill();

  } // loop over rechits

  myAH->fill1d<TH1D>(st_totalEperEv_,evtnum_,totalE_);

}                               // SplashDelayTunerAlgos::processRecHits

//======================================================================

// ------------ method called to for each event  ------------
void
SplashDelayTunerAlgos::process(const myEventData& ed)
{
  if (firstEvent_) {
    bookHistos();
    firstEvent_ = false;
  }

  lsnum_  = ed.lumiSection();
  bxnum_  = ed.bxNumber();
  evtnum_ = ed.evtNumber();

  switch (mysubdet_) {
  case HcalBarrel:
  case HcalEndcap:  processRecHits<HBHERecHit>(*(ed.hbherechits())); break;
  case HcalOuter:   processRecHits<HORecHit>  (*(ed.horechits()));   break;
  case HcalForward: processRecHits<HFRecHit>  (*(ed.hfrechits()));   break;
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
  std::cout << "Parameters being used: "  << "\n" <<
    "subdet_           = " << mysubdetstr_      << "\n" << 
    "globalToffset_    = " << globalToffset_    << "\n" << 
    "globalFlagMask_   = " << globalFlagMask_   << "\n" << 
    "minHitGeV_        = " << minHitGeV_        << "\n" << 
#if 0
    "timeWindowMinNS_  = " << timeWindowMinNS_  << "\n" << 
    "timeWindowMaxNS_  = " << timeWindowMaxNS_  << "\n" <<
#endif
    "badEventSet_      = ";
  
  std::set<int>::const_iterator it;
  for (it=badEventSet_.begin(); it!=badEventSet_.end(); it++)
    std::cout << *it << ",";
  std::cout << std::endl;

  std::cout << "acceptedBxNums_   = ";

  for (it=acceptedBxNums_.begin(); it!=acceptedBxNums_.end(); it++)
    std::cout << *it << ",";
  std::cout << std::endl;

  std::cout << "----------------------------------------" << std::endl;
}

//======================================================================

void
SplashDelayTunerAlgos::endAnal()
{
  if (mysubdet_ == HcalBarrel) {
    TProfile *tp=findCut(st_lastCut_)->histos()->get<TProfile>(st_avgTcorPerIetad1_);
    float TetaMinus1 = tp->GetBinContent(tp->GetXaxis()->FindBin(-1));
    float TetaPlus1  = tp->GetBinContent(tp->GetXaxis()->FindBin( 1));

    cout << "Global offset calculated from HB center is: ";
    cout << (TetaMinus1+TetaPlus1)/2. << endl;
  }
}

//======================================================================

void
SplashDelayTunerAlgos::endJob(const DelaySettings& oldsettings)
{
  ChannelTimes chtimes;
  detChannelTimes(chtimes);
  HcalDelayTunerAlgos::endJob(chtimes,oldsettings);
}

//======================================================================

void
SplashDelayTunerAlgos::detChannelTimesHB(std::vector<TProfile *>hbRMprofs,
					 ChannelTimes& chtimes)
{
  double maxtime=-1e99;
  double mintime= 1e99;

  printf ("\nbin\tiRM\tiRBX\tRMinRBX\tavgt(ns)  (TS)\n");
  for (uint32_t i=0; i<hbRMprofs.size(); i++) {
    TProfile *hbRMprof = hbRMprofs[i];
    for (int ibin=1; ibin<=hbRMprof->GetNbinsX(); ibin++) {
      int iRM = (int)hbRMprof->GetBinCenter(ibin);
      if (!iRM) continue;                     // skip bin at 0

      int iRBX     = (iRM - sign(iRM))/4;
      int iRMinRBX = (abs(iRM - sign(iRM))%4) + 1;
      iRBX += sign(iRM);
      char rbx[10];
      sprintf (rbx,"HB%c%02d",((iRM>0)?'P':'M'),abs(iRBX));

      HcalFrontEndId feID(std::string(rbx),iRMinRBX,0,1,0,1,0);
    
      double rmtime = hbRMprof->GetBinContent(ibin);

      if (rmtime < mintime) mintime=rmtime;
      if (rmtime > maxtime) maxtime=rmtime;

      printf ("%3d\t%3d\t%3d\t%s\t%d\t%5.1f\t%6.2f\n",
	      ibin, iRM, iRBX, rbx, iRMinRBX, rmtime,rmtime/25.);

      // same correction for all channels in this RM
      for (int i=1; i<=3; i++) {
	for (int j=0; j<=5; j++) {
	  HcalFrontEndId feID(rbx,iRMinRBX,0,1,0,i,j);
	  //cout<<"feID for "<<rbx<<"RM="<<iRMinRBX<<" is "<<feID<<endl;
	  chtimes.insert(std::pair<HcalFrontEndId,double>(feID,rmtime));
	}
      }
    } // RM loop
    cout << "time spread = " << maxtime-mintime << endl;
  }
}                          // SplashDelayTunerAlgos::detChannelTimesHB

//======================================================================

void
SplashDelayTunerAlgos::detChannelTimesHE(TProfile *heRMprof,
					 ChannelTimes& chtimes)
{
  double maxtime=-1e99;
  double mintime= 1e99;

  printf ("\nbin\tiRM\tiRBX\tRMinRBX\tavgt(ns)  (TS)\n");
  for (int ibin=1; ibin<=heRMprof->GetNbinsX(); ibin++) {
    // each bin of this histogram is expected to represent the average over
    // two adjacent RMs: (1+2) or (3+ 4)
    // have to recover the original numbering
    //
    int iRMavg2 = (int)heRMprof->GetBinCenter(ibin);
    if (!iRMavg2) continue;                     // skip bin at 0

    int iRM1     = 2*(iRMavg2-sign(iRMavg2)) + sign(iRMavg2);
    int iRM2     = iRM1+sign(iRM1);
    int iRBX     = (iRM1 - sign(iRM1))/4;
    int iRMinRBX1 = (abs(iRM1 - sign(iRM1))%4) + 1;
    int iRMinRBX2 = (abs(iRM2 - sign(iRM2))%4) + 1;

    iRBX += sign(iRM1);
    char rbx[10];
    sprintf (rbx,"HE%c%02d",((iRM1>0)?'P':'M'),abs(iRBX));
    
    double rmtime = heRMprof->GetBinContent(ibin);

    if (rmtime < mintime) mintime=rmtime;
    if (rmtime > maxtime) maxtime=rmtime;

    printf ("%3d\t%3d\t%3d\t%3d\t%s\t%d\t%d\t%5.1f\t%6.2f\n",
	    ibin, iRM1,iRM2,iRBX,rbx,iRMinRBX1,iRMinRBX2, rmtime,rmtime/25.);

    // same correction for all channels in both RMs
    for (int i=1; i<=3; i++) {
      for (int j=0; j<=5; j++) {
	HcalFrontEndId feID(rbx,iRMinRBX1,0,1,0,i,j);
	//cout<<"feID for "<<rbx<<"RM="<<iRMinRBX<<" is "<<feID<<endl;
	chtimes.insert(std::pair<HcalFrontEndId,double>(feID,rmtime));
      }
    }
    for (int i=1; i<=3; i++) {
      for (int j=0; j<=5; j++) {
	HcalFrontEndId feID(rbx,iRMinRBX2,0,1,0,i,j);
	chtimes.insert(std::pair<HcalFrontEndId,double>(feID,rmtime));
      }
    }
  } // RM loop
  cout << "time spread = " << maxtime-mintime << endl;

}                          // SplashDelayTunerAlgos::detChannelTimesHE

//======================================================================

void
SplashDelayTunerAlgos::detChannelTimes(ChannelTimes& chtimes)
{
  TFile *rootfile = new TFile(rootFilename_.c_str());

  if (rootfile->IsZombie()) {
    throw cms::Exception("File failed to open, ") << rootFilename_;
  }

  /*****************************************
   *                  HB
   ****************************************/

  std::vector<TProfile *>hbRMprofs;
  for (uint32_t i=0; i<hbRMprofileHistos_.size(); i++)
    hbRMprofs.push_back((TProfile *)rootfile->Get(hbRMprofileHistos_[i].c_str()));

  if (hbRMprofs.size())
    detChannelTimesHB(hbRMprofs,chtimes);
  else
    cout << "No profile for HB, moveon.org to the next..." << endl;

  /********* THIS IS JUST FOR PRINTING OUT BY IPHI **********/

  TProfile *hbPhiProf =
    (TProfile *)rootfile->Get("hbtimeanal/HB/cutBadEvents/p1d_avgTimePerPhid1HB");

  if (hbPhiProf) {
    printf ("\nbin\tiphi\tavgt(ns)  (TS)\n");
    for (int ibin=1; ibin<=hbPhiProf->GetNbinsX(); ibin++) {
      int iphi = (int)hbPhiProf->GetBinCenter(ibin);
      if (!iphi) continue;                     // skip bin at 0

      double avgtimeThisPhi = hbPhiProf->GetBinContent(ibin);

      printf ("%3d\t%3d\t%5.1f\t%6.2f\n",
	      ibin, iphi, avgtimeThisPhi, avgtimeThisPhi/25.);
    }
  }

  /***************************************************
   *                       HE
   *  NOTE: take the average for two consecutive RMs
   **************************************************/

  TProfile *heRMprof=0;
  if (heRMprofileHisto_.size())
    heRMprof = (TProfile *)rootfile->Get(heRMprofileHisto_.c_str());

  if (heRMprof)
    detChannelTimesHE(heRMprof,chtimes);
  else
    cout << "No profile for HE, moveon.org to the next..." << endl;

  /*****************************************
   *                  HO
   ****************************************/

  //TProfile *hoRMprof = (TProfile *)rootfile->Get();

}                               // SplashDelayTunerAlgos::detChannelTimes
