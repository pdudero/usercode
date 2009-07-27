
// -*- C++ -*-
//
// Package:    LaserDelayTunerAlgos
// Class:      LaserDelayTunerAlgos
// 
/**\class LaserDelayTunerAlgos LaserDelayTunerAlgos.cc MyEDmodules/LaserDelayTunerAlgos/src/LaserDelayTunerAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserDelayTunerAlgos.hh,v 1.5 2009/05/21 09:52:41 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>

// user include files
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"

#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"

#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"

#include "LaserDelayTunerAlgos.hh"


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
LaserDelayTunerAlgos::LaserDelayTunerAlgos(HcalSubdetector subdet,
					   const edm::ParameterSet& iConfig,
					   LaserDelayTunerTDCalgos *inTDCalgo) :
  mysubdet_(subdet),
  TDCalgo_(inTDCalgo),
  minHitGeV_(iConfig.getParameter<double>("minHitGeV")),
  recHitTscaleNbins_(iConfig.getParameter<int>("recHitTscaleNbins")),
  recHitTscaleMinNs_(iConfig.getParameter<double>("recHitTscaleMinNs")),
  recHitTscaleMaxNs_(iConfig.getParameter<double>("recHitTscaleMaxNs")),
  recHitEscaleMinGeV_(iConfig.getParameter<double>("recHitEscaleMinGeV")),
  recHitEscaleMaxGeV_(iConfig.getParameter<double>("recHitEscaleMaxGeV")),
  rundescr_(iConfig.getUntrackedParameter<std::string>("runDescription","")),
  writeBricks_(iConfig.getUntrackedParameter<bool>("writeBricks",false))
{
  // cut string vector initialized in order
  v_cuts_.push_back("cutNone");
  v_cuts_.push_back("cutTDCwindow");
  v_cuts_.push_back("cutMinHitGeV");
  v_cuts_.push_back("cutAll");

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
}

//======================================================================

bool
LaserDelayTunerAlgos::convertIdNumbers(const std::vector<int>& v_idnumbers,
				       HcalDetId& detId)
{
  // convert det ID numbers to valid detIds:
  if (v_idnumbers.size()!=3) {
    return false;
  }

  int ieta  = v_idnumbers[0];
  int iphi  = v_idnumbers[1];
  int depth = v_idnumbers[2];

  enum HcalSubdetector subdet;
  if      (depth == 4)       subdet = HcalOuter;
  else if (abs(ieta) <= 16)  subdet = HcalBarrel;
  else if (abs(ieta) <= 28)  subdet = HcalOuter; // gud enuf fer gubmint werk
  else if (abs(ieta) <= 41)  subdet = HcalForward;
  else return false;

  if (!HcalDetId::validDetId(subdet,ieta,iphi,depth))
    return false;

  detId = HcalDetId(subdet,ieta,iphi,depth);
  return true;
}                                   // HFtrigAnalAlgos::convertIdNumbers

//======================================================================

void
LaserDelayTunerAlgos::bookHistos(void)
{
  // Initialize the cuts for the run and add them to the global map

  m_cuts_.clear();

  cout << "Booking histos for subdet = " << mysubdetstr_ << std::endl;
//edm::LogInfo(

  for (unsigned i=0; i<v_cuts_.size(); i++)
    m_cuts_[v_cuts_[i]] = new myAnalCut(i,v_cuts_[i],mysubdetstr_);

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d; 
  std::vector<myAnalHistos::HistoParams_t> v_hpars1dprof;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d; 
  std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof;

  myAnalHistos::HistoParams_t hpars1d;
  myAnalHistos::HistoParams_t hpars2d;

  /*****************************************
   * 1-D HISTOGRAMS FIRST:                 *
   *****************************************/
  hpars1d.nbinsy = 0;

  avgPulse_   = "h1d_pulse" + mysubdetstr_;
  hpars1d.name   = avgPulse_;
  hpars1d.title  = "Average Pulse Shape, " + mysubdetstr_;
  hpars1d.nbinsx = 10;
  hpars1d.minx   = -0.5;
  hpars1d.maxx   =  9.5;

  v_hpars1d.push_back(hpars1d);

  digiColSize_ = "DigiCollectionSize" + mysubdetstr_;
  hpars1d.name   = digiColSize_;
  hpars1d.title  = "Digi Collection Size, " + mysubdetstr_;
  hpars1d.nbinsx = 5201; // 72chan/RBX*72RBX = 5184, more than HF or HO
  hpars1d.minx   = -0.5;
  hpars1d.maxx   = 5200.5;

  v_hpars1d.push_back(hpars1d);

  rhColSize_ = "RechitCollectionSize" + mysubdetstr_;
  hpars1d.name   = rhColSize_;
  hpars1d.title  = "Rechit Collection Size, " + mysubdetstr_;
  hpars1d.nbinsx = 5201; // 72chan/RBX*72RBX = 5184, more than HF or HO
  hpars1d.minx   = -0.5;
  hpars1d.maxx   = 5200.5;

  v_hpars1d.push_back(hpars1d);

  rhUncorTimesPlus_ = "h1d_rhUncorTimesPlus" + mysubdetstr_;
  hpars1d.name   = rhUncorTimesPlus_;
  hpars1d.title  = "RecHit Times (uncorrected), " + mysubdetstr_ + "P; Rechit Time (ns)";
  hpars1d.nbinsx = recHitTscaleNbins_;
  hpars1d.minx   = recHitTscaleMinNs_;
  hpars1d.maxx   = recHitTscaleMaxNs_;

  v_hpars1d.push_back(hpars1d);

  rhUncorTimesMinus_ = "h1d_rhUncorTimesMinus" + mysubdetstr_;
  hpars1d.name   = rhUncorTimesMinus_;
  hpars1d.title  = "RecHit Times (uncorrected), " + mysubdetstr_ + "M; Rechit Time (ns)";
  hpars1d.nbinsx = recHitTscaleNbins_;
  hpars1d.minx   = recHitTscaleMinNs_;
  hpars1d.maxx   = recHitTscaleMaxNs_;

  v_hpars1d.push_back(hpars1d);

  rhCorTimesPlus_ = "h1d_rhCorTimesPlus" + mysubdetstr_;
  hpars1d.name   = rhCorTimesPlus_;
  hpars1d.title  = "RecHit Times (TDC-corrected), " + mysubdetstr_ + "P; Rechit Time (ns)";
  hpars1d.nbinsx = recHitTscaleNbins_;
  hpars1d.minx   = recHitTscaleMinNs_;
  hpars1d.maxx   = recHitTscaleMaxNs_;

  v_hpars1d.push_back(hpars1d);

  rhCorTimesMinus_ = "h1d_rhCorTimesMinus" + mysubdetstr_;
  hpars1d.name   = rhCorTimesMinus_;
  hpars1d.title  = "RecHit Times (TDC-corrected), " + mysubdetstr_ + "M; Rechit Time (ns)";
  hpars1d.nbinsx = recHitTscaleNbins_;
  hpars1d.minx   = recHitTscaleMinNs_;
  hpars1d.maxx   = recHitTscaleMaxNs_;

  v_hpars1d.push_back(hpars1d);

  rhEnergies_    = "h1d_RHEnergies" + mysubdetstr_;
  hpars1d.name   = rhEnergies_;
  hpars1d.title  = "RecHit Energies, " + mysubdetstr_ + "; Rechit Energy (GeV)";
  hpars1d.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars1d.minx   = recHitEscaleMinGeV_;
  hpars1d.maxx   = recHitEscaleMaxGeV_;

  v_hpars1d.push_back(hpars1d);

  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  if (mysubdet_ != HcalOuter) { 
    avgTimePerRMd1_ =  "p1d_avgTimePerRMd1" + mysubdetstr_;
    hpars1d.name   = avgTimePerRMd1_;
    hpars1d.title  = "Averaged Time (Depth 1), " + mysubdetstr_ + "; iRM; Time (ns)";
    hpars1d.nbinsx = 145;
    hpars1d.minx   = -72.5;
    hpars1d.maxx   =  72.5;

    v_hpars1dprof.push_back(hpars1d);

    avgTimePerRBXd1_ =  "p1d_avgTimePerRBXd1" + mysubdetstr_;
    hpars1d.name   = avgTimePerRBXd1_;
    hpars1d.title  = "Averaged Time (Depth 1), " + mysubdetstr_ + "; iRBX; Time (ns)";
    hpars1d.nbinsx = 37;
    hpars1d.minx   = -18.5;
    hpars1d.maxx   =  18.5;

    v_hpars1dprof.push_back(hpars1d);
 
    avgTimePerRMd2_ =  "p1d_avgTimePerRMd2" + mysubdetstr_;
    hpars1d.name   = avgTimePerRMd2_;
    hpars1d.title  = "Averaged Time (Depth 2), " + mysubdetstr_ + "; iRM; Time (ns)";
    hpars1d.nbinsx = 145;
    hpars1d.minx   = -72.5;
    hpars1d.maxx   =  72.5;

    v_hpars1dprof.push_back(hpars1d);

    avgTimePerRBXd2_ =  "p1d_avgTimePerRBXd2" + mysubdetstr_;
    hpars1d.name   = avgTimePerRBXd2_;
    hpars1d.title  = "Averaged Time (Depth 2), " + mysubdetstr_ + "; iRBX; Time (ns)";
    hpars1d.nbinsx = 37;
    hpars1d.minx   = -18.5;
    hpars1d.maxx   =  18.5;

    v_hpars1dprof.push_back(hpars1d);

    if (mysubdet_ == HcalEndcap) {
      avgTimePerRMd3_ =  "p1d_avgTimePerRMd3HE";
      hpars1d.name   = avgTimePerRMd3_;
      hpars1d.title  = "Averaged Time (Depth 3), HE; iRM; Time (ns)";
      hpars1d.nbinsx = 145;
      hpars1d.minx   = -72.5;
      hpars1d.maxx   =  72.5;

      v_hpars1dprof.push_back(hpars1d);

      avgTimePerRBXd3_ =  "p1d_avgTimePerRBXd3" + mysubdetstr_;
      hpars1d.name   = avgTimePerRBXd3_;
      hpars1d.title  = "Averaged Time (Depth 3), " + mysubdetstr_ + "; iRBX; Time (ns)";
      hpars1d.nbinsx = 37;
      hpars1d.minx   = -18.5;
      hpars1d.maxx   =  18.5;

      v_hpars1dprof.push_back(hpars1d);

    }
  } else {
    avgTimePerRMd4_ = "p1d_avgTimePerRMd4HO";
    hpars1d.name   = avgTimePerRMd4_;
    hpars1d.title  = "Averaged Time (Depth 4), HO; iRM; Time (ns)";
    hpars1d.nbinsx = 145;
    hpars1d.minx   = -72.5;
    hpars1d.maxx   =  72.5;

    v_hpars1dprof.push_back(hpars1d);

    avgTimePerRBXd4_ =  "p1d_avgTimePerRBXd4" + mysubdetstr_;
    hpars1d.name   = avgTimePerRBXd4_;
    hpars1d.title  = "Averaged Time (Depth 4), " + mysubdetstr_ + "; iRBX; Time (ns)";
    hpars1d.nbinsx = 37;
    hpars1d.minx   = -18.5;
    hpars1d.maxx   =  18.5;

    v_hpars1dprof.push_back(hpars1d);
  }

  /*****************************************
   * 2-D HISTOGRAMS AFTER:                 *
   *****************************************/

  rhEmap_        = "h2d_rhEperIetaIphi" + mysubdetstr_;
  hpars2d.name   = rhEmap_;
  hpars2d.title  = "RecHit Energy Map (#Sigma depths), " + mysubdetstr_ + "; ieta; iphi";
  hpars2d.nbinsx =  83;
  hpars2d.minx   =  -41.5;
  hpars2d.maxx   =   41.5;
  hpars2d.nbinsy =  72;
  hpars2d.miny   =    0.5;
  hpars2d.maxy   =   72.5;

  v_hpars2d.push_back(hpars2d);

  uncorTimingVsE_ = "h2d_uncorTimingVsE" + mysubdetstr_;
  hpars2d.name   = uncorTimingVsE_;
  hpars2d.title  = "RecHit Timing (uncorrected) vs. Energy, " + mysubdetstr_ + "; Rechit Energy (GeV); Rechit Time (ns)";
  hpars2d.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars2d.minx   = recHitEscaleMinGeV_;
  hpars2d.maxx   = recHitEscaleMaxGeV_;
  hpars2d.nbinsy = recHitTscaleNbins_;
  hpars2d.miny   = recHitTscaleMinNs_;
  hpars2d.maxy   = recHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  corTimingVsE_ = "h2d_corTimingVsE" + mysubdetstr_;
  hpars2d.name   = corTimingVsE_;
  hpars2d.title  = "RecHit Timing (TDC-corrected) vs. Energy, " + mysubdetstr_ + "; Rechit Energy (GeV); Rechit Time (ns)";
  hpars2d.nbinsx = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);
  hpars2d.minx   = recHitEscaleMinGeV_;
  hpars2d.maxx   = recHitEscaleMaxGeV_;
  hpars2d.nbinsy = recHitTscaleNbins_;
  hpars2d.miny   = recHitTscaleMinNs_;
  hpars2d.maxy   = recHitTscaleMaxNs_;

  v_hpars2d.push_back(hpars2d);

  /*****************************************
   * 2-D PROFILES:                         *
   *****************************************/

  if (mysubdet_ != HcalOuter) { 
    rhTprofd1_  = "p2d_rhTperIetaIphiD1" + mysubdetstr_;
    hpars2d.name   = rhTprofd1_;
    hpars2d.title  = "Depth 1 RecHit Time Map-Profile, " + mysubdetstr_ + "; ieta; iphi";
    hpars2d.nbinsx =  83;
    hpars2d.minx   =  -41.5;
    hpars2d.maxx   =   41.5;
    hpars2d.nbinsy =  72;
    hpars2d.miny   =    0.5;
    hpars2d.maxy   =   72.5;

    v_hpars2dprof.push_back(hpars2d);

    rhTprofRBXd1_  = "p2d_rhTperRBXD1" + mysubdetstr_;
    hpars2d.name   = rhTprofRBXd1_;
    hpars2d.title  = "Depth 1 RecHit Time RBX Map-Profile, " + mysubdetstr_ + "; ieta; iRBX";
    hpars2d.nbinsx =  6;
    hpars2d.minx   =  -3.0;
    hpars2d.maxx   =   3.0;
    hpars2d.nbinsy =  18;
    hpars2d.miny   =    0.5;
    hpars2d.maxy   =   18.5;

    v_hpars2dprof.push_back(hpars2d);

    rhTprofd2_  = "p2d_rhTperIetaIphiD2" + mysubdetstr_;
    hpars2d.name   = rhTprofd2_;
    hpars2d.title  = "Depth 2 RecHit Time Map-Profile, " + mysubdetstr_ + "; ieta; iphi";
    hpars2d.nbinsx =  83;
    hpars2d.minx   =  -41.5;
    hpars2d.maxx   =   41.5;
    hpars2d.nbinsy =  72;
    hpars2d.miny   =    0.5;
    hpars2d.maxy   =   72.5;

    v_hpars2dprof.push_back(hpars2d);

    rhTprofRBXd2_  = "p2d_rhTperRBXD2" + mysubdetstr_;
    hpars2d.name   = rhTprofRBXd2_;
    hpars2d.title  = "Depth 2 RecHit Time RBX Map-Profile, " + mysubdetstr_ + "; ieta; iRBX";
    hpars2d.nbinsx =  6;
    hpars2d.minx   =  -3.0;
    hpars2d.maxx   =   3.0;
    hpars2d.nbinsy =  18;
    hpars2d.miny   =    0.5;
    hpars2d.maxy   =   18.5;

    v_hpars2dprof.push_back(hpars2d);

    if (mysubdet_ == HcalEndcap) {
      rhTprofd3_  = "p2d_rhTperIetaIphiD3HE";
      hpars2d.name   = rhTprofd3_;
      hpars2d.title  = "Depth 3 RecHit Time Map-Profile, HE; ieta; iphi";
      hpars2d.nbinsx =  83;
      hpars2d.minx   =  -41.5;
      hpars2d.maxx   =   41.5;
      hpars2d.nbinsy =  72;
      hpars2d.miny   =    0.5;
      hpars2d.maxy   =   72.5;

      v_hpars2dprof.push_back(hpars2d);

      rhTprofRBXd3_  = "p2d_rhTperRBXD3" + mysubdetstr_;
      hpars2d.name   = rhTprofRBXd3_;
      hpars2d.title  = "Depth 3 RecHit Time RBX Map-Profile, " + mysubdetstr_ + "; ieta; iRBX";
      hpars2d.nbinsx =  6;
      hpars2d.minx   =  -3.0;
      hpars2d.maxx   =   3.0;
      hpars2d.nbinsy =  18;
      hpars2d.miny   =    0.5;
      hpars2d.maxy   =   18.5;

      v_hpars2dprof.push_back(hpars2d);
    }
  } else {
    rhTprofd4_  = "p2d_rhTperIetaIphiD4HO";
    hpars2d.name   = rhTprofd4_;
    hpars2d.title  = "Depth 4 RecHit Time Map-Profile, HO; ieta; iphi";
    hpars2d.nbinsx =  83;
    hpars2d.minx   =  -41.5;
    hpars2d.maxx   =   41.5;
    hpars2d.nbinsy =  72;
    hpars2d.miny   =    0.5;
    hpars2d.maxy   =   72.5;

    v_hpars2dprof.push_back(hpars2d);

    rhTprofRBXd4_  = "p2d_rhTperRBXD4" + mysubdetstr_;
    hpars2d.name   = rhTprofRBXd4_;
    hpars2d.title  = "Depth 4 RecHit Time RBX Map-Profile, " + mysubdetstr_ + "; ieta; iRBX";
    hpars2d.nbinsx =  6;
    hpars2d.minx   =  -3.0;
    hpars2d.maxx   =   3.0;
    hpars2d.nbinsy =  18;
    hpars2d.miny   =    0.5;
    hpars2d.maxy   =   18.5;

    v_hpars2dprof.push_back(hpars2d);
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

    TProfile2D *tp = myAH->get<TProfile2D>(rhTprofRBXd1_);
    if (tp) {
      TAxis *xax = tp->GetXaxis();
      xax->SetBinLabel(1,"HF-");
      xax->SetBinLabel(2,"HE-");
      xax->SetBinLabel(3,"HB-");
      xax->SetBinLabel(4,"HB+");
      xax->SetBinLabel(5,"HE+");
      xax->SetBinLabel(6,"HF+");
    }

    tp = myAH->get<TProfile2D>(rhTprofRBXd2_);
    if (tp) {
      TAxis *xax = tp->GetXaxis();
      xax->SetBinLabel(1,"HF-");
      xax->SetBinLabel(2,"HE-");
      xax->SetBinLabel(3,"HB-");
      xax->SetBinLabel(4,"HB+");
      xax->SetBinLabel(5,"HE+");
      xax->SetBinLabel(6,"HF+");
    }

    tp = myAH->get<TProfile2D>(rhTprofRBXd3_);
    if (tp) {
      TAxis *xax = tp->GetXaxis();
      xax->SetBinLabel(1,"HF-");
      xax->SetBinLabel(2,"HE-");
      xax->SetBinLabel(3,"HB-");
      xax->SetBinLabel(4,"HB+");
      xax->SetBinLabel(5,"HE+");
      xax->SetBinLabel(6,"HF+");
    }
  }
}                                                          // bookHistos

