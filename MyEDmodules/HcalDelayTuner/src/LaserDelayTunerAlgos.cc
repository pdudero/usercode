
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
// $Id: LaserDelayTunerAlgos.cc,v 1.1 2009/11/09 00:57:58 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>
#include <math.h> // floor

// user include files
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"

#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"
#include "EventFilter/HcalRawToDigi/interface/HcalDCCHeader.h"

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
LaserDelayTunerAlgos::LaserDelayTunerAlgos(const edm::ParameterSet& iConfig) :
  HcalDelayTunerAlgos(iConfig),
  minHitGeV_(iConfig.getParameter<double>("minHitGeV")),
  recHitTscaleNbins_(iConfig.getParameter<int>("recHitTscaleNbins")),
  recHitTscaleMinNs_(iConfig.getParameter<double>("recHitTscaleMinNs")),
  recHitTscaleMaxNs_(iConfig.getParameter<double>("recHitTscaleMaxNs")),
  recHitEscaleMinGeV_(iConfig.getParameter<double>("recHitEscaleMinGeV")),
  recHitEscaleMaxGeV_(iConfig.getParameter<double>("recHitEscaleMaxGeV")),
  rundescr_(iConfig.getUntrackedParameter<std::string>("runDescription",""))
{
  // cut string vector initialized in order
  v_cuts_.push_back("cutNone");
  v_cuts_.push_back("cutTDCwindow");
  v_cuts_.push_back("cutMinHitGeV");
  v_cuts_.push_back("cutAll");

  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());

  TDCalgo_   = new
    LaserDelayTunerTDCalgos(iConfig.getUntrackedParameter<edm::ParameterSet>("TDCpars"));

  firstEvent_ = true;
}                          // LaserDelayTunerAlgos::LaserDelayTunerAlgos

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
  else if (abs(ieta) <= 28)  subdet = HcalEndcap; // gud enuf fer gubmint werk
  else if (abs(ieta) <= 41)  subdet = HcalForward;
  else return false;

  if (!HcalDetId::validDetId(subdet,ieta,iphi,depth))
    return false;

  detId = HcalDetId(subdet,ieta,iphi,depth);
  return true;
}                                   // HFtrigAnalAlgos::convertIdNumbers

//======================================================================

