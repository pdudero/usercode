
// -*- C++ -*-
//
// Package:    BeamTimingAnalAlgos
// Class:      BeamTimingAnalAlgos
// 
/**\class BeamTimingAnalAlgos BeamTimingAnalAlgos.cc MyEDmodules/HcalTimingAnal/src/BeamTimingAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: BeamTimingAnalAlgos.cc,v 1.3 2010/08/10 23:43:10 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Provenance/interface/MinimalEventID.h"
#include "DataFormats/Provenance/interface/LuminosityBlockID.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"
#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "RecoLocalCalo/HcalRecAlgos/interface/HcalCaloFlagLabels.h"

#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"

#include "../interface/BeamTimingAnalAlgos.hh"


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

static bool isDepth1 (const HcalDetId& id) { return ((HcalDetId(id)).depth()==1); }
static bool isDepth2 (const HcalDetId& id) { return ((HcalDetId(id)).depth()==2); }
static bool isHEP28d1(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==1) &&
						 (did.subdet()==HcalEndcap) &&
						 (did.ieta()==28)); }
static bool isHEP28d2(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==2) &&
						 (did.subdet()==HcalEndcap) &&
						 (did.ieta()==28)); }
static bool isHEP28d3(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==3) &&
						 (did.subdet()==HcalEndcap) &&
 						 (did.ieta()==28)); }
static bool isHEP29d1(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==1) &&
						 (did.subdet()==HcalEndcap) &&
						 (did.ieta()==29)); }
static bool isHEP29d2(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==2) &&
						 (did.subdet()==HcalEndcap) &&
						 (did.ieta()==29)); }
static bool isHEM28d1(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==1) &&
						 (did.subdet()==HcalEndcap) &&
						 (did.ieta()==-28)); }
static bool isHEM28d2(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==2) &&
						 (did.subdet()==HcalEndcap) &&
      						 (did.ieta()==-28)); }
static bool isHEM28d3(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==3) &&
						 (did.subdet()==HcalEndcap) &&
						 (did.ieta()==-28)); }
static bool isHEM29d1(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==1) &&
						 (did.subdet()==HcalEndcap) &&
						 (did.ieta()==-29)); }
static bool isHEM29d2(const DetId& id) { HcalDetId did=HcalDetId(id);
                                         return ((did.depth()==2) &&
						 (did.subdet()==HcalEndcap) &&
						 (did.ieta()==-29)); }

std::string
BeamTimingAnalAlgos::addCut(const std::string& descr,
			    bool doInverted)
{
  size_t N = v_nestedCuts_.size();
  string cutstr = "cut" + int2str(N) + descr;
  v_nestedCuts_.push_back(cutstr);
  m_cuts_[cutstr] = new myAnalCut(N,cutstr,*mysubdetRootDir_,doInverted);
  return cutstr;
}

std::string
BeamTimingAnalAlgos::addHitCategory(const std::string& descr,
				    bool doInverted)
{
  size_t N = v_nestedCuts_.size()+v_hitCategories_.size();
  string cutstr = "cut" + int2str(N) + descr;
  v_hitCategories_.push_back(cutstr);
  m_cuts_[cutstr] = new myAnalCut(N,cutstr,*mysubdetRootDir_,doInverted);
  return cutstr;
}

//
// constructors and destructor
//
BeamTimingAnalAlgos::BeamTimingAnalAlgos(const edm::ParameterSet& iConfig,
					 BeamHitTimeCorrector *timecor) :
  HcalTimingAnalAlgos(iConfig),
  timecor_(timecor)
{

  cout << "My subdet is " << mysubdetstr_ << endl;

  std::vector<int> badEventVec =
    iConfig.getParameter<vector<int> >("badEventList");
  for (size_t i=0; i<badEventVec.size(); i++)
    badEventSet_.insert(badEventVec[i]);

  std::vector<int> acceptedBxVec =
    iConfig.getParameter<vector<int> >("acceptedBxNums");
  for (size_t i=0; i<acceptedBxVec.size(); i++)
    acceptedBxNums_.insert(acceptedBxVec[i]);

  std::vector<int> acceptedPkTSvec =
    iConfig.getParameter<vector<int> >("acceptedPkTSnumbers");
  for (size_t i=0; i<acceptedPkTSvec.size(); i++)
    acceptedPkTSnums_.insert(acceptedPkTSvec[i]);

  if ((mysubdet_ == HcalBarrel) ||
      (mysubdet_ == HcalEndcap) )
    hbheOutOfTimeFlagBit_ = iConfig.getUntrackedParameter<int>("hbheOutOfTimeFlagBit", 8);

  //======================================================================
  // Initialize the cuts for the run and add them to the global map
  // Nested cut string vector initialized in order
  //======================================================================

  m_cuts_.clear();

  const bool doInverted = true;

  edm::Service<TFileService> fs;
  mysubdetRootDir_ = new TFileDirectory(fs->mkdir(mysubdetstr_));

  // all cuts applied on top of the previous one
  //
  st_cutNone_ = addCut("none");

  if (badEventVec.size())   st_cutBadEv_ = addCut("badEv",doInverted);
  if (acceptedBxVec.size()) st_cutBadBx_ = addCut("bxnum",doInverted);

  if (globalFlagMask_)   st_cutBadFlags_ = addCut("badFlags",doInverted);

  st_cutHitEwindow_ = addCut("hitEwindow"); // ,doInverted);

  //if (!acceptedPkTSnums_.empty())
  if ((mysubdet_ == HcalBarrel) ||
      (mysubdet_ == HcalEndcap) ) {

    st_fraction2ts_  = addHitCategory("fraction2ts",doInverted); getCut(st_fraction2ts_) ->setFlag(st_fillDetail_);
                                                                 getCut(st_fraction2ts_) ->setFlagInv(st_perEvtDigi_);
    st_cutOutOfTime_ = addHitCategory("outOfTime",  doInverted); getCut(st_cutOutOfTime_)->setFlag(st_fillDetail_);

  }
  if (mysubdet_ == HcalForward) {
    st_PMThits_  = addHitCategory("keepPMThits");
    getCut(st_PMThits_)->setFlag(st_fillDetail_);

    st_PMTpartners_  = addHitCategory("keepPMTpartners");
    getCut(st_PMTpartners_)->setFlag(st_fillDetail_);

    st_dubiousHits_ = addHitCategory("keepDubiousHits");
    getCut(st_dubiousHits_)->setFlag(st_fillDetail_);

    st_goodHits_ = addHitCategory("keepGoodHits");
    st_lastCut_  = st_goodHits_;

    st_region1_  = addHitCategory("region1"); getCut(st_region1_)->setFlag(st_fillDetail_);
    st_region2_  = addHitCategory("region2"); getCut(st_region2_)->setFlag(st_fillDetail_);
    st_region3_  = addHitCategory("region3"); getCut(st_region3_)->setFlag(st_fillDetail_);
    st_region4_  = addHitCategory("region4"); getCut(st_region4_)->setFlag(st_fillDetail_);
                                              getCut(st_region4_)->setFlag(st_perEvtDigi_);
    st_region5_  = addHitCategory("region5"); getCut(st_region5_)->setFlag(st_fillDetail_);
                                              getCut(st_region5_)->setFlag(st_perEvtDigi_);

  } else {
    st_lastCut_ = v_nestedCuts_[v_nestedCuts_.size()-1];
  }

  getCut(st_lastCut_)->setFlag(st_fillDetail_);
  if (doPerChannel_)
    getCut(st_lastCut_)->setFlag(st_doPerChannel_);

}                        // BeamTimingAnalAlgos::BeamTimingAnalAlgos

//==================================================================

float calcR(float Elong, float Eshort)
{
  float ratio = 0.0;
  float diff = Elong - Eshort;
  float sum  = Elong + Eshort;
  if (sum != 0.0)
    ratio = diff/sum;

  return ratio;
}

float calcR2(float Elong, float Eshort)
{
  float ratio  = 0.0;
  float numer  = Eshort;
  float denom  = Elong + Eshort;
  if (denom != 0.0)
    ratio = numer/denom;
  
  return ratio;
}

inline
float calcElimit4long(int ieta) {
  return (162.4 - (10.19*abs(ieta)) + (0.21*ieta*ieta));
}

inline
float calcElimit4short(int ieta) {
  return (130 - (6.61*abs(ieta)) + (0.1153*ieta*ieta));
}

//==================================================================
// Igor Vodopiyanov's algorithm for determining PMT hit
//
bool
isHFPMThit(const HFRecHit& queried, const HFRecHit& partner)
{
  assert ((queried.id().ieta()  == partner.id().ieta()) &&
	  (queried.id().iphi()  == partner.id().iphi()) &&
	  (queried.id().depth() != partner.id().depth()) );

  bool isPMThit = false;
  int ieta   = queried.id().ieta();
  int depth  = queried.id().depth();
  float eGeV = queried.energy();

  switch (depth) {
  case 1: // LONG
    if (eGeV > 1.2) {
      float ratio  = calcR(eGeV,partner.energy());
      float elimit = calcElimit4long(ieta);
      isPMThit     = (ratio > 0.98) && (eGeV > elimit);
    }
    break;
  case 2: // SHORT
    if (eGeV > 1.8) {
      float ratio  = calcR(partner.energy(),eGeV);
      float elimit = calcElimit4short(ieta);
      isPMThit     = (ratio < -0.98) && (eGeV > elimit);
    }
    break;
  }
  return isPMThit;
}                                                      // isHFPMThit

//==================================================================
// Official Hit Reflagger version
//
inline bool
isHFPMThit(const HFRecHit& queried)
{
  //  return queried.flagField(HcalCaloFlagLabels::UserDefinedBit0);
  return queried.flagField(HcalCaloFlagLabels::HFLongShort);

}                                                      // isHFPMThit

//==================================================================
void
BeamTimingAnalAlgos::bookHistos4allCuts(void)
{
  std::string titlestr;
  
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars1d_af;
  std::vector<myAnalHistos::HistoParams_t>   v_hpars2d;
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars2d_af;
  
  HcalTimingAnalAlgos::bookHistos4allCuts();

  if (mysubdet_ != HcalOther) {
    titlestr = "#Sigma_{i=2maxTS} fC_{i}, "+mysubdetstr_+", "+rundescr_+"; fC";
    add1dAFhisto("h1d_summedfC" + mysubdetstr_,titlestr,
		 recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,
		 (void *)&fCamplitude_,NULL,NULL,v_hpars1d_af);

    titlestr = "#Sigma_{i=2maxTS} fC_{i}, "+mysubdetstr_+"D1, "+rundescr_+"; fC";
    add1dAFhisto("h1d_sumfC2ts" + mysubdetstr_+"D1",titlestr,
		 recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,
		 (void *)&fCamplitude_,NULL,(detIDfun_t)&isDepth1,v_hpars1d_af);

    titlestr = "#Sigma_{i=2maxTS} fC_{i}, "+mysubdetstr_+"D2, "+rundescr_+"; fC";
    add1dAFhisto("h1d_sumfC2ts" + mysubdetstr_+"D2",titlestr,
		 recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,
		 (void *)&fCamplitude_,NULL,(detIDfun_t)&isDepth2,v_hpars1d_af);

    titlestr = "RecHit Timing vs #Sigma_{i=2maxTS} fC_{i}, "+mysubdetstr_+"; fC; Rechit Time (ns)";
    add2dAFhisto("h2d_corTimingVsCh" + mysubdetstr_,titlestr,
	       recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&fCamplitude_,(void *)&corTime_,NULL,NULL, v_hpars2d_af);

    add2dAFhisto("h2d_sumfCperIetaIphi" + mysubdetstr_,
		 "#Sigma_{i=2maxTS,depths} fC_{i} Map, "+mysubdetstr_+", "+rundescr_+"; i#eta; i#phi",
		 83, -41.5,  41.5, 72,   0.5,  72.5, 
		 (void *)&fieta_,(void *)&fiphi_,(void *)&fCamplitude_,NULL,v_hpars2d_af);
  }
  if (mysubdet_ == HcalForward) {
    titlestr    =
      "Hits/Tower vs. (L-S)/(L+S) & E_{twr}, "+rundescr_+"; (L-S)/(L+S); E_{twr} (GeV)";
    st_RvsEtwr_ = "h2d_RvsEtwrHF";
    add2dHisto(st_RvsEtwr_, titlestr, 200,-1.0,1.0,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_, v_hpars2d);
    
    titlestr     = "Hits/Tower vs. S/(L+S) & E_{twr}, "+rundescr_+"; S/(L+S); E_{twr} (GeV)";
    st_R2vsEtwr_ = "h2d_R2vsEtwrHF";
    add2dHisto(st_R2vsEtwr_, titlestr, 200,-1.0,1.0,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_, v_hpars2d);
    
    titlestr    = "Hits/Tower vs. (L-S)/(L+S) & i#eta, "+rundescr_+"; (L-S)/(L+S); i#eta";
    st_RvsIeta_ = "h2d_RvsIetaHF";
    add2dHisto(st_RvsIeta_, titlestr, 200,-1.0,1.0, 13,28.5,41.5, v_hpars2d);
    
    titlestr     = "Hits/Tower vs. S/(L+S) & i#eta, "+rundescr_+"; S/(L+S); i#eta";
    st_R2vsIeta_ = "h2d_R2vsIetaHF";
    add2dHisto(st_R2vsIeta_, titlestr, 200,-1.0,1.0, 13,28.5,41.5, v_hpars2d);
  }
  /*******************
   * BOOK 'EM, DANNO *
   *******************/

  std::map<std::string,myAnalCut *>::const_iterator cutit;
  for (cutit = m_cuts_.begin(); cutit != m_cuts_.end(); cutit++) {
    myAnalHistos *myAH = cutit->second->cuthistos();
    myAH->book1d<TH1F>(v_hpars1d_af);
    myAH->book2d<TH2F>(v_hpars2d);
    myAH->book2d<TH2F>(v_hpars2d_af);
    if (cutit->second->doInverted()) {
      myAH = cutit->second->invhistos();
      myAH->book1d<TH1F>(v_hpars1d_af);
      myAH->book2d<TH2F>(v_hpars2d);
      myAH->book2d<TH2F>(v_hpars2d_af);
    }
  }
}                         // BeamTimingAnalAlgos::bookHistos4allCuts