//======================================================================

template<class RecHit, class DataFrame>
void
LaserDelayTunerAlgos::fillHistos4cut(const std::string& cutstr,
				       const RecHit&     rh,
				       const DataFrame&  df,
				       double tdccorrectedTime)
{
  //edm::LogInfo("Filling histograms for subdet ") << mysubdetstr_ << std::endl;

  myAnalHistos *myAH = m_cuts_[cutstr]->histos();

  HcalDetId detId = rh.id();
  double    htime = rh.time();
  double   energy = rh.energy();
  int        ieta = detId.ieta();
  int        iphi = detId.iphi();
  int       depth = detId.depth();
  int       zside = detId.zside();

  // need front end id: 
  HcalFrontEndId feID = lmap_->getHcalFrontEndId(detId);
  int            iRBX = atoi(((feID.rbx()).substr(3,2)).c_str());
  int             iRM = zside * ((iRBX-1)*4 + feID.rm());

  //cout << detId << "\t" << feID.rbx() << "\t" << feID.rbx().substr(3,2) << "\t";
  //cout << feID.rm() << " maps to RBX/RM# " << iRBX << "/" << iRM << endl;

  /* ALL subdet Histos: */
  myAH->fill1d<TH1D>(rhEnergies_,energy);
  myAH->fill2d<TH2D>(rhEmap_,ieta,iphi,energy);
  myAH->fill2d<TH2D>(uncorTimingVsE_,energy,htime);
  myAH->fill2d<TH2D>(corTimingVsE_,energy,tdccorrectedTime);

  /* Per Depth Histos: */

  std::string rhTprof, rhTprofRBX, avgRMt, avgRBXt, sduncorTime, sdcorTime;
  switch(depth) {
  case 1: rhTprof=rhTprofd1_; rhTprofRBX=rhTprofRBXd1_; avgRMt=avgTimePerRMd1_; avgRBXt=avgTimePerRBXd1_; break;
  case 2: rhTprof=rhTprofd2_; rhTprofRBX=rhTprofRBXd2_; avgRMt=avgTimePerRMd2_; avgRBXt=avgTimePerRBXd2_; break;
  case 3: rhTprof=rhTprofd3_; rhTprofRBX=rhTprofRBXd3_; avgRMt=avgTimePerRMd3_; avgRBXt=avgTimePerRBXd3_; break;
  case 4: rhTprof=rhTprofd4_; rhTprofRBX=rhTprofRBXd4_; avgRMt=avgTimePerRMd4_; avgRBXt=avgTimePerRBXd4_; break;
  default:
    edm::LogWarning("Invalid depth in rechit collection! detId = ") << detId << std::endl;
  }

  float fsubdet = 0;
  switch (mysubdet_) {
  case HcalBarrel:  fsubdet = zside*0.5; break;
  case HcalEndcap:  fsubdet = zside*1.5; break;
  case HcalForward: fsubdet = zside*2.5; break;
    //case HcalOuter:   fsubdet = zside*??
  default: break;
  }

  if (zside > 0) { sduncorTime = rhUncorTimesPlus_;  sdcorTime = rhCorTimesPlus_; }
  else           { sduncorTime = rhUncorTimesMinus_; sdcorTime = rhCorTimesMinus_; }

  myAH->fill1d<TProfile>(avgRMt,iRM, tdccorrectedTime);
  myAH->fill1d<TProfile>(avgRBXt,zside*iRBX, tdccorrectedTime);
  myAH->fill1d<TH1D>(sduncorTime, htime);
  myAH->fill1d<TH1D>(sdcorTime, tdccorrectedTime);

  myAH->fill2d<TProfile2D>(rhTprof,ieta,iphi,htime);
  myAH->fill2d<TProfile2D>(rhTprofRBX,fsubdet,iRBX,htime);

  // Digi histogram(s)
#if 0 // this kills the run with a segviol
  const HcalQIECoder *qieCoder = conditions_->getHcalCoder( df.id() );
  const HcalQIEShape *qieShape = conditions_->getHcalShape();
  HcalCoderDb coder( *qieCoder, *qieShape );
    
  CaloSamples dfC; // dfC is the linearized (fC) digi
  coder.adc2fC( df, dfC );

  // Loop over all timeslices
  for( int ts = 0; ts <= 9; ts++ )
    myAH->fill1d<TH1D>(avgPulse_, ts, dfC[ts] );
#endif
}

