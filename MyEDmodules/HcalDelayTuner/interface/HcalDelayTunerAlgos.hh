#ifndef  _MYEDMODULESHCALDELAYTUNERALGOS
#define  _MYEDMODULESHCALDELAYTUNERALGOS

// -*- C++ -*-
//
// Package:    HcalDelayTunerAlgos
// Class:      HcalDelayTunerAlgos
// 
/*
 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalDelayTunerAlgos.hh,v 1.18 2010/04/22 03:29:44 dudero Exp $
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

#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerInput.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"

//
// class declaration
//

class HcalDelayTunerAlgos {
public:
  explicit HcalDelayTunerAlgos(const edm::ParameterSet& iConfig);
  virtual ~HcalDelayTunerAlgos() {}

  virtual void process  (const myEventData&);
  virtual void beginJob (const edm::EventSetup& iSetup,const myEventData& ed);
  virtual void endJob();

  void detChannelTimes(TimesPerDetId& chtimes);

protected:

  // ---------- private types ---------------------------

  struct  tCorrection {
    tCorrection() : correctByPhi(false),correctByEta(false),applyExternal(false),
		    selfSynchronize(false),ietamin(-41),ietamax(41),iphimin(1),iphimax(72),
		    devthresh(0.0),mirrorCorrection(false) {}
    bool  correctByPhi;
    bool  correctByEta;
    bool  applyExternal;
    bool  selfSynchronize;
    int   ietamin;
    int   ietamax;
    int   iphimin;
    int   iphimax;
    float devthresh; // definition depends on whether you're self-syncing or not
    bool  mirrorCorrection;

    HcalDelayTunerInput *extcorInput;
  };

  // ---------- private methods ---------------------------

  virtual void  bookHistos4allCuts   (void);
  virtual void  bookDetailHistos4cut (myAnalCut& cut);

  virtual bool  buildMaskSet       (const std::vector<int>& v_idnumbers);

  virtual myAnalCut *getCut        (const std::string& cutstr);
  virtual void     fillHistos4cut  (myAnalCut& cut);

  virtual void  compileCorrections (const std::vector<edm::ParameterSet>& corList);
#if 0
  template<class Digi,class RecHit>
  void    processDigisAndRecHits   (const edm::SortedCollection<Digi>&   digis,
				    const edm::SortedCollection<RecHit>& rechits);

  template<class Digi,class RecHit>
  void    processDigisAndRecHits   (const edm::Handle<edm::SortedCollection<Digi> >& digihandle,
				    const edm::Handle<edm::SortedCollection<RecHit> >& rechithandle);
#endif
  void    fillDigiPulseHistos    (TProfile   *hpulse,
				  TProfile2D *hpulsePerE=NULL,
				  TProfile   *hpulseE=NULL,
				  TProfile2D *hpulseEPerE=NULL);

  virtual void add1dHisto        (const std::string& name, const std::string& title,
				  int nbinsx, double minx, double maxx,
				  std::vector<myAnalHistos::HistoParams_t>& v_hpars1d);

  virtual void add2dHisto        (const std::string& name, const std::string& title,
				  int nbinsx, double minx, double maxx,
				  int nbinsy, double miny, double maxy,
				  std::vector<myAnalHistos::HistoParams_t>& v_hpars2d);

  virtual void add1dAFhisto      (const std::string& name, const std::string& title,
				  int nbinsx, double minx, double maxx,
				  void *filladdrx, void *filladdrw,
				  detIDfun_t detIDfun,
				  std::vector<myAnalHistos::HistoAutoFill_t>& v_hpars1d);

  virtual void add2dAFhisto      (const std::string& name, const std::string& title,
				  int nbinsx, double minx, double maxx,
				  int nbinsy, double miny, double maxy,
				  void *filladdrx, void *filladdry, void *filladdrw,
				  detIDfun_t detIDfun,
				  std::vector<myAnalHistos::HistoAutoFill_t>& v_hpars2d);
  
  virtual void detChanCorsByPhiDepth   (const std::vector<TProfile *>& profsByPhi,
					const tCorrection& tcor,
					TimesPerDetId& chtimes);
  virtual void detChanCorsByEtaDepth   (const std::vector<TProfile *>& profsByEta,
					const tCorrection& tcor,
					TimesPerDetId& chtimes);
  virtual void detChanCorsByRingHO     (TimesPerDetId& chtimes);
  virtual void detChanCorsIndividually (const std::vector<TProfile2D *>& profs2d,
					const tCorrection& tcor,
					TimesPerDetId& chtimes);
  virtual void applyExternalCorrections(TimesPerDetId& extcors,
					TimesPerDetId& chtimes);
  virtual void processCorrection       (const tCorrection& tcor,
					TimesPerDetId& chtimes);
  virtual void writeCorrections        (const TimesPerDetId& chtimes);
  virtual void projectResults          (const TimesPerDetId& chtimes,
					int iterationnumber);

  float   recoTimeFromAvgPulseHF       (TProfile *avgPulse);

  void bookHOdetail       (myAnalHistos *myAH);
  void bookHEdetail       (myAnalHistos *myAH);
  void bookD1D2detail     (myAnalHistos *myAH);
  void bookHFbasicProfiles(myAnalHistos *myAH);
  void bookPerChanDetail  (myAnalHistos *myAH);
  void computeAverages    (myAnalHistos *myAH);

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
  bool              normalizeDigis_;
  bool              doPerChannel_;
  bool              doTree_;

  HcalSubdetector   mysubdet_;
  TimesPerDetId     exthitcors_; // misc. external hit time corrections

  int   firstsamp_, nsamps_, presamples_;

  std::vector<tCorrection> corList_;
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

  float           totalE_;
  int             neventsProcessed_;

  CaloSamples        digifC_;
  std::vector<float> digiGeV_;
  float              twoTSratio_; // for HF.

  edm::ESHandle<HcalDbService>  conditions_;

  std::string     runnumstr_;

  // names of common cut flags
  static const std::string st_fillDetail_;
  static const std::string st_doPerChannel_;

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
  std::string st_pulsePerEbinPlus_, st_pulsePerEbinMinus_;

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

#endif // _MYEDMODULESHCALDELAYTUNERALGOS
