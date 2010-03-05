
// -*- C++ -*-
//
// Package:    HcaDelayTunerAlgos
// Class:      HcaDelayTunerAlgos
// 
/**\class HcaDelayTunerAlgos HcaDelayTunerAlgos.cc MyEDmodules/HcaDelayTunerAlgos/src/HcaDelayTunerAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalDelayTunerAlgos.cc,v 1.11 2010/03/05 13:25:21 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>
#include <stdlib.h> // itoa
#include <math.h> // floor

// user include files
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"
#include "MyEDmodules/HcalDelayTuner/interface/HcalDelayTunerAlgos.hh"


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

const int nEbins= 12;
static const double digEbins[] = {
  1.0,2.0,3.0,4.0,5.0,7.0,10.0,20.0,40.0,100.0,300.0,600.0,1000.0
};

//
// constructors and destructor
//
HcalDelayTunerAlgos::HcalDelayTunerAlgos(const edm::ParameterSet& iConfig) :
  mysubdetstr_(iConfig.getUntrackedParameter<std::string>("subdet")),
  globalToffset_(iConfig.getParameter<double>("globalTimeOffset")),
  globalFlagMask_(iConfig.getParameter<int>("globalRecHitFlagMask")),
  rundescr_(iConfig.getUntrackedParameter<std::string>("runDescription",""))
{
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
  maxEventNum2plot_   = iConfig.getParameter<int>   ("maxEventNum2plot");
  normalizeDigis_     = iConfig.getParameter<bool>  ("normalizeDigis");
  doPerChannel_       = iConfig.getParameter<bool>  ("doPerChannel");

  std::vector<int> v_maskidnumbers =
    iConfig.getParameter<vector<int> >("detIds2mask");

  if (!buildMaskSet(v_maskidnumbers))
    throw cms::Exception("Invalid detID vector");

  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());

  if (!mysubdetstr_.compare("HB")) mysubdet_ = HcalBarrel;  else 	 
  if (!mysubdetstr_.compare("HE")) mysubdet_ = HcalEndcap;  else 	 
  if (!mysubdetstr_.compare("HO")) mysubdet_ = HcalOuter;   else 	 
  if (!mysubdetstr_.compare("HF")) mysubdet_ = HcalForward; else { 	 
    edm::LogWarning("Warning: subdetector set to 'other', ") << mysubdetstr_; 	 
    mysubdet_ = HcalOther;
    mysubdetstr_ = "FU";
  } 	 

  firstEvent_ = true;
}                            // HcalDelayTunerAlgos::HcalDelayTunerAlgos

//======================================================================

bool
HcalDelayTunerAlgos::buildMaskSet(const std::vector<int>& v_idnumbers)
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
}                                   // HcalDelayTunerAlgos::buildMaskSet

//======================================================================

void
HcalDelayTunerAlgos::compileCorrections
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
}                             // HcalDelayTunerAlgos::compileCorrections

//======================================================================

void
HcalDelayTunerAlgos::add1dHisto(const std::string& name, const std::string& title,
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
}                                     // HcalDelayTunerAlgos::add1dHisto

//======================================================================

void
HcalDelayTunerAlgos::add2dHisto(const std::string& name, const std::string& title,
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
}                                     // HcalDelayTunerAlgos::add2dHisto

//======================================================================

myAnalHistos *
HcalDelayTunerAlgos::getHistos4cut(const std::string& cutstr)
{
  std::map<std::string,myAnalCut *>::const_iterator it = m_cuts_.find(cutstr);
  if (it == m_cuts_.end())
    throw cms::Exception("Cut not found, you numnutz! You changed the name: ") << cutstr << endl;

  return it->second->histos();
}                                  // HcalDelayTunerAlgos::getHistos4cut

//======================================================================

void
HcalDelayTunerAlgos::bookHistos4allCuts(void)
{
  // Initialize the cuts for the run and add them to the global map
  m_cuts_.clear();

  string runnumstr = int2str(runnum_);

  for (unsigned i=0; i<v_cuts_.size(); i++)
    m_cuts_[v_cuts_[i]] = new myAnalCut(i,v_cuts_[i],mysubdetstr_);

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d; 
  std::vector<myAnalHistos::HistoParams_t> v_hpars1dprof;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d; 
  //std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof; // all in last cut only

  /*****************************************
   * 1-D HISTOGRAMS FIRST:                 *
   *****************************************/

  //==================== Collection sizes ====================

  st_digiColSize_ = "DigiCollectionSize" + mysubdetstr_;
  add1dHisto( st_digiColSize_, "Digi Collection Size, "+mysubdetstr_+", Run "+runnumstr,
	      5201,-0.5, 5200.5, v_hpars1d); // 72chan/RBX*72RBX = 5184, more than HF or HO

  st_rhColSize_ = "RechitCollectionSize" + mysubdetstr_;
  add1dHisto( st_rhColSize_, "Rechit Collection Size, "+mysubdetstr_+", Run "+runnumstr,
	      5201,-0.5, 5200.5,v_hpars1d);  // 72chan/RBX*72RBX = 5184, more than HF or HO

  //==================== Total time/energy/flags distros ====================
	     
  st_rhUncorTimes_ = "h1d_rhUncorTimes" + mysubdetstr_;
  add1dHisto( st_rhUncorTimes_,
	      "RecHit Times (uncorrected), "+mysubdetstr_+", Run "+runnumstr+"; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhCorTimes_ = "h1d_rhCorTimes" + mysubdetstr_;
  add1dHisto( st_rhCorTimes_,
	      "RecHit Times, "+mysubdetstr_+", Run "+runnumstr+"; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhCorTimesPlus_ = "h1d_rhCorTimes" + mysubdetstr_ + "P";
  add1dHisto( st_rhCorTimesPlus_,
	      "RecHit Times, "+mysubdetstr_+"P, Run "+runnumstr+"; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhCorTimesMinus_ = "h1d_rhCorTimes" + mysubdetstr_ + "M";
  add1dHisto( st_rhCorTimesMinus_,
	      "RecHit Times, "+mysubdetstr_+"M, Run "+runnumstr+"; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhCorTimesD1_ = "h1d_rhCorTimesD1" + mysubdetstr_;
  add1dHisto( st_rhCorTimesD1_,
	      "Depth 1 RecHit Times, "+mysubdetstr_+", Run "+runnumstr+"; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhEnergies_ = "h1d_RHEnergies" + mysubdetstr_;
  add1dHisto( st_rhEnergies_,
	      "RecHit Energies, "+mysubdetstr_+", Run "+runnumstr+"; Rechit Energy (GeV)",
	      (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	      recHitEscaleMinGeV_,recHitEscaleMaxGeV_,v_hpars1d);

  st_rhFlagBits_ = "h1d_rhFlagBits" + mysubdetstr_;
  add1dHisto( st_rhFlagBits_,
	      "RecHit Quality Flag Bits, "+mysubdetstr_+", Run "+runnumstr+"; Flag Name",
	      20,0.5,20.5,v_hpars1d);  // see below (post-booking) for bin labeling
  
  st_rhHBHEtimingShapedCuts_ = "h1d_HBHEtimingShapedCuts" + mysubdetstr_;
  add1dHisto( st_rhHBHEtimingShapedCuts_,
	      "RecHit Timing Shaped Cuts Bits, "+mysubdetstr_+", Run "+runnumstr+"; Quality (0-7)",
	      10,-0.5,9.5,v_hpars1d);

  //==================== ...by Event ====================

  st_totalEperEv_ = "h1d_totalEperEvIn" + mysubdetstr_;
  add1dHisto( st_totalEperEv_,
  "#Sigma RecHit Energy Per Event in "+mysubdetstr_+", Run "+runnumstr+"; Event Number; Total Energy (GeV)",
//	      (maxEventNum2plot_+1),-0.5,((float)maxEventNum2plot_)+0.5,
	      100,-0.5,((float)maxEventNum2plot_)+0.5,
	      v_hpars1d);

  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  st_avgPulse_   = "h1d_pulse" + mysubdetstr_;
  add1dHisto(st_avgPulse_,"Average Pulse Shape, " + mysubdetstr_,10,-0.5,9.5,v_hpars1dprof);

  st_avgPulsePlus_   = "h1d_pulse" + mysubdetstr_ + "P";
  add1dHisto(st_avgPulsePlus_,"Average Pulse Shape, " + mysubdetstr_+"P",10,-0.5,9.5,v_hpars1dprof);

  st_avgPulseMinus_   = "h1d_pulse" + mysubdetstr_ + "M";
  add1dHisto(st_avgPulseMinus_,"Average Pulse Shape, " + mysubdetstr_+"M",10,-0.5,9.5,v_hpars1dprof);

  st_avgTperEvD1_ = "h1d_avgTperEvIn" + mysubdetstr_;
  add1dHisto( st_avgTperEvD1_,
	      "Depth 1 Averaged Time Per Event in "+mysubdetstr_+", Run "+runnumstr+"; Event Number; Average Time (ns)",
//	      (maxEventNum2plot_+1),-0.5,((float)maxEventNum2plot_)+0.5,
	      100,-0.5,((float)maxEventNum2plot_)+0.5,
	      v_hpars1dprof);

  /*****************************************
   * 2-D HISTOGRAMS AFTER:                 *
   *****************************************/

  st_rhEmap_        = "h2d_rhEperIetaIphi" + mysubdetstr_;
  add2dHisto(st_rhEmap_,
	     "RecHit Energy Map (#Sigma depths), "+mysubdetstr_+", Run "+runnumstr+"; i#eta; i#phi",
	     83, -41.5,  41.5, 72,   0.5,  72.5, v_hpars2d);

  st_uncorTimingVsE_ = "h2d_uncorTimingVsE" + mysubdetstr_;
  add2dHisto(st_uncorTimingVsE_,
"RecHit Timing (uncorrected) vs. Energy, "+mysubdetstr_+"; Rechit Energy (GeV); Rechit Time (ns)",
	     (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	     recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	     v_hpars2d);

  st_corTimingVsE_ = "h2d_corTimingVsE" + mysubdetstr_;
  add2dHisto(st_corTimingVsE_,
"RecHit Timing vs. Energy, "+mysubdetstr_+"; Rechit Energy (GeV); Rechit Time (ns)",
	     (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	     recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	     v_hpars2d);

  st_corTimingVsED1_ = "h2d_corTimingVsED1" + mysubdetstr_;
  add2dHisto(st_corTimingVsED1_,
"RecHit Timing vs. Energy, "+mysubdetstr_+" Depth 1; Rechit Energy (GeV); Rechit Time (ns)",
	     (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	     recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	     v_hpars2d);

  st_nHitsPerIetaIphi_  = "p2d_NhitsPerIetaIphi" + mysubdetstr_;
  add2dHisto(st_nHitsPerIetaIphi_,
	     "RecHit Occupancy Map, "+mysubdetstr_+", Run "+runnumstr+"; i#eta; i#phi",
	     83, -41.5,  41.5, 72, 0.5, 72.5,
	     v_hpars2d);

  /*****************************************
   * BOOK 'EM, DANNO...                    *
   *****************************************/

  uint32_t total = v_hpars1d.size()+v_hpars1dprof.size()+v_hpars2d.size();

  cout<<mysubdetstr_<<": Booking "<<total<<" histos for each of "<<v_cuts_.size()<<" cuts..."<<std::endl;
//edm::LogInfo(

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
  cout<<"Done."<<std::endl;

}                           // HcalDelayTunerAlgos::bookHistos4allCuts

//======================================================================

void
HcalDelayTunerAlgos::bookHistos4lastCut(void)
{
  char title[128]; std::string titlestr;

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d;
  //std::vector<myAnalHistos::HistoParams_t> v_hpars2d;  // in all cuts
  std::vector<myAnalHistos::HistoParams_t> v_hpars1dprof;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof;

  string runnumstr = int2str(runnum_);

  /*****************************************
   * 1-D HISTOGRAMS:                       *
   *****************************************/

  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  st_avgTimePerRBX_ =  "p1d_avgTimePerRBX" + mysubdetstr_;
  sprintf (title, "Avg. Time/RBX, %s, Run %d; iRBX; Time (ns)",
	   mysubdetstr_.c_str(), runnum_);
  titlestr = string(title);
  add1dHisto( st_avgTimePerRBX_, titlestr, 37,-18.5, 18.5, v_hpars1dprof);

  //==================== ...by phi/eta/depth ====================

  if (mysubdet_ != HcalOuter) { 
    st_avgTimePerRMd1_ =  "p1d_avgTimePerRMd1" + mysubdetstr_;
    sprintf (title, "Avg. Time, %s Depth 1 (Run %d); iRM; Time (ns)",mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_avgTimePerRMd1_, titlestr, 145,-72.5, 72.5, v_hpars1dprof);

    st_avgTimePerRMd2_ =  "p1d_avgTimePerRMd2" + mysubdetstr_;
    sprintf (title, "Avg. Time, %s Depth 2 (Run %d); iRM; Time (ns)",mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_avgTimePerRMd2_,titlestr, 145,-72.5, 72.5, v_hpars1dprof);

    st_avgTimePerPhid1_ =  "p1d_avgTimePerPhid1" + mysubdetstr_;
    sprintf (title, "Avg. Time, %s Depth 1 (Run %d); i#phi; Time (ns)",mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_avgTimePerPhid1_,titlestr, 145,-72.5, 72.5, v_hpars1dprof);

    st_avgTimePerPhid2_ =  "p1d_avgTimePerPhid2" + mysubdetstr_;
    sprintf (title, "Avg. Time, %s Depth 2 (Run %d); i#phi; Time (ns)",mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_avgTimePerPhid2_, titlestr, 145,-72.5, 72.5, v_hpars1dprof);

    st_avgTuncPerIetad1_ =  "p1d_avgTuncPerIetad1" + mysubdetstr_;
    sprintf (title, "Avg. Time (Uncorrected), %s Depth 1, Run %d; i#eta; Time (ns)",
	     mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_avgTuncPerIetad1_, titlestr,	83,-41.5, 41.5, v_hpars1dprof);

    st_avgTuncPerIetad2_ =  "p1d_avgTuncPerIetad2" + mysubdetstr_;
    sprintf (title, "Avg. Time (Uncorrected), %s Depth 2, Run %d; i#eta; Time (ns)",
	     mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_avgTuncPerIetad2_, titlestr, 83,-41.5, 41.5, v_hpars1dprof);

    st_avgTcorPerIetad1_ =  "p1d_avgTcorPerIetad1" + mysubdetstr_;
    sprintf (title, "Avg. Time, %s Depth 1, Run %d; i#eta; Time (ns)",
	     mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_avgTcorPerIetad1_, titlestr, 83,-41.5, 41.5, v_hpars1dprof);

    st_avgTcorPerIetad2_ =  "p1d_avgTcorPerIetad2" + mysubdetstr_;
    sprintf (title, "Avg. Time, %s Depth 2, Run %d; i#eta; Time (ns)",
	     mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_avgTcorPerIetad2_, titlestr, 83,-41.5, 41.5, v_hpars1dprof);

    if (mysubdet_ == HcalEndcap) {
      //
      // Detail histos for the last cut only
      //
      st_avgTimePer2RMs_ =  "p1d_avgTimePer2RMs" + mysubdetstr_;
      add1dHisto( st_avgTimePer2RMs_,
		  "Avg. Time over 2RMs, HE, Run "+runnumstr+"; iRM/2; Time (ns)",
		  73,-36.5, 36.5,v_hpars1dprof);

      st_avgTimePerRMd3_ =  "p1d_avgTimePerRMd3HE";
      add1dHisto( st_avgTimePerRMd3_,
		  "Avg. Time (Depth 3), HE, Run "+runnumstr+"; iRM; Time (ns)",
		  145,-72.5, 72.5,v_hpars1dprof);

      st_avgTimePerPhid3_ =  "p1d_avgTimePerPhid3HE";
      add1dHisto( st_avgTimePerPhid3_,
		  "Avg. Time (Depth 3), HE, Run "+runnumstr+"; i#phi; Time (ns)",
		  145,-72.5, 72.5,v_hpars1dprof);

      st_avgTuncPerIetad3_ =  "p1d_avgTuncPerIetad3HE";
      add1dHisto( st_avgTuncPerIetad3_,
		  "Avg. Time (Depth 3), HE, Run "+runnumstr+"; i#eta; Time (ns)",
		  61,-30.5, 30.5,v_hpars1dprof);

      st_avgTcorPerIetad3_ =  "p1d_avgTcorPerIetad3HE";
      add1dHisto( st_avgTcorPerIetad3_,
		  "Avg. Time (Depth 3), HE, Run "+runnumstr+"; i#eta; Time (ns)",
		  61,-30.5, 30.5,v_hpars1dprof);

#if 0
      //
      // Make profiles of timing vs RM for individual pixels
      //
      char name[40];
      for (int ipix=1; ipix<=19; ipix++) {
	sprintf(name,"p1d_rhTvsRM4pix%02dHE",ipix);
	sprintf(title,"RecHit Time/RM for pixel %d, HE, Run %d",ipix, runnum_);
	v_st_rhTvsRMperPixHE_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsRMperPixHE_[ipix-1],titlestr,145,-72.5,72.5,v_hpars1dprof);
      }

      //
      // Make profiles of timing vs RM and phi for individual ietas/depths
      //
      for (int ieta=18; ieta<=29; ieta++) {
	
	sprintf(name,"p1d_rhTvsRM4ieta%02dd2HEP",ieta);
	sprintf(title,"RecHit Time/RM for i#eta=%d, depth=2 HEP, Run %d",ieta, runnum_);
	v_st_rhTvsRMperIetaD2HEP_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsRMperIetaD2HEP_[ieta-18],titlestr,72,0.5,72.5,v_hpars1dprof);
      
	sprintf(name,"p1d_rhTvsRM4ieta%02dd2HEM",ieta);
	sprintf(title,"RecHit Time/RM for i#eta=-%d, depth=2 HEM, Run %d",ieta, runnum_);
	v_st_rhTvsRMperIetaD2HEM_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsRMperIetaD2HEM_[ieta-18],titlestr,72,0.5,72.5,v_hpars1dprof);
      
	sprintf(name,"p1d_rhTvsPhi4ieta%02dd2HEP",ieta);
	sprintf(title,"RecHit Time/i#phi for i#eta=%d, depth=2 HEP, Run %d",ieta, runnum_);
	v_st_rhTvsPhiperIetaD2HEP_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsPhiperIetaD2HEP_[ieta-18],titlestr,72,0.5,72.5,v_hpars1dprof);
	
	sprintf(name,"p1d_rhTvsPhi4ieta%02dd2HEM",ieta);
	sprintf(title,"RecHit Time/i#phi for i#eta=-%d, depth=2 HEM, Run %d",ieta, runnum_);
	v_st_rhTvsPhiperIetaD2HEM_.push_back(string(name));
	titlestr = string(title);
	add1dHisto(v_st_rhTvsPhiperIetaD2HEM_[ieta-18],titlestr,72,0.5,72.5,v_hpars1dprof);

      }           // loop over ieta
#endif
    }   // if HE

  } else {     // HO:

    st_avgTimePerRMd4_ = "p1d_avgTimePerRMd4HO";
    add1dHisto( st_avgTimePerRMd4_,
		"Avg. Time/iRM, HO, Run "+runnumstr+"; iRM; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

  //==================== ...by Ring ====================

    st_avgTimePerPhiRing0_ = "p1d_avgTimePerPhiHORing0";
    add1dHisto( st_avgTimePerPhiRing0_,
		"Avg. Time/i#phi (YB0), HO, Run "+runnumstr+"; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing1M_ = "p1d_avgTimePerPhiHORing1M";
    add1dHisto( st_avgTimePerPhiRing1M_,
		"Avg. Time/i#phi (YB-1), HO, Run "+runnumstr+"; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing1P_ = "p1d_avgTimePerPhiHORing1P";
    add1dHisto( st_avgTimePerPhiRing1P_,
		"Avg. Time/i#phi (YB+1), HO, Run "+runnumstr+"; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing2M_ = "p1d_avgTimePerPhiHORing2M";
    add1dHisto( st_avgTimePerPhiRing2M_,
		"Avg. Time/i#phi (YB-2), HO, Run "+runnumstr+"; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTimePerPhiRing2P_ = "p1d_avgTimePerPhiHORing2P";
    add1dHisto( st_avgTimePerPhiRing2P_,
		"Avg. Time/i#phi (YB+2), HO, Run "+runnumstr+"; i#phi; Time (ns)",
		72, 0.5, 72.5,v_hpars1dprof);

    st_avgTuncPerIetad4_ = "p1d_avgTuncPerIetad4HO";
    add1dHisto( st_avgTuncPerIetad4_,
		"Avg. Time/i#eta (Uncorrected), HO, Run "+runnumstr+"; i#eta; Time (ns)",
		61,-30.5, 30.5,v_hpars1dprof);

    st_avgTcorPerIetad4_ = "p1d_avgTcorPerIetad4HO";
    add1dHisto( st_avgTcorPerIetad4_,
		"Avg. Time/i#eta, HO, Run "+runnumstr+"; i#eta; Time (ns)",
		61,-30.5, 30.5,v_hpars1dprof);
  } // HO

  /*****************************************
   * 2-D PROFILES:                         *
   *****************************************/

  if (mysubdet_ != HcalOuter) { 
    st_rhTuncProfd1_  = "p2d_rhTuncPerIetaIphiD1" + mysubdetstr_;
    add2dHisto(st_rhTuncProfd1_,
 "Depth 1 RecHit Time Map (uncorrected), "+mysubdetstr_+", Run "+runnumstr+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72, 0.5, 72.5,
	       v_hpars2dprof);

    st_rhTcorProfd1_  = "p2d_rhTcorPerIetaIphiD1" + mysubdetstr_;
    add2dHisto(st_rhTcorProfd1_,
 "Depth 1 RecHit Time Map, "+mysubdetstr_+", Run "+runnumstr+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72, 0.5, 72.5,
	       v_hpars2dprof);

    st_rhTuncProfd2_  = "p2d_rhTuncPerIetaIphiD2" + mysubdetstr_;
    add2dHisto(st_rhTuncProfd2_,
 "Depth 2 RecHit Time Map (uncorrected), "+mysubdetstr_+", Run "+runnumstr+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       v_hpars2dprof);

    st_rhTcorProfd2_  = "p2d_rhTcorPerIetaIphiD2" + mysubdetstr_;
    add2dHisto(st_rhTcorProfd2_,
 "Depth 2 RecHit Time Map, "+mysubdetstr_+", Run "+runnumstr+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72, 0.5, 72.5,
	       v_hpars2dprof);

    st_rhTprofRBXd1_  = "p2d_rhTcorPerRBXD1" + mysubdetstr_;
    add2dHisto(st_rhTprofRBXd1_,
"Depth 1 RecHit Time RBX Map, "+mysubdetstr_+", Run "+runnumstr+"; i#eta; iRBX",
	       6, -3.0,  3.0, 18,   0.5,  18.5,
	       v_hpars2dprof);

    st_rhTprofRBXd2_  = "p2d_rhTcorPerRBXD2" + mysubdetstr_;
    add2dHisto(st_rhTprofRBXd2_,
"Depth 2 RecHit Time RBX Map, "+mysubdetstr_+", Run "+runnumstr+"; i#eta; iRBX",
	       6, -3.0,  3.0, 18,   0.5,  18.5,
	       v_hpars2dprof);

    st_rhTvsEtaEnergy_  = "p2d_rhTvsEtaEnergy" + mysubdetstr_;
    add2dHisto(st_rhTvsEtaEnergy_,
 "RecHit Time Vs. #eta and Energy, "+mysubdetstr_+", Run "+runnumstr+"; i#eta; Hit Energy (GeV)",
	       83, -41.5,  41.5, (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	      recHitEscaleMinGeV_,recHitEscaleMaxGeV_, v_hpars2dprof);

    if (mysubdet_ == HcalEndcap) {
      st_rhTuncProfd3_  = "p2d_rhTuncPerIetaIphiD3HE";
      add2dHisto(st_rhTuncProfd3_,
	 "Depth 3 RecHit Time Map (uncorrected), HE, Run "+runnumstr+"; i#eta; i#phi",
		 83, -41.5,  41.5, 72,   0.5,  72.5,
		 v_hpars2dprof);

      st_rhTcorProfd3_  = "p2d_rhTcorPerIetaIphiD3HE";
      add2dHisto(st_rhTcorProfd3_,
	 "Depth 3 RecHit Time Map, HE, Run "+runnumstr+"; i#eta; i#phi",
		 83, -41.5,  41.5, 72,   0.5,  72.5,
		 v_hpars2dprof);

      st_rhTprofRBXd3_  = "p2d_rhTcorPerRBXD3" + mysubdetstr_;
      add2dHisto(st_rhTprofRBXd3_,
		 "Depth 3 RecHit Time RBX Map, HE, Run "+runnumstr+"; i#eta; iRBX",
		 6, -3.0,  3.0, 18,   0.5,  18.5,
		 v_hpars2dprof);
    }
  } else {
    st_rhTuncProfd4_  = "p2d_rhTuncPerIetaIphiD4HO";
    add2dHisto(st_rhTuncProfd4_,
	       "Depth 4 RecHit Time Map (uncorrected), HO, Run "+runnumstr+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       v_hpars2dprof);

    st_rhTcorProfd4_  = "p2d_rhTcorPerIetaIphiD4HO";
    add2dHisto(st_rhTcorProfd4_,
	       "Depth 4 RecHit Time Map, HO, Run "+runnumstr+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       v_hpars2dprof);

    st_rhTprofRBXd4_  = "p2d_rhTperRBXD4" + mysubdetstr_;
    add2dHisto(st_rhTprofRBXd4_,
	       "Depth 4 RecHit Time RBX Map, HO, Run "+runnumstr+"; i#eta; iRBX",
	       6, -3.0,  3.0, 18,   0.5,  18.5,
	       v_hpars2dprof);
  }

  /*****************************************
   * BOOK 'EM, DANNO...                    *
   *****************************************/

  // Variable size binned histos
  //
  st_rhTprofplusd1_   = "p2d_rhTperIetaIphiPlusD1HF";
  st_rhTprofplusd2_   = "p2d_rhTperIetaIphiPlusD2HF";
  st_rhTprofminusd2_  = "p2d_rhTperIetaIphiMinusD2HF";
  st_rhTprofminusd1_  = "p2d_rhTperIetaIphiMinusD1HF";
  st_pulsePerEbinPlus_   = "p2d_pulsePerEbin" + mysubdetstr_ + "P";
  st_pulsePerEbinMinus_   = "p2d_pulsePerEbin" + mysubdetstr_ + "M";

  uint32_t total = v_hpars1dprof.size()+v_hpars2dprof.size();

  cout << mysubdetstr_ << ": Booking " << total << " histos for the last cut" << std::endl;
//edm::LogInfo(

  myAnalHistos *myAH = getHistos4cut(st_lastCut_);
  myAH->book1d<TH1F>      (v_hpars1d);
  //myAH->book2d<TH2F>      (v_hpars2d);
  myAH->book1d<TProfile>  (v_hpars1dprof);
  myAH->book2d<TProfile2D>(v_hpars2dprof);

  // Digi pulses binned by energy, by hemisphere
  titlestr = "Average Pulse Shape vs E, "+mysubdetstr_+"P"+"; Sample Number; E_{hit} (GeV); fC";
  myAH->book2dvary<TProfile2D>(st_pulsePerEbinPlus_, titlestr.c_str(),
			       10,-0.5,9.5, nEbins, digEbins);
  titlestr = "Average Pulse Shape vs E, "+mysubdetstr_+"M"+"; Sample Number; E_{hit} (GeV); fC";
  myAH->book2dvary<TProfile2D>(st_pulsePerEbinMinus_, titlestr.c_str(),
			       10,-0.5,9.5, nEbins, digEbins);

  if (mysubdet_ == HcalForward) {
    // variable bin timing maps for HF
    myAH->book2dvarx<TProfile2D>(st_rhTprofplusd1_,
				 "RecHit Timing, HFP Depth 1; radius; iphi",
				 28, hftwrEdges,36, 0.0, 6.28);
    myAH->book2dvarx<TProfile2D>(st_rhTprofplusd2_,
				 "RecHit Timing, HFP Depth 2; radius; iphi",
				 28, hftwrEdges,36, 0.0, 6.28);
    myAH->book2dvarx<TProfile2D>(st_rhTprofminusd1_,
				 "RecHit Timing, HFM Depth 1; radius; iphi",
				 28, hftwrEdges,36, 0.0, 6.28);
    myAH->book2dvarx<TProfile2D>(st_rhTprofminusd2_,
				 "RecHit Timing, HFM Depth 2; radius; iphi",
				 28, hftwrEdges,36, 0.0, 6.28);
  }

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
  cout<<"Done."<<std::endl;

  /*****************************************
   * To put in a separate directory
   ****************************************/

  if (doPerChannel_) {

    // make digi subfolder in the final cut folder
    myAnalHistos *myAH = getHistos4cut(st_lastCut_);
    digidir_ = new TFileDirectory(myAH->dir()->mkdir("DigisPerID"));
    rhdir_   = new TFileDirectory(myAH->dir()->mkdir("corTimesPerID"));
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
  }
}                             // HcalDelayTunerAlgos::bookHistos4lastCut

//======================================================================

std::pair<std::map<uint32_t,TProfile*>::iterator, bool>
HcalDelayTunerAlgos::bookDigiHisto(HcalDetId detId)
{
  uint32_t denseId = detID_.denseIndex();
  stringstream name;
  name << detId;
  return digisPerId_.insert
    (std::pair<uint32_t,TProfile*>
     (denseId,(TProfile *)digidir_->make<TProfile>
      (name.str().c_str(),name.str().c_str(),10,-0.5,9.5)));
}

//======================================================================

std::pair<std::map<uint32_t,TProfile2D*>::iterator, bool>
HcalDelayTunerAlgos::bookDigiPerEhisto(HcalDetId detId)
{
  uint32_t denseId = detID_.denseIndex();
  stringstream title;
  title << detId;
  string name = title.str() + "perE";
  title << "; ; E_{hit}(GeV)";
  return digisPerIdPerE_.insert
    (std::pair<uint32_t,TProfile2D*>
     (denseId,(TProfile2D *)digidir_->make<TProfile2D>
      (name.c_str(),title.str().c_str(),10,-0.5,9.5,nEbins,digEbins)));
}

//======================================================================

std::pair<std::map<uint32_t,TH1D*>::iterator, bool>
HcalDelayTunerAlgos::bookCorTimeHisto(HcalDetId detId)
{
  uint32_t denseId = detID_.denseIndex();
  stringstream name, title;
  name << detId;
  title << name.str() << "; RecHit Time (ns)";
  return corTimesPerId_.insert
    (std::pair<uint32_t,TH1D*>
     (denseId,(TH1D *)rhdir_->make<TH1D>
      (name.str().c_str(),title.str().c_str(),
       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_)));
}

//======================================================================
inline int    sign   (double x) { return (x>=0) ? 1 : -1; }

void
HcalDelayTunerAlgos::fillHistos4cut(const std::string& cutstr)
{
  //edm::LogInfo("Filling histograms for subdet ") << mysubdetstr_ << std::endl;

  myAnalHistos *myAH =   getHistos4cut(cutstr);

  int        ieta = detID_.ieta();
  int     absieta = detID_.ietaAbs();
  int        iphi = detID_.iphi();
  int       depth = detID_.depth();
  int       zside = detID_.zside();

  /* All-subdet Histos: */

  // global rechit info
  myAH->fill1d<TH1F>(st_rhEnergies_,    hitenergy_);
  myAH->fill1d<TH1F>(st_rhCorTimes_,    corTime_);
  myAH->fill1d<TH1F>(st_rhUncorTimes_,  hittime_);
  myAH->fill2d<TH2F>(st_rhEmap_,        ieta,iphi,hitenergy_);
  myAH->fill2d<TH2F>(st_uncorTimingVsE_,hitenergy_,hittime_);
  myAH->fill2d<TH2F>(st_corTimingVsE_,  hitenergy_,corTime_);

  myAH->fill1d<TH1F>(((zside>0)?st_rhCorTimesPlus_:st_rhCorTimesMinus_),corTime_);

  if (depth==1) {
    myAH->fill1d<TH1F> (st_rhCorTimesD1_, corTime_);
    myAH->fill1d<TProfile> (st_avgTperEvD1_, evtnum_, corTime_);
    myAH->fill2d<TH2F>(st_corTimingVsED1_,  hitenergy_,corTime_);
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

  myAH->fill2d<TH2D> (st_nHitsPerIetaIphi_, ieta, iphi);

  fillDigiPulseHistos(myAH->get<TProfile>(st_avgPulse_), NULL);

  // segregate +side/-side digis for timing comparison
  fillDigiPulseHistos(getHistos4cut(cutstr)->get<TProfile>
		      (((zside>0)?st_avgPulsePlus_:st_avgPulseMinus_)),
		      getHistos4cut(cutstr)->get<TProfile2D>
		      (((zside>0)?st_pulsePerEbinPlus_:st_pulsePerEbinMinus_)));

  if (cutstr == st_lastCut_) {
    // need front end id: 
    int            iRBX = atoi(((feID_.rbx()).substr(3,2)).c_str());
    int        iRMinRBX = feID_.rm();
    //int            ipix = feID_.pixel();
    int             iRM = zside * ((iRBX-1)*4 + iRMinRBX);
    int     signed_iphi = zside*iphi;
    
    //cout << detId << "\t" << feID.rbx() << "\t" << feID.rbx().substr(3,2) << "\t";
    //cout << feID.rm() << " maps to RBX/RM# " << iRBX << "/" << iRM << endl;

    myAH->fill1d<TProfile>(st_avgTimePerRBX_, zside*iRBX, corTime_);

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
      break;
    case 2:
      rhTuncProf = st_rhTuncProfd2_;
      rhTcorProf = st_rhTcorProfd2_;
      rhTprofRBX = st_rhTprofRBXd2_;
      avgRMt     = st_avgTimePerRMd2_;
      avgPhiT    = st_avgTimePerPhid2_;
      avgIetaTunc= st_avgTuncPerIetad2_;
      avgIetaTcor= st_avgTcorPerIetad2_;
      break;
    case 3:
      rhTuncProf = st_rhTuncProfd3_;
      rhTcorProf = st_rhTcorProfd3_;
      rhTprofRBX = st_rhTprofRBXd3_;
      avgRMt     = st_avgTimePerRMd3_;
      avgPhiT    = st_avgTimePerPhid3_;
      avgIetaTunc= st_avgTuncPerIetad3_;
      avgIetaTcor= st_avgTcorPerIetad3_;
      break;
    case 4:
      rhTuncProf = st_rhTuncProfd4_;
      rhTcorProf = st_rhTcorProfd4_;
      rhTprofRBX = st_rhTprofRBXd4_;
      avgRMt     = st_avgTimePerRMd4_;
      //avgPhiT    = st_avgTimePerPhid4_; // we do it by ring, see below 
      avgIetaTunc= st_avgTuncPerIetad4_;
      avgIetaTcor= st_avgTcorPerIetad4_;
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

    myAH->fill1d<TProfile>   (avgIetaTunc,ieta,         hittime_);
    myAH->fill1d<TProfile>   (avgIetaTcor,ieta,         corTime_);
    myAH->fill2d<TProfile2D> (rhTuncProf, ieta, iphi,   hittime_);
    myAH->fill2d<TProfile2D> (rhTcorProf, ieta, iphi,   corTime_);
    myAH->fill2d<TProfile2D> (rhTprofRBX, fsubdet,iRBX, corTime_);

    // subdet-specific histos

    if (mysubdet_ == HcalOuter) {
      if      ((ieta>=-15)&&(ieta<=-11))myAH->fill1d<TProfile>(st_avgTimePerPhiRing2M_,iphi,corTime_);
      else if ((ieta>=-10)&&(ieta<= -5))myAH->fill1d<TProfile>(st_avgTimePerPhiRing1M_,iphi,corTime_);
      else if ((ieta>= -4)&&(ieta<=  4))myAH->fill1d<TProfile>(st_avgTimePerPhiRing0_, iphi,corTime_);
      else if ((ieta>=  5)&&(ieta<= 10))myAH->fill1d<TProfile>(st_avgTimePerPhiRing1P_,iphi,corTime_);
      else if ((ieta>= 11)&&(ieta<= 15))myAH->fill1d<TProfile>(st_avgTimePerPhiRing2P_,iphi,corTime_);

    } else if (mysubdet_ == HcalForward) {
      double angle  = TMath::Pi()*(iphi-1)/36.;
      double radius = hftwrRadii[41-absieta];
      //cout << ieta << " --> " << radius << endl;
      switch (depth) {
      case 1: myAH->fill2d<TProfile2D>((zside>0)?st_rhTprofplusd1_:st_rhTprofminusd1_,
				       radius,angle,corTime_); break;
      case 2: myAH->fill2d<TProfile2D>((zside>0)?st_rhTprofplusd2_:st_rhTprofminusd2_,
				       radius,angle,corTime_); break;
      }
    } else {
      myAH->fill1d<TProfile> (avgRMt,     iRM,      corTime_);
      myAH->fill1d<TProfile> (avgPhiT, signed_iphi, corTime_);
      myAH->fill2d<TProfile2D>(st_rhTvsEtaEnergy_, ieta, hitenergy_, corTime_);
    }

    // per-channel histograms
    uint32_t denseId = detID_.denseIndex();
#if 0
    std::map<uint32_t,TH1F *>::const_iterator it=m_perChHistos_.find(denseId);
    if (it==m_perChHistos_.end())
      cerr << "No histo booked for detId " << detID_ << "!!!" << endl;
    else
      it->second->Fill(corTime_);
#endif

    // Now per channel...
    if (doPerChannel_) {
      std::map<uint32_t,TProfile*>::const_iterator it = digisPerId_.find(denseId);
      if (it == digisPerId_.end()) {
	std::pair<std::map<uint32_t,TProfile*>::iterator, bool>
	  retval = bookDigiHisto(detID_);
	it = retval.first;
      }
      std::map<uint32_t,TProfile2D*>::const_iterator it2 = digisPerIdPerE_.find(denseId);
      if (it2 == digisPerIdPerE_.end()) {
	std::pair<std::map<uint32_t,TProfile2D*>::iterator, bool>
	  retval = bookDigiPerEhisto(detID_);
	it2 = retval.first;
      }
      fillDigiPulseHistos(it->second, it2->second);

      // Corrected hit time per channel
      std::map<uint32_t,TH1D*>::const_iterator it3 = corTimesPerId_.find(denseId);
      if (it3 == corTimesPerId_.end()) {
	std::pair<std::map<uint32_t,TH1D*>::iterator, bool>
	  retval = bookCorTimeHisto(detID_);
	it3 = retval.first;
      }
      it3->second->Fill(corTime_);

    }
  } // if last cut
}                                 // HcalDelayTunerAlgos::fillHistos4cut

//======================================================================

void
HcalDelayTunerAlgos::fillDigiPulseHistos(TProfile   *pulseHist,
					 TProfile2D *pulseHistPerE)
{
  int digisize = std::min(10,digifC_.size());

  CaloSamples filldigifC = digifC_;

  if (normalizeDigis_) {
    double sum = 0.0;
    for (int its=0; its<digisize; ++its) sum += digifC_[its];
    if (sum > 0.0)  filldigifC *= 1.0/sum;
  }

  for (int its=0; its<digisize; ++its) {
    if (pulseHist)
      pulseHist->Fill(its,filldigifC[its]);

    // ...binned by energy...
    if (pulseHistPerE)
      pulseHistPerE->Fill(its, hitenergy_, filldigifC[its] );
  }
}                            // HcalDelayTunerAlgos::fillDigiPulseHistos

//======================================================================

// ------------ method called to for each event  ------------
void
HcalDelayTunerAlgos::process(const myEventData& ed)
{
  if (firstEvent_) {
    runnum_  = ed.runNumber();
    bookHistos4allCuts();
    bookHistos4lastCut();
    firstEvent_ = false;
  }

  lsnum_  = ed.lumiSection();
  bxnum_  = ed.bxNumber();
  evtnum_ = ed.evtNumber();

  // leave the rest to the child class?
}

//======================================================================

void
HcalDelayTunerAlgos::beginJob()
{
  neventsProcessed_=0;

  edm::Service<TFileService> fs;

  tree_ = fs->make<TTree>("mytree","Hcal Results Tree");
  tree_->Branch("feID",         &feID_,  32000, 1);
  tree_->Branch("detID",        &detID_, 32000, 1);
  tree_->Branch("bxnum",        &bxnum_, 32000, 1);
  tree_->Branch("lsnum",        &lsnum_, 32000, 1);
  tree_->Branch("evtnum",       &evtnum_,32000, 1);
  tree_->Branch("hittime",      &hittime_);
  tree_->Branch("hitenergy",    &hitenergy_);
  tree_->Branch("hitflags",     &hitflags_);
  tree_->Branch("corhittime_ns",&corTime_);
  tree_->Branch("correction_ns",&correction_ns_);

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
HcalDelayTunerAlgos::writeCorrections(const TimesPerDetId& chtimes)
{
  char name[40];
  FILE *fp=0;
  if (chtimes.size() > 0) {
    sprintf(name,"timeCorrs_run%d_%s.csv",runnum_,mysubdetstr_.c_str());
    fp = fopen(name,"w");
    fprintf(fp,"SD\tieta\tiphi\tdpth\tRBX\tRM\tcard\tadc\tAvgT(ns)\n");
  }

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
}                             // HcalDelayTunerAlgos::writeCorrections

//======================================================================

void
HcalDelayTunerAlgos::projectResults(const TimesPerDetId& chtimes, int itnum)
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
      string("Depth 1 RecHit Time Map (projected), "+mysubdetstr_+";i#eta;i#phi").c_str(),
				   83, -41.5,  41.5, 72, 0.5, 72.5);

    v_old[1] = lastAH->get<TProfile2D>(st_rhTcorProfd2_);
    v_new[1] = itdir->make<TProfile2D>(string("h2d_rhTprojPerIetaIphiD2"+mysubdetstr_).c_str(),
      string("Depth 2 RecHit Time Map (projected), "+mysubdetstr_+";i#eta;i#phi").c_str(),
				   83, -41.5,  41.5, 72, 0.5, 72.5);

    if (mysubdet_ == HcalEndcap) {
      v_old[2] = lastAH->get<TProfile2D>(st_rhTcorProfd3_);
      v_new[2] = itdir->make<TProfile2D>(string("h2d_rhTprojPerIetaIphiD3"+mysubdetstr_).c_str(),
      string("Depth 3 RecHit Time Map (projected), " + mysubdetstr_ + "; i#eta; i#phi").c_str(),
				     83, -41.5,  41.5, 72, 0.5, 72.5);
    }
  } else {
    v_old[3] = lastAH->get<TProfile2D>(st_rhTcorProfd4_);
    v_new[3] = itdir->make<TProfile2D>("h2d_rhTprojPerIetaIphiD4HO",
      string("Depth 4 RecHit Time Map (projected), " + mysubdetstr_ + "; i#eta; i#phi").c_str(),
				   83, -41.5,  41.5, 72, 0.5, 72.5);
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
}                               // HcalDelayTunerAlgos::projectResults

//======================================================================

void
HcalDelayTunerAlgos::endJob()
{
  cout << neventsProcessed_ << " events processed for subdet " << mysubdetstr_ << endl;
  if (!neventsProcessed_) return;

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
}                                         // HcalDelayTunerAlgos::endJob

//======================================================================

void
HcalDelayTunerAlgos::detChanCorsByPhiDepth(const std::vector<TProfile *>& profsByPhi,
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
}                        // HcalDelayTunerAlgos::detChanCorsByPhiDepth

//======================================================================

void
HcalDelayTunerAlgos::detChanCorsByEtaDepth(const std::vector<TProfile *>& profsByEta,
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
}                        // HcalDelayTunerAlgos::detChanCorsByEtaDepth

//======================================================================

void
HcalDelayTunerAlgos::detChanCorsByRingHO(TimesPerDetId& chtimes)
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

}                          // HcalDelayTunerAlgos::detChanCorsByRingHO

//======================================================================

void
HcalDelayTunerAlgos::detChanCorsIndividually (const std::vector<TProfile2D *>& profs2d,
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
}                      // HcalDelayTunerAlgos::detChanCorsIndividually

//======================================================================

void
HcalDelayTunerAlgos::applyExternalCorrections(TimesPerDetId& extcors,
						TimesPerDetId& chtimes)
{
  TimesPerDetId::const_iterator it;
  for (it=extcors.begin(); it!=extcors.end(); it++) {
    std::pair<TimesPerDetId::iterator,bool> retval;
    retval = chtimes.insert(*it);
    if (!retval.second)
      (retval.first)->second += it->second;  // pile correction on top of previous one.
  }
}                     // HcalDelayTunerAlgos::applyExternalCorrections

//======================================================================

void
HcalDelayTunerAlgos::processCorrection(const tCorrection& tcor,
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
}                            // HcalDelayTunerAlgos::processCorrection

//======================================================================

void
HcalDelayTunerAlgos::detChannelTimes(TimesPerDetId& chtimes)
{
  last2dprof_=(TProfile2D*)0;
  last1ddist_=(TH1F*)0;
  for (size_t i=0; i<corList_.size(); i++) {
    tCorrection tcor = corList_[i];
    processCorrection(tcor,chtimes);
    projectResults(chtimes, i);
  }
}
