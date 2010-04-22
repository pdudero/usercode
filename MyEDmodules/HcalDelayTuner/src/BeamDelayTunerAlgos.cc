
// -*- C++ -*-
//
// Package:    BeamDelayTunerAlgos
// Class:      BeamDelayTunerAlgos
// 
/**\class BeamDelayTunerAlgos BeamDelayTunerAlgos.cc MyEDmodules/HcalDelayTuner/src/BeamDelayTunerAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: BeamDelayTunerAlgos.cc,v 1.14 2010/04/12 10:22:27 dudero Exp $
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
#include "MyEDmodules/HcalDelayTuner/interface/BeamDelayTunerAlgos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"


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

std::string
BeamDelayTunerAlgos::addCut(const std::string& descr,
			    bool doInverted)
{
  size_t N = v_nestedCuts_.size();
  string cutstr = "cut" + int2str(N) + descr;
  v_nestedCuts_.push_back(cutstr);
  m_cuts_[cutstr] = new myAnalCut(N,cutstr,mysubdetstr_,doInverted);
  return cutstr;
}

std::string
BeamDelayTunerAlgos::addHitCategory(const std::string& descr)
{
  size_t N = v_nestedCuts_.size()+v_hitCategories_.size();
  string cutstr = "cut" + int2str(N) + descr;
  v_hitCategories_.push_back(cutstr);
  m_cuts_[cutstr] = new myAnalCut(N,cutstr,mysubdetstr_);
  return cutstr;
}

//
// constructors and destructor
//
BeamDelayTunerAlgos::BeamDelayTunerAlgos(const edm::ParameterSet& iConfig,
					 BeamHitTimeCorrector *timecor) :
  HcalDelayTunerAlgos(iConfig),
  timecor_(timecor)
{
  stringstream cutstr;

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

  //======================================================================
  // Initialize the cuts for the run and add them to the global map
  // Nested cut string vector initialized in order
  //======================================================================

  m_cuts_.clear();

  const bool doInverted = true;

  // all cuts applied on top of the previous one
  //
  st_cutNone_ = addCut("none");

  if (badEventVec.size())   st_cutBadEv_ = addCut("badEv",doInverted);
  if (acceptedBxVec.size()) st_cutBadBx_ = addCut("bxnum",doInverted);

  st_cutBadFlags_   = addCut("badFlags",doInverted);
  st_cutHitEwindow_ = addCut("hitEwindow",doInverted);

  if (!acceptedPkTSnums_.empty())
    st_cutOutOfTime_ = addCut("outOfTime",doInverted);

  if (mysubdet_ == HcalForward) {
    st_PMThits_  = addHitCategory("keepPMThits");
    getCut(st_PMThits_)->setFlag(st_fillDetail_);

    st_PMTpartners_  = addHitCategory("keepPMTpartners");
    getCut(st_PMTpartners_)->setFlag(st_fillDetail_);

    st_dubiousHits_ = addHitCategory("keepDubiousHits");
    getCut(st_dubiousHits_)->setFlag(st_fillDetail_);

    st_goodHits_    = addHitCategory("keepGoodHits");
    st_lastCut_     = st_goodHits_;
  } else {
    st_lastCut_ = v_nestedCuts_[v_nestedCuts_.size()-1];
  }

  getCut(st_lastCut_)->setFlag(st_fillDetail_);
  if (doPerChannel_)
    getCut(st_lastCut_)->setFlag(st_doPerChannel_);

}                        // BeamDelayTunerAlgos::BeamDelayTunerAlgos

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
  return queried.flagField(HcalCaloFlagLabels::UserDefinedBit0);

}                                                      // isHFPMThit

//==================================================================
void
BeamDelayTunerAlgos::bookHistos4allCuts(void)
{
  std::string titlestr;
  
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d;
  
  HcalDelayTunerAlgos::bookHistos4allCuts();

  if ((mysubdet_ == HcalForward) ||
      (mysubdet_ == HcalOther)    )  {
    titlestr       =
      "Vertex Z correction vs. Hit Time, Run "+runnumstr_+"; Hit Time (ns); Correction (ns)",
    st_TcorVsThit_ = "h2d_TcorVsThit";
    add2dHisto(st_TcorVsThit_, titlestr, 
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       81,-2.025,2.025, v_hpars2d);

    if (mysubdet_ == HcalForward) {
      titlestr    =
	"Hits/Tower vs. (L-S)/(L+S) & E_{twr}, Run "+runnumstr_+"; (L-S)/(L+S); E_{twr} (GeV)";
      st_RvsEtwr_ = "h2d_RvsEtwrHF";
      add2dHisto(st_RvsEtwr_, titlestr, 200,-1.0,1.0,
		 recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_, v_hpars2d);
    
      titlestr     = "Hits/Tower vs. S/(L+S) & E_{twr}, Run "+runnumstr_+"; S/(L+S); E_{twr} (GeV)";
      st_R2vsEtwr_ = "h2d_R2vsEtwrHF";
      add2dHisto(st_R2vsEtwr_, titlestr, 200,-1.0,1.0,
		 recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_, v_hpars2d);
    
      titlestr    = "Hits/Tower vs. (L-S)/(L+S) & i#eta, Run "+runnumstr_+"; (L-S)/(L+S); i#eta";
      st_RvsIeta_ = "h2d_RvsIetaHF";
      add2dHisto(st_RvsIeta_, titlestr, 200,-1.0,1.0, 13,28.5,41.5, v_hpars2d);
    
      titlestr     = "Hits/Tower vs. S/(L+S) & i#eta, Run "+runnumstr_+"; S/(L+S); i#eta";
      st_R2vsIeta_ = "h2d_R2vsIetaHF";
      add2dHisto(st_R2vsIeta_, titlestr, 200,-1.0,1.0, 13,28.5,41.5, v_hpars2d);
    }
    /*******************
     * BOOK 'EM, DANNO *
     *******************/
    std::map<std::string,myAnalCut *>::const_iterator cutit;
    for (cutit = m_cuts_.begin(); cutit != m_cuts_.end(); cutit++) {
      cutit->second->cuthistos()->book2d<TH2F>(v_hpars2d);
      if (cutit->second->doInverted())
	cutit->second->invhistos()->book2d<TH2F>(v_hpars2d);
    }
  }
}                         // BeamDelayTunerAlgos::bookHistos4allCuts

