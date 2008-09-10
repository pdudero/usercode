#include <iostream>
#include "MyEDmodules/HFtrigAnal/src/HFtrigAnalAlgos.hh"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "math.h"
#include "TMath.h"
#include "TF1.h"

using namespace std;

// <use name=FWCore/ServiceRegistry> # GF: Not sure whether really needed...


//======================================================================
// Got this from
// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html

void Tokenize(const string& str,
	      vector<string>& tokens,
	      const string& delimiters = " ")
{
  // Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos) {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}                                                            // Tokenize

//======================================================================

HFtrigAnalAlgos::HFtrigAnalAlgos(bool verbosity,
                               const edm::ParameterSet& iConfig) :
  verbose_(verbosity)
{
  lutFileName_     = iConfig.getUntrackedParameter<string>("lutFileName");

  sampleWindowLeft_    = iConfig.getParameter<int>("digiSampleWindowMin");
  sampleWindowRight_   = iConfig.getParameter<int>("digiSampleWindowMax");

  minGeVperRecHit_     = iConfig.getParameter<double>("minGeVperRecHit");

  totalRHenergyThresh4Plotting_ = iConfig.getParameter<double>("totalEthresh4eventPlotting");

  //eventNumberMin_       = (uint32_t)iConfig.getParameter<int>("eventNumberMin");
  //eventNumberMax_       = (uint32_t)iConfig.getParameter<int>("eventNumberMax");

  adcTrigThreshold_       = (uint32_t)iConfig.getParameter<int>("adcTrigThreshold");

  digiSpectrumHp_.nbins = iConfig.getUntrackedParameter<int>   ("digiSpectrumNbins");
  digiSpectrumHp_.min   = iConfig.getUntrackedParameter<double>("digiSpectrumMinADC");
  digiSpectrumHp_.max   = iConfig.getUntrackedParameter<double>("digiSpectrumMaxADC");

  rhTotalEnergyHp_.nbins= iConfig.getUntrackedParameter<int>   ("rhTotalEnergyNbins");
  rhTotalEnergyHp_.min  = iConfig.getUntrackedParameter<double>("rhTotalEnergyMinGeV");
  rhTotalEnergyHp_.max  = iConfig.getUntrackedParameter<double>("rhTotalEnergyMaxGeV");

}                                    // HFtrigAnalAlgos::HFtrigAnalAlgos

//======================================================================
#define NUMEBINS 46
static const double logEbins[NUMEBINS+1] = {
  0.0,10.0,20.0,30.0,40.0,50.0,60.0,70.0,80.0,90.0,
  100.0,200.0,300.0,400.0,500.0,600.0,700.0,800.0,900.0,
  1000.0,2000.0,3000.0,4000.0,5000.0,6000.0,7000.0,8000.0,9000.0,
  10000.0,20000.0,30000.0,40000.0,50000.0,60000.0,70000.0,80000.0,90000.0,
  100000.0,200000.0,300000.0,400000.0,500000.0,600000.0,700000.0,800000.0,900000.0,
  1000000.0
};

void
HFtrigAnalAlgos::beginJob(void)
{
  edm::Service<TFileService> fs;

  DigiSubDir_ = new TFileDirectory(fs->mkdir( "TPGET_spectra" ));
  RHsubDir_   = new TFileDirectory(fs->mkdir( "RecHitEnergies" ));

  if (!readLUTfromTextFile()) {
    throw cms::Exception("Reading LUT text file failed");
  }
  dumpLUT();

  detId2Mask_ = HcalDetId(HcalForward,32,67,2);

}                                           // HFtrigAnalAlgos::beginJob

//======================================================================

void HFtrigAnalAlgos::insertLUTelement(int ieta,int depth, int lutval)
{
  IetaDepth_t id(ieta,depth);

  map<int,vector<int> >::iterator it = m_LUT_.find(id.toCode());
  if (it == m_LUT_.end()) {
    vector<int> v(1,lutval);
    m_LUT_[id.toCode()] = v;
  } else {
    it->second.push_back(lutval);
  }
#if 0
  if (depth == 1) {
    if      (ieta >=  29 ) h_inputLUT1_->Fill(ieta-28,lutval); 
    else if (ieta <= -29 ) h_inputLUT1_->Fill(ieta+28,lutval);
  } else if (depth == 2) {
    if      (ieta >=  29 ) h_inputLUT2_->Fill(ieta-28,lutval); 
    else if (ieta <= -29 ) h_inputLUT2_->Fill(ieta+28,lutval);
  }
#endif
}                                   // HFtrigAnalAlgos::insertLUTelement

//======================================================================

bool HFtrigAnalAlgos::readLUTfromTextFile(void)
{
  char linein[512];
  FILE *fp = fopen(lutFileName_.c_str(),"r");
  int nline=0;
  while (!feof(fp) && fgets(linein, 512, fp))  {
    char *ptr = strchr(linein, '\n'); if (ptr) *ptr = 0; // null the newline
    nline++;

    if (nline <= 8) continue;

    vector<string> tokens;
    Tokenize(linein, tokens," \t");

    if (tokens.size() != 4) { // 52) {
      cerr << "# of entries in line " << nline << " = " << tokens.size() << " - incorrect" << endl;
      return false;
    }

    int itok = 0;
    for (int depth=1; depth<=2; depth++) {
      for (int ieta=-29; ieta>=-41; ieta--) insertLUTelement(ieta,depth,atoi(tokens[itok].c_str()));
      itok++;
      for (int ieta= 29; ieta<= 41; ieta++) insertLUTelement(ieta,depth,atoi(tokens[itok].c_str()));
      itok++;
    }
  } // loop over lines in text file

  return true;
}                                // HFtrigAnalAlgos::readLUTfromTextFile

//======================================================================

void HFtrigAnalAlgos::dumpLUT(void)
{
  map<int,vector<int> >::iterator it;
  for (it = m_LUT_.begin();
       it != m_LUT_.end();
       it++) {
    cout << "ID: " << it->first << " LUT:";
    for (uint32_t i=0; i<it->second.size(); i++)
      cout << " " << it->second[i];
    cout << endl;
  }
}

//======================================================================

void
HFtrigAnalAlgos::bookPerRunHistos(uint32_t runnum)
{
  edm::Service<TFileService> fs;
  char name[80];
  char title[128];

  sprintf(name,"run%dbxnumh",runnum);
  sprintf(title,"Run #%d Bunch #s", runnum);
  bxhist_ = fs->make<TH1F>(name,title, 3601, -0.5, 3600.5);

#if 0
  h_inputLUT1_ = new TH1F("inputLUTd1h", "input LUT depth 1",
			  27,-13.5,13.5);

  h_inputLUT2_ = new TH1F("inputLUTd2h", "input LUT depth 2",
			  27,-13.5,13.5);
#endif

  sprintf(name,"run%dCoEinPhiPlush",runnum);
  sprintf(title,"HFP Center of Energy in Phi, Run #%d;iphi", runnum);
  h_CoEinPhiPlus_  = RHsubDir_->make<TH1F>(name, title,72,-0.5,71.5);

  sprintf(name,"run%dCoEinPhiMinush",runnum);
  sprintf(title,"HFM Center of Energy in Phi, Run #%d;iphi", runnum);
  h_CoEinPhiMinus_ = RHsubDir_->make<TH1F>(name,title,72,-0.5,71.5);

  sprintf(name,"run%dCoEinEtaPlush",runnum);
  sprintf(title,"HFP Center of Energy in Eta, Run #%d;ieta", runnum);
  h_CoEinEtaPlus_  = RHsubDir_->make<TH1F>(name,title,13,28.5,41.5);

  sprintf(name,"run%dCoEinEtaMinush",runnum);
  sprintf(title,"HFM Center of Energy in Eta, Run #%d;ieta", runnum);
  h_CoEinEtaMinus_ = RHsubDir_->make<TH1F>(name,title,13,-41.5,-28.5);

  sprintf(name,"run%dtotalEh",runnum);
  sprintf(title,"Total HF RecHit Energy, Run #%d;GeV", runnum);
  h_totalE_ = RHsubDir_->make<TH1F>(name, title, 46,logEbins);
#if 0
				    rhTotalEnergyHp_.nbins,
				    rhTotalEnergyHp_.min,
				    rhTotalEnergyHp_.max);
#endif

  sprintf(name,"run%dtotalEperEtah",runnum);
  sprintf(title,"Total HF RecHit Energy vs Eta, Run #%d;ieta", runnum);
  h_totEvsIeta_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  // ...but renumber the ticks so there's no confusion.
  for (int ibin=1; ibin<=13; ibin++) {
    ostringstream binlabel;
    binlabel<<(ibin-42); h_totEvsIeta_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str()); binlabel.str("");
    binlabel<<(ibin+28); h_totEvsIeta_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
  }

  sprintf(name,"run%dtotalEperPhih",runnum);
  sprintf(title,"Total HF RecHit Energy vs Phi, Run #%d;iphi", runnum);
  h_totEvsIphi_ = RHsubDir_->make<TH1F>(name,title, 72,-0.5,71.5);

  sprintf(name,"run%dpulseProfileMaxADC",runnum);
  sprintf(title,"HF Pulse Profile from max ADC, Run #%d", runnum);
  h_PulseProfileMax_ = fs->make<TH1F>(name,title,11,-0.5,10.5);

}                                   // HFtrigAnalAlgos::bookPerRunHistos

