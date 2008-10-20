#ifndef _HFTRIGANALSUPPORTTYPES_HH
#define _HFTRIGANALSUPPORTTYPES_HH

#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"

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

  struct deltaAvg_t {
    int deltaIeta;
    int deltaIphi;
    float avgIeta;
    float avgIphi;
  };

  struct HFtrigAnalEvent_t {
    HFDigiCollection hfdigis;
    HFRecHitCollection hfrechits;
    std::vector<HFRecHit> filtrh;
    uint32_t runnum;
    uint32_t lsnum;
    uint32_t evtnum;
    uint16_t bxnum;
    bool     isGoodBx;
    std::vector<triggerWedge_t>  sortedWedges;
    std::vector<TowerEnergies_t> towersOverThresh;
    std::vector<TowerEnergies_t> PMThits;
  };

#endif