//==================================================================

void
BeamDelayTunerAlgos::bookDetailHistos4cut(myAnalCut& cut)
{
  assert(cut.flagSet(st_fillDetail_));

  myAnalHistos *myAH = cut.cuthistos();

  char title[128]; std::string titlestr;

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof;

  HcalDelayTunerAlgos::bookDetailHistos4cut(cut);

  // broken down by depth per Z-side
  if (mysubdet_ == HcalForward) {
    st_2TSratioVsEallHF_ = "h2d_2TSratioVsEallHF";
    titlestr = "2TS Ratio vs Energy, All HF, Run "+runnumstr_+";E (GeV); 2TS Ratio";
    add2dHisto(st_2TSratioVsEallHF_,titlestr,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       200,-1.0,3.0,v_hpars2d);

    st_2TSratioVsEallHFD1_ = "h2d_2TSratioVsEallHFD1";
    titlestr = "2TS Ratio vs Energy, HF Depth 1, Run "+runnumstr_+";E (GeV); 2TS Ratio";
    add2dHisto(st_2TSratioVsEallHFD1_,titlestr,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       200,-1.0,3.0,v_hpars2d);

    st_2TSratioVsEallHFD2_ = "h2d_2TSratioVsEallHFD2";
    titlestr = "2TS Ratio vs Energy, HF Depth 2, Run "+runnumstr_+";E (GeV); 2TS Ratio";
    add2dHisto(st_2TSratioVsEallHFD2_,titlestr,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       200,-1.0,3.0,v_hpars2d);

    // study 2TS algo performance
    //
    st_TcorVs2TSratioAllHF_ = "h2d_TcorVs2TSratioAllHF";
    titlestr = "T_{2ts} vs. 2TS Ratio, all HF, Run "+runnumstr_+"; 2TS Ratio; T_{2ts}";
    add2dHisto(st_TcorVs2TSratioAllHF_,titlestr,200,-1.0,3.0,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       v_hpars2d);

    // ...over energy too
    //
    st_TcorVs2TSratioAndEallHF_ = "h2d_TcorVs2TSratioAndEallHF";
    titlestr = "T_{2ts} Profile vs. 2TS Ratio and Energy, all HF, Multiple runs; 2TS Ratio; E (GeV)";
    add2dHisto(st_TcorVs2TSratioAndEallHF_,titlestr,200,-1.0,3.0,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       v_hpars2dprof);

    st_OccVsEtaEnergyBothOverThresh_ = "h2d_OccVsEtaEnergyBothOverThreshHF";
    titlestr  = "Nhits/tower Vs i#eta & E_{hit,min}, L,S>E_{hit,min}, HF";
    titlestr += ", Run "+runnumstr_+"; |i#eta|; E_{hit,min} (GeV)";
    
    add2dHisto(st_OccVsEtaEnergyBothOverThresh_, titlestr, 41, 0.5,  41.5,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       v_hpars2d);
              
    st_LvsSHF_ = "h2d_LvsSHF";
    add2dHisto(st_LvsSHF_, "Short E Vs. Long E, HF; E_{LONG} (GeV); E_{SHORT} (GeV)",
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       v_hpars2d);

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

    st_rhDeltaTdepthsVsEtaEnergy_ = "p2d_rhDeltaTdepthsVsEtaEnergy" + mysubdetstr_;
    titlestr = "T_{d2}-T_{d1} Vs. #eta and Energy, "+mysubdetstr_+", Run "+runnumstr_+"; i#eta; Hit Energy (GeV)";
    add2dHisto(st_rhDeltaTdepthsVsEtaEnergy_,titlestr,
	       83, -41.5,  41.5, recHitEscaleNbins_, recHitEscaleMinGeV_,recHitEscaleMaxGeV_,v_hpars2dprof);

    st_lowEtimingMapD1_     = "p2d_lowEtimingMapD1";
    titlestr = "Hit Time Map (5GeV< E_{hit}< 10GeV) for Depth 1, Run "+runnumstr_+"; i#eta; i#phi";
    add2dHisto(st_lowEtimingMapD1_,titlestr,83,-41.5,41.5,72,0.5,72.5,v_hpars2dprof);

    st_lowEtimingMapD2_     = "p2d_lowEtimingMapD2";
    titlestr = "Hit Time Map (5GeV< E_{hit}< 10GeV) for Depth 2, Run "+runnumstr_+"; i#eta; i#phi";
    add2dHisto(st_lowEtimingMapD2_,titlestr,83,-41.5,41.5,72,0.5,72.5,v_hpars2dprof);

    st_lateHitsTimeMapD1_ = "p2d_lateHitsTimeMapD1";
    titlestr = "Hit Time Map (T_{hit} > 12ns) for Depth 1, Run "+runnumstr_+"; i#eta; i#phi";
    add2dHisto(st_lateHitsTimeMapD1_,titlestr,83,-41.5,41.5,72,0.5,72.5,v_hpars2dprof);

    st_lateHitsTimeMapD2_ = "p2d_lateHitsTimeMapD2";
    titlestr = "Hit Time Map (T_{hit} > 12ns) for Depth 2, Run "+runnumstr_+"; i#eta; i#phi";
    add2dHisto(st_lateHitsTimeMapD2_,titlestr,83,-41.5,41.5,72,0.5,72.5,v_hpars2dprof);

    st_RvsTHFd1_ = "h2d_RvsTHFd1";
    titlestr = "R=(L-S)/(L+S) vs. T_{2TS}, Depth 1, Run "+runnumstr_+"; T_{2TS} (ns); R";
    add2dHisto(st_RvsTHFd1_, titlestr, recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       200,-1.0,1.0, v_hpars2d);

    st_RvsTHFd2_ = "h2d_RvsTHFd2";
    titlestr = "R=(L-S)/(L+S) vs. T_{2TS}, Depth 2, Run "+runnumstr_+"; T_{2TS} (ns); R";
    add2dHisto(st_RvsTHFd2_, titlestr, recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       200,-1.0,1.0, v_hpars2d);

    // per-channel beam-specific histos
    if (cut.flagSet(st_doPerChannel_)) {
      myAH->mkSubdir<uint32_t>("_2TSratioVsEperID");
      myAH->mkSubdir<uint32_t>("_TcorVs2TSratioPerID");
    }
  }

  // These are not per-hit histos, but per-event histos
  //
  st_rhCorTimesPlusVsMinus_ = "h2d_rhCorTimesPlusVsMinus" + mysubdetstr_;
  titlestr = "T_{avg}/Event, Plus vs. Minus, "+mysubdetstr_;
  titlestr += ", E_{tot,+},E_{tot,-}>10GeV, Run "+runnumstr_+"; Minus (ns); Plus (ns)";
  add2dHisto(st_rhCorTimesPlusVsMinus_,titlestr,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	     recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_, v_hpars2d);

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
  myAH->book2d<TH2F>       (v_hpars2d);
  myAH->book2d<TProfile2D> (v_hpars2dprof);

  if (cut.doInverted()) {
    myAnalHistos *myAH = cut.invhistos();
    myAH->book1d<TH1F>       (v_hpars1d);
    myAH->book2d<TH2F>       (v_hpars2d);
    myAH->book2d<TProfile2D> (v_hpars2dprof);
  }
}                       // BeamDelayTunerAlgos::bookDetailHistos4cut

