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
// $Id: BeamDelayTunerAlgos.hh,v 1.8 2009/12/04 14:36:00 dudero Exp $
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

private:

  // ---------- private methods ---------------------------

  template<class Digi>
  void   fillDigiPulse          (TProfile *pulseHist,
				 const Digi& frame);
#if 0
  template<class Digi,class RecHit>
  void   processDigisAndRecHits (const edm::SortedCollection<Digi>&   digis,
				 const edm::SortedCollection<RecHit>& rechits);
#endif
  template<class Digi,class RecHit>
  void   processDigisAndRecHits (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
				 const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle);

  // ----------member data ---------------------------

  BeamHitTimeCorrector *timecor_; // beam hit time (vertex displacement) corrector

  // The collection of names of histos per subdetector
};

#endif // _MYEDMODULESBEAMDELAYTUNERALGOS
