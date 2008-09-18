#ifndef _HFTRIGANALHISTOS_HH
#define _HFTRIGANALHISTOS_HH

#include <string>
#include <map>
#include <vector>
#include <set>

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"

#include "TH1F.h"
#include "TH1S.h"
#include "TH2F.h"
#include "TFile.h"

class HFtrigAnalHistos {
public:

  HFtrigAnalHistos(const edm::ParameterSet& iConfig);

  void beginJob                 (void);
  void endJob                   (int nTotalNonPMTevents);

  void  bookPerRunHistos        (uint32_t runnum);
  TH2F *book2dEnergyHisto       (uint32_t evtnum, uint32_t runnum);

  void fillNwedges              (bool goodBx,
				 int  nWedgesOverThresh,
				 int  nWedgesOverThreshPlus,
				 int  nWedgesOverThreshMinus);

  void fillTrigEffHistos        (bool goodBx, uint32_t thresh);

  void fillDigiSpectra          (int ieta, int depth,
				 uint32_t maxadc, uint32_t runnum);

  void fillOccupancyHistos      (HcalDetId idA,
				 HcalDetId idB,
				 uint32_t runnum);

  void fillPulseProfileHisto    (std::vector<int>& v_samples);

  void fillRHetaPhiDistHistos   (int ieta, int iphi, float rhenergy);

  void fillTotalEnergyHistos    (uint32_t evtnum,
				 uint32_t runnum,
				 float totalE);

  void fillCenterOfEnergyHistos (float coEinPhiMinus,
				 float coEinPhiPlus,
				 float coEinEtaMinus,
				 float coEinEtaPlus);

  void fillTowerEhistos         (int   ntowers,
				 int   ieta,
				 float twrTotalE,
				 float twrShortE,
				 float twrLongE);

  void fillNtowersHisto         (int ntowers);

  void fillPMTeventHistos       (bool     pmPMTevent,
				 int      deltaIphi,
				 int      deltaIeta,
				 float    avgIphi,
				 float    avgIeta,
				 uint32_t nPMThits);

  void fillEvtInfoHistos        (uint16_t bxnum,
				 uint32_t lsnum,
				 bool     goodBx);

private:

  // ---------- internal types ---------------------------

  class IetaDepth_t {
  public:
    inline int sign(int x) const { return ((x<0) ? -1 : 1); }
    IetaDepth_t (int inieta, int indepth) : ieta_(inieta), depth_(indepth) {}
    IetaDepth_t (int code) { ieta_ = code/10; depth_ = abs(code)%10; }
    int toCode (void) const { return (ieta_*10)+(sign(ieta_)*depth_); }
    int ieta   (void) const { return ieta_; }
    int depth  (void) const { return depth_; }
    bool operator<(const IetaDepth_t& right) const {
      if (depth_ < right.depth_) return true;
      else if (ieta_ < right.ieta_) return true;
      return false;
    }
  private:
    int ieta_;
    int depth_;
  };

  struct HistoParams_t {
    int nbins;
    double min;
    double max;
  };

  // ---------- internal methods ---------------------------

  TH1F *bookSpectrumHisto  (IetaDepth_t& id, uint32_t runnum);
  TH2F *bookOccHisto       (int depth, uint32_t runnum, bool ismaxval=true);
  TH1F *bookEperEventHisto (uint32_t nkevents, uint32_t runnum);

  // ----------member data ---------------------------

  // parameters
  HistoParams_t                 digiSpectrumHp_;
  HistoParams_t                 ePerEventHp_;
  HistoParams_t                 rhTotalEnergyHp_;
  HistoParams_t                 nWedgesHp_;
  HistoParams_t                 lumiSegHp_;
  TFileDirectory               *DigiSubDir_;
  TFileDirectory               *RHsubDir_;

  // histos
  TH1S                         *h_bx_;
  TH1S                         *h_lumiseg_;
  TH1S                         *h_lumisegGoodBx_;
  TH1F                         *h_totalE_;

  TH1F                         *h_EvsIphi_;
  TH1F                         *h_EvsIeta_;
  TH1F                         *h_EvsIeta_nTlt3_;
  TH1F                         *h_EvsIeta_nTlt5_;
  TH1F                         *h_EvsIeta_nTge5_;
  TH1F                         *h_EvsIetaNonPMT_;

  TH2F                         *h_LongVsShortE_nTlt3_;
  TH2F                         *h_LongVsShortE_nTlt5_;
  TH2F                         *h_LongVsShortE_nTge5_;
  TH2F                         *h_LongVsShortE_;

  TH1F                         *h_PulseProfileMax_;

  TH1F                         *h_nWedgesOverThreshGoodBx_;
  TH1F                         *h_nWedgesOverThreshBadBx_;

  TH1F                         *h_nTowersOverThresh_;

  TH1F                         *h_PlusMinusTrigger_;
  TH1F                         *h_PlusMinusTriggerBadBx_;

  TH1F                         *h_trigEffvsThresh_;
  TH1F                         *h_trigEffvsThreshBadBx_;

  TH1F                         *h_CoEinPhiPlus_;
  TH1F                         *h_CoEinPhiMinus_;
  TH1F                         *h_CoEinEtaPlus_;
  TH1F                         *h_CoEinEtaMinus_;

  TH1F                         *h_nPMThits_;
  TH1F                         *h_PMThitsAvgIeta_;
  TH1F                         *h_PMThitsAvgIphi_;
  TH1F                         *h_PMThitsDeltaIphi_;
  TH1F                         *h_PMThitsDeltaIeta_;
  TH1F                         *h_PMThitClassification_;

  std::vector<TH1F *>           v_ePerEventHistos_;
  std::map<int,TH1F *>          m_hSpectra_;
  std::map<int,TH2F *>          m_hOccupancies1_;
  std::map<int,TH2F *>          m_hOccupancies2_;

};

#endif // _HFTRIGANALHISTOS_HH