//==================================================================

void
BeamDelayTunerAlgos::fillHistos4cut(myAnalCut& thisCut)
{
  HcalDelayTunerAlgos::fillHistos4cut(thisCut);

  myAnalHistos *myAH;
  if (thisCut.isActive()) {
    if (thisCut.doInverted()) myAH = thisCut.invhistos();
    else                      return;
  } else
    myAH = thisCut.cuthistos();

  if ((mysubdet_ == HcalForward) &&
      thisCut.flagSet(st_fillDetail_)) {
    myAH->fill2d<TH2F>(st_TcorVsThit_,         hittime_,correction_ns_);
    myAH->fill2d<TH2F>(st_2TSratioVsEallHF_,   hitenergy_,twoTSratio_);
    myAH->fill2d<TH2F>(st_TcorVs2TSratioAllHF_,twoTSratio_, corTime_);
    myAH->fill2d<TProfile2D>(st_TcorVs2TSratioAndEallHF_,twoTSratio_,hitenergy_,corTime_);

    switch (detID_.depth()) {
    case 1: myAH->fill2d<TH2F>(st_2TSratioVsEallHFD1_,hitenergy_,twoTSratio_); break;
    case 2: myAH->fill2d<TH2F>(st_2TSratioVsEallHFD2_,hitenergy_,twoTSratio_); break;
    }

    if (hitenergy_ <= 10.0) {
      myAH->fill2d<TH2F>((detID_.depth()==1)?
			 st_lowEtimingMapD1_:
			 st_lowEtimingMapD2_,
			 detID_.ieta(),
			 detID_.iphi(),
			 corTime_);
    }
    if (corTime_ > 12.0) {
      myAH->fill2d<TH2F>((detID_.depth()==1)?
			 st_lateHitsTimeMapD1_:
			 st_lateHitsTimeMapD2_,
			 detID_.ieta(),
			 detID_.iphi(),
			 corTime_);
    }

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

      perChanHistos *twoTSratVsEperIDhos = myAH->getAttachedHisto<uint32_t>("_2TSratioVsEperID");
      perChanHistos *TcorVs2TSratPerIDhos= myAH->getAttachedHisto<uint32_t>("_TcorVs2TSratioPerID");
      TH2F     *h2tsVsE;
      TProfile *TcorVs2TSratProf;

      perChanHistos::HistoParams_t
	hpars(title.str(),title.str(),
	      recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	      200,-1.0,3.0);

      h2tsVsE = twoTSratVsEperIDhos->exists(dix) ?
	twoTSratVsEperIDhos->get<TH2F>(dix) : 
	twoTSratVsEperIDhos->book2d<TH2F>(dix,hpars,false);
      
      if (h2tsVsE) h2tsVsE->Fill(hitenergy_,twoTSratio_);

      name = title.str();
      title << "; MaxTS+2TS Ratio (0.5 = TS3/4 boundary); T_{2TS} (ns)";

      if (hitenergy_ > 30.0) {
	// study 2TS algo performance across channels
	perChanHistos::HistoParams_t hpars1d(name,title.str(), 200,-1.0,3.0);
	TcorVs2TSratProf = TcorVs2TSratPerIDhos->exists(dix) ?
	  TcorVs2TSratPerIDhos->get<TProfile>(dix) : 
	  TcorVs2TSratPerIDhos->book1d<TProfile>(dix,hpars1d,false);
	if (TcorVs2TSratProf) TcorVs2TSratProf->Fill(twoTSratio_,corTime_);
      }

    } // if doPerChannel
  } // if fillDetail for HF
}                             // BeamDelayTunerAlgos::fillHistos4cut