//======================================================================

TH2F *
HFtrigAnalAlgos::bookOccHisto(int depth, uint32_t runnum, bool ismaxval)
{
  edm::Service<TFileService> fs;

  char name[80];
  char title[128];

  if (!depth) return NULL;

  if (ismaxval) {
    sprintf(name,"run%dHF_MaxADC_occupancy_depth=%d",runnum,depth);
    sprintf(title,"Run# %d HF MaxADC occupancy, depth=%d",runnum,depth);
  } else {
    sprintf(name,"run%dHF_Next2MaxADC_occupancy_depth=%d",runnum,depth);
    sprintf(title,"Run# %d HF Next2MaxADC occupancy, depth=%d",runnum,depth);
  }

  // HF ietas crunched down on plots so there isn't a gap in between
  TH2F *h_occ   = fs->make<TH2F>( name,name,
				  27,-13.5,13.5,
				  72,-0.5,71.5);

  // ...but renumber the ticks so there's no confusion.
  for (int ibin=1; ibin<=13; ibin++) {
    ostringstream binlabel;
    binlabel << (ibin-42);
    h_occ->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    binlabel.str("");
    binlabel << (ibin+28);
    h_occ->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
  }

  if (ismaxval) m_hOccupancies1_[depth] = h_occ;
  else          m_hOccupancies2_[depth] = h_occ;

  return h_occ;
}                                       // HFtrigAnalAlgos::bookOccHisto

