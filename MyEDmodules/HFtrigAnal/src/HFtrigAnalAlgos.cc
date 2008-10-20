#include <iostream>
#include "MyEDmodules/HFtrigAnal/src/HFtrigAnalAlgos.hh"
#include "MyEDmodules/HFtrigAnal/src/inSet.hh"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "math.h"
#include "TMath.h"
#include "TF1.h"

using namespace std;

// <use name=FWCore/ServiceRegistry> # GF: Not sure whether really needed...

class compareADC {
public:
  bool operator()(const triggerWedge_t& w1,
		  const triggerWedge_t& w2) const {
    return w1.maxadc>w2.maxadc;
  }
};//needed for sorting by maxadc

//======================================================================
// gives delta iphi in range 0-36, doesn't check if iphi1, iphi2 in range
//
static int deltaIphiUnsigned(int iphi1, int iphi2)
{
  int dPhi=abs(iphi1-iphi2);
  if (dPhi>36) dPhi=72-dPhi;
  return dPhi;
}

//======================================================================
// gives delta iphi in range -36->35, doesn't check if iphi1, iphi2 in range
//
static int deltaIphiSigned(int iphi1, int iphi2)
{
  int dPhi=iphi2-iphi1;
  if (dPhi<-36) dPhi += 72;
  if (dPhi> 35) dPhi -= 72;
  return dPhi;
}

//======================================================================

static float averageIphi(int iphi1, int iphi2)
{
  int sum = iphi1+iphi2;
  int dPhi=abs(iphi1-iphi2);
  if (dPhi > 36) sum += 72;
  float avgphi = (float)sum/2.0;

  if (avgphi > 72.0) avgphi -= 72.0;

  return avgphi;
}

//======================================================================

inline bool HFtrigAnalAlgos::isGoodBx(uint16_t bxnum) {
  return inSet<uint16_t>(s_validBxNums_,bxnum);
}

//======================================================================

HFtrigAnalAlgos::HFtrigAnalAlgos(bool verbosity,
				 const edm::ParameterSet& iConfig) :
  verbose_(verbosity)
{
  vector<int> v_maskidnumbers;
  vector<int> v_validBXnumbers;

  sampleWindowLeft_        = iConfig.getParameter<int>("digiSampleWindowMin");
  sampleWindowRight_       = iConfig.getParameter<int>("digiSampleWindowMax");

  minGeVperRecHitLong_     = iConfig.getParameter<double>("minGeVperRecHitLong");
  minGeVperRecHitShort_    = iConfig.getParameter<double>("minGeVperRecHitShort");
  minGeVperTower_          = iConfig.getParameter<double>("minGeVperTower");

  maxGeVperRecHit4BeamGasMedium_ = iConfig.getParameter<double>("maxGeVperRecHit4BeamGasMedium");
  maxGeVperRecHit4BeamGasTight_  = iConfig.getParameter<double>("maxGeVperRecHit4BeamGasTight");
  totalRHenergyThresh4Plotting_  = iConfig.getParameter<double>("totalEthresh4eventPlotting");

  minGeVperRecHit4PMT_     = iConfig.getParameter<double>("minGeVperRecHit4PMT");

  //eventNumberMin_        = (uint32_t)iConfig.getParameter<int>("eventNumberMin");
  //eventNumberMax_        = (uint32_t)iConfig.getParameter<int>("eventNumberMax");

  adcTrigThreshold_        = (uint32_t)iConfig.getParameter<int>("adcTrigThreshold");
  v_maskidnumbers          = iConfig.getParameter<vector<int> > ("detIds2Mask");
  v_validBXnumbers         = iConfig.getParameter<vector<int> > ("validBxNumbers");

  shortEwindowMinGeV_      =  iConfig.getParameter<double>("shortEwindowMinGeV");
  shortEwindowMaxGeV_      =  iConfig.getParameter<double>("shortEwindowMaxGeV");

  if (!convertIdNumbers(v_maskidnumbers, detIds2mask_))
    throw cms::Exception("Invalid detID vector");

  for (uint32_t i=0; i<v_validBXnumbers.size(); i++)
    s_validBxNums_.insert((uint16_t)v_validBXnumbers[i]);

  lut_ = new linearizerLUT(iConfig,detIds2mask_);
  histos_ = new HFtrigAnalHistos(iConfig);

  nTotalNonPMTevents_ = 0;
}                                    // HFtrigAnalAlgos::HFtrigAnalAlgos

