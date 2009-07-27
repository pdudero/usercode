#ifndef  _MYEDMODULESLASERDELAYTUNERALGOS
#define  _MYEDMODULESLASERDELAYTUNERALGOS

// -*- C++ -*-
//
// Package:    LaserDelayTunerAlgos
// Class:      LaserDelayTunerAlgos
// 
/**\class LaserDelayTunerAlgos LaserDelayTunerAlgos.cc MyEDmodules/LaserDelayTunerAlgos/src/LaserDelayTunerAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserDelayTunerAlgos.hh,v 1.5 2009/05/21 09:52:41 dudero Exp $
//
//


// system include files
#include <string>
#include <vector>

// user include files
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"
#include "DataFormats/Common/interface/SortedCollection.h"

#include "MyEDmodules/LaserDelayTuner/src/LaserDelayTunerXML.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"

#include "LaserDelayTunerTDCalgos.hh"

//
// class declaration
//

class LaserDelayTunerAlgos {
public:
  explicit LaserDelayTunerAlgos(HcalSubdetector,
				const edm::ParameterSet&,
				LaserDelayTunerTDCalgos *);
  ~LaserDelayTunerAlgos() {}
  void process(const myEventData& ed);
  void beginJob(const edm::EventSetup&);
  void endJob();

private:
  void   bookHistos      (void);
  bool   convertIdNumbers(const std::vector<int>& v_idnumbers, HcalDetId& detId);

  template<class RecHit, class DataFrame>
  void   fillHistos4cut  (const std::string& cutstr,
			  const RecHit&    rh,
			  const DataFrame& df,  // ("digi")
			  double correctedTime);

  template<class RecHit, class Digi >
  void    processRecHitsAndDigis (const edm::SortedCollection<RecHit>& rechits,
				  const edm::SortedCollection<Digi>&   digis);

  void    positivize             (myAnalHistos *myAH, TH2D *h2);
  void    positivize             (myAnalHistos *myAH, TProfile2D *h2);

  void    positivize2dHistos     (void);
  void    writeSettings          (void);

  // ----------member data ---------------------------

  edm::ESHandle<HcalDbService>  conditions_;
  HcalSubdetector               mysubdet_;
  LaserDelayTunerTDCalgos      *TDCalgo_;
  std::string                   mysubdetstr_;
  LaserDelayTunerXML            xml_;

  // Parameters:
  double          minHitGeV_;
  uint32_t        recHitTscaleNbins_;
  double          recHitTscaleMinNs_;
  double          recHitTscaleMaxNs_;
  double          recHitEscaleMinGeV_;
  double          recHitEscaleMaxGeV_;
  std::string     rundescr_;
  bool            writeBricks_;

  // The collection of names of histos per subdetector
  std::string rhEnergies_;
  std::string avgPulse_;
  std::string digiColSize_;
  std::string rhColSize_;
  std::string TDCLaserFireTime_;
  std::string rhUncorTimesPlus_, rhUncorTimesMinus_;
  std::string rhCorTimesPlus_,   rhCorTimesMinus_;
  std::string avgTimePerRMd1_,  avgTimePerRMd2_, avgTimePerRMd3_,  avgTimePerRMd4_;
  std::string avgTimePerRBXd1_,  avgTimePerRBXd2_, avgTimePerRBXd3_,  avgTimePerRBXd4_;
  std::string rhTprofd1_, rhTprofd2_, rhTprofd3_, rhTprofd4_;
  std::string rhTprofRBXd1_, rhTprofRBXd2_, rhTprofRBXd3_, rhTprofRBXd4_;
  std::string rhEmap_;
  std::string uncorTimingVsE_, corTimingVsE_;


  bool firstEvent_;
  std::vector<std::string> v_cuts_;             // vector of cut strings
  std::map<std::string, myAnalCut *> m_cuts_;
  HcalLogicalMap *lmap_;
};

#endif // _MYEDMODULESLASERDELAYTUNERALGOS