void
LaserDelayTunerAlgos::add1dHisto(const std::string& name, const std::string& title,
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
LaserDelayTunerAlgos::add2dHisto(const std::string& name, const std::string& title,
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

  /*****************************************
   * 1-D HISTOGRAMS FIRST:                 *
   *****************************************/

  avgPulse_   = "h1d_pulse" + mysubdetstr_;
  add1dHisto(avgPulse_,"Average Pulse Shape, " + mysubdetstr_,10,-0.5,9.5,v_hpars1d);

  digiColSize_ = "DigiCollectionSize" + mysubdetstr_;
  add1dHisto( digiColSize_, "Digi Collection Size, " + mysubdetstr_, 
	      5201,-0.5, 5200.5, v_hpars1d); // 72chan/RBX*72RBX = 5184, more than HF or HO

  rhColSize_ = "RechitCollectionSize" + mysubdetstr_;
  add1dHisto( rhColSize_, "Rechit Collection Size, " + mysubdetstr_,
	      5201,-0.5, 5200.5,v_hpars1d);  // 72chan/RBX*72RBX = 5184, more than HF or HO
	     
  rhUncorTimesPlus_ = "h1d_rhUncorTimesPlus" + mysubdetstr_;
  add1dHisto( rhUncorTimesPlus_,
	      "RecHit Times (uncorrected), " + mysubdetstr_ + "P; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  rhUncorTimesMinus_ = "h1d_rhUncorTimesMinus" + mysubdetstr_;
  add1dHisto( rhUncorTimesMinus_,
	      "RecHit Times (uncorrected), " + mysubdetstr_ + "M; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  rhCorTimesPlus_ = "h1d_rhCorTimesPlus" + mysubdetstr_;
  add1dHisto( rhCorTimesPlus_,
	      "RecHit Times (TDC-corrected), " + mysubdetstr_ + "P; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  rhCorTimesMinus_ = "h1d_rhCorTimesMinus" + mysubdetstr_;
  add1dHisto( rhCorTimesMinus_,
	      "RecHit Times (TDC-corrected), " + mysubdetstr_ + "M; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  rhEnergies_    = "h1d_RHEnergies" + mysubdetstr_;
  add1dHisto( rhEnergies_,
	      "RecHit Energies, " + mysubdetstr_ + "; Rechit Energy (GeV)",
	      (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	      recHitEscaleMinGeV_,recHitEscaleMaxGeV_,v_hpars1d);

  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  if (mysubdet_ != HcalOuter) { 
    avgTimePerRMd1_ =  "p1d_avgTimePerRMd1" + mysubdetstr_;
    add1dHisto( avgTimePerRMd1_,
		"Averaged Time (Depth 1), " + mysubdetstr_ + "; iRM; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

    avgTimePerRBXd1_ =  "p1d_avgTimePerRBXd1" + mysubdetstr_;
    add1dHisto( avgTimePerRBXd1_,
		"Averaged Time (Depth 1), " + mysubdetstr_ + "; iRBX; Time (ns)",
		37,-18.5, 18.5,v_hpars1dprof);
 
    avgTimePerRMd2_ =  "p1d_avgTimePerRMd2" + mysubdetstr_;
    add1dHisto( avgTimePerRMd2_,
		"Averaged Time (Depth 2), " + mysubdetstr_ + "; iRM; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

    avgTimePerRBXd2_ =  "p1d_avgTimePerRBXd2" + mysubdetstr_;
    add1dHisto( avgTimePerRBXd2_,
		"Averaged Time (Depth 2), " + mysubdetstr_ + "; iRBX; Time (ns)",
		37,-18.5, 18.5,v_hpars1dprof);

    if (mysubdet_ == HcalEndcap) {
      avgTimePerRMd3_ =  "p1d_avgTimePerRMd3HE";
      add1dHisto( avgTimePerRMd3_,"Averaged Time (Depth 3), HE; iRM; Time (ns)",
		  145,-72.5, 72.5,v_hpars1dprof);

      avgTimePerRBXd3_ =  "p1d_avgTimePerRBXd3" + mysubdetstr_;
      add1dHisto( avgTimePerRBXd3_,
		  "Averaged Time (Depth 3), " + mysubdetstr_ + "; iRBX; Time (ns)",
		  37,-18.5, 18.5,v_hpars1dprof);
    }
  } else {
    avgTimePerRMd4_ = "p1d_avgTimePerRMd4HO";
    add1dHisto( avgTimePerRMd4_,"Averaged Time (Depth 4), HO; iRM; Time (ns)",
		145,-72.5, 72.5,v_hpars1dprof);

    avgTimePerRBXd4_ =  "p1d_avgTimePerRBXd4" + mysubdetstr_;
    add1dHisto( avgTimePerRBXd4_,
		"Averaged Time (Depth 4), " + mysubdetstr_ + "; iRBX; Time (ns)",
		37,-18.5, 18.5,v_hpars1dprof);
  }

  /*****************************************
   * 2-D HISTOGRAMS AFTER:                 *
   *****************************************/

  rhEmap_        = "h2d_rhEperIetaIphi" + mysubdetstr_;
  add2dHisto(rhEmap_, "RecHit Energy Map (#Sigma depths), " + mysubdetstr_ + "; ieta; iphi",
	     83, -41.5,  41.5, 72,   0.5,  72.5, v_hpars2d);

  uncorTimingVsE_ = "h2d_uncorTimingVsE" + mysubdetstr_;
  add2dHisto(uncorTimingVsE_,
	     "RecHit Timing (uncorrected) vs. Energy, " + mysubdetstr_ + "; Rechit Energy (GeV); Rechit Time (ns)",
	     (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	     recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	     v_hpars2d);

  corTimingVsE_ = "h2d_corTimingVsE" + mysubdetstr_;
  add2dHisto(corTimingVsE_,
	     "RecHit Timing (TDC-corrected) vs. Energy, " + mysubdetstr_ + "; Rechit Energy (GeV); Rechit Time (ns)",
	     (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_),
	     recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	     v_hpars2d);

  /*****************************************
   * 2-D PROFILES:                         *
   *****************************************/

  if (mysubdet_ != HcalOuter) { 
    rhTprofd1_  = "p2d_rhTperIetaIphiD1" + mysubdetstr_;
    add2dHisto(rhTprofd1_,"Depth 1 RecHit Time Map-Profile, " + mysubdetstr_ + "; ieta; iphi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       v_hpars2dprof);

    rhTprofRBXd1_  = "p2d_rhTperRBXD1" + mysubdetstr_;
    add2dHisto(rhTprofRBXd1_,"Depth 1 RecHit Time RBX Map-Profile, " + mysubdetstr_ + "; ieta; iRBX",
	       6, -3.0,  3.0, 18,   0.5,  18.5,
	       v_hpars2dprof);

    rhTprofd2_  = "p2d_rhTperIetaIphiD2" + mysubdetstr_;
    add2dHisto(rhTprofd2_,"Depth 2 RecHit Time Map-Profile, " + mysubdetstr_ + "; ieta; iphi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       v_hpars2dprof);

    rhTprofRBXd2_  = "p2d_rhTperRBXD2" + mysubdetstr_;
    add2dHisto(rhTprofRBXd2_,"Depth 2 RecHit Time RBX Map-Profile, " + mysubdetstr_ + "; ieta; iRBX",
	       6, -3.0,  3.0, 18,   0.5,  18.5,
	       v_hpars2dprof);

    if (mysubdet_ == HcalEndcap) {
      rhTprofd3_  = "p2d_rhTperIetaIphiD3HE";
      add2dHisto(rhTprofd3_,"Depth 3 RecHit Time Map-Profile, HE; ieta; iphi",
		 83, -41.5,  41.5, 72,   0.5,  72.5,
		 v_hpars2dprof);

      rhTprofRBXd3_  = "p2d_rhTperRBXD3" + mysubdetstr_;
      add2dHisto(rhTprofRBXd3_,"Depth 3 RecHit Time RBX Map-Profile, " + mysubdetstr_ + "; ieta; iRBX",
		 6, -3.0,  3.0, 18,   0.5,  18.5,
		 v_hpars2dprof);
    }
  } else {
    rhTprofd4_  = "p2d_rhTperIetaIphiD4HO";
    add2dHisto(rhTprofd4_,"Depth 4 RecHit Time Map-Profile, HO; ieta; iphi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       v_hpars2dprof);

    rhTprofRBXd4_  = "p2d_rhTperRBXD4" + mysubdetstr_;
    add2dHisto(rhTprofRBXd4_,"Depth 4 RecHit Time RBX Map-Profile, " + mysubdetstr_ + "; ieta; iRBX",
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

inline int    sign   (double x) { return (x>=0) ? 1 : -1; }

void
LaserDelayTunerAlgos::detChannelTimes(TimesPerDetId& chtimes)
{
  myAnalHistos *myAH = m_cuts_["cutAll"]->histos();
  TProfile *tp = myAH->get<TProfile>(avgTimePerRMd1_);

  float minRMtime = 1e99;
  for (int ibin=1; ibin<=tp->GetNbinsX(); ibin++) {
    int iRM = (int)tp->GetBinCenter(ibin);
    if (!iRM) continue;                      // skip bin at 0

    float rmtime = tp->GetBinContent(ibin);
    if (rmtime < minRMtime) minRMtime=rmtime;
  }

  // no rounding.
  int iFloorRMtime = (int)(floor(minRMtime));


  printf("Settings for subdet %s Depth 1:\n",mysubdetstr_.c_str());
  printf ("bin\tiRM\tiRBX\tiRM%%4\tavgRMt\tSetting\n");
  for (int ibin=1; ibin<=tp->GetNbinsX(); ibin++) {
    int iRM = (int)tp->GetBinCenter(ibin);
    if (!iRM) continue;                     // skip bin at 0

    int iRBX     = (iRM - sign(iRM))/4;
    int iRMinRBX = (abs(iRM - sign(iRM))%4) + 1;
    iRBX += sign(iRM);
    char rbx[10];
    sprintf (rbx,"%s%c%02d",mysubdetstr_.c_str(),((iRM>0)?'P':'M'),iRBX);

    double avgtimeThisRM = tp->GetBinContent(ibin);
    double time = (int)(avgtimeThisRM - (double)iFloorRMtime);

    for (int i=1; i<4; i++) {
      for (int j=0; j<5; j++) {
#if 0
	HcalFrontEndId feID(string(rbx),iRMinRBX,0,1,0,i,j);
	chtimes.insert(std::pair<HcalFrontEndId,double>(feID,time));
#endif
	printf ("%3d\t%3d\t%3d\t%d\t%6.3f\n",
		ibin, iRM, iRBX, iRMinRBX, avgtimeThisRM);
      }
    }
  }
}                               // LaserDelayTunerAlgos::detChannelTimes

//======================================================================

// ------------ method called to for each event  ------------
void
LaserDelayTunerAlgos::process(const myEventData& ed)
{
  bool isLocalHCALrun = ed.hcaltbtrigdata().isValid();
  bool isLaserEvent   = false;
  if (isLocalHCALrun) {
    TDCalgo_->process(ed);
    if (ed.hcaltbtrigdata()->wasLaserTrigger()) isLaserEvent = true;
  }

  // Abort Gap laser 
  if ((!isLocalHCALrun || !isLaserEvent) &&
      ed.fedrawdata().isValid())
  {
    //checking FEDs for calibration information
    for (int i=FEDNumbering::getHcalFEDIds().first;i<=FEDNumbering::getHcalFEDIds().second; i++) {
      const FEDRawData& fedData = ed.fedrawdata()->FEDData(i) ;
      if ( fedData.size() < 24 ) continue ;
      int value = ((const HcalDCCHeader*)(fedData.data()))->getCalibType() ;
      if(value==hc_HBHEHPD || value==hc_HOHPD || value==hc_HFPMT){ isLaserEvent=true; break;} 
    }
  }   

  if(!isLaserEvent) return;
  else
    nlaserEv_++;

  if (firstEvent_) {
    bookHistos();
    firstEvent_ = false;
    if (isLocalHCALrun) 
      cout << "Local TB trigger data detected\n" << endl;
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

void
LaserDelayTunerAlgos::endAnal()
{
  positivize2dHistos();
}

//======================================================================
