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
  bool operator()(const HFtrigAnalAlgos::triggerWedge_t& w1,
		  const HFtrigAnalAlgos::triggerWedge_t& w2) const {
    return w1.maxadc>w2.maxadc;
  }
};//needed for sorting by maxadc

//======================================================================

static int deltaIphi(int iphi1, int iphi2)
{
  int dPhi=abs(iphi1-iphi2);
  if (dPhi>36) dPhi=72-dPhi;
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

  sampleWindowLeft_    = iConfig.getParameter<int>("digiSampleWindowMin");
  sampleWindowRight_   = iConfig.getParameter<int>("digiSampleWindowMax");

  minGeVperRecHit_     = iConfig.getParameter<double>("minGeVperRecHit");

  totalRHenergyThresh4Plotting_ = iConfig.getParameter<double>("totalEthresh4eventPlotting");

  //eventNumberMin_       = (uint32_t)iConfig.getParameter<int>("eventNumberMin");
  //eventNumberMax_       = (uint32_t)iConfig.getParameter<int>("eventNumberMax");

  adcTrigThreshold_       = (uint32_t)iConfig.getParameter<int>("adcTrigThreshold");
  v_maskidnumbers         = iConfig.getParameter<vector<int> > ("detIds2Mask");
  v_validBXnumbers        = iConfig.getParameter<vector<int> > ("validBxNumbers");

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
  if (v_maskidnumbers.empty() || (v_maskidnumbers.size()%3)) {
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
  histos_->endJob(nTotalNonPMTevents_);
}

//======================================================================

void HFtrigAnalAlgos::filterRHs(const HFRecHitCollection& unfiltrh,
				std::vector<HFRecHit>& filtrh)
{
  HFRecHitCollection::const_iterator it;

  for (it=unfiltrh.begin(); it!=unfiltrh.end(); it++){
    if (!maskedId(it->id()) && 
	(it->energy() > minGeVperRecHit_))
      filtrh.push_back(*it);
  }
}                                        //  HFtrigAnalAlgos::filterRHs

//======================================================================

void HFtrigAnalAlgos::findMaxWedges(const HFDigiCollection& hfdigis,
				    std::vector<triggerWedge_t>& sortedWedges)
{
  // First collect the highest ADC value (from LUT output) per wedge
  //
  std::map<int,triggerWedge_t> m_wedges;
  for (unsigned idig = 0; idig < hfdigis.size (); ++idig) {
    const HFDataFrame& frame = hfdigis[idig];
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
  sortedWedges.clear();
  std::map<int,triggerWedge_t>::iterator it;
  for (it =  m_wedges.begin(); it != m_wedges.end(); it++)
    sortedWedges.push_back(it->second);

  // sort by decreasing maxADC
  //
  std::sort(sortedWedges.begin(), sortedWedges.end(), compareADC());


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
}                                       //  HFtrigAnalAlgos::fillNwedges

//======================================================================

void HFtrigAnalAlgos::doDigiSpectra(const triggerWedge_t& maxwedge,
				    uint32_t runnum)
{
  HcalDetId detId = maxwedge.frame.id();
  histos_->fillDigiSpectra(detId.ieta(),detId.depth(),
			   maxwedge.maxadc,runnum);

}                                   // HFtrigAnalAlgos::doDigiSpectra

//======================================================================

void HFtrigAnalAlgos::doOccupancy(const vector<triggerWedge_t>& sortedWedges,
				    uint32_t runnum)
{
  HcalDetId idA = sortedWedges[0].frame.id();
  HcalDetId idB = HcalDetId::Undefined;

  if (sortedWedges.size() > 1)
    idB = sortedWedges[1].frame.id();

  histos_->fillOccupancyHistos(idA, idB, runnum);

}                                     //  HFtrigAnalAlgos::doOccupancy

//======================================================================

void HFtrigAnalAlgos::doPulseProfile(const HFDataFrame& maxframe)
{
  vector<int> v_samples;
  for (int isample = 0; isample < std::min(10,maxframe.size()); ++isample)
    v_samples.push_back(maxframe[isample].adc());

  histos_->fillPulseProfileHisto(v_samples);

}                                   // HFtrigAnalAlgos::doPulseProfile

//======================================================================

void HFtrigAnalAlgos::sumEnergies(const std::vector<HFRecHit>& hfrechits,
				  map<int,TowerEnergies_t>& m_TowerEnergies,
				  float& totalE,
				  float& coEinPhiMinus,
				  float& coEinEtaMinus,
				  float& coEinPhiPlus,
				  float& coEinEtaPlus)
{
  // Total energy plots
  //
  float sumEtimesPhiPl = 0.0;
  float sumEtimesPhiMn = 0.0;
  float sumEtimesEtaPl = 0.0;
  float sumEtimesEtaMn = 0.0;
  float totalEplus     = 0.0;
  float totalEminus    = 0.0;

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

  // Center of energy plots
  //
  coEinPhiMinus = (totalEminus != 0.0) ? (sumEtimesPhiMn/totalEminus) : -1.0;
  coEinEtaMinus = (totalEminus != 0.0) ? (sumEtimesEtaMn/totalEminus) :  0.0;
  coEinPhiPlus  = (totalEplus  != 0.0) ? (sumEtimesPhiPl/totalEplus)  : -1.0;
  coEinEtaPlus  = (totalEplus  != 0.0) ? (sumEtimesEtaPl/totalEplus)  :  0.0;

}                                        // HFtrigAnalAlgos::sumEnergies

//======================================================================

void HFtrigAnalAlgos::doRhHistos(const std::vector<HFRecHit>& hfrechits,
				   uint32_t evtnum,
				   uint32_t runnum)
{
  map<int,TowerEnergies_t> m_TowerEnergies;

  float totalE;
  float coEinPhiMinus;
  float coEinEtaMinus;
  float coEinPhiPlus;
  float coEinEtaPlus;

  sumEnergies(hfrechits, m_TowerEnergies,
	      totalE, coEinPhiMinus, coEinEtaMinus, coEinPhiPlus, coEinEtaPlus);

  histos_->fillTotalEnergyHistos(evtnum,runnum,totalE);
  histos_->fillCenterOfEnergyHistos(coEinPhiMinus,coEinPhiPlus,
				    coEinEtaMinus,coEinEtaPlus);

  // Event display plotting
  //
  if (totalE >  totalRHenergyThresh4Plotting_) {
    TH2F *h2p = histos_->book2dEnergyHisto(evtnum,runnum);

    for (unsigned ihit = 0; ihit < hfrechits.size (); ++ihit) {
      const HFRecHit& rh = hfrechits[ihit];
      HcalDetId detId = rh.id();

      int ieta2fill = 0;
      if      (detId.ieta() >=  29 ) ieta2fill = detId.ieta()-28;
      else if (detId.ieta() <= -29 ) ieta2fill = detId.ieta()+28;

      if (rh.energy() > minGeVperRecHit_)
	h2p->Fill(ieta2fill,(detId.iphi()+detId.depth()-1),rh.energy());
    }
  }

  // Tower Energy plots:
  // 1) count up # towers over threshold
  // 2) collect PMT hits
  //
  vector<TowerEnergies_t> v_PMThits;
  map<int,TowerEnergies_t>::const_iterator it;
  int ntowers = 0;
  for (it  = m_TowerEnergies.begin();
       it != m_TowerEnergies.end(); it++) {
    TowerEnergies_t t = it->second;
    if (t.totalE > 20.0) ntowers++;

    // collect all towers with PMT hits:
    if (((t.shortE > 20.0) && (t.longE  < 10.0)) ||
	((t.longE  > 20.0) && (t.shortE < 10.0))   ) { // PMT hit?
      v_PMThits.push_back(t);
    }
  }

  histos_->fillNtowersHisto(ntowers);

  for (it  = m_TowerEnergies.begin();
       it != m_TowerEnergies.end(); it++) {
    IetaIphi_t ieip(it->first);
    TowerEnergies_t t = it->second;
    histos_->fillTowerEhistos(ntowers, ieip.ieta(),t.totalE, t.shortE, t.longE);
  }

  //  now we're looking for a pair of PMT hits, one in plus and one in minus:

  bool pmPMTevent = false;
  int   dltIphi=0,  dltIeta=0;
  float avgIphi=0., avgIeta=0.;
  if (v_PMThits.size() == 2) {
    IetaIphi_t ieip0 = v_PMThits[0].ieip;
    IetaIphi_t ieip1 = v_PMThits[1].ieip;
    if ( ((ieip0.ieta() < 0) && (ieip1.ieta() > 0)) ||
	 ((ieip1.ieta() < 0) && (ieip0.ieta() > 0))   ) {
      dltIphi    = deltaIphi(ieip0.iphi(),ieip1.iphi());
      dltIeta    = abs(abs(ieip0.ieta()) - abs(ieip1.ieta()));
      avgIphi    = averageIphi(ieip0.iphi(),ieip1.iphi());
      avgIeta    = (float)(abs(ieip0.ieta()) + abs(ieip1.ieta()))/2.0;
      pmPMTevent = true;
    }
  }

  histos_->fillPMTeventHistos(pmPMTevent,
			      dltIphi,dltIeta,
			      avgIphi,avgIeta,
			      v_PMThits.size());

}                                         // HFtrigAnalAlgos::doRhHistos

//======================================================================

void HFtrigAnalAlgos::analyze(const HFDigiCollection&   hfdigis,
			      const HFRecHitCollection& hfrechits,
			      uint16_t bxnum,
			      uint32_t evtnum,
			      uint32_t runnum,
			      uint32_t lsnum)
{
  //std::vector<HFDataFrame> maxdigis;
  HFDataFrame maxframe, next2maxframe;
  uint32_t maxadc;

  if (notInSet<uint32_t>(s_runs_,runnum)) {
    histos_->bookPerRunHistos(runnum);
    s_runs_.insert(runnum);
  }

  std::vector<triggerWedge_t> sortedWedges;

  findMaxWedges(hfdigis, sortedWedges);
  //dumpWedges(sortedWedges);

  maxadc = sortedWedges[0].maxadc;
#if 0
  if (maxadc < adcTrigThreshold_) {
    cout << " maxadc = " << maxadc << " less than adcTrigThreshold_ " << adcTrigThreshold_;
    cout << "; HF Digi Collection size = " << hfdigis.size() << endl;
  } else if (evtnum == 1) 
    cout << "HF Digi Collection size = " << hfdigis.size() << endl;

  if (maxadc < 5) return;
#endif

  histos_->fillEvtInfoHistos(bxnum,lsnum,isGoodBx(bxnum));

  doNwedges      (sortedWedges,bxnum);
  doDigiSpectra  (sortedWedges[0],runnum);
  doPulseProfile (sortedWedges[0].frame);
  doOccupancy    (sortedWedges,runnum);

  std::vector<HFRecHit> filtrh;
  filterRHs(hfrechits, filtrh);
  if (isGoodBx(bxnum))
    doRhHistos(filtrh,evtnum,runnum);
}                                            // HFtrigAnalAlgos::analyze

//======================================================================