//==================================================================

static const int nEbins= 12;
static const double Ebins[] = {
  1.0,2.0,3.0,4.0,5.0,7.0,10.0,20.0,40.0,100.0,300.0,600.0,1000.0
};

static const int nChbins= 12;
static const double chbins[] = {
  5.0,10.0,15.0,20.0,25.0,35.0,50.0,100.0,200.0,500.0,1500.0,3000.0,5000.0
};

void
BeamTimingAnalAlgos::bookDetailHistos4cut(myAnalCut& cut)
{
  assert(cut.flagSet(st_fillDetail_));

  myAnalHistos *myAH = cut.cuthistos();

  char title[128]; std::string titlestr;

  std::vector<myAnalHistos::HistoParams_t>   v_hpars1d;
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars1d_af;
  std::vector<myAnalHistos::HistoParams_t>   v_hpars2d;
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars2d_af;
  std::vector<myAnalHistos::HistoParams_t>   v_hpars2dprof;
  std::vector<myAnalHistos::HistoParams_t>   v_hpars3d;

  HcalTimingAnalAlgos::bookDetailHistos4cut(cut);


  titlestr =
 "Number of hits vs i#eta,i#phi and Energy (#Sigma depths), "+mysubdetstr_+"; i#eta; i#phi; E (GeV)";
  add3dHisto("h3d_NhitsPerIetaIphiEn"+mysubdetstr_, titlestr,
	     83, -41.5,  41.5, 72, 0.5, 72.5, 50,0,2500,v_hpars3d);

  titlestr = "RecHit Occupancy Map above 50GeV, "+mysubdetstr_+", "+rundescr_+"; i#eta; i#phi";
  add2dHisto("h2d_NhitsPerIetaIphiOver50" + mysubdetstr_, titlestr,
	     83, -41.5,  41.5, 72, 0.5, 72.5, v_hpars2d);

  // broken down by depth per Z-side
  if (mysubdet_ == HcalForward) {
    titlestr =
      "Vertex Z correction vs. Hit Time, "+rundescr_+"; Hit Time (ns); Correction (ns)";
    add2dAFhisto("h2d_TcorVsThit", titlestr, 
		 recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,81,-2.025,2.025,
		 (void *)&hittime_,(void *)&correction_ns_,NULL,NULL,v_hpars2d_af);

    titlestr  = "2TS Ratio vs #Sigma_{i=2maxTS,depths} fC_{i}, "+rundescr_;
    titlestr += "; fC; 2TS Ratio of E samples (0.5 = TS3/4 boundary)";
    add2dAFhisto("h2d_2TSratioVsEallHF",titlestr,
		 recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,200,-1.0,3.0,
		 (void *)&fCamplitude_,(void *)&twoTSratio_,NULL,NULL,v_hpars2d_af);

    titlestr  = "2TS Ratio vs #Sigma_{i=2maxTS} fC_{i}, HF Depth 1, "+rundescr_;
    titlestr += "; fC; 2TS Ratio of E samples (0.5 = TS3/4 boundary)";
    add2dAFhisto("h2d_2TSratioVsEHFD1",titlestr,
		 recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,200,-1.0,3.0,
		 (void *)&fCamplitude_,(void *)&twoTSratio_,NULL,(detIDfun_t)&isDepth1,v_hpars2d_af);

    titlestr  = "2TS Ratio vs #Sigma_{i=2maxTS} fC_{i}, HF Depth 2, "+rundescr_;
    titlestr += "; fC; 2TS Ratio of E samples (0.5 = TS3/4 boundary)";
    add2dAFhisto("h2d_2TSratioVsEHFD2",titlestr,
		 recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,200,-1.0,3.0,
		 (void *)&fCamplitude_,(void *)&twoTSratio_,NULL,(detIDfun_t)&isDepth2,v_hpars2d_af);

    // study 2TS algo performance
    //
    titlestr  = "T_{2TS} vs. 2TS Ratio, all HF, "+rundescr_;
    titlestr += "; 2TS Ratio of E samples (0.5 = TS3/4 boundary); T_{2ts}";
    add2dAFhisto("h2d_TcorVs2TSratioAllHF",titlestr,200,-1.0,3.0,
		 recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
		 (void *)&twoTSratio_,(void *)&corTime_,NULL,NULL,v_hpars2d_af);

    // study 3TS algo performance
    //
    titlestr  = "T_{LUT}-T_{3TS} vs.Charge, all HF, "+rundescr_;
    titlestr += ";#Sigma_{i=2maxTS} fC_{i}; T_{LUT}-T_{3TS} (ns)";
    add2dAFhisto("h2d_Tlut-T3TSvsChargeAllHF",titlestr,
		 recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,
		 recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
		 (void *)&fCamplitude_,(void *)&tLUTminust3TS_,NULL,NULL,v_hpars2d_af);

    titlestr  = "T_{LUT}-T_{3TS} vs.Energy, all HF, "+rundescr_;
    titlestr += ";#Sigma_{i=2maxTS} fC_{i}; T_{LUT}-T_{3TS} (ns)";
    add2dAFhisto("h2d_Tlut-T3TSvsEnergyAllHF",titlestr,
		 recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
		 recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
		 (void *)&hitenergy_,(void *)&tLUTminust3TS_,NULL,NULL,v_hpars2d_af);

    titlestr  = "T(3TS) vs. Energy, HF; Rechit Energy (GeV); T(3TS) (ns)";
    titlestr += "; Rechit Energy (GeV); T_{3TS} (ns)";
    add2dAFhisto("h2d_t3tsVsEHF",titlestr,
		 recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
		 recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
		 (void *)&hitenergy_,(void *)&treco3ts_,NULL,NULL, v_hpars2d_af);

    titlestr  = "T(3TS) vs. #Sigma_{i=2maxTS} fC_{i}, HF";
    titlestr += "; #Sigma_{i=2maxTS} fC_{i}; T(3TS) (ns)";
    add2dAFhisto("h2d_t3tsVsChHF",titlestr,
		 recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
		 recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
		 (void *)&hitenergy_,(void *)&treco3ts_,NULL,NULL, v_hpars2d_af);
    
    // Study occupancy  of good hits
    //
    st_OccVsEtaEnergyBothOverThresh_ = "h2d_OccVsEtaEnergyBothOverThreshHF";
    titlestr  = "Nhits/tower Vs i#eta & E_{hit,min}, L,S>E_{hit,min}, HF";
    titlestr += ", "+rundescr_+"; |i#eta|; E_{hit,min} (GeV)";
    
    add2dHisto(st_OccVsEtaEnergyBothOverThresh_, titlestr, 41, 0.5,  41.5,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       v_hpars2d);
              
    st_LvsSHF_ = "h2d_LvsSHF";
    add2dHisto(st_LvsSHF_, "Short E Vs. Long E, HF; E_{LONG} (GeV); E_{SHORT} (GeV)",
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       v_hpars2d);
              
    // Study depth timing differences
    //
    st_deltaTvsLandSenHF_ = "p2d_deltaTvsLandSenHF";
    titlestr  = "T_{d2}-T_{d1} Vs. Long and Short Energies, HF";
    titlestr += "; E_{LONG} (GeV); E_{SHORT} (GeV)";
    add2dHisto(st_deltaTvsLandSenHF_, titlestr,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       v_hpars2dprof);
              
    st_deltaTvsLandSchHF_ = "p2d_deltaTvsLandSchHF";
    titlestr  = "T_{d2}-T_{d1} Vs. Long and Short Charge Sums, HF";
    titlestr += "; fC_{LONG}; fC_{SHORT}";
    add2dHisto(st_deltaTvsLandSchHF_, titlestr,
	       recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,
	       recHitEscaleNbins_,recHitEscaleMinfC_,recHitEscaleMaxfC_,
	       v_hpars2dprof);

    string fmt("Average Times Per Event, Depth 2 vs. Depth 1, HF%c, Run %d; ");
    fmt += string("Depth 1 Hit Time (ns); Depth 2 Hit Time (ns)");

    sprintf (title,fmt.c_str(),'P',runnum_); titlestr = string(title);
    st_rhCorTimesD1vsD2plus_ = "h2d_rhCorTimesD1vsD2HFP";
    add2dHisto(st_rhCorTimesD1vsD2plus_, titlestr,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_, v_hpars2d);

    sprintf (title,fmt.c_str(),'M',runnum_); titlestr = string(title);
    st_rhCorTimesD1vsD2minus_ = "h2d_rhCorTimesD1vsD2HFM";
    add2dHisto(st_rhCorTimesD1vsD2minus_, titlestr,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_, v_hpars2d);
#if 0
    st_lowEtimingMapD1_     = "p2d_lowEtimingMapD1";
    titlestr = "Hit Time Map (5GeV< E_{hit}< 10GeV) for Depth 1, "+rundescr_+"; i#eta; i#phi";
    add2dHisto(st_lowEtimingMapD1_,titlestr,83,-41.5,41.5,72,0.5,72.5,v_hpars2dprof);

    st_lowEtimingMapD2_     = "p2d_lowEtimingMapD2";
    titlestr = "Hit Time Map (5GeV< E_{hit}< 10GeV) for Depth 2, "+rundescr_+"; i#eta; i#phi";
    add2dHisto(st_lowEtimingMapD2_,titlestr,83,-41.5,41.5,72,0.5,72.5,v_hpars2dprof);

    st_lateHitsTimeMapD1_ = "p2d_lateHitsTimeMapD1";
    titlestr = "Hit Time Map (T_{hit} > 12ns) for Depth 1, "+rundescr_+"; i#eta; i#phi";
    add2dHisto(st_lateHitsTimeMapD1_,titlestr,83,-41.5,41.5,72,0.5,72.5,v_hpars2dprof);

    st_lateHitsTimeMapD2_ = "p2d_lateHitsTimeMapD2";
    titlestr = "Hit Time Map (T_{hit} > 12ns) for Depth 2, "+rundescr_+"; i#eta; i#phi";
    add2dHisto(st_lateHitsTimeMapD2_,titlestr,83,-41.5,41.5,72,0.5,72.5,v_hpars2dprof);

    st_RvsTandfCHFd1_ = "h3d_RvsTandfCHFd1";
    titlestr = "R=(L-S)/(L+S) vs. T_{2TS}, Depth 1, "+rundescr_+"; T_{2TS} (ns); R; Hit Energy (GeV)";
    add3dHisto(st_RvsTandfCHFd1_, titlestr,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,200,-1.0,1.0,
 	       50,recHitEscaleMinfC_,recHitEscaleMaxfC_, v_hpars3d);

    st_RvsTandfCHFd2_ = "h3d_RvsTandfCHFd2";
    titlestr = "R=(L-S)/(L+S) vs. T_{2TS}, Depth 2, "+rundescr_+"; T_{2TS} (ns); R";
    add3dHisto(st_RvsTandfCHFd2_, titlestr,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,200,-1.0,1.0,
 	       50,recHitEscaleMinfC_,recHitEscaleMaxfC_,v_hpars3d);
#endif
    // per-channel beam-specific histos
    if (cut.flagSet(st_doPerChannel_)) {
      myAH->mkSubdir<uint32_t>("_2TSratioVsEperID");
    }
  } // if HF
  else if (mysubdet_ == HcalEndcap) {
    // Tower 28/29 histos
    for (int ieta=28; ieta<=29; ieta++){
      for (int depth=1; depth<=3; depth++) {
	static const detIDfun_t HEPdetIDfun[] = {
	  (detIDfun_t)&isHEP28d1,(detIDfun_t)&isHEP28d2,(detIDfun_t)&isHEP28d3,
	  (detIDfun_t)&isHEP29d1,(detIDfun_t)&isHEP29d2,NULL};
	static const detIDfun_t HEMdetIDfun[] = {
	  (detIDfun_t)&isHEM28d1,(detIDfun_t)&isHEM28d2,(detIDfun_t)&isHEM28d3,
	  (detIDfun_t)&isHEM29d1,(detIDfun_t)&isHEM29d2,NULL};

	int funlkup = 3*(ieta-28) + (depth-1);

	if (!HEPdetIDfun[funlkup]) continue;

	stringstream name;  name  << "h1d_rhTimesHEP" << ieta << "d" << depth;
	stringstream title; title<<"RecHit Times for HEP i#eta="<<ieta<<" depth "<<depth<<"; Rechit Time (ns)";
	add1dAFhisto(name.str(), title.str(),
		     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
		     (void *)&corTime_,NULL,HEPdetIDfun[funlkup],v_hpars1d_af);

	name.str("");  name << "h1d_rhTimesHEM" << ieta << "d" << depth;
	title.str(""); title<<"RecHit Times for HEM i#eta="<<-ieta<<" depth "<<depth<<"; Rechit Time (ns)";
	add1dAFhisto(name.str(), title.str(),
		     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
		     (void *)&corTime_,NULL,HEMdetIDfun[funlkup],v_hpars1d_af);
      }
    }
  } // if HE

  // These are not per-hit histos, but per-event histos
  //
  st_rhCorTimesPlusVsMinus_ = "h2d_rhCorTimesPlusVsMinus" + mysubdetstr_;
  titlestr = "T_{avg}/Event, Plus vs. Minus, "+mysubdetstr_;
  titlestr += ", E_{tot,+},E_{tot,-}>10GeV, "+rundescr_+"; Minus (ns); Plus (ns)";
  add2dHisto(st_rhCorTimesPlusVsMinus_,titlestr,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_, v_hpars2d);

  st_rhDeltaTvsVertexCor_ = "h2d_rhDeltaTvsVertexCor" + mysubdetstr_;
  titlestr = "T_{"+mysubdetstr_+"P-"+mysubdetstr_+"M}/Event vs. Vertez z correction, "+mysubdetstr_;
  titlestr += ", E_{tot,+},E_{tot,-}>10GeV, "+rundescr_;
  titlestr += "; T_{avg,+}-T_{avg,-} (ns); Vertex Z Correction (ns)";
  add2dHisto(st_rhDeltaTvsVertexCor_,titlestr,100,-10.,10.,100,-20.,20.,v_hpars2d);

  st_nHitsPlus_ = "h1d_nHits"+mysubdetstr_+"P";
  sprintf (title, "# Hits, %sP, Run %d; # Hits", mysubdetstr_.c_str(), runnum_);
  titlestr = string(title);
  add1dHisto( st_nHitsPlus_, titlestr, 1301,-0.5, 1300.5, v_hpars1d);

  st_nHitsMinus_ = "h1d_nHits"+mysubdetstr_+"M";
  sprintf (title, "# Hits, %sM, Run %d; # Hits", mysubdetstr_.c_str(), runnum_);
  titlestr = string(title);
  add1dHisto( st_nHitsMinus_, titlestr, 1301,-0.5, 1300.5, v_hpars1d);

  st_totalEplus_ = "h1d_totalE"+mysubdetstr_+"P";
  sprintf (title, "#Sigma_{hits}  E_{hit}, %sP, Run %d; #Sigma E (GeV)", mysubdetstr_.c_str(), runnum_);
  titlestr = string(title);
  add1dHisto( st_totalEplus_, titlestr, 100,0.,1000., v_hpars1d);

  st_totalEminus_ = "h1d_totalE"+mysubdetstr_+"M";
  sprintf (title, "#Sigma_{hits}  E_{hit}, %sM, Run %d; #Sigma E (GeV)", mysubdetstr_.c_str(), runnum_);
  titlestr = string(title);
  add1dHisto( st_totalEminus_, titlestr, 100,0.,1000., v_hpars1d);

  myAH->book1d<TH1F>       (v_hpars1d);
  myAH->book1d<TH1F>       (v_hpars1d_af);
  myAH->book2d<TH2F>       (v_hpars2d);
  myAH->book2d<TH2F>       (v_hpars2d_af);
  myAH->book2d<TProfile2D> (v_hpars2dprof);
  myAH->book3d<TH3F>       (v_hpars3d);

  if (mysubdet_==HcalForward) {
    st_rhDeltaTdepthsVsEtaEnHad_ = "p2d_rhDeltaTdepthsVsEtaEnHad" + mysubdetstr_;
    st_rhDeltaTdepthsVsEtaChHad_ = "p2d_rhDeltaTdepthsVsEtaChHad" + mysubdetstr_;
    st_rhDeltaTdepthsVsEtaEnEM_  = "p2d_rhDeltaTdepthsVsEtaEnEM"  + mysubdetstr_;
    st_rhDeltaTdepthsVsEtaChEM_  = "p2d_rhDeltaTdepthsVsEtaChEM"  + mysubdetstr_;

    titlestr  = "T_{d2}-T_{d1} Vs. #eta, Energy, E_{L}/2<E_{S}<2E_{L}";
    titlestr += ", "+rundescr_+"; i#eta; Average over Depths E_{hit} (GeV)";
    myAH->book2d<TProfile2D>(st_rhDeltaTdepthsVsEtaEnHad_,titlestr.c_str(),
			     83,-41.5,41.5, nEbins, Ebins);

    titlestr  = "T_{d2}-T_{d1} Vs. #eta, Charge, E_{L}/2<E_{S}<2E_{L} ";
    titlestr += ", "+rundescr_+"; i#eta; Average over Depths #Sigma_{i=2TS} fC_{i} (fC)";
    myAH->book2d<TProfile2D>(st_rhDeltaTdepthsVsEtaChHad_,titlestr.c_str(),
			     83,-41.5,41.5, nChbins, chbins);

    titlestr  = "T_{d2}-T_{d1} Vs. #eta, Energy, E_{L} > 2E_{S}";
    titlestr += ", "+rundescr_+"; i#eta; Average over Depths E_{hit} (GeV)";
    myAH->book2d<TProfile2D>(st_rhDeltaTdepthsVsEtaEnEM_,titlestr.c_str(),
			     83,-41.5,41.5, nEbins, Ebins);

    titlestr  = "T_{d2}-T_{d1} Vs. #eta, Charge, E_{L} > 2E_{S}";
    titlestr += ", "+rundescr_+"; i#eta; Average over Depths #Sigma_{i=2TS} fC_{i} (fC)";
    myAH->book2d<TProfile2D>(st_rhDeltaTdepthsVsEtaChEM_,titlestr.c_str(),
			     83,-41.5,41.5, nChbins, chbins);
  }

  if (cut.doInverted()) {
    myAnalHistos *myAH = cut.invhistos();
    myAH->book1d<TH1F>       (v_hpars1d);
    myAH->book1d<TH1F>       (v_hpars1d_af);
    myAH->book2d<TH2F>       (v_hpars2d);
    myAH->book2d<TH2F>       (v_hpars2d_af);
    myAH->book2d<TProfile2D> (v_hpars2dprof);
    myAH->book3d<TH3F>       (v_hpars3d);

    if (mysubdet_==HcalForward) {
      titlestr  = "T_{d2}-T_{d1} Vs. #eta, Energy, E_{L}/2<E_{S}<2E_{L}";
      titlestr += ", "+rundescr_+"; i#eta; Average over Depths E_{hit} (GeV)";
      myAH->book2d<TProfile2D>(st_rhDeltaTdepthsVsEtaEnHad_,titlestr.c_str(),
			       83,-41.5,41.5, nEbins, Ebins);

      titlestr  = "T_{d2}-T_{d1} Vs. #eta, Charge, E_{L}/2<E_{S}<2E_{L} ";
      titlestr += ", "+rundescr_+"; i#eta; Average over Depths #Sigma_{i=2TS} fC_{i} (fC)";
      myAH->book2d<TProfile2D>(st_rhDeltaTdepthsVsEtaChHad_,titlestr.c_str(),
			       83,-41.5,41.5, nChbins, chbins);

      titlestr  = "T_{d2}-T_{d1} Vs. #eta, Energy, E_{L} > 2E_{S}";
      titlestr += ", "+rundescr_+"; i#eta; Average over Depths E_{hit} (GeV)";
      myAH->book2d<TProfile2D>(st_rhDeltaTdepthsVsEtaEnEM_,titlestr.c_str(),
			       83,-41.5,41.5, nEbins, Ebins);

      titlestr  = "T_{d2}-T_{d1} Vs. #eta, Charge, E_{L} > 2E_{S}";
      titlestr += ", "+rundescr_+"; i#eta; Average over Depths #Sigma_{i=2TS} fC_{i} (fC)";
      myAH->book2d<TProfile2D>(st_rhDeltaTdepthsVsEtaChEM_,titlestr.c_str(),
			       83,-41.5,41.5, nChbins, chbins);
    }
  }
}                       // BeamTimingAnalAlgos::bookDetailHistos4cut