//==================================================================
// specifically for HF: - handling both depths from a tower simultaneously

void
BeamDelayTunerAlgos::fillHFD1D2histos(myAnalHistos *myAH,
				      const HFRecHit& tgt, float tgtTime,
				      const HFRecHit& prt, float prtTime)
{
  assert ((tgt.id().ieta()  == prt.id().ieta()) &&
	  (tgt.id().iphi()  == prt.id().iphi()) &&
	  (tgt.id().depth() != prt.id().depth())    );

  int    zside = tgt.id().zside();
  int  absieta = tgt.id().ietaAbs();
  bool tgtisd1 = (tgt.id().depth() == 1);

  const HFRecHit& rhd1 = (tgtisd1) ? tgt : prt;
  const HFRecHit& rhd2 = (tgtisd1) ? prt : tgt;

  float L=rhd1.energy();
  float S=rhd2.energy();

  int ntwr = 18*2;  // = 18 iphi towers in ieta=40,41 * 2 sides
  if (absieta < 40)  { ntwr *= 2; }
  double weight = 1.0/(double)ntwr;

  float R =calcR (L,S);
  float R2=calcR2(L,S);

  myAH->fill2d<TH2F>((tgtisd1 ? st_RvsTHFd1_: st_RvsTHFd2_),tgtTime,R);

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
      myAH->fill2d<TProfile2D>(st_rhDeltaTdepthsVsEtaEnergy_,
			       fieta_,hitenergy_,prtTime-tgtTime);
      myAH->fill2d<TH2F>(((zside > 0) ?
			  st_rhCorTimesD1vsD2plus_ :
			  st_rhCorTimesD1vsD2minus_),
			 tgtTime,prtTime);
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
}                           // BeamDelayTunerAlgos::fillHFD1D2histos

