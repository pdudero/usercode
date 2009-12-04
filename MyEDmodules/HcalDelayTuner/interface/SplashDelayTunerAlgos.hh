#ifndef  _MYEDMODULESSPLASHDELAYTUNERALGOS
#define  _MYEDMODULESSPLASHDELAYTUNERALGOS

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
// $Id: SplashDelayTunerAlgos.hh,v 1.7 2009/11/30 09:46:32 dudero Exp $
//
//


// system include files

// user include files
#include "HcalDelayTunerAlgos.hh"
#include "SplashHitTimeCorrector.hh"
#include "TTree.h"
#include "TH1F.h"
#include "TProfile2D.h"

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerInput.hh"

//
// class declaration
//

class SplashDelayTunerAlgos : public HcalDelayTunerAlgos {
public:
  SplashDelayTunerAlgos(const edm::ParameterSet&,
			SplashHitTimeCorrector *);
  ~SplashDelayTunerAlgos() {}

  void process      (const myEventData&);

private:

  // ---------- private methods ---------------------------

  template<class Digi>
  void   fillDigiPulse          (TH1F *pulseHist,
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

  SplashHitTimeCorrector *timecor_; // splash hit time geometry corrector

  // The collection of names of histos per subdetector
};

#endif // _MYEDMODULESSPLASHDELAYTUNERALGOS