//======================================================================

template<class RecHit, class Digi >
void LaserDelayTunerAlgos::processRecHitsAndDigis (const edm::SortedCollection<RecHit>& rechits,
						   const edm::SortedCollection<Digi>&   digis)
{
  myAnalHistos *myAH = m_cuts_["cutNone"]->histos();
  myAH->fill1d<TH1D>(digiColSize_,digis.size());
  myAH->fill1d<TH1D>(rhColSize_,rechits.size());

  for (unsigned irh = 0, idf = 0;
       irh < rechits.size() && idf < digis.size();
       ++irh, ++idf) {

    const RecHit& rh = rechits[irh];
    const Digi&   df =   digis[idf];

    if (rh.id().subdet() != mysubdet_)
      continue; // HB and HE handled by separate instances of this class!

    if (rh.id() != df.id()) {
      edm::LogError("Digis and Rechits aren't tracking!") << df.id() << rh.id() << std::endl;
      return;
    }

    double modtime       = rh.time();
    double correctedTime = TDCalgo_->correctTimeForJitter(modtime);

    //Performs modulus on times (corrects for latency jumps)
    modtime       = TDCalgo_->modulusTheTime(rh.time());
    correctedTime = TDCalgo_->modulusTheTime(correctedTime);

    RecHit modrh(rh.id(),rh.energy(),modtime);

    fillHistos4cut("cutNone",modrh,df,correctedTime);

    bool isOverThresh = (modrh.energy() > minHitGeV_);
    if (isOverThresh)
      fillHistos4cut("cutMinHitGeV",modrh,df,correctedTime);

    if (TDCalgo_->isWithinWindow()) {
      fillHistos4cut("cutTDCwindow",modrh,df,correctedTime);
      if (isOverThresh)
	fillHistos4cut("cutAll",modrh,df,correctedTime);
    }
  } // loop over rechits

}                        // LaserDelayTunerAlgos::processRecHitsAndDigis

