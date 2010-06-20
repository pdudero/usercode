#ifndef  _MYEDMODULESBEAMTIMINGANALALGOS
#define  _MYEDMODULESBEAMTIMINGANALALGOS

// -*- C++ -*-
//
// Package:    BeamTimingAnalAlgos
// Class:      BeamTimingAnalAlgos
// 
/**\class BeamTimingAnalAlgos BeamTimingAnalAlgos.cc MyEDmodules/SplashTimingAnalAlgos/src/BeamTimingAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: BeamTimingAnalAlgos.hh,v 1.14 2010/06/12 09:34:48 dudero Exp $
//
//


// system include files

// user include files
#include "HcalTimingAnalAlgos.hh"
#include "BeamHitTimeCorrector.hh"
#include "TTree.h"
#include "TH1F.h"
#include "TProfile2D.h"

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"

//
// class declaration
//

class BeamTimingAnalAlgos : public HcalTimingAnalAlgos {
public:
  BeamTimingAnalAlgos(const edm::ParameterSet&,
		      BeamHitTimeCorrector *);
  ~BeamTimingAnalAlgos() {}

  void process      (const myEventData&);
  void beginJob     (const edm::EventSetup& iSetup,
		     const myEventData& ed);

private:
  typedef HFRecHitCollection::const_iterator HFRecHitIt;
  typedef   HFDigiCollection::const_iterator HFDigiIt;

  // ---------- private methods ---------------------------

  std::string addCut            (const std::string& descr,
				 bool doInverted=false);
  std::string addHitCategory    (const std::string& descr);

  void   fillHFD1D2histos       (myAnalHistos *myAH,
				 const HFRecHit& rhd1, float corTime1,
				 const HFRecHit& rhd2, float corTime2);

  void   bookHistos4allCuts     (void);
  void   bookDetailHistos4cut   (myAnalCut& cut);

  void   fillPerEvent           (void);

  void   fillHistos4cut         (myAnalCut& cut);

  void   logLSBX                (const std::string& cutstr);

  template<class Digi>
  void   processZDCDigi         (const Digi& df);

  template<class Digi>
  int    processDigi            (const Digi& df,
				 CaloSamples& digifC,
				 std::vector<float>& digiGeV,
				 float& twoTSratio,
				 float& fCamplitude);

  template<class Digi,class RecHit>
  void   processDigisAndRecHits (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
				 const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle);
  void   findConfirmedHits      (const edm::Handle<HFRecHitCollection>& rechithandle,
				 const edm::Handle<HFDigiCollection>&   digihandle);

  void   processHFconfirmedHits (const HFRecHit& targethit,
				 const pair<HFRecHitIt,HFDigiIt>& partner);

  void   processHFunconfirmedHit(const HFRecHit& hfrh);


  // ----------member data ---------------------------

  TFileDirectory *mysubdetRootDir_;
  int hbheOutOfTimeFlagBit_;
  BeamHitTimeCorrector *timecor_; // beam hit time (vertex displacement) corrector

  float avgTminus_,  avgTplus_;
  int   nhitsminus_, nhitsplus_;
  float totalEminus_, totalEplus_;
  float fCamplitude_, partnerfCamplitude_;

  float treco3ts_;                    // time reconstructed with 3TS charge weighted algo
  float tLUTminust3TS_;

  CaloSamples        partnerdigifC_;
  std::vector<float> partnerdigiGeV_;

  // for HF, map of confirmed hits:
  std::map<uint32_t,std::pair<HFRecHitIt,HFDigiIt> > m_confirmedHits_;

  // cuts and names of cuts
  std::vector<std::string> v_nestedCuts_;      // vector of nested cut strings
  std::vector<std::string> v_hitCategories_;   // vector of parallel cut strings

  std::string st_cutNone_, st_cutHitEwindow_, st_cutBadFlags_, st_cutBadBx_, st_cutBadEv_, st_cutOutOfTime_;

  // names of hit categories
  std::string st_goodHits_, st_PMThits_,st_dubiousHits_, st_PMTpartners_;

  // The collection of names of histos per subdetector
  std::string st_rhCorTimesPlusVsMinus_,st_rhDeltaTvsVertexCor_;
  std::string st_nHitsPlus_;
  std::string st_nHitsMinus_;
  std::string st_totalEplus_;
  std::string st_totalEminus_;

  std::string st_rhCorTimesD1vsD2plus_;
  std::string st_rhCorTimesD1vsD2minus_;

  std::string st_OccVsEtaEnergyBothOverThresh_;

  std::string st_RvsEtwr_;
  std::string st_R2vsEtwr_;
  std::string st_RvsIeta_;
  std::string st_R2vsIeta_;

  std::string st_LvsSHF_;
  std::string st_RvsTandfCHFd1_;
  std::string st_RvsTandfCHFd2_;
  std::string st_rhDeltaTdepthsVsEtaEnHad_;
  std::string st_rhDeltaTdepthsVsEtaChHad_;
  std::string st_rhDeltaTdepthsVsEtaEnEM_;
  std::string st_rhDeltaTdepthsVsEtaChEM_;
  std::string st_deltaTvsLandSenHF_;
  std::string st_deltaTvsLandSchHF_;
  std::string st_lowEtimingMapD1_, st_lowEtimingMapD2_;
  std::string st_lateHitsTimeMapD1_, st_lateHitsTimeMapD2_;
};

#endif // _MYEDMODULESBEAMTIMINGANALALGOS