//==================================================================

void
BeamTimingAnalAlgos::fillHistos4cut(myAnalCut& thisCut)
{
  HcalTimingAnalAlgos::fillHistos4cut(thisCut);

  myAnalHistos *myAH;
  if (thisCut.isActive()) {
    if (thisCut.doInverted()) myAH = thisCut.invhistos();
    else                      return;
  } else
    myAH = thisCut.cuthistos();

  if (thisCut.flagSet(st_fillDetail_)) {
    myAH->fill3d<TH3F>("h3d_NhitsPerIetaIphiEn"+mysubdetstr_,fieta_,fiphi_,hitenergy_);
    if (hitenergy_>50.) myAH->fill2d<TH2F>("h2d_NhitsPerIetaIphiOver50" + mysubdetstr_,fieta_,fiphi_);

    if (mysubdet_ == HcalForward) {
#if 0
    if (hitenergy_ <= 10.0) {
      myAH->fill2d<TProfile2D>((detID_.depth()==1)?
			       st_lowEtimingMapD1_:
			       st_lowEtimingMapD2_,
			       detID_.ieta(),
			       detID_.iphi(),
			       corTime_);
    }
    if (corTime_ > 12.0) {
      myAH->fill2d<TProfile2D>((detID_.depth()==1)?
			       st_lateHitsTimeMapD1_:
			       st_lateHitsTimeMapD2_,
			       detID_.ieta(),
			       detID_.iphi(),
			       corTime_);
    }
#endif
#if 0
    // per-channel beam-specific histos
    if (thisCut.flagSet(st_doPerChannel_)) {
      uint32_t dix;
      stringstream title;
      string name;

      if (mysubdet_ == HcalOther) {
	dix = zdcDetID_.denseIndex(); title << zdcDetID_;
      } else {
	dix = detID_.denseIndex(); title << detID_;
      }

      perChanHistos *twoTSratVsEperIDhos =
	myAH->getAttachedHisto<uint32_t>("_2TSratioVsEperID");

      name = title.str();
      title << "; MaxTS+2TS Ratio (0.5 = TS3/4 boundary); T_{2TS} (ns)";

      perChanHistos::HistoParams_t
	hpars(name,title.str(),
	      recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	      200,-1.0,3.0);

      TH2F *h2tsVsE = twoTSratVsEperIDhos->exists(dix) ?
	twoTSratVsEperIDhos->get<TH2F>(dix) : 
	twoTSratVsEperIDhos->book2d<TH2F>(dix,hpars,false);
      
      if (h2tsVsE) h2tsVsE->Fill(hitenergy_,twoTSratio_);
    } // if doPerChannel
#endif
    } // if HF
  }// if Filldetail
}                             // BeamTimingAnalAlgos::fillHistos4cut

