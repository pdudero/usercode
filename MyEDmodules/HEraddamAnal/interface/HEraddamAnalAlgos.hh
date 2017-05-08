#ifndef  _MYEDMODULESHERADDAMANALALGOS
#define  _MYEDMODULESHERADDAMANALALGOS

// -*- C++ -*-
//
// Package:    HEraddamAnalAlgos
// Class:      HEraddamAnalAlgos
// 
/**\class HEraddamAnalAlgos HEraddamAnalAlgos.cc MyEDmodules/HEraddamAnalAlgos/src/HEraddamAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HEraddamAnalAlgos.hh,v 1.1 2013/02/18 20:12:59 dudero Exp $
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
class HEraddamAnal;

#define mySample HcalQIESample
#define myDigiCollection HBHEDigiCollection
#define myDataFrame HBHEDataFrame
//#define mySample QIE11DataFrame::Sample
//#define myDigiCollection QIE11DigiCollection
//#define myDataFrame QIE11DataFrame
//#define mySample HcalUpgradeQIESample
//#define myDigiCollection HcalUpgradeDigiCollection
//#define myDataFrame HcalUpgradeDataFrame

class HEraddamAnalAlgos {
public:
  HEraddamAnalAlgos(const edm::ParameterSet&);
  ~HEraddamAnalAlgos() {}
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
  void    processDigi   (const myDataFrame& df);
  void    processDigis  (const edm::Handle<HcalCalibDigiCollection>& digihandle);
  void    processDigis  (const edm::Handle<myDigiCollection>& digihandle);

  bool  buildChannelSet (const std::vector<int>& v_idnumbers);
  bool  buildChannelSet (const std::vector<DetId>& heCells);
  bool  buildCalibChannelSet (void);

  void  parseTDCwindows (const std::string& tdcwindowsfile);
  bool   isWithinWindow (void);

  void     readRunDates (const std::string& rundatesfile);
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
  std::string       lumiprofilefile_;

  int             ididx_;
  int             cbididx_;
  HcalFrontEndId  feID_;
  HcalDetId       detID_;
  HcalCalibDetId  calibID_;

  static const int ncells = 2592;
  static const int ncalibcells = 216;

  // Tree/branch/histo autofill vars:
  TTree          *tree_;
  uint32_t        lsnum_;
  uint32_t        bxnum_;
  uint32_t        evtnum_;
  float           fevtnum_;
  uint32_t        runnum_;
  int             year_;
  int             dayofyear_;
  float           intlumipbofday_;
  float           intlumipbofyear_;
  float           TDCphase_;
  int             TDCwinstart_[ncells];
  int             TDCwinwidth_[ncells];
  uint32_t        denseID_[ncells];
  int             iRBX_[ncells];
  int             iRM_[ncells];
  int             ieta_[ncells];
  int             iphi_[ncells];
  int             depth_[ncells];
  float           fCamplitude_[ncells];
  int             neventsProcessed_;
  int             pkSample_;

  int             s0adc_[ncells];
  int             s1adc_[ncells];
  int             s2adc_[ncells];

  int             cbieta_[ncalibcells];
  int             cbiphi_[ncalibcells];
  int             cbtype_[ncalibcells];
  float           HEPmixhi_[ncalibcells];
  float           HEPmixlo_[ncalibcells];
  float           HEMmixhi_[ncalibcells];
  float           HEMmixlo_[ncalibcells];

  float           HEPlay0rm4_[ncalibcells];
  float           HEMlay0rm4_[ncalibcells];
  float           HEPlay7rm4_[ncalibcells];
  float           HEMlay7rm4_[ncalibcells];
  float           HEPlay0rm1_[ncalibcells];
  float           HEMlay0rm1_[ncalibcells];
  float           HEPlay7rm1_[ncalibcells];
  float           HEMlay7rm1_[ncalibcells];

  CaloSamples     digifC_;
  CaloSamples     digiGeV_;
  CaloSamples     calibdigifC_;

  edm::ESHandle<HcalDbService>  conditions_;
  edm::ESHandle<CaloGeometry>   geometry_;
  edm::ESHandle<HcalElectronicsMap> hcalemap_;

  std::string st_lastCut_;
  HcalLogicalMap *lmap_;

  // Whole subdetector
  std::string st_s0adc_;
  std::string st_s1pluss2adc_;
  std::string st_fCamplitude_;
  std::string st_avgPulse_;
  std::string st_digiColSize_;

  std::string st_TDCLaserFireTime_;

  // Plus/Minus Breakdown
  std::string st_fCamplitudeMinus_;
  std::string st_fCamplitudePlus_;
  std::string st_avgPulsePlus_;
  std::string st_avgPulseMinus_;

  std::map<int,int>  channeldetIds_; // key=dense index, value=array index in tree
  std::map<int,int>  calibchanneldetIds_; // key=dense index, value=array index in tree
  std::map<int,std::pair<int,int> > m_tdcwins_;
  std::map<int,int> m_rundates_;
  std::map<int,std::pair<double,double> > m_lumiprofile_;

};

#endif // _MYEDMODULESHERADDAMANALALGOS