//======================================================================

TH1F *
HFtrigAnalAlgos::bookSpectrumHisto(IetaDepth_t& id, uint32_t runnum)
{
  char name[80];
  char title[128];

  sprintf(name,"run%d_HF_ieta=%d_depth=%d_Spectrum",
	  runnum,id.ieta(),id.depth());
  sprintf(title,"Run# %d HF ieta=%d,depth=%d Spectrum; ET (LUT units)",
	  runnum,id.ieta(),id.depth());
  cout << "booking histo " << name << endl;
  TH1F *hp = DigiSubDir_->make<TH1F>(name,title,
				    digiSpectrumHp_.nbins,
				    digiSpectrumHp_.min,
				    digiSpectrumHp_.max);
  m_hSpectra_[id.toCode()] = hp;

  return hp;
}                                  // HFtrigAnalAlgos::bookSpectrumHisto

//======================================================================

TH1F *
HFtrigAnalAlgos::bookEperEventHisto(uint32_t nkevent,uint32_t runnum)
{
  if ((nkevent+1) > v_ePerEventHistos_.size())
    v_ePerEventHistos_.resize(nkevent+1);

  char name[80];
  char title[128];

  uint32_t minev = nkevent*1000;
  uint32_t maxev = minev+1000;

  sprintf (name,"run%dHFtotalE%05dto%05d", runnum,minev,maxev);
  sprintf (title,
"Run# %d HF total Rechit Energy, Ev#%d-%d; Event Number; Total Rechit E (GeV)",
	   runnum,minev,maxev);

  TH1F *hp = RHsubDir_->make<TH1F>(name,title,1001,(float)minev-0.5,(float)maxev+0.5);

  v_ePerEventHistos_[nkevent] = hp;

  return hp;
}                                 // HFtrigAnalAlgos::bookEperEventHisto

