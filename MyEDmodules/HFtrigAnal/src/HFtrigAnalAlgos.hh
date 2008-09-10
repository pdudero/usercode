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
#include "TH2F.h"
#include "TFile.h"

class HFtrigAnalAlgos {
public:
  HFtrigAnalAlgos(bool verbosity,
		  const edm::ParameterSet& iConfig);

  void beginJob(void);
  void endJob();

  void analyze(const HFDigiCollection&   hfdigis,
	       const HFRecHitCollection& hfrechits,
	       uint32_t   bxnum,
	       uint32_t   evtnum,
	       uint32_t   runnum);

private:

  bool intheSameHFWedge  (const HcalDetId& id1,const HcalDetId& id2);

  void findMaxChannels   (const HFDigiCollection& hfdigis,
			  HFDataFrame& maxframe,
			  HFDataFrame& next2maxframe,
			  int& maxval,
			  int& next2maxval);
  void fillDigiSpectra   (const HFDataFrame& maxframe,
			  int& maxadc,
			  uint32_t runnum);
  void fillOccupancy     (const HFDataFrame& maxframe,
			  const HFDataFrame& next2maxframe,
			  uint32_t runnum);
  void fillPulseProfile  (const HFDataFrame& maxframe);

  void fillBxNum         (uint32_t bxnum);
  void fillRhHistos      (const std::vector<HFRecHit>& hfrechits,
			  uint32_t evtnum,
			  uint32_t runnum);

  bool readLUTfromTextFile(void);
  void insertLUTelement(int ieta,int depth, int lutval);
  void dumpLUT(void);

  class IetaDepth_t {
  public:
    inline int sign(int x) const { return ((x<0) ? -1 : 1); }
    IetaDepth_t (int inieta, int indepth) : ieta_(inieta), depth_(indepth) {}
    IetaDepth_t (uint code) { ieta_ = code/10; depth_ = abs(code)%10; }
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
  std::set<int>                 s_runs_;
  std::string                   outRootFileName_;
  std::string                   lutFileName_;
  int                           sampleWindowLeft_;
  int                           sampleWindowRight_;
  HistoParams_t                 digiSpectrumHp_;
  HistoParams_t                 ePerEventHp_;
  HistoParams_t                 rhTotalEnergyHp_;
  uint32_t                      eventNumberMin_;
  uint32_t                      eventNumberMax_;
  double                        minGeVperRecHit_;
  double                        totalRHenergyThresh4Plotting_;
  uint32_t                      adcTrigThreshold_;

  TFileDirectory               *DigiSubDir_;
  TFileDirectory               *RHsubDir_;

  // histos
  TH1F                         *bxhist_;
  TH1F                         *h_totalE_;
  TH1F                         *h_totEvsIeta_;
  TH1F                         *h_totEvsIphi_;
  TH1F                         *h_inputLUT1_;
  TH1F                         *h_inputLUT2_;
  TH1F                         *h_PulseProfileMax_;

  TH1F                         *h_CoEinPhiPlus_;
  TH1F                         *h_CoEinPhiMinus_;
  TH1F                         *h_CoEinEtaPlus_;
  TH1F                         *h_CoEinEtaMinus_;

  std::vector<TH1F *>           v_ePerEventHistos_;
  std::map<int,TH1F *>          m_hSpectra_;
  std::map<int,TH2F *>          m_hOccupancies1_;
  std::map<int,TH2F *>          m_hOccupancies2_;

  std::map<int,std::vector<int> > m_LUT_;

  HcalDetId                     detId2Mask_;
};

#endif // _HFTRIGANALALGOS_HH
