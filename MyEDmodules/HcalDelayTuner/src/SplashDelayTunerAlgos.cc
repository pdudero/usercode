
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
// $Id: SplashDelayTunerAlgos.cc,v 1.15 2010/04/22 03:22:59 dudero Exp $
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
  // HB towers with ieta >= this parameter are excluded from the HB average
  // over ieta and have their phi profiles plotted separately
  //
  unravelHBatIeta_    = abs(iConfig.getParameter<int>("unravelHBatIeta"));

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

  getCut(st_lastCut_)->setFlag(st_fillDetail_);

}                    // SplashDelayTunerAlgos::SplashDelayTunerAlgos

//==================================================================

void
SplashDelayTunerAlgos::bookDetailHistos4cut(myAnalCut& cut)
{
  HcalDelayTunerAlgos::bookDetailHistos4cut(cut);

  /********** SPLASH-SPECIFIC HISTOS: ************/

  char name[40];   std::string namestr;
  char title[128]; std::string titlestr;
  std::vector<myAnalHistos::HistoParams_t> v_hpars1dprof;

  if (mysubdet_ == HcalBarrel) {
    st_unravelTimePerRMd1_ =  "p1d_unravelTimePerRMd1" + mysubdetstr_;
    sprintf (title, "Avg. Time (Depth 1, abs(i#eta)=1-%d), %s (Run %d); iRM; Time (ns)",
	     std::min(16,unravelHBatIeta_-1), mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_unravelTimePerRMd1_, titlestr, 145,-72.5, 72.5, v_hpars1dprof);

    st_unravelTimePerRMd2_ =  "p1d_unravelTimePerRMd2" + mysubdetstr_;
    sprintf (title, "Avg. Time (Depth 2, abs(i#eta)=1-%d), %s (Run %d); iRM; Time (ns)",
	     std::min(16,unravelHBatIeta_-1), mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_unravelTimePerRMd2_,titlestr, 145,-72.5, 72.5, v_hpars1dprof);
    
    st_unravelTimePerPhid1_ =  "p1d_unravelTimePerPhid1" + mysubdetstr_;
    sprintf (title, "Avg. Time (Depth 1, abs(i#eta)=1-%d), %s (Run %d); i#phi; Time (ns)",
	     std::min(16,unravelHBatIeta_-1), mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_unravelTimePerPhid1_,titlestr, 145,-72.5, 72.5, v_hpars1dprof);

    st_unravelTimePerPhid2_ =  "p1d_unravelTimePerPhid2" + mysubdetstr_;
    sprintf (title, "Avg. Time (Depth 2, abs(i#eta)=1-%d), %s (Run %d); i#phi; Time (ns)",
	     std::min(16,unravelHBatIeta_-1), mysubdetstr_.c_str(), runnum_);
    titlestr = string(title);
    add1dHisto( st_unravelTimePerPhid2_, titlestr, 145,-72.5, 72.5, v_hpars1dprof);

    //
    // Make profiles of timing vs RM and phi for individual ietas/depths
    //
    for (int absieta=unravelHBatIeta_; absieta<=16; absieta++) {
      int maxdepth = (absieta<15) ? 1:2;
      for (int depth=1; depth<=maxdepth; depth++) {
	// vs. RM
	sprintf(name,"p1d_rhTvsRM4ieta%02dd%dHB",absieta,depth);
	sprintf(title,"RecHit Time/RM for |i#eta|=%d, depth=%d, Run %d; iRM; Time (ns)",
		absieta,depth,runnum_);
	titlestr = string(title);
	namestr  = string(name);
	int key=(absieta*10)+depth;
	m_unravelHBperRM_.insert(std::pair<int,string>(key,namestr));
	add1dHisto(namestr,titlestr,145,-72.5,72.5,v_hpars1dprof);
      
	// vs. phi
	sprintf(name,"p1d_rhTvsPhi4ieta%02dd%dHB",absieta,depth);
	sprintf(title,"RecHit Time/#iphi for |i#eta|=%d, depth=%d, Run %d; i#phi; Time (ns)",
		absieta,depth,runnum_);
	titlestr = string(title);
	namestr  = string(name);
	m_unravelHBperPhi_.insert(std::pair<int,string>(key,namestr));
	add1dHisto(namestr,titlestr,145,-72.5,72.5,v_hpars1dprof);

      } // loop over depth
    } // loop over ieta

    cut.cuthistos()->book1d<TProfile>(v_hpars1dprof);

  } // if HB
}                     // SplashDelayTunerAlgos::bookDetailHistos4cut

//==================================================================

