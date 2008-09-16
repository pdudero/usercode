#ifndef _HFTRIGANALALGOS_HH
#define _HFTRIGANALALGOS_HH

#include <string>
#include <map>
#include <vector>
#include <set>

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"

#include "TH1F.h"
#include "TH1S.h"
#include "TH2F.h"
#include "TFile.h"

class HFtrigAnalAlgos {
public:

  class HFwedgeID_t {
  public:
    // assign iphi of wedge to be the iphi between the
    // two grouped phis. 1/71 gets iphi 72 so to distinguish
    // plus and minus sides.
    //
    HFwedgeID_t (int iniphi, int inzside) {
      if      ((iniphi % 4) == 3)   iphi_ = iniphi+1;
      else if ((iniphi % 4) == 1) {
	iphi_ = iniphi-1;
	if (iphi_ <= 0) iphi_ += 72;
      }
      iphi_ *= inzside;
    }
    HFwedgeID_t (int codediphi) : iphi_(codediphi) {}
    int id() { return iphi_; }
    bool operator<(const HFwedgeID_t& right) const {
      if (iphi_ < right.iphi_) return true;
      return false;
    }
  private:
    int iphi_;
  };

  struct triggerWedge_t {
    triggerWedge_t(HFwedgeID_t& inwid, HFDataFrame inframe, uint32_t inmaxadc, int insamplenum) :
      wid(inwid), frame(inframe), maxadc(inmaxadc), maxsamplenum(insamplenum) {}
    HFwedgeID_t  wid;
    HFDataFrame  frame;
    uint32_t     maxadc;
    int          maxsamplenum;
  };

  HFtrigAnalAlgos(bool verbosity,
		  const edm::ParameterSet& iConfig);

  void beginJob(void);
  void endJob();

  void analyze(const HFDigiCollection&   hfdigis,
	       const HFRecHitCollection& hfrechits,
	       boost::uint16_t   bxnum,
	       boost::uint32_t   evtnum,
	       boost::uint32_t   runnum,
	       boost::uint32_t   lsnum);

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

  class IetaIphi_t {
  public:
    inline int sign(int x) const { return ((x<0) ? -1 : 1); }
    IetaIphi_t (int inieta, int iniphi) : ieta_(inieta), iphi_(iniphi) {}
    IetaIphi_t (int code) { ieta_ = code/100; iphi_ = abs(code)%100; }
    int toCode (void) const { return (ieta_*100)+(sign(ieta_)*iphi_); }
    int ieta   (void) const { return ieta_; }
    int iphi  (void) const { return iphi_; }
    bool operator<(const IetaIphi_t& right) const {
      if (ieta_ < right.ieta_) return true;
      else if (iphi_ < right.iphi_) return true;
      return false;
    }
  private:
    int ieta_;
    int iphi_;
  };

  struct TowerEnergies_t {
    TowerEnergies_t () : totalE(0.0), longE(0.0), shortE(0.0) {}
    double totalE;
    double longE;
    double shortE;
  };

  struct HistoParams_t {
    int nbins;
    double min;
    double max;
  };

  // ---------- internal methods ---------------------------

  bool convertIdNumbers  (std::vector<int>& v_maskidnumbers,
			  std::vector<HcalDetId>& detIds2mask);

  bool maskedId          (const HcalDetId& id);

  bool intheSameHFWedge  (const HcalDetId& id1,const HcalDetId& id2);

  void findMaxWedges     (const HFDigiCollection& hfdigis,
			  std::vector<triggerWedge_t>& sortedWedges);

  void dumpWedges        (std::vector<triggerWedge_t>& wedges);

  void fillNwedges       (std::vector<triggerWedge_t>& sortedWedges,
			  uint16_t bxnum);

  void fillDigiSpectra   (const triggerWedge_t& maxwedge,
			  uint32_t runnum);
  void fillOccupancy     (const std::vector<triggerWedge_t>& sortedWedges,
			  uint32_t runnum);
  void fillPulseProfile  (const HFDataFrame& maxframe);

