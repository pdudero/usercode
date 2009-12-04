
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
// $Id: SplashDelayTunerAlgos.cc,v 1.8 2009/12/02 13:45:54 dudero Exp $
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
#include "MyEDmodules/HcalDelayTuner/interface/SplashDelayTunerAlgos.hh"


#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TProfile.h"
#include "TProfile2D.h"

//
// constructors and destructor
//
SplashDelayTunerAlgos::SplashDelayTunerAlgos(const edm::ParameterSet& iConfig,
					     SplashHitTimeCorrector *timecor) :
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

}                    // SplashDelayTunerAlgos::SplashDelayTunerAlgos

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
void SplashDelayTunerAlgos::processDigisAndRecHits
  (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
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
  cout << evtnum_ << "\t" << totalE_ << endl;

  myAH->fill1d<TH1F>(st_totalEperEv_,evtnum_,totalE_);

}                       // SplashDelayTunerAlgos::processDigisAndRecHits

//======================================================================

// ------------ method called to for each event  ------------
void
SplashDelayTunerAlgos::process(const myEventData& ed)
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