//======================================================================

#if 0
void
LaserDelayTunerAlgos::processHFrechits
(const edm::Handle<HFRecHitCollection>& hfrechits)
{
  for (unsigned irh = 0; irh < hfrechits->size (); ++irh) {
    const HFRecHit& rh = (*hfrechits)[irh];
    HcalDetId detId = rh.id();

    std::string rhTprof;
    switch(detId.depth()) {
    case 1: rhTprof = rhTprofd1_; break;
    case 2: rhTprof = rhTprofd2_; break;
      //case 3: rhTprof = rhTprofd3_; break; // not for hf!
      //case 4: rhTprof = rhTprofd4_; break; // not for hf!
    default:
      edm::LogWarning("Invalid depth in rechit collection! detId = ") << detId << std::endl;
      continue;
    }

    double minHitGeV = lookupThresh(detId);

    m_cuts_["cutNone"]->histos()->fill2d<TH2D>(hfTimingVsE_,rh.energy(),rh.time());

    if (rh.energy() > minHitGeV) {
      myAnalHistos *myAH = m_cuts_["cutMinHitGeV"]->histos();
      myAH->fill2d<TH2D>(rhEmap_,
			 detId.ieta(), detId.iphi()+detId.depth()-1,
			 rh.energy());

      myAH->fill2d<TH2D>(hfTimingVsE_,rh.energy(),rh.time());

      if (rh.energy() > 50.0)
	myAH->fill2d<TProfile2D>(rhTprof,detId.ieta(),detId.iphi(),rh.time());
    }
  } // loop over HF rechits
}                              // LaserDelayTunerAlgos::processHFrechits
#endif

