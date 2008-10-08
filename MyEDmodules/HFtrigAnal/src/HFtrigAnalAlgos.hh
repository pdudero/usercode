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
#include "MyEDmodules/HFtrigAnal/src/linearizerLUT.hh"
#include "MyEDmodules/HFtrigAnal/src/HFtrigAnalHistos.hh"

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
    TowerEnergies_t () : ieip(0,0), totalE(0.0), longE(0.0), shortE(0.0) {}
    IetaIphi_t ieip;
    double totalE;
    double longE;
    double shortE;
  };

  // ---------- internal methods ---------------------------

  inline bool isGoodBx   (uint16_t bxnum);

  bool convertIdNumbers  (std::vector<int>& v_maskidnumbers,
			  std::vector<HcalDetId>& detIds2mask);

  bool maskedId          (const HcalDetId& id);

  bool intheSameHFWedge  (const HcalDetId& id1,const HcalDetId& id2);

  void findMaxWedges     (const HFDigiCollection& hfdigis,
			  std::vector<triggerWedge_t>& sortedWedges);

  void dumpWedges        (std::vector<triggerWedge_t>& wedges);

  void countNwedgesOver  (uint32_t threshold,
			  std::vector<triggerWedge_t>& sortedWedges,
			  int& nWtotal,
			  int& nWplus,
			  int& nWminus);

  void doNwedges         (std::vector<triggerWedge_t>& sortedWedges,
			  uint16_t bxnum);

  void doDigiSpectra     (const triggerWedge_t& maxwedge,
			  uint32_t runnum);
  void doOccupancy       (const std::vector<triggerWedge_t>& sortedWedges,
			  uint32_t runnum);
  void doPulseProfile    (const HFDataFrame& maxframe);

  void sumEnergies       (const std::vector<HFRecHit>& hfrechits,
			  std::map<int,TowerEnergies_t>& m_TowerEnergies,
			  float& totalE);

  void doEventDisplayHistos (const std::vector<HFRecHit>& hfrechits,
			     int evtnum,int runnum,float totalE);

  void doPMThistos       (std::vector<TowerEnergies_t>& v_PMThits);

  void doRhHistos        (const std::vector<HFRecHit>& hfrechits,
			  uint32_t evtnum,
			  uint32_t runnum);

  void filterRHs         (const HFRecHitCollection& unfiltrh,
			  std::vector<HFRecHit>& filtrh);

  // ----------member data ---------------------------

  // parameters
  bool                          verbose_;
  std::set<uint32_t>            s_runs_;
  std::set<uint16_t>            s_validBxNums_;
  std::vector<HcalDetId>        detIds2mask_;
  int                           sampleWindowLeft_;
  int                           sampleWindowRight_;
  uint32_t                      eventNumberMin_;
  uint32_t                      eventNumberMax_;
  double                        minGeVperRecHit_;
  double                        totalRHenergyThresh4Plotting_;
  uint32_t                      adcTrigThreshold_;

  HFtrigAnalHistos             *histos_;
  linearizerLUT                *lut_;
  int                           nTotalNonPMTevents_;
};

#endif // _HFTRIGANALALGOS_HH