//======================================================================

void
HFtrigAnalAlgos::beginJob(void)
{
  histos_->beginJob();

  if (!lut_->readFromTextFile()) {
    throw cms::Exception("Reading LUT text file failed");
  }
  lut_->dump();

}                                           // HFtrigAnalAlgos::beginJob

//======================================================================

bool HFtrigAnalAlgos::convertIdNumbers(std::vector<int>& v_maskidnumbers,
				       std::vector<HcalDetId>& detIds2mask)
{
  // convert det ID numbers to valid detIds:
  if (v_maskidnumbers.size()%3) {
    return false;
  }
  for (uint32_t i=0; i<v_maskidnumbers.size(); i+=3) {
    int ieta = v_maskidnumbers[i];
    int iphi = v_maskidnumbers[i+1];
    int depth = v_maskidnumbers[i+2];
    if ((abs(ieta) < 29) || (abs(ieta) > 41)) return false;
    if (    (iphi  <  0) ||    (iphi   > 71)) return false;
    if (   (depth !=  1) &&    (depth !=  2)) return false;
    HcalDetId id(HcalForward,ieta,iphi,depth);
    detIds2mask.push_back(id);
  }
  return true;
}                                   // HFtrigAnalAlgos::convertIdNumbers

//======================================================================

bool HFtrigAnalAlgos::maskedId(const HcalDetId& id)
{
  for (uint32_t i=0; i<detIds2mask_.size(); i++)
    if (id == detIds2mask_[i]) return true;
  return false;
}

//======================================================================

void
HFtrigAnalAlgos::endJob()
{
  histos_->endJob();
}

//======================================================================

void HFtrigAnalAlgos::filterRHs(const HFRecHitCollection& unfiltrh,
				std::vector<HFRecHit>& filtrh)
{
  HFRecHitCollection::const_iterator it;

  for (it=unfiltrh.begin(); it!=unfiltrh.end(); it++){
    double threshold =
      (it->id().depth() == 1) ? minGeVperRecHitLong_ : minGeVperRecHitShort_;
      
    if (!maskedId(it->id()) && 
	(it->energy() > threshold))
      filtrh.push_back(*it);
  }
}                                        //  HFtrigAnalAlgos::filterRHs

//======================================================================

void HFtrigAnalAlgos::findMaxWedges(HFtrigAnalEvent_t& ev)
{
  // First collect the highest ADC value (from LUT output) per wedge
  //
  std::map<int,triggerWedge_t> m_wedges;
  for (unsigned idig = 0; idig < ev.hfdigis.size (); ++idig) {
    const HFDataFrame& frame = ev.hfdigis[idig];
    HcalDetId detId = frame.id();

    int zside = detId.zside();
    int iphi  = detId.iphi();
    HFwedgeID_t wid(iphi,zside);

    int isample;
    uint32_t max4thisdigi=0;
    int maxsamplenum = -1;
    for (isample = std::max(0,sampleWindowLeft_);
	 isample <= std::min(sampleWindowRight_,frame.size()-1);
	 ++isample) {

      uint32_t lutval = lut_->lookupVal(detId,frame[isample].adc());

      if (lutval > max4thisdigi) {
	max4thisdigi = lutval;
	maxsamplenum = isample;
      }
    } // loop over samples in digi
      
    triggerWedge_t twedge(wid,frame,max4thisdigi,maxsamplenum);
    std::map<int,triggerWedge_t>::iterator it =  m_wedges.find(wid.id());

    if (it == m_wedges.end()) m_wedges.insert(std::pair<int,triggerWedge_t>(wid.id(),twedge));
    else if (max4thisdigi > it->second.maxadc) it->second = twedge;

  } // loop over digi collection

  // Now push these into the wedge collection and sort:
  ev.sortedWedges.clear();
  std::map<int,triggerWedge_t>::iterator it;
  for (it =  m_wedges.begin(); it != m_wedges.end(); it++)
    ev.sortedWedges.push_back(it->second);

  // sort by decreasing maxADC
  //
  std::sort(ev.sortedWedges.begin(), ev.sortedWedges.end(), compareADC());


#if 0
  cout << "Max/Next2Max frames: ";
  cout << maxframe.id() << " ADC = " << maxval << "; ";
  cout << next2maxframe.id() << " ADC = " << next2maxval << "; ";
  cout << endl;
#endif
}                                      // HFtrigAnalAlgos::findMaxWedges

