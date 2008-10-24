#ifndef _HFTRIGANALHISTOS_HH
#define _HFTRIGANALHISTOS_HH

#include <string>
#include <map>
#include <vector>
#include <set>

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"
#include "MyEDmodules/HFtrigAnal/src/HFtrigAnalSupportTypes.hh"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"

#include "TH1F.h"
#include "TH1S.h"
#include "TH2F.h"
#include "TFile.h"

class HFtrigAnalHistos {
public:

  HFtrigAnalHistos(const edm::ParameterSet& iConfig);

  void setGeom(const CaloGeometry* geo) { geo_ = geo; }

  void beginJob                 (void);
  void endJob                   (void);

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

  void fillnTowersOverThresh    (uint32_t nTowersOverThreshold);
  void fillTowerEhistos         (uint32_t nTowersOverThreshold,
				 std::vector<TowerEnergies_t>& v_towers,
				 bool isGoodBx);
  void fillLooseBeamGasHistos   (const HFtrigAnalEvent_t& ev);
  void fillMediumBeamGasHistos  (const HFtrigAnalEvent_t& ev);
  void fillTightBeamGasHistos   (const HFtrigAnalEvent_t& ev);

  void fillPMTeventHistos       (const std::vector<TowerEnergies_t>& v_towers,
				 const std::vector<deltaAvg_t>& sameSidePMTpairs,
				 const std::vector<deltaAvg_t>& oppoSidePMTpairs,
				 uint32_t lsnum,
				 bool     isGoodBx);

  void fillEvtInfoHistos        (const HFtrigAnalEvent_t& ev);

  void fillLongEhisto           (double longE);

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

  struct BeamGasInfo_t {
    BeamGasInfo_t(uint32_t inrn,uint32_t inlsn,uint32_t inen, uint16_t inbx) :
      runnum(inrn),lsnum(inlsn),evnum(inen),bxnum(inbx),ntowOverThresh(0),Emax(0.),Eavg(0.),
      EweightedEta(0.),EweightedPhi(0.),EweightedEta2ndMom(0.),EweightedPhi2ndMom(0.) {}
    uint32_t runnum;
    uint32_t lsnum;
    uint32_t evnum;
    uint16_t bxnum;
    uint32_t ntowOverThresh;
    double Emax;
    double Eavg;
    double EweightedEta;
    double EweightedPhi;
    double EweightedPhiMag2;
    double EweightedEta2ndMom;
    double EweightedPhi2ndMom;
  };

  struct BeamGasHistos_t {
    TH1F   *h_EvsIeta;
    TH1F   *h_Eavg;
    TH1F   *h_Emax;
    TH1F   *h_Espectrum;
    TH1F   *h_EweightedEta;
    TH1F   *h_EweightedPhi;
    TH1F   *h_2ndMomEweightedEta;
    TH1F   *h_2ndMomEweightedPhi;
    TH2F   *h_nHitsVsLumiSection;
    std::vector<BeamGasInfo_t> v_bginfo;
  };

  // ---------- internal methods ---------------------------

  void  getEtaPhi          (IetaIphi_t ieip, double& eta, double& phi);
  TH1F *bookSpectrumHisto  (IetaDepth_t& id, uint32_t runnum);
  TH2F *bookOccHisto       (int depth, uint32_t runnum, bool ismaxval=true);
  TH1F *bookEperEventHisto (uint32_t nkevents, uint32_t runnum);
  void fillBeamGasHistos   (const HFtrigAnalEvent_t& ev,
			    BeamGasHistos_t& BG);
  void dumpBGevents        (std::vector<BeamGasInfo_t>& v_bginfo,
			    std::string criterion);

  // ----------member data ---------------------------

  // parameters
  HistoParams_t                 digiSpectrumHp_;
  HistoParams_t                 ePerEventHp_;
  HistoParams_t                 rhTotalEnergyHp_;
  HistoParams_t                 towerEnergyHp_;
  HistoParams_t                 nWedgesHp_;
  HistoParams_t                 lumiSegHp_;
  TFileDirectory               *DigiSubDir_;
  TFileDirectory               *RHsubDir_;
  TFileDirectory               *BGsubDir_;
  TFileDirectory               *PMTsubDir_;

  // histos
  TH1S                         *h_bx_;
  TH1S                         *h_lumiseg_;
  TH1S                         *h_lumisegGoodBx_;
  TH1F                         *h_totalE_;
  TH1F                         *h_longE_;

  TH1F                         *h_EvsIphi_;
  TH1F                         *h_EvsIeta_;
  TH1F                         *h_EvsIeta_nTlt3_;
  TH1F                         *h_EvsIeta_nTlt5_;
  TH1F                         *h_EvsIeta_nTge5_;

  //                           Beam Gas Histos

  TH2F                         *h_LongVsShortE_nTlt3_;
  TH2F                         *h_LongVsShortE_nTlt5_;
  TH2F                         *h_LongVsShortE_nTge5_;
  TH2F                         *h_LongVsShortE_;
  TH2F                         *h_LongVsShortEBadBx_;

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
  TH2F                         *h_nPMThitsVsLumiSection_;
  TH1F                         *h_nPMTplus_;
  TH1F                         *h_ePMTplus_;
  TH1F                         *h_nPMTminus_;
  TH1F                         *h_ePMTminus_;
  TH1F                         *h_nPMTasym_;
  TH1F                         *h_ePMTasym_;

  TH1F                         *h_nPMThitsBadBx_;
  TH2F                         *h_nPMThitsVsLumiSectionBadBx_;
  TH1F                         *h_nPMTplusBadBx_;
  TH1F                         *h_ePMTplusBadBx_;
  TH1F                         *h_nPMTminusBadBx_;
  TH1F                         *h_ePMTminusBadBx_;
  TH1F                         *h_nPMTasymBadBx_;
  TH1F                         *h_ePMTasymBadBx_;

  TH1F                         *h_nPMTsamesideHits_;
  TH1F                         *h_nPMTopposideHits_;
  TH1F                         *h_PMThitsAvgIeta_;
  TH1F                         *h_PMThitsAvgIphi_;
  TH1F                         *h_PMThitsDeltaIphi_;
  TH1F                         *h_PMThitsDeltaIeta_;
  TH1F                         *h_samesidePMThitsDeltaIphi_;
  TH1F                         *h_samesidePMThitsDeltaIeta_;
  TH1F                         *h_samesidePMThitsAvgIphi_;
  TH1F                         *h_samesidePMThitsAvgIeta_;
  TH1F                         *h_PMThitClassification_;

  std::vector<TH1F *>           v_ePerEventHistos_;
  std::map<int,TH1F *>          m_hSpectra_;
  std::map<int,TH2F *>          m_hOccupancies1_;
  std::map<int,TH2F *>          m_hOccupancies2_;

  BeamGasHistos_t               bgloose_;
  BeamGasHistos_t               bgmedium_;
  BeamGasHistos_t               bgtight_;

  const CaloGeometry*           geo_;
};

#endif // _HFTRIGANALHISTOS_HH