//==================================================================
//PMT hits in HF require additional simultaneous handling of depths 1
//and 2 hits.  Only a small portion of hits (<~5%) in the collection
//are paired due to zero suppression; need both to fill d1 vs d2
//timing histos.
//
void BeamDelayTunerAlgos::findConfirmedHits(
   const edm::Handle<HFRecHitCollection>& rechithandle)
{
  const HFRecHitCollection& rechits = *rechithandle;

  m_confirmedHits_.clear();
  int npairs = 0;

  HFRecHitIt hitit1, hitit2;

  for (hitit1 = rechits.begin(); hitit1 != rechits.end(); hitit1++) {
    if (inSet<int>(detIds2mask_,hitit1->id().hashed_index())) continue;
    if (hitit1->id().depth()==1) {
      hitit2 = rechits.find(HcalDetId(HcalForward,hitit1->id().ieta(),hitit1->id().iphi(),2));
      if (hitit2 != rechits.end()) {
	if (inSet<int>(detIds2mask_,hitit2->id().hashed_index())) continue; // treat them like ZS hits
	npairs++;
	m_confirmedHits_.insert
	  (std::pair<uint32_t,HFRecHitIt>(hitit1->id().denseIndex(),hitit2));
	m_confirmedHits_.insert
	  (std::pair<uint32_t,HFRecHitIt>(hitit2->id().denseIndex(),hitit1));
      } // if there's a depth 2 hit
    } // if the hit is depth 1

  } // loop over rechits

  //cout<<"npairs = "<<npairs<<" = "<<100.*npairs/rechits.size()<<"%"<<endl;

}                          // BeamDelayTunerAlgos::findConfirmedHits

