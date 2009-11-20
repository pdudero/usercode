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
// $Id: SplashDelayTunerAlgos.hh,v 1.3 2009/11/13 15:56:07 dudero Exp $
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
			SplashHitTimeCorrector *); // used for analysis
  SplashDelayTunerAlgos(const edm::ParameterSet&); // used for brick-writing
  ~SplashDelayTunerAlgos() {}

  void process      (const myEventData&);
  void beginJob     (const edm::EventSetup&);
  void endAnal(); // for analysis phase

  void detChannelTimes(TimesPerDetId& chtimes);

private:

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

  void   bookHistos4allCuts  (void);
  void   bookHistos4lastCut  (void);

  bool   buildMaskSet        (const std::vector<int>& v_idnumbers);

  myAnalHistos *getHistos4cut(const std::string& cutstr);
  void         fillHistos4cut(const std::string& cutstr);

  void   compileCorrections(const std::vector<edm::ParameterSet>& corList);

  template<class RecHit>
  void    processRecHits (const edm::SortedCollection<RecHit>& rechits);

  void add1dHisto        (const std::string& name, const std::string& title,
			  int nbinsx, double minx, double maxx,
			  std::vector<myAnalHistos::HistoParams_t>& v_hpars1d);

  void add2dHisto        (const std::string& name, const std::string& title,
			  int nbinsx, double minx, double maxx,
			  int nbinsy, double miny, double maxy,
			  std::vector<myAnalHistos::HistoParams_t>& v_hpars2d);

  void detChanCorsByPhiDepth   (const std::vector<TProfile *>& profsByPhi,
				const tCorrection& tcor,
				TimesPerDetId& chtimes);
  void detChanCorsByEtaDepth   (const std::vector<TProfile *>& profsByEta,
				const tCorrection& tcor,
				TimesPerDetId& chtimes);
  void detChanCorsByRingHO     (TimesPerDetId& chtimes);
  void detChanCorsIndividually (const std::vector<TProfile2D *>& profs2d,
				const tCorrection& tcor,
				TimesPerDetId& chtimes);
  void applyExternalCorrections(TimesPerDetId& extcors,
				TimesPerDetId& chtimes);
  void processCorrection       (const tCorrection& tcor,
				TimesPerDetId& chtimes);
  void writeCorrections        (const TimesPerDetId& chtimes);
  void projectResults          (const TimesPerDetId& chtimes,
				int iterationnumber);

  // ----------member data ---------------------------

  SplashHitTimeCorrector *timecor_; // splash hit time geometry corrector
  TimesPerDetId           exthitcors_; // misc. external hit time corrections

  // Timing Analyzer Parameters:
  double            globalToffset_;   // calculated from center if HB
  uint32_t          globalFlagMask_;  // hit quality flag mask

  std::string       rundescr_;
  double            minHitGeV_;       // minimum hit energy threshold
  double            timeWindowMinNS_;
  double            timeWindowMaxNS_;
  uint32_t          recHitTscaleNbins_;
  double            recHitTscaleMinNs_;
  double            recHitTscaleMaxNs_;
  double            recHitEscaleMinGeV_;
  double            recHitEscaleMaxGeV_;
  uint32_t          maxEventNum2plot_;
  bool              selfSynchronize_; // versus synchronize to the system reference

  std::vector<tCorrection> corList_;

  // Tree and branch vars:
  TTree          *tree_;
  HcalFrontEndId  feID_;
  HcalDetId       detID_;
  uint32_t        lsnum_;
  uint32_t        bxnum_;
  uint32_t        evtnum_;
  uint32_t        runnum_;
  float           hittime_;       // reconstructed hit time-globalToffset
  float           corTime_;       // corrected for both splash geometry and global offset
  uint32_t        hitflags_;
  float           hitenergy_;
  float           splashCor_ns_;  // splash geometry correction in nanoseconds

  float           totalE_;

  // The collection of names of histos per subdetector
  std::string st_rhEnergies_;
  std::string st_totalEperEv_;
  std::string st_avgTperEvD1_;
  std::string st_avgPulse_;
  std::string st_digiColSize_;
  std::string st_rhColSize_;
  std::string st_rhUncorTimes_;
  std::string st_rhCorTimes_;
  std::string st_rhCorTimesD1_;
  std::string st_rhFlagBits_;
  std::string st_rhHBHEtimingShapedCuts_;

  std::vector<std::string>         v_st_rhTvsRMperPixHE_;
  std::vector<std::string>         v_st_rhTvsRMperIetaD2HEP_;
  std::vector<std::string>         v_st_rhTvsRMperIetaD2HEM_;
  std::vector<std::string>         v_st_rhTvsPhiperIetaD2HEP_;
  std::vector<std::string>         v_st_rhTvsPhiperIetaD2HEM_;

  std::string st_avgTimePerPhid1_, st_avgTimePerPhid2_;
  std::string st_avgTimePerPhid3_, st_avgTimePerPhid4_;
  std::string st_avgTimePer2RMs_;
  std::string st_avgTimePerRMd1_,  st_avgTimePerRMd2_;
  std::string st_avgTimePerRMd3_,  st_avgTimePerRMd4_;
  std::string st_avgTimePerRBXd1_, st_avgTimePerRBXd2_;
  std::string st_avgTimePerRBXd3_, st_avgTimePerRBXd4_;
  std::string st_avgTuncPerIetad1_,st_avgTuncPerIetad2_;
  std::string st_avgTuncPerIetad3_,st_avgTuncPerIetad4_;
  std::string st_avgTcorPerIetad1_,st_avgTcorPerIetad2_;
  std::string st_avgTcorPerIetad3_,st_avgTcorPerIetad4_;
  std::string st_avgTimePerPhiRing0_;
  std::string st_avgTimePerPhiRing1M_, st_avgTimePerPhiRing1P_;
  std::string st_avgTimePerPhiRing2M_, st_avgTimePerPhiRing2P_;
  std::string st_rhTuncProfd1_,st_rhTuncProfd2_,st_rhTuncProfd3_,st_rhTuncProfd4_;
  std::string st_rhTcorProfd1_,st_rhTcorProfd2_,st_rhTcorProfd3_,st_rhTcorProfd4_;
  std::string st_rhTprofRBXd1_,st_rhTprofRBXd2_,st_rhTprofRBXd3_,st_rhTprofRBXd4_;
  std::string st_rhEmap_;
  std::string st_uncorTimingVsE_, st_corTimingVsE_;

  bool firstEvent_;
  std::vector<std::string> v_cuts_;             // vector of cut strings
  std::map<std::string, myAnalCut *> m_cuts_;
  std::map<uint32_t,TH1F *> m_perChHistos_;
  std::set<int>  badEventSet_;
  std::set<int>  acceptedBxNums_;
  std::set<int>  detIds2mask_;
  std::string st_lastCut_;
  HcalLogicalMap *lmap_;

  TProfile2D *last2dprof_;
  TH1F       *last1ddist_;
};

#endif // _MYEDMODULESSPLASHDELAYTUNERALGOS
