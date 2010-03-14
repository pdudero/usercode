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
// $Id: BeamDelayTunerAlgos.hh,v 1.4 2010/03/05 13:49:11 dudero Exp $
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
  void beginJob     (const edm::EventSetup& iSetup);

private:

  // ---------- private methods ---------------------------

  bool   isHFPMThit             (const HFRecHit& queried,
				 float partnerEnergy);

  void   fillHFD1D2histos       (const std::string& cutstr,
				 const HFRecHit& rhd1, float corTime1,
				 const HFRecHit& rhd2, float corTime2);

  void   bookHistos4allCuts     (void);
  void   bookHistos4lastCut     (void);

  template<class Digi>
  void   processDigi            (const Digi& df);

  template<class Digi,class RecHit>
  void   processDigisAndRecHits (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
				 const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle);
  void   findConfirmedHits      (const edm::Handle<HFRecHitCollection>& rechithandle);

  // ----------member data ---------------------------

  BeamHitTimeCorrector *timecor_; // beam hit time (vertex displacement) corrector

  // for HF, map of confirmed hits:
  typedef HFRecHitCollection::const_iterator HFRecHitIt;
  std::map<uint32_t,std::pair<HFRecHitIt,HFRecHitIt> > m_confirmedHits_;

  // names of cuts
  std::string st_cutNone_, st_cutMinGeV_, st_cutFlags_, st_cutBx_, st_cutEv_, st_cutPMT_;

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

  std::string st_LvsSHF_;
};

#endif // _MYEDMODULESBEAMDELAYTUNERALGOS
