
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
// $Id: BeamDelayTunerAlgos.cc,v 1.9 2009/12/04 14:36:00 dudero Exp $
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
#include "MyEDmodules/HcalDelayTuner/interface/BeamDelayTunerAlgos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"


#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TProfile.h"
#include "TProfile2D.h"

//
// constructors and destructor
//
BeamDelayTunerAlgos::BeamDelayTunerAlgos(const edm::ParameterSet& iConfig,
					 BeamHitTimeCorrector *timecor) :
  HcalDelayTunerAlgos(iConfig),
  timecor_(timecor)
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

}                        // BeamDelayTunerAlgos::BeamDelayTunerAlgos

//==================================================================

template<class Digi>
void
BeamDelayTunerAlgos::fillDigiPulse(TProfile *pulseHist,
				   const Digi& frame)
{
  float integral = pulseHist->Integral();
  if (integral != 0.0) pulseHist->Scale(1./integral);

  for (int isample = 0; isample < std::min(10,frame.size()); ++isample) {
    int rawadc = frame[isample].adc();
    pulseHist->Fill(isample,rawadc);

  } // loop over samples in digi
}

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

    fillHistos4cut("cut0none");
    if (digihandle.isValid() && (idig < digihandle->size())) {
      const Digi& frame = (*digihandle)[idig];
      if (frame.id() != rh.id()) {
	cerr << "WARNING: digis and rechits aren't tracking..." << endl;
      }
      fillDigiPulse(getHistos4cut("cut0none")->get<TProfile>(st_avgPulse_),frame);
      fillDigiPulse(getHistos4cut("cut0none")->get<TProfile>(((zside>0)?st_avgPulsePlus_:st_avgPulseMinus_)),frame);
    }

    if (rh.energy() > minHitGeV_) {
      fillHistos4cut("cut1minHitGeV");
      if (digihandle.isValid() && (idig < digihandle->size())) {
	const Digi& frame = (*digihandle)[idig];
	fillDigiPulse(getHistos4cut("cut1minHitGeV")->get<TProfile>(st_avgPulse_),frame);
	fillDigiPulse(getHistos4cut("cut1minHitGeV")->get<TProfile>
		      (((zside>0)?st_avgPulsePlus_:st_avgPulseMinus_)),frame);
      }
      if (acceptedBxNums_.empty() || inSet<int>(acceptedBxNums_,bxnum_)) {
	fillHistos4cut("cut2bxnum");
	if (digihandle.isValid() && (idig < digihandle->size())) {
	  const Digi& frame = (*digihandle)[idig];
	  fillDigiPulse(getHistos4cut("cut2bxnum")->get<TProfile>(st_avgPulse_),frame);
	  fillDigiPulse(getHistos4cut("cut2bxnum")->get<TProfile>
			(((zside>0)?st_avgPulsePlus_:st_avgPulseMinus_)),frame);
	}
	if (!(hitflags_ & globalFlagMask_)) {
	  fillHistos4cut("cut3badFlags");
	  if (digihandle.isValid() && (idig < digihandle->size())) {
	    const Digi& frame = (*digihandle)[idig];
	    fillDigiPulse(getHistos4cut("cut3badFlags")->get<TProfile>(st_avgPulse_),frame);
	    fillDigiPulse(getHistos4cut("cut3badFlags")->get<TProfile>
			  (((zside>0)?st_avgPulsePlus_:st_avgPulseMinus_)),frame);
	  }
	  if (badEventSet_.empty() || notInSet<int>(badEventSet_,evtnum_)) {
//	  if (evtnum_<1061000) {                      // for run 120042
	    fillHistos4cut("cut4badEvents");
	    if (digihandle.isValid() && (idig < digihandle->size())) {
	      const Digi& frame = (*digihandle)[idig];
	      fillDigiPulse(getHistos4cut("cut4badEvents")->get<TProfile>(st_avgPulse_),frame);
	      fillDigiPulse(getHistos4cut("cut4badEvents")->get<TProfile>
			    (((zside>0)?st_avgPulsePlus_:st_avgPulseMinus_)),frame);

	      // for comparison of +/- timing
	      if (zside > 0) { totalEplus  += hitenergy_; weightedTplus  += hitenergy_*corTime_; nhitsplus++;  }
	      else           { totalEminus += hitenergy_; weightedTminus += hitenergy_*corTime_; nhitsminus++; }
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
    if (avgTplus >50) cerr<<"avgTplus="<<avgTplus<<", nhitsplus="<<nhitsplus<<", totalEplus="<<totalEplus<<endl;
  }

}                       // BeamDelayTunerAlgos::processDigisAndRecHits

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
  case HcalForward: processDigisAndRecHits<HFDataFrame,HFRecHit>    (ed.hfdigis(),  ed.hfrechits());   break;
  default: break;
  }
  neventsProcessed_++;
}