//======================================================================

TH2F *
HFtrigAnalAlgos::book2dEnergyHisto(uint32_t evtnum, uint32_t runnum)
{
  char name[80];
  char title[128];

  sprintf (name,"run%drhEforEvent%05d", runnum,evtnum);
  sprintf (title, "Run# %d HF Rechit Energies for Ev#%d; ieta; iphi+depth-1",
	   runnum,evtnum);

  // HF ietas crunched down on plots so there isn't a gap in between
  TH2F *h2p = RHsubDir_->make<TH2F>(name,title,
				   27,-13.5,13.5,
				   72,-0.5,71.5);

  // ...but renumber the ticks so there's no confusion.
  for (int ibin=1; ibin<=13; ibin++) {
    ostringstream binlabel;
    binlabel << (ibin-42);
    h2p->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    binlabel.str("");
    binlabel << (ibin+28);
    h2p->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
  }

  return h2p;
}                                  // HFtrigAnalAlgos::book2dEnergyHisto

//======================================================================

void
HFtrigAnalAlgos::endJob()
{
}

//======================================================================

void HFtrigAnalAlgos::filterRHs(const HFRecHitCollection& unfiltrh,
				std::vector<HFRecHit>& filtrh)
{
  HFRecHitCollection::const_iterator it;

  for (it=unfiltrh.begin(); it!=unfiltrh.end(); it++){
    if ((it->id().iphi() != 67) &&
	(it->energy() > minGeVperRecHit_))
      filtrh.push_back(*it);
  }
}                                        //  HFtrigAnalAlgos::filterRHs

//======================================================================

bool HFtrigAnalAlgos::intheSameHFWedge(const HcalDetId& id1,
				       const HcalDetId& id2)
{
  if ((id1.subdet() == HcalForward) &&
      (id2.subdet() == HcalForward)   )
    if (id1.zside() == id2.zside()) {
      int iphi1 = id1.iphi();
      int iphi2 = id2.iphi();
      
      if (iphi1 == iphi2) return true;
      if (iphi1 < iphi2) iphi1 = (iphi1+2) % 72;
      else               iphi2 = (iphi2+2) % 72;

      return (iphi1 == iphi2);
    }
  return false;
}                                   // HFtrigAnalAlgos::intheSameHFWedge

//======================================================================

void HFtrigAnalAlgos::findMaxChannels(const HFDigiCollection& hfdigis,
				      HFDataFrame& maxframe,
				      HFDataFrame& next2maxframe,
				      int& maxval,
				      int& next2maxval)
{
  maxval = -INT_MAX;
  next2maxval = -INT_MAX;

  for (unsigned idig = 0; idig < hfdigis.size (); ++idig) {
    const HFDataFrame& frame = hfdigis[idig];
    HcalDetId detId = frame.id();

    if (detId.iphi() == 67) continue;
    //if (detId == detId2Mask_) continue;

    int ieta = detId.ieta();
    int iphi = detId.iphi();
    int depth = detId.depth();
    IetaDepth_t id(ieta,depth); // for LUT lookup

    if (verbose_) std::cout <<"HF digi # " <<idig<<": eta/phi/depth: "
			   <<ieta<<'/'<<iphi<<'/'<< depth << std::endl;
    int isample;
    int max4thisdigi=-INT_MAX;
    for (isample = std::max(0,sampleWindowLeft_);
	 isample <= std::min(sampleWindowRight_,frame.size()-1);
	 ++isample) {
      int rawadc = frame[isample].adc();
#if 0
      int capid = frame[isample].capid ();
      double linear_ADC = frame[isample].nominal_fC();
      double nominal_fC = detId.subdet () == HcalForward ? 2.6 *  linear_ADC : linear_ADC;
#endif

      map<int,vector<int> >::const_iterator it = m_LUT_.find(id.toCode());
      if (it == m_LUT_.end()) {
	cout << "LUT entry for id " << id.ieta() << " " << id.depth();
	cout << " not found." << endl;
	throw cms::Exception("LUT entry for id not found");
      }

      int lutval = it->second[rawadc];

      if (lutval > max4thisdigi)
	max4thisdigi = lutval;

    } // loop over samples in digi

    if (max4thisdigi >= maxval) {
      if (!intheSameHFWedge(frame.id(),maxframe.id())) {
	next2maxval   = maxval;
	next2maxframe = maxframe;
      }
      maxval   = max4thisdigi;
      maxframe = frame;
    } else if (max4thisdigi >= next2maxval) {
      if (!intheSameHFWedge(frame.id(),maxframe.id())) {
	next2maxval   = maxval;
	next2maxframe = maxframe;
      }
    }
  } // loop over digi collection

#if 0
  cout << "Max/Next2Max frames: ";
  cout << maxframe.id() << " ADC = " << maxval << "; ";
  cout << next2maxframe.id() << " ADC = " << next2maxval << "; ";
  cout << endl;
#endif
}                                    // HFtrigAnalAlgos::findMaxChannels