//======================================================================

void HFtrigAnalAlgos::dumpWedges(std::vector<triggerWedge_t>& wedges)
{
  cout << " =======> Wedges: <========" << endl;
  for (uint32_t i = 0; i<wedges.size(); i++) {
    cout << i << ": wid = "          << wedges[i].wid.id();
    cout      << ", "                << wedges[i].frame.id();
    cout      << ", maxadc = "       << wedges[i].maxadc;
    cout      << ", maxsamplenum = " << wedges[i].maxsamplenum << endl;
  }
}

//======================================================================

void HFtrigAnalAlgos::countNwedgesOver(uint32_t threshold,
				       std::vector<triggerWedge_t>& sortedWedges,
				       int& nWtotal,
				       int& nWplus,
				       int& nWminus)
{
  nWtotal = 0;
  nWplus  = 0;
  nWminus = 0;

  for (uint32_t i = 0; i<sortedWedges.size(); i++) {
    if (sortedWedges[i].maxadc < threshold) break;
    nWtotal++;
    if (sortedWedges[i].wid.id() < 0) nWminus++;
    else                              nWplus++;
  }
}                                   // HFtrigAnalAlgos::countNwedgesOver

//======================================================================

void HFtrigAnalAlgos::doNwedges(std::vector<triggerWedge_t>& sortedWedges,
				  uint16_t bxnum)
{
  int nWedgesOverThresh = 0;
  int nWedgesOverThreshPlus = 0;
  int nWedgesOverThreshMinus = 0;

  countNwedgesOver(adcTrigThreshold_,sortedWedges, nWedgesOverThresh,
		   nWedgesOverThreshPlus, nWedgesOverThreshMinus);

  histos_->fillNwedges(isGoodBx(bxnum),
		       nWedgesOverThresh,
		       nWedgesOverThreshPlus,
		       nWedgesOverThreshMinus);

  // Now generate trigger efficiency plot as a function of threshold:
  for (uint32_t thresh=0; thresh<1023; thresh++) {
    countNwedgesOver(thresh,sortedWedges, nWedgesOverThresh,
		     nWedgesOverThreshPlus, nWedgesOverThreshMinus);

    if (nWedgesOverThresh >= 2) {
      histos_->fillTrigEffHistos(isGoodBx(bxnum),thresh);
    }
    else if (!nWedgesOverThresh)
      break;
  }
}                                         //  HFtrigAnalAlgos::doNwedges

//======================================================================

void HFtrigAnalAlgos::doDigiSpectra(const triggerWedge_t& maxwedge,
				    uint32_t runnum)
{
  HcalDetId detId = maxwedge.frame.id();
  histos_->fillDigiSpectra(detId.ieta(),detId.depth(),
			   maxwedge.maxadc,runnum);

}                                      // HFtrigAnalAlgos::doDigiSpectra

//======================================================================

void HFtrigAnalAlgos::doOccupancy(const vector<triggerWedge_t>& sortedWedges,
				    uint32_t runnum)
{
  HcalDetId idA = sortedWedges[0].frame.id();
  HcalDetId idB = HcalDetId::Undefined;

  if (sortedWedges.size() > 1)
    idB = sortedWedges[1].frame.id();

  histos_->fillOccupancyHistos(idA, idB, runnum);

}                                       //  HFtrigAnalAlgos::doOccupancy

//======================================================================

void HFtrigAnalAlgos::doPulseProfile(const HFDataFrame& maxframe)
{
  vector<int> v_samples;
  for (int isample = 0; isample < std::min(10,maxframe.size()); ++isample)
    v_samples.push_back(maxframe[isample].adc());

  histos_->fillPulseProfileHisto(v_samples);

}                                     // HFtrigAnalAlgos::doPulseProfile

//======================================================================