//======================================================================

// ------------ method called to for each event  ------------
void
LaserDelayTunerAlgos::process(const myEventData& ed)
{
  if (firstEvent_) {
    bookHistos();
    firstEvent_ = false;
  }

  switch (mysubdet_) {
  case HcalBarrel:
  case HcalEndcap:
    processRecHitsAndDigis<HBHERecHit,HBHEDataFrame>(*(ed.hbherechits()),
						     *(ed.hbhedigis()));
    break;
  case HcalOuter:
    processRecHitsAndDigis<HORecHit,HODataFrame>(*(ed.horechits()),
						 *(ed.hodigis()));
    break;
  case HcalForward:
    processRecHitsAndDigis<HFRecHit,HFDataFrame>(*(ed.hfrechits()),
						 *(ed.hfdigis()));
    break;
  default:
    break;
  }
}

//======================================================================

void
LaserDelayTunerAlgos::beginJob(const edm::EventSetup& iSetup)
{
  iSetup.get<HcalDbRecord>().get( conditions_ );
}

//======================================================================

void
LaserDelayTunerAlgos::positivize(myAnalHistos *myAH, TProfile2D *h2)
{
  int locminx, locminy,locminz;
  double h2min = h2->GetMinimum();
  h2->GetMinimumBin(locminx,locminy,locminz);

  if (h2min >= 0.0) return;

  int numts;
  for (numts=0; h2min < 0; h2min+=25.0,numts++);

  double offset = numts*25.0;

  cout << "Positivizing histo " << h2->GetName();
  cout << ", minimum was "<<(h2min-offset)<<"@x,y="<<locminx<<","<<locminy;
  cout << ", offset=" << offset << endl;

  string *newname = new string(string(h2->GetName()) + "_offset");
  string newtitle =
    string(h2->GetTitle()) + " (+" + int2str(numts) + "TS offset)";

  myAH->bookClone<TProfile2D>(*newname,*h2);
  TProfile2D *h2offset = (TProfile2D *)myAH->get<TProfile2D>(*newname);
  h2offset->Clear();

  int nbinsx = h2->GetNbinsX();
  int nbinsy = h2->GetNbinsY();
  for (int biny=1;biny<=nbinsy;biny++) {
    for (int binx=1;binx<=nbinsx;binx++) {
      int ibin  = h2->GetBin(binx, biny);
      h2offset->SetBinContent(ibin,offset + h2->GetBinContent(ibin));
    }
  }

  //h2->Add(&f1,numts*25.0); "not implemented"!

  h2offset->SetTitle(newtitle.c_str());
}