//======================================================================

void HFtrigAnalAlgos::fillDigiSpectra(const HFDataFrame& maxframe,
				      int& maxadc,
				      uint32_t runnum)
{
  // Fill histos with maximum ADC
  //
  HcalDetId detId = maxframe.id();
  IetaDepth_t id(detId.ieta(),detId.depth());
  std::map<int,TH1F *>::iterator it = m_hSpectra_.find(id.toCode());
  TH1F *hp = 0;

  if (it == m_hSpectra_.end()) hp = bookSpectrumHisto(id,runnum);
  else                         hp = it->second;

  if (hp) {
    //cout << "Filling ieta=" << id.ieta() << ", depth=" << id.depth();
    //cout << ", maxadc = " << maxadc << endl;
    hp->Fill(maxadc);
  }
}                                   // HFtrigAnalAlgos::fillDigiSpectra

//======================================================================

void HFtrigAnalAlgos::fillOccupancy(const HFDataFrame& maxframe,
				    const HFDataFrame& next2maxframe,
				    uint32_t runnum)
{
  HcalDetId id = maxframe.id();

  std::map<int,TH2F *>::iterator it = m_hOccupancies1_.find(id.depth());
  TH2F *hp = 0;
  if (it == m_hOccupancies1_.end()) hp = bookOccHisto(id.depth(),runnum);
  else                              hp = it->second;

  if (hp) {
    if      (id.ieta() >=  29 ) hp->Fill(id.ieta()-28,id.iphi()); 
    else if (id.ieta() <= -29 ) hp->Fill(id.ieta()+28,id.iphi());
    //cout << "Filling ieta=" << id.ieta() << ", iphi=" << id.iphi() << endl;
  } else {
    cout << "pointer is zero!";
    cout << " ieta=" << id.ieta() << ", iphi=" << id.iphi();
    cout << ", depth = " << id.depth() << endl;
  }

  id = next2maxframe.id();

  it = m_hOccupancies2_.find(id.depth());
  hp = 0;
  if (it == m_hOccupancies2_.end()) hp = bookOccHisto(id.depth(),runnum,false);
  else                              hp = it->second;

  if (hp) {
    if      (id.ieta() >=  29 ) hp->Fill(id.ieta()-28,id.iphi()); 
    else if (id.ieta() <= -29 ) hp->Fill(id.ieta()+28,id.iphi());
    //cout << "Filling ieta=" << id.ieta() << ", iphi=" << id.iphi() << endl;
  } else {
    cout << "pointer is zero!";
    cout << " ieta=" << id.ieta() << ", iphi=" << id.iphi();
    cout << ", depth = " << id.depth() << endl;
  }
}                                     //  HFtrigAnalAlgos::fillOccupancy

//======================================================================

void HFtrigAnalAlgos::fillPulseProfile(const HFDataFrame& maxframe)
{
  for (int isample = 0; isample < std::min(10,maxframe.size()); ++isample) {
    int rawadc = maxframe[isample].adc();
    h_PulseProfileMax_->Fill(isample,rawadc);

  } // loop over samples in digi
}                                   // HFtrigAnalAlgos::fillPulseProfile

//======================================================================

void HFtrigAnalAlgos::fillBxNum(uint32_t bxnum)
{
  bxhist_->Fill((float)bxnum);
}

