
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
// $Id: BeamDelayTunerAlgos.cc,v 1.2 2010/02/02 19:44:06 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>
#include <stdlib.h> // itoa
#include <math.h> // floor

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

void
BeamDelayTunerAlgos::bookHistos4lastCut(void)
{
  char title[128]; std::string titlestr;

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d;

  HcalDelayTunerAlgos::bookHistos4lastCut();

  string runnumstr = int2str(runnum_);

  st_rhCorTimesPlusVsMinus_ = "p2d_rhCorTimesPlusVsMinus" + mysubdetstr_;
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
#if 0
	    if (abs(detID_.ieta()) != 16) {  fillHistos4cut("cut5tower16");
	      if ((corTime_ >= timeWindowMinNS_) &&
		  (corTime_ <= timeWindowMaxNS_)   ) { fillHistos4cut("cut6aInTimeWindow"); }
	      if ((corTime_ < timeWindowMinNS_) ||
		  (corTime_ > timeWindowMaxNS_)   )  { fillHistos4cut("cut6bOutOfTimeWindow"); }
	    }
#endif
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
  case HcalForward: processDigisAndRecHits<HFDataFrame,HFRecHit>    (ed.hfdigis(),  ed.hfrechits());   break;
  default: break;
  }
  neventsProcessed_++;
}
