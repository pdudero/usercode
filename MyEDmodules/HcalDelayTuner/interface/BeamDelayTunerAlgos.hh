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
// $Id: BeamDelayTunerAlgos.hh,v 1.3 2010/03/02 21:08:16 dudero Exp $
//
//


// system include files

// user include files
#include "HcalDelayTunerAlgos.hh"
#include "BeamHitTimeCorrector.hh"
#include "TTree.h"
#include "TH1F.h"
#include "TProfile2D.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
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

  void   fillHFD1D2histos       (const HFRecHit& rhd1, float corTime1,
				 const HFRecHit& rhd2, float corTime2);

  void   bookHistos4lastCut     (void);

  template<class Digi,class RecHit>
  void   processDigisAndRecHits (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
				 const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle);
  void   processHFPMThits       (const edm::Handle<HFRecHitCollection>& rechithandle);

  // ----------member data ---------------------------

  BeamHitTimeCorrector *timecor_; // beam hit time (vertex displacement) corrector

  edm::ESHandle<HcalDbService>  conditions_;

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
};

#endif // _MYEDMODULESBEAMDELAYTUNERALGOS