//======================================================================

void HFtrigAnalAlgos::fillRhHistos(const std::vector<HFRecHit>& hfrechits,
				   uint32_t evtnum,
				   uint32_t runnum)
{
  float totalE = 0.0;
  uint32_t nkevents = (evtnum-1)/1000;
  TH1F *h1p;

  if ((nkevents+1) > v_ePerEventHistos_.size())
    h1p = bookEperEventHisto(nkevents,runnum);
  else if (!v_ePerEventHistos_[nkevents])
    h1p = bookEperEventHisto(nkevents,runnum);
  else
    h1p = v_ePerEventHistos_[nkevents];

  // Total energy plots
  //
  float sumEtimesPhiPl = 0.0;
  float sumEtimesPhiMn = 0.0;
  float sumEtimesEtaPl = 0.0;
  float sumEtimesEtaMn = 0.0;
  float totalEplus     = 0.0;
  float totalEminus    = 0.0;

  // total energy first
  //
  for (unsigned ihit = 0; ihit < hfrechits.size (); ++ihit)
    totalE += hfrechits[ihit].energy();

  if (totalE < 500.0) return;

  h1p->Fill(evtnum,totalE);
  h_totalE_->Fill(totalE);

  for (unsigned ihit = 0; ihit < hfrechits.size (); ++ihit) {
    const HFRecHit& rh  = hfrechits[ihit];
    const HcalDetId& id = rh.id();
    int   ieta          = id.ieta();
    int   iphi          = id.iphi();
    float rhenergy      = rh.energy();

    if      (ieta >=  29 ) h_totEvsIeta_->Fill(ieta-28,rhenergy); 
    else if (ieta <= -29 ) h_totEvsIeta_->Fill(ieta+28,rhenergy);
    h_totEvsIphi_->Fill(iphi,rhenergy);

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
  }

  // Center of energy plots
  //
  if (totalEminus != 0.0) {
    h_CoEinPhiMinus_->Fill(sumEtimesPhiMn/totalEminus);
    h_CoEinEtaMinus_->Fill(sumEtimesEtaMn/totalEminus);
  }
  if (totalEplus != 0.0) {
    h_CoEinPhiPlus_->Fill(sumEtimesPhiPl/totalEplus);
    h_CoEinEtaPlus_->Fill(sumEtimesEtaPl/totalEplus);
  }

  // Event display plotting
  //
  if (totalE >  totalRHenergyThresh4Plotting_) {
    TH2F *h2p = book2dEnergyHisto(evtnum,runnum);

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
}                                       // HFtrigAnalAlgos::fillRhHistos

//======================================================================

void HFtrigAnalAlgos::analyze(const HFDigiCollection&   hfdigis,
			      const HFRecHitCollection& hfrechits,
			      uint32_t bxnum,
			      uint32_t evtnum,
			      uint32_t runnum)
{
  //std::vector<HFDataFrame> maxdigis;
  HFDataFrame maxframe, next2maxframe;
  int maxadc, next2maxadc;

  if (s_runs_.find(runnum) == s_runs_.end()) {
    bookPerRunHistos(runnum);
    s_runs_.insert(runnum);
  }

  findMaxChannels(hfdigis,maxframe, next2maxframe, maxadc, next2maxadc);
  //findMaxChannels(hfdigis,maxdigis,maxadc);

  if (maxadc < adcTrigThreshold_) {
    cout << " maxadc = " << maxadc << " less than adcTrigThreshold_ " << adcTrigThreshold_;
    cout << "; HF Digi Collection size = " << hfdigis.size() << endl;
  } else if (evtnum == 1) 
    cout << "HF Digi Collection size = " << hfdigis.size() << endl;

  if (maxadc < 5) return;

  std::vector<HFRecHit> filtrh;
  filterRHs(hfrechits, filtrh);
  fillDigiSpectra(maxframe,maxadc,runnum);
  fillPulseProfile(maxframe);
  fillOccupancy(maxframe,next2maxframe,runnum);
  fillRhHistos(filtrh,evtnum,runnum);
  fillBxNum(bxnum);
}                                            // HFtrigAnalAlgos::analyze

//======================================================================