  void fillRhHistos      (const std::vector<HFRecHit>& hfrechits,
			  uint32_t evtnum,
			  uint32_t runnum);

  bool readLUTfromTextFile(void);
  void insertLUTelement(int ieta,int depth, uint32_t lutval);
  void dumpLUT(void);
  uint32_t lookupLinearizerLUTval(IetaDepth_t& id, int rawadc);

  void  filterRHs          (const HFRecHitCollection& unfiltrh,
			    std::vector<HFRecHit>& filtrh);
  TH1F *bookSpectrumHisto  (IetaDepth_t& id, uint32_t runnum);
  void  bookPerRunHistos   (uint32_t runnum);
  TH2F *bookOccHisto       (int depth, uint32_t runnum, bool ismaxval=true);
  TH1F *bookEperEventHisto (uint32_t nkevents, uint32_t runnum);
  TH2F *book2dEnergyHisto  (uint32_t evtnum, uint32_t runnum);

  // ----------member data ---------------------------

  // parameters
  bool                          verbose_;
  std::set<uint32_t>            s_runs_;
  std::set<uint16_t>            s_validBxNums_;
  std::vector<HcalDetId>        detIds2mask_;
  std::string                   outRootFileName_;
  std::string                   lutFileName_;
  int                           sampleWindowLeft_;
  int                           sampleWindowRight_;
  HistoParams_t                 digiSpectrumHp_;
  HistoParams_t                 ePerEventHp_;
  HistoParams_t                 rhTotalEnergyHp_;
  HistoParams_t                 nWedgesHp_;
  HistoParams_t                 lumiSegHp_;
  uint32_t                      eventNumberMin_;
  uint32_t                      eventNumberMax_;
  double                        minGeVperRecHit_;
  double                        totalRHenergyThresh4Plotting_;
  uint32_t                      adcTrigThreshold_;
  TFileDirectory               *DigiSubDir_;
  TFileDirectory               *RHsubDir_;

  // histos
  TH1S                         *h_bx_;
  TH1S                         *h_lumiseg_;
  TH1S                         *h_lumisegGoodBx_;
  TH1F                         *h_totalE_;
  TH1F                         *h_EvsIeta_;
  TH1F                         *h_EvsIeta_nTlt3_;
  TH1F                         *h_EvsIeta_nTlt5_;
  TH1F                         *h_EvsIeta_nTge5_;
  TH1F                         *h_EvsIetaNonPMT_;
  TH2F                         *h_LongVsShortE_nTlt3_;
  TH2F                         *h_LongVsShortE_nTlt5_;
  TH2F                         *h_LongVsShortE_nTge5_;
  TH2F                         *h_LongVsShortE_;
  TH1F                         *h_EvsIphi_;
  TH1F                         *h_inputLUT1_;
  TH1F                         *h_inputLUT2_;
  TH1F                         *h_PulseProfileMax_;
  TH1F                         *h_nWedgesOverThreshGoodBx_;
  TH1F                         *h_nWedgesOverThreshBadBx_;
  TH1F                         *h_nTowersOverThresh_;
  TH1F                         *h_PlusMinusTrigger_;
  TH1F                         *h_PlusMinusTriggerBadBx_;
  TH1F                         *h_CoEinPhiPlus_;
  TH1F                         *h_CoEinPhiMinus_;
  TH1F                         *h_CoEinEtaPlus_;
  TH1F                         *h_CoEinEtaMinus_;

  std::vector<TH1F *>           v_ePerEventHistos_;
  std::map<int,TH1F *>          m_hSpectra_;
  std::map<int,TH2F *>          m_hOccupancies1_;
  std::map<int,TH2F *>          m_hOccupancies2_;

  std::map<int,std::vector<uint32_t> > m_LUT_;
};

#endif // _HFTRIGANALALGOS_HH