//==================================================================
// Following routine receives digi in ADC (df)
// and sets member variables digifC_ and digiGeV_ as output.
//
template<class Digi>
void
BeamDelayTunerAlgos::processZDCDigi(const Digi& df)
{
  CaloSamples dfC; // dfC is the linearized (fC) digi

  digiGeV_.clear();
  const HcalCalibrations& calibs = conditions_->getHcalCalibrations(df.id());
  const HcalQIECoder *qieCoder   = conditions_->getHcalCoder( df.id() );
  const HcalQIEShape *qieShape   = conditions_->getHcalShape();
  HcalCoderDb coder( *qieCoder, *qieShape );
  coder.adc2fC( df, dfC );
  digiGeV_.resize(dfC.size());

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
}                             // BeamDelayTunerAlgos::processZDCDigi

//==================================================================
// Following routine receives digi in ADC (df)
// and sets member variables digifC_ and digiGeV_ as output.
// duplicates hit time reconstruction algo
//
template<class Digi>
int
BeamDelayTunerAlgos::processDigi(const Digi& df,
				 CaloSamples& digifC,
				 std::vector<float>& digiGeV,
				 float& twoTSratio,
				 float& fCamplitude)
{
  int   maxts = -1;
  float maxta = -9e99;

  CaloSamples dfC; // dfC is the linearized (fC) digi

  digiGeV.clear();
  const HcalCalibrations& calibs = conditions_->getHcalCalibrations(df.id());
  const HcalQIECoder *qieCoder   = conditions_->getHcalCoder( df.id() );
  const HcalQIEShape *qieShape   = conditions_->getHcalShape();
  HcalCoderDb coder( *qieCoder, *qieShape );
  coder.adc2fC( df, dfC );
  digiGeV.resize(dfC.size());
  for (int i=0; i<dfC.size(); i++) {
    int capid=df[i].capid();
    digifC[i]  = dfC[i]   -calibs.pedestal(capid); // pedestal subtraction
    digiGeV[i] = digifC[i]*calibs.respcorrgain(capid) ;    // fC --> GeV
    // Find and return maximum samples within the reco window.
    if ((i>=firstsamp_) &&
	(i<firstsamp_+nsamps_) &&
	(digiGeV[i] > maxta)) {
      maxta = digiGeV[i];
      maxts = i;
    }
  }

  float t0   = (maxts>0)                      ? digiGeV[maxts-1] : 0.0;
  float t2   = (maxts<(int)digiGeV_.size()-2) ? digiGeV[maxts+1] : 0.0;
  float t0fC = (maxts>0)                      ? digifC [maxts-1] : 0.0;
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
    fCamplitude = t0fC+digifC[maxts];
  } else {
    // this would represent an *earlier* phase in the LUT than above,
    // so must increment the peak sample by one to get the time right
    maxts++;
    wpksamp = maxta+t2;
    if (wpksamp != 0.f) wpksamp = t2/wpksamp;
    fCamplitude = t2fC+digifC[maxts];
  }

  twoTSratio   = maxts - dfC.presamples() + wpksamp;
  //twoTSratio   = (fCamplitude > 0.0f) ? ts4/fCamplitude : -1.0;

#if 0
  for (int its=0; its<digisize; ++its)
    printf("%5.1f\t",digiGeV_[its]);
  printf ("twoTSratio=%5.3f\n",twoTSratio_);
#endif

  return maxts;
}                                // BeamDelayTunerAlgos::processDigi


//==================================================================

void
BeamDelayTunerAlgos::fillPerEvent(void)
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
    }
  }

  firstAH->fill1d<TH1F>(st_totalEperEv_,fevtnum_,totalE_);

  if ((totalEminus_ > 10.0) && (totalEplus_ > 10.0)) {
    lastAH->fill2d<TProfile2D>(st_rhCorTimesPlusVsMinus_,avgTminus_,avgTplus_);
    lastAH->fill1d<TH1F>(st_nHitsPlus_, nhitsplus_);
    lastAH->fill1d<TH1F>(st_nHitsMinus_,nhitsminus_);
    lastAH->fill1d<TH1F>(st_totalEplus_, totalEplus_);
    lastAH->fill1d<TH1F>(st_totalEminus_,totalEminus_);

    if (avgTminus_>50)
cerr<<"avgTminus="<<avgTminus_<<", nhitsminus="<<nhitsminus_<<", totalEminus="<<totalEminus_<<endl;
    if (avgTplus_ >50)
cerr<<"avgTplus=" <<avgTplus_ <<", nhitsplus=" <<nhitsplus_ <<", totalEplus=" <<totalEplus_<<endl;
  }

}                               // BeamDelayTunerAlgos::fillPerEvent