void
SplashDelayTunerAlgos::fillHistos4cut(myAnalCut& cut)
{
  HcalDelayTunerAlgos::fillHistos4cut(cut);

  myAnalHistos *myAH = cut.cuthistos();
  int        absieta = detID_.ietaAbs();
  int           iphi = detID_.iphi();
  int          depth = detID_.depth();
  int          zside = detID_.zside();
  int           iRBX = atoi(((feID_.rbx()).substr(3,2)).c_str());
  int       iRMinRBX = feID_.rm();
  int            iRM = zside * ((iRBX-1)*4 + iRMinRBX);
  int    signed_iphi = zside*iphi;

  // Splash-specific histos
  if (cut.flagSet(st_fillDetail_)) {
    if (mysubdet_ == HcalBarrel) {
      if (absieta >= unravelHBatIeta_) {
	int key = (absieta*10) + depth;
	myAH->fill1d<TProfile> (m_unravelHBperRM_[key], iRM, corTime_);
	myAH->fill1d<TProfile> (m_unravelHBperPhi_[key], signed_iphi, corTime_);
      } else {
	std::string unravelTimePerPhi, unravelTimePerRM;
	switch(depth) {
	case 1:
	  unravelTimePerPhi = st_unravelTimePerPhid1_;
	  unravelTimePerRM  = st_unravelTimePerRMd1_;
	  break;
	case 2:
	  unravelTimePerPhi = st_unravelTimePerPhid2_;
	  unravelTimePerRM  = st_unravelTimePerRMd2_;
	  break;
	default:
	  edm::LogWarning("Invalid depth in rechit collection! detId = ") << detID_ << std::endl;
	}
	myAH->fill1d<TProfile> (unravelTimePerPhi, signed_iphi, corTime_);
	myAH->fill1d<TProfile> (unravelTimePerRM,     iRM,      corTime_);
      }
    } else if (mysubdet_ == HcalEndcap) {
#if 0
      int iRMavg2 = ((iRM-sign(iRM))/2)+sign(iRM);
      myAH->fill1d<TProfile> (st_avgTimePer2RMs_, iRMavg2, corTime_);
      myAH->fill1d<TProfile> (v_st_rhTvsRMperPixHE_[ipix-1],iRM,corTime_);
      if (depth==2) {
	const std::string& hRM =(ieta>0)?v_st_rhTvsRMperIetaD2HEP_[ieta-18]:v_st_rhTvsRMperIetaD2HEM_[-ieta-18];
	const std::string& hPhi=(ieta>0)?v_st_rhTvsPhiperIetaD2HEP_[ieta-18]:v_st_rhTvsPhiperIetaD2HEM_[-ieta-18];
	myAH->fill1d<TProfile>(hRM,iRM,corTime_);
	myAH->fill1d<TProfile>(hPhi,iphi,corTime_);
      }
#endif
    }
  }
}                           // SplashDelayTunerAlgos::fillHistos4cut

//==================================================================

template<class Digi,class RecHit>
void SplashDelayTunerAlgos::processDigisAndRecHits
  (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
   const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle)
{
  const edm::SortedCollection<RecHit>& rechits = *rechithandle;

  myAnalHistos *myAH = getCut("cut0none")->cuthistos();
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

    // If we have digis, do them too.
    CaloSamples dfC; // empty digi
    if (digihandle.isValid() && (idig < digihandle->size())) {
      const Digi&   df =   (*digihandle)[idig];
      if (rh.id() != df.id()) {
	edm::LogError("Digis and Rechits aren't tracking!") << df.id() << rh.id() << std::endl;
      }
      const HcalQIECoder *qieCoder = conditions_->getHcalCoder( df.id() );
      const HcalQIEShape *qieShape = conditions_->getHcalShape();
      HcalCoderDb coder( *qieCoder, *qieShape );
      coder.adc2fC( df, dfC );
    }
    digifC_ = dfC;

    fillHistos4cut(*(m_cuts_["cut0none"]));

    if (rh.energy() > minHitAmplitude_)        { fillHistos4cut(*(m_cuts_["cut1minHitGeV"]));
      if (acceptedBxNums_.empty() ||
	  inSet<int>(acceptedBxNums_,bxnum_))  { fillHistos4cut(*(m_cuts_["cut2bxnum"]));
	if (!(hitflags_ & globalFlagMask_))    { fillHistos4cut(*(m_cuts_["cut3badFlags"]));
//	  if (evtnum_<1061000) {                      // for run 120042
	  if (notInSet<int>(badEventSet_,evtnum_)){ fillHistos4cut(*(m_cuts_["cut4badEvents"]));
#if 0
	    if (abs(detID_.ieta()) != 16) {	      fillHistos4cut(*(m_cuts_["cut5tower16"]));
	      if ((corTime_ >= timeWindowMinNS_) &&
		  (corTime_ <= timeWindowMaxNS_)   ) { fillHistos4cut(*(m_cuts_["cut6aInTimeWindow"]));
	      }
	      if ((corTime_ < timeWindowMinNS_) ||
		  (corTime_ > timeWindowMaxNS_)   ) {  fillHistos4cut(*(m_cuts_["cut6bOutOfTimeWindow"]));
	      }
	    }
#endif
	  }
	}
      }
    }
    if (doTree_) tree_->Fill();

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

//======================================================================

void
SplashDelayTunerAlgos::beginJob(const edm::EventSetup& iSetup,
				const myEventData& ed)
{
  timecor_->init(iSetup);
  HcalDelayTunerAlgos::beginJob(iSetup,ed);
}