void HFtrigAnalAlgos::sumEnergies(const std::vector<HFRecHit>& hfrechits,
				  vector<TowerEnergies_t>& v_towers,
				  float& totalE)
{
  map<int,TowerEnergies_t> m_TowerEnergies;

  // Total energy plots
  //
  float sumEtimesPhiPl = 0.0;
  float sumEtimesPhiMn = 0.0;
  float sumEtimesEtaPl = 0.0;
  float sumEtimesEtaMn = 0.0;
  float totalEplus     = 0.0;
  float totalEminus    = 0.0;

  totalE = 0.0;

  for (unsigned ihit = 0; ihit < hfrechits.size (); ++ihit) {
    const HFRecHit& rh  = hfrechits[ihit];
    const HcalDetId& id = rh.id();
    int   ieta          = id.ieta();
    int   iphi          = id.iphi();
    float rhenergy      = rh.energy();

    // total energy first
    //
    totalE += rh.energy();

    histos_->fillRHetaPhiDistHistos(ieta, iphi, rhenergy);

    if (rhenergy > 1000.0) {
      if (id.zside() > 0) {
	sumEtimesPhiPl += rhenergy * iphi;
	sumEtimesEtaPl += rhenergy * ieta;
	totalEplus     += rhenergy;
      } else {
	sumEtimesPhiMn += rhenergy * iphi;
	sumEtimesEtaMn += rhenergy * ieta;
	totalEminus    += rhenergy;
      }
    }

    // add up tower energies
    IetaIphi_t ieip(id.ieta(),id.iphi());
    map<int,TowerEnergies_t>::iterator it = m_TowerEnergies.find(ieip.toCode());
    TowerEnergies_t tower;

    tower.ieip = ieip;

    if (verbose_) {
      cout << "adding RecHit id " << id << " " << rhenergy << " GeV to tower (";
      cout << tower.ieip.ieta() << "," << tower.ieip.iphi() << ")" << endl;
    }

    if (it == m_TowerEnergies.end()) {
      tower.totalE = rhenergy;
      if      (id.depth() == 1) tower.longE  = rhenergy;
      else if (id.depth() == 2) tower.shortE = rhenergy;
      m_TowerEnergies[ieip.toCode()] = tower;
    }
    else {
      it->second.totalE += rhenergy;
      if      (id.depth() == 1) it->second.longE  += rhenergy;
      else if (id.depth() == 2) it->second.shortE += rhenergy;
    }
  }

  map<int,TowerEnergies_t>::iterator it;
  for (it  = m_TowerEnergies.begin();
       it != m_TowerEnergies.end(); it++)
    v_towers.push_back(it->second);


  // Center of energy plots
  //
  float coEinPhiMinus = (totalEminus != 0.0) ? (sumEtimesPhiMn/totalEminus) : -1.0;
  float coEinEtaMinus = (totalEminus != 0.0) ? (sumEtimesEtaMn/totalEminus) :  0.0;
  float coEinPhiPlus  = (totalEplus  != 0.0) ? (sumEtimesPhiPl/totalEplus)  : -1.0;
  float coEinEtaPlus  = (totalEplus  != 0.0) ? (sumEtimesEtaPl/totalEplus)  :  0.0;

  histos_->fillCenterOfEnergyHistos(coEinPhiMinus,coEinPhiPlus,
				    coEinEtaMinus,coEinEtaPlus);

}                                        // HFtrigAnalAlgos::sumEnergies

//======================================================================

void HFtrigAnalAlgos::doEventDisplays(const std::vector<HFRecHit>& hfrechits,
				      int evtnum,int runnum,float totalE)
{
  // Event display plotting
  //
  if (totalE >  totalRHenergyThresh4Plotting_) {
    TH2F *h2p = histos_->book2dEnergyHisto(evtnum,runnum);

    for (unsigned ihit = 0; ihit < hfrechits.size (); ++ihit) {
      const HFRecHit& rh = hfrechits[ihit];
      HcalDetId detId    = rh.id();

      int ieta2fill = 0;
      if      (detId.ieta() >=  29 ) ieta2fill = detId.ieta()-28;
      else if (detId.ieta() <= -29 ) ieta2fill = detId.ieta()+28;

      h2p->Fill(ieta2fill,(detId.iphi()+detId.depth()-1),rh.energy());
    }
  }
}                                    // HFtrigAnalAlgos::doEventDisplays

//======================================================================