//======================================================================

void
LaserDelayTunerAlgos::positivize2dHistos(void)
{
  // auto-offset 2d plots so that they are in the positive -
  // empty bins show up easier.
  //

  std::map<string, myAnalCut *>::const_iterator it;
  for (it = m_cuts_.begin(); it != m_cuts_.end(); it++) {
    myAnalHistos *myAH = it->second->histos();

    if (mysubdet_ == HcalOuter) {
      positivize(myAH,myAH->get<TProfile2D>(rhTprofd4_));
      positivize(myAH,myAH->get<TProfile2D>(rhTprofRBXd4_));
    } else {
      
      positivize(myAH,myAH->get<TProfile2D>(rhTprofd1_));
      positivize(myAH,myAH->get<TProfile2D>(rhTprofRBXd1_));
      positivize(myAH,myAH->get<TProfile2D>(rhTprofd2_));
      positivize(myAH,myAH->get<TProfile2D>(rhTprofRBXd2_));

      if (mysubdet_ == HcalEndcap) {
	positivize(myAH,myAH->get<TProfile2D>(rhTprofd3_));
	positivize(myAH,myAH->get<TProfile2D>(rhTprofRBXd3_));
      }
    }
  }
}                            // LaserDelayTunerAlgos::positivize2dHistos

