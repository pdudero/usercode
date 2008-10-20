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
#include "MyEDmodules/HFtrigAnal/src/HFtrigAnalSupportTypes.hh"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"

#include "TH1F.h"
#include "TH1S.h"
#include "TH2F.h"
#include "TFile.h"

class HFtrigAnalAlgos {
public:

  HFtrigAnalAlgos(bool verbosity,
		  const edm::ParameterSet& iConfig);

  void beginJob(void);
  void endJob();

  void setGeom(const CaloGeometry* geo) { histos_->setGeom(geo); }

  void analyze(HFtrigAnalEvent_t& ev);

private:

  // ---------- internal types ---------------------------

  // ---------- internal methods ---------------------------

  inline bool isGoodBx   (uint16_t bxnum);

  bool convertIdNumbers  (std::vector<int>& v_maskidnumbers,
			  std::vector<HcalDetId>& detIds2mask);

  bool maskedId          (const HcalDetId& id);

  bool intheSameHFWedge  (const HcalDetId& id1,const HcalDetId& id2);

  void findMaxWedges     (HFtrigAnalEvent_t& ev);

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

  void doPulseProfile    (const HFDataFrame&             maxframe);

  void sumEnergies       (const std::vector<HFRecHit>&   hfrechits,
			  std::vector<TowerEnergies_t>&  v_towers,
			  float&                         totalE);

  void doEventDisplays   (const std::vector<HFRecHit>&   hfrechits,
			  int                            evtnum,
			  int                            runnum,
			  float                          totalE);

  bool towerConfirmedHit (const TowerEnergies_t& tower);

  void filterTowers      (const
			  std::vector<TowerEnergies_t>&  v_towers,
			  std::vector<TowerEnergies_t>&  v_towersOverThresh,
			  std::vector<TowerEnergies_t>&  v_PMThits,
			  bool&                          oneConfirmedHit,
			  double&                        maxGeVfound);

  void doPMThistos       (HFtrigAnalEvent_t& ev);
  void doRhHistos        (HFtrigAnalEvent_t& ev);

  void filterRHs         (const HFRecHitCollection&      unfiltrh,
			  std::vector<HFRecHit>&         filtrh);

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
  double                        minGeVperRecHitShort_;
  double                        minGeVperRecHitLong_;
  double                        maxGeVperRecHit4BeamGasMedium_;
  double                        maxGeVperRecHit4BeamGasTight_;
  double                        minGeVperRecHit4PMT_;
  double                        minGeVperTower_;
  double                        totalRHenergyThresh4Plotting_;
  uint32_t                      adcTrigThreshold_;
  double                        shortEwindowMinGeV_;
  double                        shortEwindowMaxGeV_;

  HFtrigAnalHistos             *histos_;
  linearizerLUT                *lut_;
  int                           nTotalNonPMTevents_;
};

#endif // _HFTRIGANALALGOS_HH
