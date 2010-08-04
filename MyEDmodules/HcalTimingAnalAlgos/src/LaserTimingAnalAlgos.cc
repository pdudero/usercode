
// -*- C++ -*-
//
// Package:    LaserTimingAnalAlgos
// Class:      LaserTimingAnalAlgos
// 
/**\class LaserTimingAnalAlgos LaserTimingAnalAlgos.cc MyEDmodules/LaserTimingAnalAlgos/src/LaserTimingAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserTimingAnalAlgos.cc,v 1.1 2010/06/20 12:48:45 dudero Exp $
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

#include "../interface/LaserTimingAnalAlgos.hh"


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
LaserTimingAnalAlgos::LaserTimingAnalAlgos(const edm::ParameterSet& iConfig) :
  HcalTimingAnalAlgos(iConfig)
{
  // cut string vector initialized in order
  v_cuts_.push_back("cutNone");
  v_cuts_.push_back("cutTDCwindow");
  v_cuts_.push_back("cutMinHitGeV");
  v_cuts_.push_back("cutAll");
  st_lastCut_ = "cutAll";

  edm::Service<TFileService> fs;
  mysubdetRootDir_ = new TFileDirectory(fs->mkdir(mysubdetstr_));

  for (unsigned i=0; i<v_cuts_.size(); i++)
    m_cuts_[v_cuts_[i]] = new myAnalCut(i,v_cuts_[i],*mysubdetRootDir_);

  TDCalgo_   = new
    LaserHitTimeCorrectorTDC(iConfig.getUntrackedParameter<edm::ParameterSet>("TDCpars"));

  firstEvent_ = true;
}                      // LaserTimingAnalAlgos::LaserTimingAnalAlgos

//==================================================================
// Following routine receives digi in ADC (df)
// and sets member variables digifC_ and digiGeV_ as output.
//
template<class Digi>
void
LaserTimingAnalAlgos::processZDCDigi(const Digi& df)
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
}                            // LaserTimingAnalAlgos::processZDCDigi

//==================================================================
// Following routine receives digi in ADC (df)
// and sets member variables digifC_ and digiGeV_ as output.
//
template<class Digi>
void
LaserTimingAnalAlgos::processDigi(const Digi& df)
{
  CaloSamples dfC; // dfC is the linearized (fC) digi

  const HcalCalibrations& calibs = conditions_->getHcalCalibrations(df.id());
  const HcalQIECoder *qieCoder   = conditions_->getHcalCoder( df.id() );
  const HcalQIEShape *qieShape   = conditions_->getHcalShape();
  HcalCoderDb coder( *qieCoder, *qieShape );
  coder.adc2fC( df, dfC );
  digiGeV_ = dfC;
  for (int i=0; i<dfC.size(); i++) {
    int capid=df[i].capid();
    digiGeV_[i] = (dfC[i]-calibs.pedestal(capid)); // pedestal subtraction
    digiGeV_[i]*= calibs.respcorrgain(capid) ;    // fC --> GeV
  }
  digifC_ = dfC;
}                                   // LaserTimingAnalAlgos::processDigi

//======================================================================

template<class RecHit, class Digi >
void LaserTimingAnalAlgos::processRecHitsAndDigis
  (const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle,
   const edm::Handle<edm::SortedCollection<Digi>   >& digihandle)
{
  const edm::SortedCollection<RecHit>& rechits = *rechithandle;

  myAnalHistos *myAH = m_cuts_["cutNone"]->cuthistos();

  myAH->fill1d<TH1D>(st_rhColSize_,rechits.size());
  if (digihandle.isValid())
    myAH->fill1d<TH1D>(st_digiColSize_,digihandle->size());

  for (unsigned irh = 0, idf = 0;
       irh < rechithandle->size() && idf < digihandle->size();
       ++irh, ++idf) {

    const RecHit& rh = rechits[irh];

    if (rh.id().det() == DetId::Hcal) {
      detID_ = HcalDetId(rh.id());
      feID_  = lmap_->getHcalFrontEndId(detID_);
      if (detID_.subdet() != mysubdet_)	continue; // HB and HE handled by separate instances of this class!
    }
    else if ((rh.id().det() == DetId::Calo) && 
	     (rh.id().subdetId() == 2)) // ZDC
      zdcDetID_ = HcalZDCDetId(rh.id());

    hittime_   = rh.time() - globalToffset_;
    hitenergy_ = rh.energy();
    hitflags_  = rh.flags();
    totalE_   += hitenergy_;

    corTime_   = TDCalgo_->correctTimeForJitter(hittime_);

#if 0
    //Performs modulus on times (corrects for latency jumps)
    modtime       = TDCalgo_->modulusTheTime(rh.time());
    correctedTime = TDCalgo_->modulusTheTime(correctedTime);

    RecHit modrh(rh.id(),rh.energy(),modtime);
#endif

    // Digi histogram(s)
    //
    if (digihandle.isValid() &&
	(idf < digihandle->size())) {
      const Digi&  df = (*digihandle)[idf];
      if (df.id() != rh.id())
	cerr << "WARNING: digis and rechits aren't tracking..." << endl;
      if ((rh.id().det() == DetId::Calo) && 
	  (rh.id().subdetId() == 2)) // ZDC
	processZDCDigi<Digi>(df);
      else
	processDigi<Digi>(df);
    }

    fillHistos4cut(*(m_cuts_["cutNone"]));

    bool isOverThresh = (hitenergy_ > minHitAmplitude_);

    if (isOverThresh)                 fillHistos4cut(*(m_cuts_["cutMinHitGeV"]));
    if (TDCalgo_->isWithinWindow()) { fillHistos4cut(*(m_cuts_["cutTDCwindow"]));
      if (isOverThresh)               fillHistos4cut(*(m_cuts_["cutAll"]));
    }
  } // loop over rechits

}                        // LaserTimingAnalAlgos::processRecHitsAndDigis

//======================================================================

#if 0
void
LaserTimingAnalAlgos::processHFrechits
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

    m_cuts_["cutNone"]->cuthistos()->fill2d<TH2D>(hfTimingVsE_,rh.energy(),rh.time());

    if (rh.energy() > minHitGeV) {
      myAnalHistos *myAH = m_cuts_["cutMinHitGeV"]->cuthistos();
      myAH->fill2d<TH2D>(rhEmap_,
			 detId.ieta(), detId.iphi()+detId.depth()-1,
			 rh.energy());

      myAH->fill2d<TH2D>(hfTimingVsE_,rh.energy(),rh.time());

      if (rh.energy() > 50.0)
	myAH->fill2d<TProfile2D>(rhTprof,detId.ieta(),detId.iphi(),rh.time());
    }
  } // loop over HF rechits
}                              // LaserTimingAnalAlgos::processHFrechits
#endif

//======================================================================

// ------------ method called to for each event  ------------
void
LaserTimingAnalAlgos::process(const myEventData& ed)
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
    for (int i=FEDNumbering::MINHCALFEDID;i<=FEDNumbering::MAXHCALFEDID; i++) {
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
    if (isLocalHCALrun) 
      cout << "Local TB trigger data detected\n" << endl;
  }

  HcalTimingAnalAlgos::process(ed);

  switch (mysubdet_) {
  case HcalBarrel:
  case HcalEndcap:
    processRecHitsAndDigis<HBHERecHit,HBHEDataFrame>(ed.hbherechits(),
						     ed.hbhedigis());
    break;
  case HcalOuter:
    processRecHitsAndDigis<HORecHit,HODataFrame>(ed.horechits(),
						 ed.hodigis());
    break;
  case HcalForward:
    processRecHitsAndDigis<HFRecHit,HFDataFrame>(ed.hfrechits(),
						 ed.hfdigis());
    break;
  case HcalOther:
    processRecHitsAndDigis<ZDCRecHit,ZDCDataFrame>(ed.zdcrechits(),
						   ed.zdcdigis()); 
    break;
  default:
    break;
  }
  neventsProcessed_++;
}                                       // LaserTimingAnalAlgos::process

//======================================================================

void
LaserTimingAnalAlgos::beginJob(const edm::EventSetup& iSetup,
			       const myEventData& ed)
{
  TDCalgo_->beginJob(iSetup);
  HcalTimingAnalAlgos::beginJob(iSetup,ed);
}

//======================================================================

void
LaserTimingAnalAlgos::endAnal()
{
  //positivize2dHistos();
}

//======================================================================
