#ifndef  _MYEDMODULESHCALTIMINGANALALGOS
#define  _MYEDMODULESHCALTIMINGANALALGOS

// -*- C++ -*-
//
// Package:    HcalTimingAnalAlgos
// Class:      HcalTimingAnalAlgos
// 
/*
 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalTimingAnalAlgos.hh,v 1.1 2010/06/20 12:48:44 dudero Exp $
//
//

// system include files

// user include files
#include "TTree.h"
#include "TH1F.h"
#include "TProfile2D.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/HcalDigi/interface/ZDCDataFrame.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/CaloObjects/interface/CaloSamples.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"

#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"

typedef std::map<HcalDetId,float> TimesPerDetId;

//
// class declaration
//

class HcalTimingAnalAlgos {
public:
  explicit HcalTimingAnalAlgos(const edm::ParameterSet& iConfig);
  virtual ~HcalTimingAnalAlgos() {}

  virtual void process  (const myEventData&);
  virtual void beginJob (const edm::EventSetup& iSetup,const myEventData& ed);
  virtual void endJob();

  // names of common cut flags
  static const std::string st_fillDetail_;
  static const std::string st_perEvtDigi_;
  static const std::string st_doPerChannel_;

  virtual void  bookHistos4allCuts   (void);
  virtual void  fillHistos4cut       (myAnalCut& cut);

protected:

  // ---------- private methods ---------------------------

  virtual void  bookDetailHistos4cut (myAnalCut& cut);

  virtual bool  buildMaskSet       (const std::vector<int>& v_idnumbers);

  virtual myAnalCut *getCut        (const std::string& cutstr);

#if 0
  template<class Digi,class RecHit>
  void    processDigisAndRecHits   (const edm::SortedCollection<Digi>&   digis,
				    const edm::SortedCollection<RecHit>& rechits);

  template<class Digi,class RecHit>
  void    processDigisAndRecHits   (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
				    const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle);
#endif
  void    fillDigiPulseHistos    (perChanHistos     *digiFolder,
				  uint32_t           hkey,
				  const std::string& name,
				  const CaloSamples& filldigifC);
  void    fillTSdistros          (myAnalHistos *myAH,
				  uint32_t      hkey,
				  const std::string& name);

  float   recoTimeFromAvgPulseHF       (TProfile *avgPulse);

  void bookHOdetail       (myAnalHistos *myAH);
  void bookHEdetail       (myAnalHistos *myAH);
  void bookD1D2detail     (myAnalHistos *myAH);
  void bookHFbasicProfiles(myAnalHistos *myAH);
  void bookPerChanDetail  (myAnalHistos *myAH);

  void computeChannelTimingFromAvgDigis (myAnalHistos *myAH);
  void compilePerTimeSampleStatistics   (myAnalHistos *myAH);

  // ----------member data ---------------------------

  // Timing Analyzer Parameters:
  std::string       mysubdetstr_;
  double            globalToffset_;   // calculated from center if HB
  uint32_t          globalFlagMask_;  // hit quality flag mask

  std::string       rundescr_;
  bool              ampCutsInfC_;
  double            minHitAmplitude_;       // minimum hit threshold (GeV or fC)
  double            maxHitAmplitude_;       // maximum hit threshold (GeV or fC)
  double            timeWindowMinNS_;
  double            timeWindowMaxNS_;
  uint32_t          recHitTscaleNbins_;
  double            recHitTscaleMinNs_;
  double            recHitTscaleMaxNs_;
  uint32_t          recHitEscaleNbins_;
  double            recHitEscaleMinGeV_;
  double            recHitEscaleMaxGeV_;
  double            recHitEscaleMinfC_;
  double            recHitEscaleMaxfC_;
  uint32_t          maxEventNum2plot_;
  uint32_t          minEvents4avgT_;
  bool              selfSynchronize_; // versus synchronize to the system reference
  bool              doDigis_;
  bool              normalizeDigis_;
  bool              doPerChannel_;
  bool              doTree_;

  HcalSubdetector   mysubdet_;
  TimesPerDetId     exthitcors_; // misc. external hit time corrections

  int   firstsamp_, nsamps_, presamples_;

  std::vector<edm::EventRange>           v_events2process_;
  std::vector<edm::LuminosityBlockRange>  v_lumis2process_;

  // Tree/branch/histo autofill vars:
  TTree          *tree_;
  HcalFrontEndId  feID_;
  HcalDetId       detID_;
  HcalZDCDetId    zdcDetID_;
  float           fsubdet_;
  float           fring_;
  float           fRBX_;
  float           fRBXsigned_;
  float           fRM_;
  float           fieta_; // everything is float for the autofill routines.
  float           fiphi_;
  float           fiphisigned_;
  float           fangle_;
  float           fradius_;
  uint32_t        lsnum_;
  uint32_t        bxnum_;
  uint32_t        evtnum_;
  float           fevtnum_;
  uint32_t        runnum_;
  float           hittime_;        // reconstructed hit time-globalToffset
  float           corTime_;        // corrected for both global offset and algo-specific
  float           correction_ns_;  // algo-specific hit time correction in nanoseconds
  uint32_t        maxts_;
  uint32_t        hitflags_;
  float           hitenergy_;
  float           fCamplitude_;
  float           totalE_;
  int             neventsProcessed_;

  CaloSamples     digifC_;
  CaloSamples     digiGeV_;
  float           twoTSratio_; // for HF.

  edm::ESHandle<HcalDbService>  conditions_;

  std::string     runnumstr_;

  // keep histo names if they have to be individually referred to
  // for any reason (non-autofilled, post-processing corrections, etc.)
  //

  std::string st_bxnum_;

  // Whole subdetector
  std::string st_rhCorTimes_;
  std::string st_totalEperEv_;
  std::string st_avgPulse_;
  std::string st_avgPulseTeq0_;
  std::string st_digiColSize_;
  std::string st_rhColSize_;
  std::string st_rhFlagBits_;
  std::string st_rhHBHEtimingShapedCuts_;
  std::string st_rhFraction2TSvsE_;

  std::string st_avgPulseTerr_;
  std::string st_avgPulseHPDmult_;
  std::string st_TvsEpulseErr_;
  std::string st_hbheTvsEhpdMult_;

  // Plus/Minus Breakdown
  std::string st_avgPulsePlus_;
  std::string st_avgPulseMinus_;

  // eta/phi/depth breakdowns:
  std::string st_avgTimePerPhid1_;
  std::string st_avgTimePerPhid2_;
  std::string st_avgTimePerPhid3_;
  std::string st_avgTcorPerIetad1_;
  std::string st_avgTcorPerIetad2_;
  std::string st_avgTcorPerIetad3_;
  std::string st_rhTcorProfd1_;
  std::string st_rhTcorProfd2_;
  std::string st_rhTcorProfd3_;
  std::string st_rhTcorProfd4_;
  std::string st_avgTimePerPhiRing2M_;
  std::string st_avgTimePerPhiRing1M_;
  std::string st_avgTimePerPhiRing0_;
  std::string st_avgTimePerPhiRing1P_;
  std::string st_avgTimePerPhiRing2P_;
  std::string st_rhTprofRBX_;
  std::string st_avgTimePerRMd1_;

  std::string st_rhTavgCorProfHFPd1_,st_rhTavgCorProfHFPd2_,st_rhTavgCorProfHFMd1_,st_rhTavgCorProfHFMd2_;
  std::string st_ts43ratioProfHFPd1_,st_ts43ratioProfHFPd2_,st_ts43ratioProfHFMd1_,st_ts43ratioProfHFMd2_;
  std::string st_ts43ratioPolarProfHFPd1_,st_ts43ratioPolarProfHFPd2_;
  std::string st_ts43ratioPolarProfHFMd1_,st_ts43ratioPolarProfHFMd2_;
  std::string st_rhTavgCorPlus_,st_rhTavgCorMinus_;
  std::string st_rhEmapHFPd1_,st_rhEmapHFPd2_,st_rhEmapHFMd1_,st_rhEmapHFMd2_;
  std::string st_rhOccMapHFPd1_,st_rhOccMapHFPd2_,st_rhOccMapHFMd1_,st_rhOccMapHFMd2_;
  std::string st_rhTprofHFPd1_, st_rhTprofHFPd2_, st_rhTprofHFMd2_, st_rhTprofHFMd1_;
  std::string st_pulsePerEbin_;

  std::string st_rhTprofd1hpdMult_;

  std::map<uint32_t,TH1F *> m_perChHistos_;

  bool firstEvent_;
  std::map<std::string, myAnalCut *> m_cuts_;
  std::set<int>  badEventSet_;
  std::set<int>  acceptedBxNums_;
  std::set<int>  acceptedPkTSnums_;
  std::set<int>  detIds2mask_;
  std::string st_lastCut_;
  HcalLogicalMap *lmap_;


  TProfile2D *last2dprof_;
  TH1F       *last1ddist_;

};

#endif // _MYEDMODULESHCALTIMINGANALALGOS
