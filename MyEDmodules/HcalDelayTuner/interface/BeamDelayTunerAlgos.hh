#ifndef  _MYEDMODULESBEAMDELAYTUNERALGOS
#define  _MYEDMODULESBEAMDELAYTUNERALGOS

// -*- C++ -*-
//
// Package:    BeamDelayTunerAlgos
// Class:      BeamDelayTunerAlgos
// 
/**\class BeamDelayTunerAlgos BeamDelayTunerAlgos.cc MyEDmodules/SplashDelayTunerAlgos/src/BeamDelayTunerAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: BeamDelayTunerAlgos.hh,v 1.8 2010/04/06 10:46:38 dudero Exp $
//
//


// system include files

// user include files
#include "HcalDelayTunerAlgos.hh"
#include "BeamHitTimeCorrector.hh"
#include "TTree.h"
#include "TH1F.h"
#include "TProfile2D.h"

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerInput.hh"

//
// class declaration
//

class BeamDelayTunerAlgos : public HcalDelayTunerAlgos {
public:
  BeamDelayTunerAlgos(const edm::ParameterSet&,
		      BeamHitTimeCorrector *);
  ~BeamDelayTunerAlgos() {}

  void process      (const myEventData&);
  void beginJob     (const edm::EventSetup& iSetup,
		     const myEventData& ed);

private:
  typedef HFRecHitCollection::const_iterator HFRecHitIt;

  // ---------- private methods ---------------------------

  bool   isHFPMThit             (const HFRecHit& queried,
				 float partnerEnergy);

  void   fillHFD1D2histos       (const std::string& cutstr,
				 const HFRecHit& rhd1, float corTime1,
				 const HFRecHit& rhd2, float corTime2);

  void   bookHistos4allCuts     (void);
  void   bookHistos4lastCut     (void);

  void   fillPerEvent           (void);

  void   fillHistos4cut         (const std::string& cutstr,
				 bool filldetail=false);

  void   logLSBX                (const std::string& cutstr);

  template<class Digi>
  void   processZDCDigi         (const Digi& df);

  template<class Digi>
  int    processDigi            (const Digi& df);

  template<class Digi,class RecHit>
  void   processDigisAndRecHits (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
				 const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle);
  void   findConfirmedHits      (const edm::Handle<HFRecHitCollection>& rechithandle);

  void   processHFconfirmedHits (const HFRecHitIt& hitit1,
				 const HFRecHitIt& hitit2);

  void   processHFunconfirmedHit(void);

  // ----------member data ---------------------------

  BeamHitTimeCorrector *timecor_; // beam hit time (vertex displacement) corrector

  float avgTminus_,  avgTplus_;
  int   nhitsminus_, nhitsplus_;
  float totalEminus_, totalEplus_;

  // for HF, map of confirmed hits:
  std::map<uint32_t,std::pair<HFRecHitIt,HFRecHitIt> > m_confirmedHits_;

  // names of cuts
  std::string st_cutNone_, st_cutMinGeV_, st_cutFlags_, st_cutBx_, st_cutEv_, st_cutPMT_;
  std::string st_cutOutOfTime_;

  // The collection of names of histos per subdetector
  std::string st_rhCorTimesPlusVsMinus_;
  std::string st_nHitsPlus_;
  std::string st_nHitsMinus_;
  std::string st_totalEplus_;
  std::string st_totalEminus_;

  std::string st_rhCorTimesD1vsD2plusVerified_;
  std::string st_rhCorTimesD1vsD2minusVerified_;
  std::string st_rhCorTimesD1vsD2plusPMT_;
  std::string st_rhCorTimesD1vsD2minusPMT_;

  std::string st_OccVsEtaEnergyBothOverThresh_;

  std::string st_RvsEtwr_;
  std::string st_R2vsEtwr_;
  std::string st_RvsIeta_;
  std::string st_R2vsIeta_;

  std::string st_TcorVsThit_;
  std::string st_LvsSHF_;
  std::string st_LvsSpmtHitsHF_;
  std::string st_LvsSucAndPMThitsHF_;
  std::string st_ts43ratioVsEallHFucAndPMT_;
  std::string st_ts43ratioVsEallHFverified_;
  std::string st_ts43ratioVsEallHFPMT_;
};

#endif // _MYEDMODULESBEAMDELAYTUNERALGOS
