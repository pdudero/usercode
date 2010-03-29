
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
// $Id: BeamDelayTunerAlgos.cc,v 1.11 2010/03/27 21:22:59 dudero Exp $
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

  // cut string vector initialized in order
  // all cuts applied on top of the previous one
  //
  st_cutNone_   = "cut0none";
  v_cuts_.push_back(st_cutNone_);
  if (badEventVec.size()) {
    cutstr.str("");
    cutstr <<  "cut" << v_cuts_.size() << "badEvents";
    st_cutEv_ = cutstr.str();
    v_cuts_.push_back(st_cutEv_);
  }

  if (acceptedBxVec.size()) {
    cutstr.str("");
    cutstr << "cut" << v_cuts_.size() << "bxnum";
    st_cutBx_ = cutstr.str();
    v_cuts_.push_back(st_cutBx_);
  }
  
  cutstr.str("");
  cutstr <<  "cut" << v_cuts_.size() << "badFlags";
  st_cutFlags_ = cutstr.str();
  v_cuts_.push_back(st_cutFlags_);
  
  cutstr.str("");
  cutstr <<  "cut" << v_cuts_.size() << "minHitGeV";
  st_cutMinGeV_ = cutstr.str();
  v_cuts_.push_back(st_cutMinGeV_);
  
  if (mysubdet_ == HcalForward) {
    cutstr.str("");
    cutstr <<  "cut" << v_cuts_.size() << "ncHits"; // non-correlated (PMT or otherwise) hits
    st_cutPMT_ = cutstr.str();
    v_cuts_.push_back(st_cutPMT_);
  }
  
  st_lastCut_ = v_cuts_[v_cuts_.size()-1];

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
BeamDelayTunerAlgos::isHFPMThit(const HFRecHit& queried,
				float partnerEnergy)
{
  bool isPMThit = false;
  int ieta   = queried.id().ieta();
  int depth  = queried.id().depth();
  float eGeV = queried.energy();

  switch (depth) {
  case 1: // LONG
    if (eGeV > 1.2) {
      float ratio  = calcR(eGeV,partnerEnergy);
      float elimit = calcElimit4long(ieta);
      isPMThit     = (ratio > 0.98) && (eGeV > elimit);
    }
    break;
  case 2: // SHORT
    if (eGeV > 1.8) {
      float ratio  = calcR(partnerEnergy,eGeV);
      float elimit = calcElimit4short(ieta);
      isPMThit     = (ratio < -0.98) && (eGeV > elimit);
    }
    break;
  }
  return isPMThit;
}                                 // BeamDelayTunerAlgos::isHFPMThit

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
      add2dHisto(st_RvsEtwr_, titlestr, 40,-1.0,1.0,
		 recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_, v_hpars2d);
    
      titlestr     = "Hits/Tower vs. S/(L+S) & E_{twr}, Run "+runnumstr_+"; S/(L+S); E_{twr} (GeV)";
      st_R2vsEtwr_ = "h2d_R2vsEtwrHF";
      add2dHisto(st_R2vsEtwr_, titlestr, 40,-1.0,1.0,
		 recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_, v_hpars2d);
    
      titlestr    = "Hits/Tower vs. (L-S)/(L+S) & i#eta, Run "+runnumstr_+"; (L-S)/(L+S); i#eta";
      st_RvsIeta_ = "h2d_RvsIetaHF";
      add2dHisto(st_RvsIeta_, titlestr, 40,-1.0,1.0, 13,28.5,41.5, v_hpars2d);
    
      titlestr     = "Hits/Tower vs. S/(L+S) & i#eta, Run "+runnumstr_+"; S/(L+S); i#eta";
      st_R2vsIeta_ = "h2d_R2vsIetaHF";
      add2dHisto(st_R2vsIeta_, titlestr, 40,-1.0,1.0, 13,28.5,41.5, v_hpars2d);
    }
    /*******************
     * BOOK 'EM, DANNO *
     *******************/
    std::map<std::string,myAnalCut *>::const_iterator cutit;
    for (cutit = m_cuts_.begin(); cutit != m_cuts_.end(); cutit++)
      cutit->second->histos()->book2d<TH2F>(v_hpars2d);

    for (cutit = m_ercuts_.begin(); cutit != m_ercuts_.end(); cutit++)
      cutit->second->histos()->book2d<TH2F>(v_hpars2d);
  }
}                         // BeamDelayTunerAlgos::bookHistos4allCuts

//==================================================================

void
BeamDelayTunerAlgos::fillHistos4cut(const std::string& cutstr,
				    bool filldetail)
{
  HcalDelayTunerAlgos::fillHistos4cut(cutstr,filldetail);

  if ((mysubdet_ == HcalForward) ||
      (mysubdet_ == HcalOther) ) {
    myAnalHistos *myAH =   getHistos4cut(cutstr);
    myAH->fill2d<TH2D>(st_TcorVsThit_,hittime_,correction_ns_);
  }
}                             // BeamDelayTunerAlgos::fillHistos4cut