bool HFtrigAnalAlgos::towerConfirmedHit(const TowerEnergies_t& tower)
{
  double shortminGeV = max(minGeVperRecHitShort_,(tower.longE)/10.0);
  double longminGeV  = max(minGeVperRecHitLong_, (tower.longE)/5.0);

  return ((tower.longE  >= longminGeV) &&
	  (tower.shortE >= shortminGeV)   );
}

//======================================================================

void HFtrigAnalAlgos::filterTowers(const
				   vector<TowerEnergies_t>&  v_towers,
				   vector<TowerEnergies_t>&  v_towersOverThresh,
				   vector<TowerEnergies_t>&  v_PMThits,
				   bool&                     oneConfirmedHit,
				   double&                   maxGeVfound)
{
  // Tower Energy plots:
  // 1) collect towers over threshold
  // 2) collect PMT hits
  // 3) determine if there's a confirmed hit
  //
  oneConfirmedHit = false;
  maxGeVfound     = 0.0;

  v_towersOverThresh.clear();

  for (uint32_t i=0; i<v_towers.size(); i++) {
    const TowerEnergies_t& t = v_towers[i];
    bool confirmedHit  = false;

    confirmedHit    = towerConfirmedHit(t);
    oneConfirmedHit = oneConfirmedHit || confirmedHit;

    if (confirmedHit) {

      // poor mans attempt at counting cluster cores...
      if (t.totalE > minGeVperTower_) // used for beam-gas cluster counting
	v_towersOverThresh.push_back(t);

      if (t.shortE > maxGeVfound) maxGeVfound = t.shortE;
      if (t.longE  > maxGeVfound) maxGeVfound = t.longE;

    } else {
      // collect all towers with PMT hits:
      if ((t.shortE > minGeVperRecHit4PMT_) ||
	  (t.longE  > minGeVperRecHit4PMT_)   )
	v_PMThits.push_back(t);
    }
  }
}                                       // HFtrigAnalAlgos::filterTowers

//======================================================================

void HFtrigAnalAlgos::doPMThistos(HFtrigAnalEvent_t& ev)
{
  //  calc pair-wise delta eta and phi PMT hit pairs, keeping
  //  same-side and opposite-side separated. Plot delta ieta,delta iphi
  //
  vector<deltaAvg_t> v_opPMTdas, v_ssPMTdas;
#if 0
  for (uint32_t i=0; i<ev.PMThits.size()-1; i++) {
    for (uint32_t j=i+1; j< ev.PMThits.size(); j++) {
      IetaIphi_t& ieip0 = ev.PMThits[i].ieip;
      IetaIphi_t& ieip1 = ev.PMThits[j].ieip;
      double    totalE0 = ev.PMThits[i].totalE;
      double    totalE1 = ev.PMThits[j].totalE;
	
      deltaAvg_t da;

      da.avgIeta   = (float)(abs(ieip0.ieta()) + abs(ieip1.ieta()))/2.0;
      da.avgIphi   = averageIphi(ieip0.iphi(),ieip1.iphi());

      if ( (ieip0.ieta() < 0) && (ieip1.ieta() > 0) ) {
	da.deltaIeta = abs(ieip1.ieta()) - abs(ieip0.ieta());
	da.deltaIphi = deltaIphiSigned(ieip0.iphi(),ieip1.iphi());
	v_opPMTdas.push_back(da);
      } else
      if ( (ieip1.ieta() < 0) && (ieip0.ieta() > 0) ) {
	da.deltaIeta = abs(ieip0.ieta()) - abs(ieip1.ieta());
	da.deltaIphi = deltaIphiSigned(ieip1.iphi(),ieip0.iphi());
	v_opPMTdas.push_back(da);
      } else                      // same side, order by decreasing energy
      if (totalE0>totalE1) {
	da.deltaIeta = abs(ieip1.ieta()) - abs(ieip0.ieta());
	da.deltaIphi = deltaIphiSigned(ieip0.iphi(),ieip1.iphi());
	v_ssPMTdas.push_back(da);
      } else {
	da.deltaIeta = abs(ieip0.ieta()) - abs(ieip1.ieta());
	da.deltaIphi = deltaIphiSigned(ieip0.iphi(),ieip1.iphi());
	v_ssPMTdas.push_back(da);
      }
    }
  }
#endif
  histos_->fillPMTeventHistos(ev.PMThits,v_ssPMTdas,v_opPMTdas,ev.lsnum,ev.isGoodBx);

}                                        // HFtrigAnalAlgos::doPMThistos