//==================================================================
// specifically for HF: - handling both depths from a tower simultaneously

void
BeamTimingAnalAlgos::fillHFD1D2histos(myAnalHistos *myAH,
				      const HFRecHit& tgt, float tgtTime,
				      const HFRecHit& prt, float prtTime)
{
  assert ((tgt.id().ieta()  == prt.id().ieta()) &&
	  (tgt.id().iphi()  == prt.id().iphi()) &&
	  (tgt.id().depth() != prt.id().depth())    );

  int    zside =  tgt.id().zside();
  int  absieta =  tgt.id().ietaAbs();
  bool tgtisd1 = (tgt.id().depth() == 1);

  const HFRecHit& rhd1 = (tgtisd1) ? tgt : prt;
  const HFRecHit& rhd2 = (tgtisd1) ? prt : tgt;

  float L=rhd1.energy();
  float S=rhd2.energy();
  float LfC=(tgtisd1) ? fCamplitude_ : partnerfCamplitude_;
  float SfC=(tgtisd1) ? partnerfCamplitude_ : fCamplitude_;

  int ntwr = 18*2;  // = 18 iphi towers in ieta=40,41 * 2 sides
  if (absieta < 40)  { ntwr *= 2; }
  double weight = 1.0/(double)ntwr;

  float R =calcR (L,S);
  float R2=calcR2(L,S);

#if 0
  if (tgtisd1)  myAH->fill3d<TH3F>(st_RvsTandfCHFd1_,tgtTime,R,LfC);
  else          myAH->fill3d<TH3F>(st_RvsTandfCHFd2_,tgtTime,R,SfC);
#endif

  // However, histos that require both hits simultaneously get filled when
  // depth 1 hit is "the target"; don't double-count when depth 2 comes around!
  //
  if (tgtisd1) {
    myAH->fill2d<TH2F>(st_RvsEtwr_,  R, (L+S),weight/13.);
    myAH->fill2d<TH2F>(st_R2vsEtwr_, R2,(L+S),weight/13.);
    myAH->fill2d<TH2F>(st_RvsIeta_,  R, absieta,weight);
    myAH->fill2d<TH2F>(st_R2vsIeta_, R2,absieta,weight);
    myAH->fill2d<TH2F>(st_LvsSHF_,L,S);

    if ((L>0) && (S>0)) {
      if ((S > L/2.f) && (S < 2.f*L)) {
	myAH->fill2d<TProfile2D>(st_rhDeltaTdepthsVsEtaEnHad_,
				 fieta_,(L+S)/2.f,prtTime-tgtTime);
	myAH->fill2d<TProfile2D>(st_rhDeltaTdepthsVsEtaChHad_,
				 fieta_,(LfC+SfC)/2.f,prtTime-tgtTime);
      } else if (S < L/2.f) {
	myAH->fill2d<TProfile2D>(st_rhDeltaTdepthsVsEtaEnEM_,
				 fieta_,(L+S)/2.f,prtTime-tgtTime);
	myAH->fill2d<TProfile2D>(st_rhDeltaTdepthsVsEtaChEM_,
				 fieta_,(LfC+SfC)/2.f,prtTime-tgtTime);
      }
      myAH->fill2d<TH2F>(((zside > 0) ?
			  st_rhCorTimesD1vsD2plus_ :
			  st_rhCorTimesD1vsD2minus_),
			 tgtTime,prtTime);

      myAH->fill2d<TProfile2D>(st_deltaTvsLandSenHF_,L,  S,  prtTime-tgtTime);
      myAH->fill2d<TProfile2D>(st_deltaTvsLandSchHF_,LfC,SfC,prtTime-tgtTime);

    } // else is unmatched hit

    // occupancy plot:
    // Increment *all* bins with energy <= hit energy at the given ieta
    //
    TH2F *h2Docc = myAH->get<TH2F>(st_OccVsEtaEnergyBothOverThresh_.c_str());
    for (int ibin=1;ibin<h2Docc->GetNbinsY(); ibin++) {
      double binmin = h2Docc->GetYaxis()->GetBinLowEdge(ibin);
      double binctr = h2Docc->GetYaxis()->GetBinCenter(ibin);
      int nch = 18*2;  // = 18 iphi towers in ieta=40,41 * 2 sides
      if (absieta  < 40) { nch = 36*2; }
      if (absieta  < 29) { nch = 36*3; }
      if (absieta  < 26) { nch = 36*2; }
      if (absieta  < 21) { nch = 72*2; }
      if (absieta == 17) { nch = 72*1; }
      if (absieta == 16) { nch = 72*3; }
      if (absieta == 15) { nch = 72*2; }
      if (absieta  < 15) { nch = 72*1; }
      
      double weight = 1.0/(double)nch;
      if ((L >= binmin) && (S >= binmin))
	h2Docc->Fill(absieta, binctr, weight);
    }
  }
}                           // BeamTimingAnalAlgos::fillHFD1D2histos

