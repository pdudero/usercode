
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
// $Id: BeamDelayTunerAlgos.cc,v 1.5 2010/03/02 21:07:50 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/ESHandle.h"
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
BeamDelayTunerAlgos::bookHistos4lastCut(void)
{
  char title[128]; std::string titlestr;

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d;

  HcalDelayTunerAlgos::bookHistos4lastCut();

  string runnumstr = int2str(runnum_);

  if (mysubdet_ == HcalForward) {
    // broken down by depth per Z-side and verified/PMT hits:

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
  add2dHisto(st_rhCorTimesPlusVsMinus_,
"Average Times Per Event, Plus vs. Minus, "+mysubdetstr_+", Run "+runnumstr+"; Minus (ns); Plus (ns)",
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
  sprintf (title, "# Hits, %sP, Run %d; # Hits", mysubdetstr_.c_str(), runnum_);
  titlestr = string(title);
  add1dHisto( st_totalEplus_, titlestr, 1301,-0.5, 1300.5, v_hpars1d);

  st_totalEminus_ = "h1d_totalE"+mysubdetstr_+"M";
  sprintf (title, "# Hits, %sM, Run %d; # Hits", mysubdetstr_.c_str(), runnum_);
  titlestr = string(title);
  add1dHisto( st_totalEminus_, titlestr, 1301,-0.5, 1300.5, v_hpars1d);

  myAnalHistos *myAH = getHistos4cut(st_lastCut_);
  myAH->book1d<TH1F> (v_hpars1d);
  myAH->book2d<TH2F> (v_hpars2d);

}                         // BeamDelayTunerAlgos::bookHistos4lastCut

//==================================================================
// specifically for HF: - handling both depths from a tower simultaneously

void
BeamDelayTunerAlgos::fillHFD1D2histos(const HFRecHit& rhd1, float corTime1,
				      const HFRecHit& rhd2, float corTime2)
{
  assert ((rhd1.id().depth() == 1) &&
	  (rhd2.id().depth() == 2)   );

  int  zside = rhd1.id().zside();

  // Beam-specific histos
  myAnalHistos *myAH = getHistos4cut(st_lastCut_);

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
  }
}                           // BeamDelayTunerAlgos::fillHFD1D2histos

//==================================================================
//PMT hits in HF require additional simultaneous handling of depths 1
//and 2 hits.  Only a small portion of hits (<~5%) in the collection
//are paired due to zero suppression; need both to fill d1 vs d2
//timing histos.  DANGEROUS: this routine relies on the internal
//implementation of HcalDetId.
//
void BeamDelayTunerAlgos::processHFPMThits
  (const edm::Handle<HFRecHitCollection>& rechithandle)
{
  const HFRecHitCollection& rechits = *rechithandle;

  int npairs = 0;

  // Find the start of depth two hits
  unsigned irh2;
  for (irh2=0; irh2 < rechits.size(); irh2++)
    if (rechits[irh2].id().depth() == 2) break;

  if (irh2 >= rechits.size()) return;

  HFRecHit rh1(HcalDetId(0),0.,0.);
  HFRecHit rh2(HcalDetId(0),0.,0.);

  int etaphi1=0x3FFF;
  int etaphi2=0x3FFF;
  int depth1=1, depth2=0;
  float corTime1=0., corTime2=0.;

  for (unsigned irh1=0; ; ) {
    if (irh1 < rechits.size()) {
      rh1     = rechits[irh1];
      etaphi1 = rh1.id().rawId() & 0x3FFF;
      depth1  = rh1.id().depth();
    }
    if (irh2 < rechits.size()) {
      rh2     = rechits[irh2];
      etaphi2 = rh2.id().rawId() & 0x3FFF;
      depth2  = rh2.id().depth();
    }

    if (depth1 != 1) break;

    for (;;) {
      irh1++;
      if (irh1 >= rechits.size()) break;
      rh1      = rechits[irh1];
      //cout << "Incremented rh1 " << rh1.id() << endl;
      etaphi1  = rh1.id().rawId() & 0x3FFF;
      depth1   = rh1.id().depth();
      if (etaphi1 >= etaphi2) break;
    }
    if (irh1 >= rechits.size()) break;

    for (;;) {
      irh2++;
      if (irh2 >= rechits.size()) break;
      rh2      = rechits[irh2];
      //cout << "Incremented rh2 " << rh2.id() << endl;
      etaphi2  = rh2.id().rawId() & 0x3FFF;
      depth2   = rh2.id().depth();
      if (etaphi2 >= etaphi1) break;
    }
    if (irh2 >= rechits.size()) break;

    if (inSet<int>(detIds2mask_,rh1.id().hashed_index())) continue;
    if (inSet<int>(detIds2mask_,rh2.id().hashed_index())) continue;

    if ((etaphi1 == etaphi2) &&
	(depth1 == 1) && (depth2 == 2) ) {
      npairs++;

      corTime1 = hittime_ - timecor_->correctTime4(rh1.id());

      TimesPerDetId::const_iterator it = exthitcors_.find(rh1.id());
      if (it != exthitcors_.end()) corTime1 -= it->second;

      corTime2 = hittime_ - timecor_->correctTime4(rh2.id());

      it = exthitcors_.find(rh2.id());
      if (it != exthitcors_.end()) corTime2 -= it->second;

      // apply all cuts
      if ((rh1.energy() > minHitGeV_) ||
	  (rh2.energy() > minHitGeV_) )  {
	if (acceptedBxNums_.empty() ||
	    inSet<int>(acceptedBxNums_,bxnum_)) {
	  if (!(rh1.flags() & globalFlagMask_) &&
	      !(rh2.flags() & globalFlagMask_)   )  {
	    if (badEventSet_.empty() ||
		notInSet<int>(badEventSet_,evtnum_)) {
	      fillHFD1D2histos(rh1,corTime1,rh2,corTime2);
	    }
	  }
	}
      }
    }
  } // loop over rechits

  //cout<<"npairs = "<<npairs<<" = "<<100.*npairs/rechits.size()<<"%"<<endl;
}                     // BeamDelayTunerAlgos::processDigisAndRecHits

//==================================================================

template<class Digi,class RecHit>
void BeamDelayTunerAlgos::processDigisAndRecHits
  (const edm::Handle<edm::SortedCollection<Digi>   >& digihandle,
   const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle)
{
  const edm::SortedCollection<RecHit>& rechits = *rechithandle;

  myAnalHistos *myAH = getHistos4cut("cut0none");
  myAH->fill1d<TH1F>(st_rhColSize_,rechits.size());

  if (digihandle.isValid())
    myAH->fill1d<TH1F>(st_digiColSize_,digihandle->size());

  totalE_ = 0.0;
  float totalEplus = 0.0;
  float totalEminus = 0.0;
  float weightedTplus = 0.0;
  float weightedTminus = 0.0;

  int nhitsplus=0,nhitsminus=0;

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

    int  zside = detID_.zside();

    correction_ns_ = timecor_->correctTime4(detID_);
    corTime_       = hittime_ - correction_ns_;

    TimesPerDetId::const_iterator it = exthitcors_.find(detID_);
    if (it != exthitcors_.end()) {
      // external hit correction to apply to hits for this det ID
      corTime_ -= it->second;
    }

    // If we have digis, do them too.
    //
    CaloSamples dfC; // dfC is the linearized (fC) digi
    if (digihandle.isValid() && (idig < digihandle->size())) {
      const Digi&   df =   (*digihandle)[idig];
      if (df.id() != rh.id()) {
	cerr << "WARNING: digis and rechits aren't tracking..." << endl;
      }
      const HcalQIECoder *qieCoder = conditions_->getHcalCoder( df.id() );
      const HcalQIEShape *qieShape = conditions_->getHcalShape();
      HcalCoderDb coder( *qieCoder, *qieShape );
      coder.adc2fC( df, dfC );
    }
    digifC_ = dfC;

    fillHistos4cut("cut0none");
    if (rh.energy() > minHitGeV_) {   fillHistos4cut("cut1minHitGeV");
      if (acceptedBxNums_.empty() ||
	  inSet<int>(acceptedBxNums_,bxnum_)) { fillHistos4cut("cut2bxnum");
	if (!(hitflags_ & globalFlagMask_))    { fillHistos4cut("cut3badFlags");
	  if (badEventSet_.empty() ||
	      notInSet<int>(badEventSet_,evtnum_)) { fillHistos4cut("cut4badEvents");

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

    //tree_->Fill();

  } // loop over rechits

  // now that we have the total energy...
  // cout << evtnum_ << "\t" << totalE_ << endl;

  myAH->fill1d<TH1F>(st_totalEperEv_,evtnum_,totalE_);

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

  switch (mysubdet_) {
  case HcalBarrel:
  case HcalEndcap:  processDigisAndRecHits<HBHEDataFrame,HBHERecHit>(ed.hbhedigis(),ed.hbherechits()); break;
  case HcalOuter:   processDigisAndRecHits<HODataFrame,HORecHit>    (ed.hodigis(),  ed.horechits());   break;
  case HcalForward: processDigisAndRecHits<HFDataFrame,HFRecHit>    (ed.hfdigis(),  ed.hfrechits());
    processHFPMThits (ed.hfrechits());
    break;
  default: break;
  }
  neventsProcessed_++;
}

//======================================================================

void
BeamDelayTunerAlgos::beginJob(const edm::EventSetup& iSetup)
{
  iSetup.get<HcalDbRecord>().get( conditions_ );
  timecor_->init();

  HcalDelayTunerAlgos::beginJob();}