//==================================================================

void
BeamDelayTunerAlgos::bookHistos4lastCut(void)
{
  char title[128]; std::string titlestr;

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d;

  HcalDelayTunerAlgos::bookHistos4lastCut();

  if (mysubdet_ == HcalForward) {
    // broken down by depth per Z-side and verified/PMT hits:

    st_OccVsEtaEnergyBothOverThresh_ = "h2d_OccVsEtaEnergyBothOverThreshHF";
    titlestr  = "Nhits/tower Vs i#eta & E_{hit,min}, L,S>E_{hit,min}, HF";
    titlestr += ", Run "+runnumstr_+"; |i#eta|; E_{hit,min} (GeV)";
    
    add2dHisto(st_OccVsEtaEnergyBothOverThresh_, titlestr, 41, 0.5,  41.5,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       v_hpars2d);
              
    st_LvsSHF_ = "h2d_LvsSHF";
    add2dHisto(st_LvsSHF_, "Short E Vs. Long E, HF",
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       v_hpars2d);

    string
      fmt("Average Times Per Event, Depth 2 vs. Depth 1, %s hits, HF%c, Run %d; ");
    fmt += string("Depth 1 Hit Time (ns); Depth 2 Hit Time (ns)");

    sprintf (title,fmt.c_str(),"verified",'P',runnum_); titlestr = string(title);
    st_rhCorTimesD1vsD2plusVerified_ = "h2d_rhCorTimesD1vsD2verifiedHFP";
    add2dHisto(st_rhCorTimesD1vsD2plusVerified_, titlestr,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_, v_hpars2d);

    sprintf (title,fmt.c_str(),"verified",'M',runnum_); titlestr = string(title);
    st_rhCorTimesD1vsD2minusVerified_ = "h2d_rhCorTimesD1vsD2verifiedHFM";
    add2dHisto(st_rhCorTimesD1vsD2minusVerified_, titlestr,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_, v_hpars2d);

    sprintf (title,fmt.c_str(),"PMT",'P',runnum_); titlestr = string(title);
    st_rhCorTimesD1vsD2plusPMT_ = "h2d_rhCorTimesD1vsD2PMTHFP";
    add2dHisto(st_rhCorTimesD1vsD2plusPMT_,titlestr,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_, v_hpars2d);

    sprintf (title,fmt.c_str(),"PMT",'M',runnum_); titlestr = string(title);
    st_rhCorTimesD1vsD2minusPMT_ = "h2d_rhCorTimesD1vsD2PMTHFM";
    add2dHisto(st_rhCorTimesD1vsD2minusPMT_,titlestr,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_, v_hpars2d);
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

  myAnalHistos *myAH = getHistos4cut(st_lastCut_);
  myAH->book1d<TH1F> (v_hpars1d);
  myAH->book2d<TH2F> (v_hpars2d);

  // Book for individual event ranges
  std::map<std::string,myAnalCut *>::const_iterator cutit;
  for (cutit = m_ercuts_.begin(); cutit != m_ercuts_.end(); cutit++) {
    myAH = cutit->second->histos();
    myAH->book1d<TH1F> (v_hpars1d);
    myAH->book2d<TH2F> (v_hpars2d);
  }
}                         // BeamDelayTunerAlgos::bookHistos4lastCut

//==================================================================
// specifically for HF: - handling both depths from a tower simultaneously

void
BeamDelayTunerAlgos::fillHFD1D2histos(const std::string& cutstr,
				      const HFRecHit& rhd1, float corTime1,
				      const HFRecHit& rhd2, float corTime2)
{
  assert ((rhd1.id().depth() == 1) &&
	  (rhd2.id().depth() == 2)   );

  // Beam-specific histos
  myAnalHistos *myAH = getHistos4cut(cutstr);

  int absieta = rhd1.id().ietaAbs();
  float L=rhd1.energy();
  float S=rhd2.energy();

  int ntwr = 18*2;  // = 18 iphi towers in ieta=40,41 * 2 sides
  if (absieta < 40)  { ntwr *= 2; }
  double weight = 1.0/(double)ntwr;

  myAH->fill2d<TH2D>(st_RvsEtwr_,  calcR (L,S),(L+S),weight/13.);
  myAH->fill2d<TH2D>(st_RvsIeta_,  calcR (L,S),absieta,weight);
  myAH->fill2d<TH2D>(st_R2vsEtwr_, calcR2(L,S),(L+S),weight/13.);
  myAH->fill2d<TH2D>(st_R2vsIeta_, calcR2(L,S),absieta,weight);

  myAH->fill2d<TH2D>(st_LvsSHF_,L,S);

  int  zside = rhd1.id().zside();

  if (isHFPMThit(rhd1,rhd2.energy()) ||
      isHFPMThit(rhd2,rhd1.energy())   ) {
    myAH->fill2d<TH2D>(((zside > 0) ?
			st_rhCorTimesD1vsD2plusPMT_ :
			st_rhCorTimesD1vsD2minusPMT_),
		       corTime1,corTime2);
  } else {
    myAH->fill2d<TH2D>(((zside > 0) ?
			st_rhCorTimesD1vsD2plusVerified_ :
			st_rhCorTimesD1vsD2minusVerified_),
		       corTime1,corTime2);
    
    // occupancy plot:
    // Increment *all* bins with energy <= hit energy at the given ieta
    //
    TH2D *h2Docc = myAH->get<TH2D>(st_OccVsEtaEnergyBothOverThresh_.c_str());
    for (int ibin=1;ibin<h2Docc->GetNbinsY(); ibin++) {
      double binmin = h2Docc->GetYaxis()->GetBinLowEdge(ibin);
      double binctr = h2Docc->GetYaxis()->GetBinCenter(ibin);
      int nch = 18*2;  // = 18 iphi towers in ieta=40,41 * 2 sides
      if (absieta < 40)  { nch = 36*2; }
      if (absieta < 29)  { nch = 36*3; }
      if (absieta < 26)  { nch = 36*2; }
      if (absieta < 21)  { nch = 72*2; }
      if (absieta == 17) { nch = 72*1; }
      if (absieta == 16) { nch = 72*3; }
      if (absieta == 15) { nch = 72*2; }
      if (absieta < 15)  { nch = 72*1; }
      
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
      if (inSet<int>(detIds2mask_,hitit2->id().hashed_index())) continue;
      npairs++;
      // check for PMT hit elsewhere
      std::pair<HFRecHitIt,HFRecHitIt> hitpair(hitit1,hitit2);
      m_confirmedHits_.insert
        (std::pair<uint32_t,std::pair<HFRecHitIt,HFRecHitIt> >
         (hitit1->id().denseIndex(),hitpair));
      m_confirmedHits_.insert
        (std::pair<uint32_t,std::pair<HFRecHitIt,HFRecHitIt> >
         (hitit2->id().denseIndex(),hitpair));
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
//
template<class Digi>
void
BeamDelayTunerAlgos::processDigi(const Digi& df)
{
  CaloSamples dfC; // dfC is the linearized (fC) digi

  digiGeV_.clear();
  const HcalCalibrations& calibs = conditions_->getHcalCalibrations(df.id());
  const HcalQIECoder *qieCoder   = conditions_->getHcalCoder( df.id() );
  const HcalQIEShape *qieShape   = conditions_->getHcalShape();
  HcalCoderDb coder( *qieCoder, *qieShape );
  coder.adc2fC( df, dfC );
  digiGeV_.resize(dfC.size());
  for (int i=0; i<dfC.size(); i++) {
    int capid=df[i].capid();
    digiGeV_[i] = (dfC[i]-calibs.pedestal(capid)); // pedestal subtraction
    digiGeV_[i]*= calibs.respcorrgain(capid) ;    // fC --> GeV
  }
  digifC_ = dfC;
}                                // BeamDelayTunerAlgos::processDigi

//==================================================================

template<class Digi,class RecHit>
void BeamDelayTunerAlgos::processDigisAndRecHits
  (const edm::Handle<edm::SortedCollection<Digi>   >& digihandle,
   const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle)
{
  const edm::SortedCollection<RecHit>& rechits = *rechithandle;

  myAnalHistos *myAH = getHistos4cut("cut0none");
  myAH->fill1d<TH1F>(st_rhColSize_,rechits.size());

  if (digihandle.isValid()) {
    myAH->fill1d<TH1F>(st_digiColSize_,digihandle->size());
  }

  totalE_ = 0.0;
  float totalEplus = 0.0;
  float totalEminus = 0.0;
  float weightedTplus = 0.0;
  float weightedTminus = 0.0;

  int nhitsplus=0,nhitsminus=0;

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
	  processDigi<Digi>(df);
	}
	break;
      }
    } // while

    fillHistos4cut(st_cutNone_);
    if (badEventSet_.empty() ||
	notInSet<int>(badEventSet_,evtnum_))  {
      if (!badEventSet_.empty())               fillHistos4cut(st_cutEv_);
      if (acceptedBxNums_.empty() ||
	  inSet<int>(acceptedBxNums_,bxnum_)) { 
	if (!acceptedBxNums_.empty())          fillHistos4cut(st_cutBx_);
	if (!(hitflags_ & globalFlagMask_))  { fillHistos4cut(st_cutFlags_);
	  if (rh.energy() > minHitGeV_)      {
	    fillHistos4cut(st_cutMinGeV_,st_cutMinGeV_==st_lastCut_);

	    if (mysubdet_ == HcalForward) {
	      map<uint32_t,pair<HFRecHitIt,HFRecHitIt> >::const_iterator it =
		m_confirmedHits_.find(detID_.denseIndex());
	      if (it != m_confirmedHits_.end()) {
		
		// histos comparing short/long depths
		HFRecHitIt hitit1 = it->second.first;
		HFRecHitIt hitit2 = it->second.second;
		float corTime1= hitit1->time() - timecor_->correctTime4(hitit1->id());
		float corTime2= hitit2->time() - timecor_->correctTime4(hitit2->id());

		// external hit corrections to apply to hits for these det IDs
		TimesPerDetId::const_iterator it = exthitcors_.find(hitit1->id());
		if (it != exthitcors_.end()) corTime1 -= it->second;
		it = exthitcors_.find(hitit2->id());
		if (it != exthitcors_.end()) corTime2 -= it->second;

		if (!isHFPMThit(*hitit1,hitit1->energy()) &&
		    !isHFPMThit(*hitit2,hitit2->energy())   ) {
		  fillHistos4cut(st_cutPMT_,st_cutPMT_==st_lastCut_);

		  if (detID_.depth() == 1) // so as not to double-count
		    fillHFD1D2histos(st_cutPMT_,*hitit1, corTime1,*hitit2, corTime2);

		  if (splitByEventRange_) {
		    std::map<std::string,edm::EventRange>::const_iterator erng=m_evRanges_.begin();
		    for (int i=0; erng!=m_evRanges_.end(); erng++,i++) {
		      edm::MinimalEventID thisevent(runnum_,evtnum_);
		      if (contains(erng->second,thisevent)) {
			fillHistos4cut(erng->first,true);
			if (detID_.depth() == 1) // so as not to double-count
			  fillHFD1D2histos(erng->first,*hitit1, corTime1,*hitit2, corTime2);
		      }
		    }
		    std::map<std::string,edm::LuminosityBlockRange>::const_iterator lrng=m_lsRanges_.begin();
		    for (int i=0; lrng!=m_lsRanges_.end(); lrng++,i++) {
		      edm::LuminosityBlockID thisblock(runnum_,lsnum_);
		      if (contains(lrng->second,thisblock)) {
			fillHistos4cut(lrng->first,true);
			if (detID_.depth() == 1) // so as not to double-count
			  fillHFD1D2histos(lrng->first,*hitit1, corTime1,*hitit2, corTime2);
		      }
		    }
		  } // split by event range
		} // no PMT hits
	      } // is confirmed hit
	    } // is HF

	    // for comparison of +/- timing
	    if (zside > 0) {
	      totalEplus  += hitenergy_; weightedTplus  += hitenergy_*corTime_; nhitsplus++;
	    } else {
	      totalEminus += hitenergy_; weightedTminus += hitenergy_*corTime_; nhitsminus++;
	    }
	  }
	}
      }
    }

    if (doTree_) tree_->Fill();

  } // loop over rechits

  // now that we have the total energy...
  // cout << evtnum_ << "\t" << totalE_ << endl;

  myAH->fill1d<TH1F>(st_totalEperEv_,fevtnum_,totalE_);

  if ((totalEminus > 10.0) && (totalEplus > 10.0)) {
    float avgTminus = weightedTminus/totalEminus;
    float avgTplus  = weightedTplus/totalEplus;
    myAnalHistos *myAH =   getHistos4cut(st_lastCut_);
    myAH->fill2d<TProfile2D>(st_rhCorTimesPlusVsMinus_,avgTminus,avgTplus);
    myAH->fill1d<TH1F>(st_nHitsPlus_, nhitsplus);
    myAH->fill1d<TH1F>(st_nHitsMinus_,nhitsminus);
    myAH->fill1d<TH1F>(st_totalEplus_, totalEplus);
    myAH->fill1d<TH1F>(st_totalEminus_,totalEminus);

    if (avgTminus>50) cerr<<"avgTminus="<<avgTminus<<", nhitsminus="<<nhitsminus<<", totalEminus="<<totalEminus<<endl;
    if (avgTplus >50) cerr<<"avgTplus=" <<avgTplus <<", nhitsplus=" <<nhitsplus <<", totalEplus=" <<totalEplus<<endl;
  }
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
BeamDelayTunerAlgos::beginJob(const edm::EventSetup& iSetup)
{
  HcalDelayTunerAlgos::beginJob(iSetup);

  std::cout << "----------------------------------------"  << "\n";
  std::cout << "Cuts being applied for " << mysubdetstr_   << ":\n";
  for (unsigned i=0; i<v_cuts_.size(); i++)
    std::cout << v_cuts_[i] << "\t";
  std::cout << std::endl;
}