//==================================================================
//PMT hits in HF require additional simultaneous handling of depths 1
//and 2 hits.  Only a small portion of hits (<~5%) in the collection
//are paired due to zero suppression; need both to fill d1 vs d2
//timing histos.
//
void BeamTimingAnalAlgos::findConfirmedHits(
   const edm::Handle<HFRecHitCollection>& rechithandle,
   const edm::Handle<HFDigiCollection>&   digihandle)
{
  const HFRecHitCollection& rechits = *rechithandle;
  const HFDigiCollection&   digis   = *digihandle;

  m_confirmedHits_.clear();
  int npairs = 0;

  HFRecHitIt hitit1, hitit2;
  HFDigiIt   digit1, digit2;

  for (hitit1  = rechits.begin(), digit1  = digis.begin();
       hitit1 != rechits.end() && digit1 != digis.end();
       hitit1++,digit1++) {
    while ((hitit1->id() != digit1->id()) &&
	   (hitit1 != rechits.end()) &&
	   (digit1 != digis.end())) {
      while (hitit1->id() < digit1->id()) hitit1++;
      while (digit1->id() < hitit1->id()) digit1++;
    }
    if ((hitit1 == rechits.end()) || (digit1 == digis.end())) break;

    if (inSet<int>(detIds2mask_,hitit1->id().hashed_index())) continue;
    if (hitit1->id().depth()==1) {
      HcalDetId partnerId(HcalForward,hitit1->id().ieta(),hitit1->id().iphi(),2);
      hitit2 = rechits.find(partnerId);
      digit2 =   digis.find(partnerId);

      std::pair<HFRecHitIt,HFDigiIt> paird1(hitit1,digit1); // do sanity checks on the other side
      std::pair<HFRecHitIt,HFDigiIt> paird2(hitit2,digit2); // do sanity checks on the other side

      if (hitit2 != rechits.end()) {
	if (inSet<int>(detIds2mask_,hitit2->id().hashed_index())) continue; // treat them like ZS hits
	npairs++;
	m_confirmedHits_.insert
	  (std::pair<uint32_t,std::pair<HFRecHitIt,HFDigiIt> >(hitit1->id().denseIndex(),paird2));
	m_confirmedHits_.insert
	  (std::pair<uint32_t,std::pair<HFRecHitIt,HFDigiIt> >(hitit2->id().denseIndex(),paird1));
      } // if there's a depth 2 hit
    } // if the hit is depth 1

  } // loop over rechits

  //cout<<"npairs = "<<npairs<<" = "<<100.*npairs/rechits.size()<<"%"<<endl;

}                          // BeamTimingAnalAlgos::findConfirmedHits