//==================================================================

void BeamDelayTunerAlgos::logLSBX(const std::string& cutstr)
{
  myAnalHistos *myAH = getCut(cutstr)->cuthistos();

  int lsnum500 = (lsnum_/500) * 500;
  stringstream range;
  range << lsnum500 << "-" << lsnum500+499;
  std::string hlsnumname = "h1d_lsnum"+range.str();
  TH1F  *lsH = myAH->get<TH1F>(hlsnumname);

  if (!lsH) {
    string title = "Lumi Section Occupancy, LS="+range.str();
    title += ", "+mysubdetstr_+", Run "+runnumstr_+"; LS Number";
    lsH = myAH->dir()->make<TH1F>(hlsnumname.c_str(),title.c_str(),
				  501,(float)lsnum500,lsnum500+500.f);
  }

  lsH->Fill(lsnum_);
  myAH->fill1d<TH1F>(st_bxnum_,bxnum_);
}                                    // BeamDelayTunerAlgos::logLSBX

//==================================================================

void
BeamDelayTunerAlgos::processHFconfirmedHits(const HFRecHit& target,
					    const HFRecHit& partner)
{
  assert ((target.id().ieta()  == partner.id().ieta()) &&
	  (target.id().iphi()  == partner.id().iphi()) &&
	  (target.id().depth() != partner.id().depth()) );

  myAnalCut *cut;

  getCut(st_PMThits_)    ->Activate(true);
  getCut(st_PMTpartners_)->Activate(true);
  getCut(st_goodHits_)   ->Activate(true);
  getCut(st_dubiousHits_)->Activate(true);

  // Only fill the histos for the target rechit, we'll come back 'round
  // for the partner hit eventually
  //
  if      (isHFPMThit(target))  cut=getCut(st_PMThits_);
  else if (isHFPMThit(partner)) cut=getCut(st_PMTpartners_);
  else                          cut=getCut(st_goodHits_);

  cut->Activate(false);

  fillHistos4cut(*cut);

  // histos comparing short/long depths
  //
  float corTime1= corTime_;
  float corTime2= partner.time() - timecor_->correctTime4(partner.id());

  // external hit corrections to apply to hits for these det IDs
  
  TimesPerDetId::const_iterator it;
  it=exthitcors_.find(detID_);       if (it!=exthitcors_.end()) corTime1-=it->second;
  it=exthitcors_.find(partner.id()); if (it!=exthitcors_.end()) corTime2-=it->second;
  
  fillHFD1D2histos(cut->cuthistos(),target, corTime1,partner, corTime2);

}                     // BeamDelayTunerAlgos::processHFconfirmedHits

//==================================================================

void
BeamDelayTunerAlgos::processHFunconfirmedHit(const HFRecHit& hfrh)
{
  myAnalCut *cut;

  if (isHFPMThit(hfrh)) cut = getCut(st_PMThits_);
  else                  cut = getCut(st_dubiousHits_);

  fillHistos4cut(*cut);
  myAnalHistos *myAH = cut->cuthistos();

  myAH->fill2d<TH2F>(st_2TSratioVsEallHF_, hitenergy_,twoTSratio_);

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
}                    // BeamDelayTunerAlgos::processHFunconfirmedHit

//==================================================================

