#ifndef  _MYEDMODULESHFRADDAMANALALGOS
#define  _MYEDMODULESHFRADDAMANALALGOS

// -*- C++ -*-
//
// Package:    HFraddamAnalAlgos
// Class:      HFraddamAnalAlgos
// 
/**\class HFraddamAnalAlgos HFraddamAnalAlgos.cc MyEDmodules/HFraddamAnalAlgos/src/HFraddamAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HFraddamAnalAlgos.hh,v 1.1 2013/02/18 20:12:59 dudero Exp $
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
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"

#include "LaserTDCalgos.hh"

#include "TTree.h"

//
// class declaration
//

class HFraddamAnalAlgos {
public:
  HFraddamAnalAlgos(const edm::ParameterSet&);
  ~HFraddamAnalAlgos() {}
  void process(const myEventData& ed);
  void beginJob();
  void endAnal();

  static const std::string mysubdetstr_;

  // names of common cut flags
  static const std::string st_fillDetail_;
  static const std::string st_perEvtDigi_;
  static const std::string st_doPerChannel_;

private:
  void    processDigi   (const HcalCalibDataFrame& df);
  void    processDigi   (const HFDataFrame& df);
  void    processDigis  (const edm::Handle<HcalCalibDigiCollection>& digihandle);
  void    processDigis  (const edm::Handle<HFDigiCollection>& digihandle);

  bool  buildChannelSet (const std::vector<int>& v_idnumbers);

  void  parseTDCwindows (const std::string& tdcwindowsfile);
  bool   isWithinWindow (void);

  void     readRunDates (const std::string& rundatesfile);
  void readS2overS1means(const std::string& meansfile);
  void  readLumiProfile (const std::string& lumiprofilefile);
  void     lumiForDay   (int     dayofyear,
			 float& intlumipbofday,
			 float& intlumipbofyear);

  myAnalCut *getCut     (const std::string& cutstr);

  void bookHistos4allCuts(void);
  void bookPerChanDetail(myAnalHistos *myAH);
  void bookDetailHistos4cut(myAnalCut& cut);

  void   fillHistos4cut(myAnalCut& thisCut);
  void   fillDigiPulseHistos(perChanHistos     *digiFolder,
			     uint32_t           hkey,
			     const std::string& name,
			     const CaloSamples& filldigi);
  void   fillS3overS2forchan(perChanHistos     *perchFolder,
			     uint32_t           hkey,
			     const std::string& name,
			     const CaloSamples& filldigi);
  void   fillS4overS3forchan(perChanHistos     *perchFolder,
			     uint32_t           hkey,
			     const std::string& name,
			     const CaloSamples& filldigi);

  void   fillS2overS1forchan(perChanHistos     *perchFolder,
			     uint32_t           hkey,
			     const std::string& name,
			     float              s2overs1);
  void   fillfCamp4chan     (perChanHistos     *perchFolder,
			     uint32_t           hkey,
			     const std::string& name);

  // ----------member data ---------------------------
  // Parameters:
  bool               firstEvent_;
  int                nlaserEv_;
  std::vector<std::string> v_cuts_;             // vector of cut strings

  std::map<std::string, myAnalCut *> m_cuts_;

  // The collection of names of histos per subdetector
  std::string        TDCLaserFireTime_;

  LaserTDCalgos     *TDCalgo_;

  std::string       rundescr_;
  bool              ampCutsInfC_;
  double            minHitAmplitude_;       // minimum hit threshold (GeV or fC)
  double            maxHitAmplitude_;       // maximum hit threshold (GeV or fC)
  double            minHitAmpPerChan_;      // minimum hit threshold (GeV or fC)
  double            timeWindowMinNS_;
  double            timeWindowMaxNS_;
  uint32_t          maxEventNum2plot_;
  bool              normalizeDigis_;
  bool              doPerChannel_;
  bool              doTree_;
  
  std::string       tdcwindowsfile_;
  std::string       rundatesfile_;
  std::string       s2overs1meansfile_;
  std::string       lumiprofilefile_;

  // Tree/branch/histo autofill vars:
  TTree          *tree_;
  uint32_t        lsnum_;
  uint32_t        bxnum_;
  uint32_t        evtnum_;
  float           fevtnum_;
  uint32_t        runnum_;
  int             dayofyear_;
  float           intlumipbofday_;
  float           intlumipbofyear_;
  HcalFrontEndId  feID_;
  HcalDetId       detID_;
  int             ididx_;
  float           TDCphase_;
  int             TDCwinstart_[56];
  int             TDCwinwidth_[56];
  uint32_t        denseID_[56];
  HcalCalibDetId  calibID_;
  int             subdet_[56];
  int             iRBX_[56];
  int             iRBXsigned_;
  int             iRM_[56];
  int             ieta_[56]; // everything is float for the autofill routines.
  int             iphi_[56];
  int             iphisigned_;
  float           fangle_[56];
  float           fradius_[56];
  float           fCamplitude_[56];
  float           s2overs1_[56];
  float           s2overs1means_[56];
  float           s2overs2pluss3_[56];
  int             neventsProcessed_;
  int             pkSample_;

  int             s0adc_[56];
  int             s1adc_[56];
  int             s2adc_[56];

  // one per quadrant
  float           HFPmixhi_[4];
  float           HFPmixlo_[4];
  float           HFPscpin_[4];
  float           HFMmixhi_[4];
  float           HFMmixlo_[4];
  float           HFMscpin_[4];

  CaloSamples     digifC_;
  CaloSamples     digiGeV_;
  CaloSamples     calibdigifC_;

  edm::ESHandle<HcalDbService>  conditions_;

  std::string st_lastCut_;
  HcalLogicalMap *lmap_;

  // Whole subdetector
  std::string st_s0adc_;
  std::string st_s1pluss2adc_;
  std::string st_fCamplitude_;
  std::string st_avgPulse_;
  std::string st_digiColSize_;
  std::string st_avgS3overS2vsPhase_;
  std::string st_avgS4overS3vsPhase_;
  std::string st_avgS2overS1vsPhase_;

  std::string st_TDCLaserFireTime_;

  // Plus/Minus Breakdown
  std::string st_fCamplitudeMinus_;
  std::string st_fCamplitudePlus_;
  std::string st_avgPulsePlus_;
  std::string st_avgPulseMinus_;
  std::string st_avgS3overS2vsPhasePlus_;
  std::string st_avgS3overS2vsPhaseMinus_;
  std::string st_avgS4overS3vsPhasePlus_;
  std::string st_avgS4overS3vsPhaseMinus_;
  std::string st_avgS2overS1vsPhasePlus_;
  std::string st_avgS2overS1vsPhaseMinus_;

  std::map<int,int>  channeldetIds_; // key=dense index, value=array index in tree
  std::map<int,std::pair<int,int> > m_tdcwins_;
  std::map<int,int> m_rundates_;
  std::map<int,std::pair<double,double> > m_lumiprofile_;

};

#endif // _MYEDMODULESHFRADDAMANALALGOS