//==================================================================
// Following routine receives digi in ADC (df)
// and sets member variables digifC_ and digiGeV_ as output.
//
template<class Digi>
void
BeamTimingAnalAlgos::processZDCDigi(const Digi& df)
{
  CaloSamples dfC; // dfC is the linearized (fC) digi

  const HcalCalibrations& calibs = conditions_->getHcalCalibrations(df.id());
  const HcalQIECoder *qieCoder   = conditions_->getHcalCoder( df.id() );
  const HcalQIEShape *qieShape   = conditions_->getHcalShape();
  HcalCoderDb coder( *qieCoder, *qieShape );
  coder.adc2fC( df, dfC );
  digiGeV_ = dfC;

  double prenoise = 0; double postnoise = 0; 
  int noiseslices = 0;
  double noise = 0;
 
  for(int k = 0 ; k < dfC.size() && k < firstsamp_; k++){
    prenoise += dfC[k];
    noiseslices++;
  }
  for(int j = (nsamps_ + firstsamp_ + 1); j <dfC.size(); j++){
    postnoise += dfC[j];
    noiseslices++;
  }
     
  if(noiseslices != 0)
    noise = (prenoise+postnoise)/float(noiseslices);
  else
    noise = 0;

  for (int i=0; i<dfC.size(); i++) {
    int capid=df[i].capid();
    digiGeV_[i] = (dfC[i]-noise); // pickup noise subtraction
    digiGeV_[i]*= calibs.respcorrgain(capid) ;    // fC --> GeV
  }
  digifC_ = dfC;
}                             // BeamTimingAnalAlgos::processZDCDigi

//==================================================================
// Following routine receives digi in ADC (df) and
// duplicates hit time reconstruction algo
//
template<class Digi>
int
BeamTimingAnalAlgos::processDigi(const Digi& df,
				 CaloSamples& digifC,
				 CaloSamples& digiGeV,
				 float& ratio2ts,
				 float& fCamplitude)
{
  int   maxts = -1;
  float maxta = -9e99;

  CaloSamples dfC; // dfC is the linearized (fC) digi

  const HcalCalibrations& calibs = conditions_->getHcalCalibrations(df.id());
  const HcalQIECoder *qieCoder   = conditions_->getHcalCoder( df.id() );
  const HcalQIEShape *qieShape   = conditions_->getHcalShape();
  HcalCoderDb coder( *qieCoder, *qieShape );
  coder.adc2fC( df, dfC );
  digiGeV = dfC;
  for (int i=0; i<dfC.size(); i++) {
    int capid  = df[i].capid();
    dfC[i]    -= calibs.pedestal(capid);         // pedestal subtraction
    digiGeV[i] = dfC[i]*calibs.respcorrgain(capid) ;    // fC --> GeV
    // Find and return maximum samples within the reco window.
    if ((i>=firstsamp_) &&
	(i<firstsamp_+nsamps_) &&
	(digiGeV[i] > maxta)) {
      maxta = digiGeV[i];
      maxts = i;
    }
  }

  digifC = dfC;

  float t0   = (maxts>0)                      ? digiGeV[maxts-1] : 0.0;
  float t2   = (maxts<(int)digiGeV_.size()-2) ? digiGeV[maxts+1] : 0.0;
  float t0fC = (maxts>0)                      ? digifC [maxts-1] : 0.0;
  float t1fC =                                  digifC [maxts];
  float t2fC = (maxts<(int)digiGeV_.size()-2) ? digifC [maxts+1] : 0.0;
  float zerocorr=std::min(t0,t2);
  if (zerocorr<0.f) {
    t0    -= zerocorr;
    t2    -= zerocorr;
    maxta -= zerocorr;
  }

  // pair with the larger of the two neighboring time samples
  float wpksamp=0.f;
  if (t0>t2) {
    wpksamp = t0+maxta;
    if (wpksamp != 0.f) wpksamp = maxta/wpksamp;
    fCamplitude = t0fC+t1fC;
  } else {
    // this would represent an *earlier* phase in the LUT than above,
    // so must increment the peak sample by one to get the time right
    wpksamp = maxta+t2;
    if (wpksamp != 0.f) wpksamp = 1.+(t2/wpksamp);
    fCamplitude = t2fC+t1fC;
  }

  // for comparison
  float                wpksamp3ts =  t0fC+t1fC+t2fC;
  if (wpksamp3ts != 0) wpksamp3ts = (t1fC+2*t2fC)/wpksamp3ts;

  ratio2ts   = maxts - dfC.presamples() + wpksamp;
  //ratio2ts   = (fCamplitude > 0.0f) ? ts4/fCamplitude : -1.0;

  treco3ts_  = -15.0 + (maxts - dfC.presamples() + wpksamp3ts-0.5)*25.0f;

#if 0
  for (int its=0; its<digisize; ++its)
    printf("%5.1f\t",digiGeV_[its]);
  printf ("ratio2ts=%5.3f\n",ratio2ts_);
#endif

  return maxts;
}                                // BeamTimingAnalAlgos::processDigi