template<class Digi,class RecHit>
void BeamDelayTunerAlgos::processDigisAndRecHits
  (const edm::Handle<edm::SortedCollection<Digi>   >& digihandle,
   const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle)
{
  const edm::SortedCollection<RecHit>& rechits = *rechithandle;

  myAnalHistos *firstAH = getCut(st_cutNone_)->cuthistos();

  firstAH->fill1d<TH1F>(st_rhColSize_,rechits.size());
  if (digihandle.isValid()) {
    firstAH->fill1d<TH1F>(st_digiColSize_,digihandle->size());
  }

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

    int  zside = detID_.zside();

    //corTime   = hittime_ - correction_ns_; not yet...
    corTime_   = hittime_;

    TimesPerDetId::const_iterator it = exthitcors_.find(detID_);
    if (it != exthitcors_.end()) {
      // external hit correction to apply to hits for this det ID
      corTime_ -= it->second;
    }

    totalE_ += hitenergy_;

    // If we have digis, do them too.
    //
    while (digihandle.isValid() &&
	   (idig < digihandle->size())) {
      const Digi&  df = (*digihandle)[idig];
      if (df.id() != rh.id()) {
	//cerr << "No rechit for digi " << df.id() << ", skipping." << endl;
	idig++;
      } else {
	if ((rh.id().det() == DetId::Calo) && 
	    (rh.id().subdetId() == 2)) { // ZDC
	  processZDCDigi<Digi>(df);
	} else {
	  maxts_ = processDigi<Digi>(df,digifC_,digiGeV_,twoTSratio_,fCamplitude_);
	}
	break;
      }
    } // while


    //==================================================
    // DETERMINE STATUS OF CUTS FROM CALCULATED DATA
    //==================================================

    getCut(st_cutNone_)      ->Activate (false);

    if (!badEventSet_.empty())
    getCut(st_cutBadEv_)     ->Activate (inSet<int>(badEventSet_,evtnum_));

    if (!acceptedBxNums_.empty())
    getCut(st_cutBadBx_)     ->Activate (notInSet<int>(acceptedBxNums_,bxnum_));

    getCut(st_cutBadFlags_)  ->Activate (hitflags_ & globalFlagMask_);

    getCut(st_cutHitEwindow_)->Activate ((rh.energy() < minHitGeV_) ||
					 (rh.energy() > maxHitGeV_)   );

    if (!acceptedPkTSnums_.empty())
    getCut(st_cutOutOfTime_) ->Activate (notInSet<int>(acceptedPkTSnums_,maxts_));

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
      if (mysubdet_ == HcalForward) {
	map<uint32_t,HFRecHitIt>::const_iterator it =
	  m_confirmedHits_.find(detID_.denseIndex());

	if (it != m_confirmedHits_.end()) {
	  const HFRecHit& target  = *(HFRecHit *)&rh;
	  const HFRecHit& partner = *(it->second);
	  processHFconfirmedHits(target,partner);
	}
	else { // is unconfirmed hit
	  const HFRecHit& hfrh = *(HFRecHit *)&rh;
	  processHFunconfirmedHit(hfrh);
	}
      }
    }

    if (!badEventSet_.empty()    && !getCut(st_cutBadEv_)->isActive() &&
	!acceptedBxNums_.empty() && !getCut(st_cutBadBx_)->isActive() &&
	!getCut(st_cutBadFlags_)->isActive() ) {
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
}                         // BeamDelayTunerAlgos::processDigisAndRecHits

//======================================================================

// ------------ method called to for each event  ------------
void
BeamDelayTunerAlgos::process(const myEventData& ed)
{
  HcalDelayTunerAlgos::process(ed);

  timecor_->init(ed.vertices());

  switch (mysubdet_) {
  case HcalBarrel:
  case HcalEndcap:  processDigisAndRecHits<HBHEDataFrame,HBHERecHit>(ed.hbhedigis(),ed.hbherechits()); break;
  case HcalOuter:   processDigisAndRecHits<HODataFrame,    HORecHit>(ed.hodigis(),  ed.horechits());   break;
  case HcalOther:   processDigisAndRecHits<ZDCDataFrame,  ZDCRecHit>(ed.zdcdigis(), ed.zdcrechits());  break;
  case HcalForward:
    findConfirmedHits(ed.hfrechits());
    processDigisAndRecHits<HFDataFrame,HFRecHit>(ed.hfdigis(), ed.hfrechits());
    break;
  default: break;
  }
  neventsProcessed_++;
}

//======================================================================

void
BeamDelayTunerAlgos::beginJob(const edm::EventSetup& iSetup,const myEventData& ed)
{
  HcalDelayTunerAlgos::beginJob(iSetup,ed);

  std::cout << "----------------------------------------"  << "\n";
  std::cout << "Cuts being applied for " << mysubdetstr_   << ":\n";
  for (unsigned i=0; i<v_nestedCuts_.size(); i++)
    std::cout << v_nestedCuts_[i] << "\t";
  for (unsigned i=0; i<v_hitCategories_.size(); i++)
    std::cout << v_hitCategories_[i] << "\t";
  std::cout << std::endl;
}