//======================================================================

void HFtrigAnalAlgos::doRhHistos(HFtrigAnalEvent_t& ev)
{
  vector<TowerEnergies_t>  v_towers;
  float                    totalE;
  double                   maxHitGeV;
  bool                     confirmedHit = false;

  sumEnergies(ev.filtrh, v_towers, totalE);

  filterTowers(v_towers, ev.towersOverThresh, ev.PMThits,
	       confirmedHit, maxHitGeV);

  histos_->fillTowerEhistos(ev.towersOverThresh.size(),v_towers,ev.isGoodBx);

  if (ev.isGoodBx) {
    histos_->fillTotalEnergyHistos(ev.evtnum,ev.runnum,totalE);

    doEventDisplays(ev.filtrh,ev.evtnum,ev.runnum,totalE);

    histos_->fillnTowersOverThresh(ev.towersOverThresh.size());

    // look at long E distribution for high shortE towers
    // - helps determine cutoff between confirmed/unconfirmed hits
    //
    for (uint32_t i=0; i<v_towers.size(); i++) {
      TowerEnergies_t& t = v_towers[i];
      if ((t.shortE > shortEwindowMinGeV_) &&
	  (t.shortE < shortEwindowMaxGeV_)   )
	histos_->fillLongEhisto(t.longE);
    }

    // BEAM-GAS HISTOS:
    // Tight: no pmt,     no hits over ~100, at least 5 towers over threshold
    // Medium: no pmt,    no hits over ~200, at least 3 towers over threshold
    // Loose: allow 1-2 pmt, hits over ~100, at least 3 towers over threshold
    //
    if ((ev.towersOverThresh.size() > 2) &&
	(ev.PMThits.size()          < 3)    ) {             // Loose
      histos_->fillLooseBeamGasHistos(ev);

      if (!ev.PMThits.size() &&
	  (maxHitGeV < maxGeVperRecHit4BeamGasMedium_)) {   // Medium
	histos_->fillMediumBeamGasHistos(ev);

	if ((ev.towersOverThresh.size() > 4) &&
	    (maxHitGeV < maxGeVperRecHit4BeamGasTight_))    // Tight
	  histos_->fillTightBeamGasHistos(ev);
      }
    }
  }

  if (confirmedHit) {
    if (ev.isGoodBx) nTotalNonPMTevents_++;
  }
  else 
    doPMThistos(ev);
}                                         // HFtrigAnalAlgos::doRhHistos

//======================================================================

void HFtrigAnalAlgos::analyze(HFtrigAnalEvent_t& ev)
{
  //std::vector<HFDataFrame> maxdigis;
  HFDataFrame maxframe, next2maxframe;
  uint32_t maxadc;

  if (notInSet<uint32_t>(s_runs_,ev.runnum)) {
    histos_->bookPerRunHistos(ev.runnum);
    s_runs_.insert(ev.runnum);
  }

  findMaxWedges(ev);
  //dumpWedges(sortedWedges);

  maxadc = ev.sortedWedges[0].maxadc;
#if 0
  if (maxadc < adcTrigThreshold_) {
    cout << " maxadc = " << maxadc << " less than adcTrigThreshold_ " << adcTrigThreshold_;
    cout << "; HF Digi Collection size = " << hfdigis.size() << endl;
  } else if (evtnum == 1) 
    cout << "HF Digi Collection size = " << hfdigis.size() << endl;

  if (maxadc < 5) return;
#endif

  ev.isGoodBx = isGoodBx(ev.bxnum);
  histos_->fillEvtInfoHistos(ev);

  doNwedges      (ev.sortedWedges,ev.bxnum);
  doDigiSpectra  (ev.sortedWedges[0],ev.runnum);
  doPulseProfile (ev.sortedWedges[0].frame);
  doOccupancy    (ev.sortedWedges,ev.runnum);

  filterRHs(ev.hfrechits,ev.filtrh);
  doRhHistos(ev);
}                                            // HFtrigAnalAlgos::analyze

//======================================================================