//==================================================================

void
BeamTimingAnalAlgos::fillPerEvent(void)
{
  myAnalHistos *firstAH = getCut(st_cutNone_)->cuthistos();
  myAnalHistos *lastAH  = getCut(st_lastCut_)->cuthistos();

  logLSBX(st_cutNone_);
  if (badEventSet_.empty() ||
      notInSet<int>(badEventSet_,evtnum_))  {
    if (!badEventSet_.empty())               logLSBX(st_cutBadEv_);
    if (acceptedBxNums_.empty() ||
	inSet<int>(acceptedBxNums_,bxnum_)) { 
      if (!acceptedBxNums_.empty())          logLSBX(st_cutBadBx_);
    } else return;
  } else return;


  firstAH->fill1d<TH1F>(st_totalEperEv_,fevtnum_,totalE_);

  if ((totalEminus_ > 10.0) && (totalEplus_ > 10.0)) {
    lastAH->fill2d<TProfile2D>(st_rhCorTimesPlusVsMinus_,avgTminus_,avgTplus_);
    lastAH->fill1d<TH1F>(st_nHitsPlus_, nhitsplus_);
    lastAH->fill1d<TH1F>(st_nHitsMinus_,nhitsminus_);
    lastAH->fill1d<TH1F>(st_totalEplus_, totalEplus_);
    lastAH->fill1d<TH1F>(st_totalEminus_,totalEminus_);
    lastAH->fill2d<TProfile2D>(st_rhDeltaTvsVertexCor_,correction_ns_,avgTplus_-avgTminus_);

    if (avgTminus_>50)
cerr<<"avgTminus="<<avgTminus_<<", nhitsminus="<<nhitsminus_<<", totalEminus="<<totalEminus_<<endl;
    if (avgTplus_ >50)
cerr<<"avgTplus=" <<avgTplus_ <<", nhitsplus=" <<nhitsplus_ <<", totalEplus=" <<totalEplus_<<endl;
  }

}                               // BeamTimingAnalAlgos::fillPerEvent

//==================================================================

void BeamTimingAnalAlgos::logLSBX(const std::string& cutstr)
{
  myAnalHistos *myAH = getCut(cutstr)->cuthistos();

  int lsnum500 = (lsnum_/500) * 500;
  stringstream range;
  range << lsnum500 << "-" << lsnum500+499;
  std::string hlsnumname = "h1d_lsnum"+range.str();
  TH1F  *lsH = myAH->get<TH1F>(hlsnumname);

  if (!lsH) {
    string title = "Lumi Section Occupancy, LS="+range.str();
    title += ", "+mysubdetstr_+", "+rundescr_+"; LS Number";
    myAnalHistos::HistoParams_t hpars1d;
    hpars1d.name   = hlsnumname;
    hpars1d.title  = title;
    hpars1d.nbinsx = 501;
    hpars1d.minx   = (float)lsnum500;
    hpars1d.maxx   = lsnum500+500.f;
    hpars1d.nbinsy = 0;
    hpars1d.nbinsz = 0;
    myAH->book1d<TH1F>(hpars1d);
    lsH = myAH->get<TH1F>(hlsnumname);
  }

  if (lsH) lsH->Fill(lsnum_);
  myAH->fill1d<TH1F>(st_bxnum_,bxnum_);
}                                    // BeamTimingAnalAlgos::logLSBX

//==================================================================

void
BeamTimingAnalAlgos::processHFconfirmedHits(const HFRecHit& targethit,
					    const pair<HFRecHitIt,HFDigiIt>& partner)
{
  const HFRecHit& partnerhit = *(partner.first);

  assert ((targethit.id().ieta()  == partnerhit.id().ieta()) &&
	  (targethit.id().iphi()  == partnerhit.id().iphi()) &&
	  (targethit.id().depth() != partnerhit.id().depth()) );

  partnerfCamplitude_ = 0.0f;
  float partner2TSratio;

  //if (partner.second != NULL)
  {
    const HFDataFrame& partnerdigi = *(partner.second);
    assert (partnerdigi.id() == partnerhit.id());
    processDigi<HFDataFrame>(partnerdigi,partnerdigifC_,partnerdigiGeV_,
			     partner2TSratio,partnerfCamplitude_);
  }

  float amplitude = (ampCutsInfC_) ? partnerfCamplitude_ : partnerhit.energy();

  myAnalCut *cut;

  getCut(st_PMThits_)    ->Activate(true);
  getCut(st_PMTpartners_)->Activate(true);
  getCut(st_goodHits_)   ->Activate(true);
  getCut(st_dubiousHits_)->Activate(true);

  // Only fill the histos for the target rechit, we'll come back 'round
  // for the partner hit eventually
  //
  if      (isHFPMThit(targethit))           cut=getCut(st_PMThits_);
  else if (isHFPMThit(partnerhit))          cut=getCut(st_PMTpartners_);
  else if ((amplitude<minHitAmplitude_) ||
	   (amplitude>maxHitAmplitude_)   ) cut=getCut(st_dubiousHits_);
  else                                      cut=getCut(st_goodHits_);

  cut->Activate(false);

  fillHistos4cut(*cut);

  // histos comparing short/long depths
  //
  float corTime1= corTime_;
  float corTime2= partnerhit.time() - timecor_->correctTime4(partnerhit.id());

  // external hit corrections to apply to hits for these det IDs
  
  TimesPerDetId::const_iterator it;
  it=exthitcors_.find(detID_);          if (it!=exthitcors_.end()) corTime1-=it->second;
  it=exthitcors_.find(partnerhit.id()); if (it!=exthitcors_.end()) corTime2-=it->second;
  
  fillHFD1D2histos(cut->cuthistos(),
		   targethit,  corTime1,
		   partnerhit, corTime2);

}                     // BeamTimingAnalAlgos::processHFconfirmedHits

//==================================================================

void
BeamTimingAnalAlgos::processHFunconfirmedHit(const HFRecHit& hfrh)
{
  myAnalCut *cut;

  if (isHFPMThit(hfrh)) cut = getCut(st_PMThits_);
  else                  cut = getCut(st_dubiousHits_);

  fillHistos4cut(*cut);
  myAnalHistos *myAH = cut->cuthistos();

  int curdepth   = detID_.depth();
  if (curdepth==1) {
    HcalDetId zsedid(HcalForward,detID_.ieta(),detID_.iphi(),2);
    HFRecHit  zsedht(zsedid,0.0,0.0);
    fillHFD1D2histos(myAH,hfrh, corTime_,zsedht, 0.0);
  }
  else { // if (curdepth==2)
    HcalDetId zsedid(HcalForward,detID_.ieta(),detID_.iphi(),1);
    HFRecHit  zsedht(zsedid,0.0,0.0);
    fillHFD1D2histos(myAH,zsedht,0.0,hfrh, corTime_);
  }
}                    // BeamTimingAnalAlgos::processHFunconfirmedHit

//==================================================================

void
BeamTimingAnalAlgos::processHFtimeRegions(const HFRecHit& hfrh)
{
  myAnalCut *cut;

  if        ((hittime_ >= -22.) && (hittime_ < -10.)) { cut = getCut(st_region1_);
  } else if ((hittime_ >= -10.) && (hittime_ <   6.)) { cut = getCut(st_region2_);
  } else if ((hittime_ >=   6.) && (hittime_ <  15.)) { cut = getCut(st_region3_);
  } else if ((hittime_ >=  15.) && (hittime_ <  42.)) { cut = getCut(st_region4_);
  } else if ((hittime_ >=  42.) && (hittime_ <  60.)) { cut = getCut(st_region5_);
  } else return;

  cut->Activate(false);
  fillHistos4cut(*cut);
}                       // BeamTimingAnalAlgos::processHFtimeRegions

//==================================================================