//======================================================================
inline double tofloor(double x) { return (x>=0) ? 0 : -0.5; }
inline int    sign   (double x) { return (x>=0) ? 1 : -1; }

void
LaserDelayTunerAlgos::writeSettings(void)
{
  myAnalHistos *myAH = m_cuts_["cutAll"]->histos();
  TProfile *tp = myAH->get<TProfile>(avgTimePerRMd1_);

  double minRMtime = 1e99;
  for (int ibin=1; ibin<=tp->GetNbinsX(); ibin++) {
    int iRM = (int)tp->GetBinCenter(ibin);
    if (!iRM) continue;                      // skip bin at 0

    double rmtime = tp->GetBinContent(ibin);
    if (rmtime < minRMtime) minRMtime=rmtime;
  }

  // no rounding.
  int iFloorRMtime = (int)(minRMtime + tofloor(minRMtime));
  int oldRBX = -99;
  std::map<int,int> m_settingsPerRBX;
  m_settingsPerRBX.clear();

  char timestamp[30];
  if (writeBricks_) xml_.getTimeStamp(timestamp);

  printf("Settings for subdet %s Depth 1:\n",mysubdetstr_.c_str());
  printf ("bin\tiRM\tiRBX\tiRM%%4\tavgRMt\tSetting\n");
  for (int ibin=1; ibin<=tp->GetNbinsX(); ibin++) {
    int iRM = (int)tp->GetBinCenter(ibin);
    if (!iRM) continue;                     // skip bin at 0

    int iRBX     = (iRM - sign(iRM))/4;
    int iRMinRBX = (abs(iRM - sign(iRM))%4) + 1;

    iRBX += sign(iRM);

    if (iRBX != oldRBX) {
      if (writeBricks_ && m_settingsPerRBX.size())
	xml_.writePhiDelayBrick4rbx(mysubdetstr_,
				    oldRBX,
				    timestamp,
				    "USC55 RBX phi-tuned delays",
				    m_settingsPerRBX);
      m_settingsPerRBX.clear();
      oldRBX = iRBX;
    }

    if ((mysubdet_ == HcalBarrel) &&
	((iRBX == -5) ||
	 (iRBX == -8) ||
	 (iRBX == -9)   ))
      continue;             // broken fibers here

    double avgtimeThisRM = tp->GetBinContent(ibin);
    int setting = (int)(avgtimeThisRM - (double)iFloorRMtime);

    m_settingsPerRBX.insert(std::pair<int,int>(iRMinRBX,setting));

    printf ("%3d\t%3d\t%3d\t%d\t%6.3f\t%3d\n",
	    ibin, iRM, iRBX, iRMinRBX, avgtimeThisRM, setting);
  }
  // last box
  if (writeBricks_ && m_settingsPerRBX.size())
    xml_.writePhiDelayBrick4rbx(mysubdetstr_,
				oldRBX,
				timestamp,
				"USC55 RBX phi-tuned delays",
				m_settingsPerRBX);

}                                 // LaserDelayTunerAlgos::writeSettings

//======================================================================

void
LaserDelayTunerAlgos::endJob()
{
  positivize2dHistos();
  writeSettings();
}

//======================================================================