template<class Digi,class RecHit>
void BeamTimingAnalAlgos::processDigisAndRecHits
  (const edm::Handle<edm::SortedCollection<Digi>   >& digihandle,
   const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle)
{
  const edm::SortedCollection<RecHit>& rechits = *rechithandle;

  doDigis_ = digihandle.isValid();

  myAnalHistos *firstAH = getCut(st_cutNone_)->cuthistos();

  firstAH->fill1d<TH1F>(st_rhColSize_,rechits.size());

  if (doDigis_)
    firstAH->fill1d<TH1F>(st_digiColSize_,digihandle->size());

  totalE_ = totalEminus_ = totalEplus_ = 0.0;

  float weightedTplus = 0.0;
  float weightedTminus = 0.0;

  nhitsplus_=nhitsminus_=0;

  unsigned idig=0;
  for (unsigned irh=0; irh < rechits.size(); ++irh, idig++) {

    const RecHit& rh  = rechits[irh];

    if (rh.id().det() == DetId::Hcal) {
      detID_ = HcalDetId(rh.id());
      feID_  = lmap_->getHcalFrontEndId(detID_);
      if (detID_.subdet() != mysubdet_)	continue; // HB and HE handled by separate instances of this class!
      if (inSet<int>(detIds2mask_,detID_.hashed_index())) continue;
      correction_ns_ = timecor_->correctTime4(detID_);
    }
    else if ((rh.id().det() == DetId::Calo) && 
	     (rh.id().subdetId() == 2)) { // ZDC
      zdcDetID_ = HcalZDCDetId(rh.id());
      correction_ns_ = timecor_->correctTime4(zdcDetID_);
    }

    hittime_   = rh.time() - globalToffset_;
    hitenergy_ = rh.energy();
    hitflags_  = rh.flags();

    if (hitenergy_ > recHitEscaleMaxGeV_)
      cout << rh << endl;

    int  zside = detID_.zside();

    //corTime   = hittime_ - correction_ns_; not yet...
    corTime_   = hittime_;

    TimesPerDetId::const_iterator it = exthitcors_.find(detID_);
    if (it != exthitcors_.end()) {
      // external hit correction to apply to hits for this det ID
      corTime_ -= it->second;
    }

    totalE_ += hitenergy_;

    // Fraction2TS (6 bits):
    fraction2ts_ = (float)((hitflags_>>((int)HcalCaloFlagLabels::Fraction2TS))&0x3f);

    if ((mysubdet_ == HcalBarrel) || 
	(mysubdet_ == HcalEndcap)   )   fraction2ts_ = 0.5f + ((fraction2ts_-1.f)/100.f);
    else if (mysubdet_ == HcalForward)  fraction2ts_ = (fraction2ts_-1.f)/50.f;

    fCamplitude_=-9.99e9f;

    // If we have digis, do them too.
    //
    if (digihandle.isValid()) {
      while (idig < digihandle->size()) {
	const Digi&  df = (*digihandle)[idig];
	if (df.id() != rh.id()) {
	  //cerr << "No rechit for digi " << df.id() << ", skipping." << endl;
	  idig++;
	} else {
	  if ((rh.id().det() == DetId::Calo) && 
	      (rh.id().subdetId() == 2)) { // ZDC
	    processZDCDigi<Digi>(df);
	  } else {
	    maxts_ = processDigi<Digi>(df,digifC_,digiGeV_,
				       twoTSratio_,fCamplitude_);
	    tLUTminust3TS_ = corTime_-treco3ts_;
	  }
	  break;
	}
      } // while

      if (ampCutsInfC_ && idig>=digihandle->size()) {
	cerr << "No digi for rechit " << rh.id() << ", skipping." << endl;
	continue;
      }
    }

    //==================================================
    // DETERMINE STATUS OF CUTS FROM CALCULATED DATA
    //==================================================

    getCut(st_cutNone_)      ->Activate (false);

    if (!badEventSet_.empty())
    getCut(st_cutBadEv_)     ->Activate (inSet<int>(badEventSet_,evtnum_));

    if (!acceptedBxNums_.empty())
    getCut(st_cutBadBx_)     ->Activate (notInSet<int>(acceptedBxNums_,bxnum_));

    if (globalFlagMask_)
    getCut(st_cutBadFlags_)  ->Activate (hitflags_ & globalFlagMask_);

    if (ampCutsInfC_)
    getCut(st_cutHitEwindow_)->Activate ((fCamplitude_ < minHitAmplitude_) ||
					 (fCamplitude_ > maxHitAmplitude_)   );
    else
    getCut(st_cutHitEwindow_)->Activate ((hitenergy_ < minHitAmplitude_) ||
					 (hitenergy_ > maxHitAmplitude_)   );

    //if (!acceptedPkTSnums_.empty())
    if	((mysubdet_ == HcalBarrel) ||
	 (mysubdet_ == HcalEndcap)   ) {
      if (hbheOutOfTimeFlagBit_>=0) 
	getCut(st_cutOutOfTime_) ->Activate (hitflags_ & (1<<hbheOutOfTimeFlagBit_));

      getCut(st_fraction2ts_) ->Activate (fraction2ts_ == 0.5);
    }

    //==================================================
    // CUTS ARE DETERMINED, NOW FILL THE HISTOGRAMS
    //==================================================

    bool totalcut=false;
    for (size_t i=0; !totalcut && i<v_nestedCuts_.size(); i++) {
      myAnalCut *thiscut = getCut(v_nestedCuts_[i]);
      fillHistos4cut(*thiscut);
      totalcut = totalcut || thiscut->isActive();
    }

    if (!totalcut) {
      if ((mysubdet_ == HcalBarrel) ||
	  (mysubdet_ == HcalEndcap)   ) {
	fillHistos4cut(*getCut(st_cutOutOfTime_));
	fillHistos4cut(*getCut(st_fraction2ts_));

      } else if (mysubdet_ == HcalForward) {
	map<uint32_t,pair<HFRecHitIt,HFDigiIt> >::const_iterator it =
	  m_confirmedHits_.find(detID_.denseIndex());

	const HFRecHit& hfrh = *(HFRecHit *)&rh;

	if (it != m_confirmedHits_.end()) {
	  const HFRecHit& target  = hfrh;
	  const pair<HFRecHitIt,HFDigiIt>& partner = it->second;

	  processHFconfirmedHits(target,partner);
	}
	else { // is unconfirmed hit
	  processHFunconfirmedHit(hfrh);
	}

	processHFtimeRegions(hfrh);
      }

      // for comparison of +/- timing
      if (zside > 0) {
	totalEplus_  += hitenergy_; weightedTplus  += hitenergy_*corTime_; nhitsplus_++;
      } else {
	totalEminus_ += hitenergy_; weightedTminus += hitenergy_*corTime_; nhitsminus_++;
      }
    }

    if (doTree_) tree_->Fill();

  } // loop over rechits

  // now that we have the total energy...
  // cout << evtnum_ << "\t" << totalE_ << endl;
  avgTminus_ = weightedTminus/totalEminus_;
  avgTplus_  = weightedTplus/totalEplus_;

  fillPerEvent();
}                         // BeamTimingAnalAlgos::processDigisAndRecHits

//======================================================================

// ------------ method called to for each event  ------------
void
BeamTimingAnalAlgos::process(const myEventData& ed)
{
  HcalTimingAnalAlgos::process(ed);

  timecor_->init(ed.vertices());

  switch (mysubdet_) {
  case HcalBarrel:
  case HcalEndcap:  processDigisAndRecHits<HBHEDataFrame,HBHERecHit>(ed.hbhedigis(),ed.hbherechits()); break;
  case HcalOuter:   processDigisAndRecHits<HODataFrame,    HORecHit>(ed.hodigis(),  ed.horechits());   break;
  case HcalOther:   processDigisAndRecHits<ZDCDataFrame,  ZDCRecHit>(ed.zdcdigis(), ed.zdcrechits());  break;
  case HcalForward:
    findConfirmedHits(ed.hfrechits(),ed.hfdigis());
    processDigisAndRecHits<HFDataFrame,HFRecHit>(ed.hfdigis(), ed.hfrechits());
    break;
  default: break;
  }
  neventsProcessed_++;
}

//======================================================================

void
BeamTimingAnalAlgos::beginJob(const edm::EventSetup& iSetup,const myEventData& ed)
{
  HcalTimingAnalAlgos::beginJob(iSetup,ed);

  std::cout << "----------------------------------------"  << "\n";
  std::cout << "Cuts being applied for " << mysubdetstr_   << ":\n";
  for (unsigned i=0; i<v_nestedCuts_.size(); i++)
    std::cout << v_nestedCuts_[i] << "\t";
  for (unsigned i=0; i<v_hitCategories_.size(); i++)
    std::cout << v_hitCategories_[i] << "\t";
  std::cout << std::endl;

  if ((mysubdet_ == HcalForward) && !ed.hfdigis().isValid())
    throw cms::Exception("Must supply HF digis!!");
}
