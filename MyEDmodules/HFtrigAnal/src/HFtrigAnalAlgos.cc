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

  minGeVperRecHit_   = iConfig.getParameter<double>("minGeVperRecHit");

  totalRHenergyThresh4Plotting_ = iConfig.getParameter<double>("totalEthresh4eventPlotting");

  //eventNumberMin_       = (uint32_t)iConfig.getParameter<int>("eventNumberMin");
  //eventNumberMax_       = (uint32_t)iConfig.getParameter<int>("eventNumberMax");

  digiSpectrumHp_.nbins = iConfig.getUntrackedParameter<int>   ("digiSpectrumNbins");
  digiSpectrumHp_.min   = iConfig.getUntrackedParameter<double>("digiSpectrumMinADC");
  digiSpectrumHp_.max   = iConfig.getUntrackedParameter<double>("digiSpectrumMaxADC");

  rhTotalEnergyHp_.nbins= iConfig.getUntrackedParameter<int>   ("rhTotalEnergyNbins");
  rhTotalEnergyHp_.min  = iConfig.getUntrackedParameter<double>("rhTotalEnergyMinGeV");
  rhTotalEnergyHp_.max  = iConfig.getUntrackedParameter<double>("rhTotalEnergyMaxGeV");
}

//======================================================================

void
HFtrigAnalAlgos::beginJob(void)
{
  edm::Service<TFileService> fs;

  DigiSubDir_ = new TFileDirectory(fs->mkdir( "ET_spectra" ));
  RHsubDir_   = new TFileDirectory(fs->mkdir( "RecHitEnergies" ));

  //bxhist_ = new TH1F("bxnumhist", "Bunch #", 100, 0, 3600);

  h_totalE_ = RHsubDir_->make<TH1F>("totalEh", "Total RecHit Energy",
				   rhTotalEnergyHp_.nbins,
				   rhTotalEnergyHp_.min,
				   rhTotalEnergyHp_.max);
  h_PulseProfileMax_ = fs->make<TH1F>("pulseProfileMaxADC",
				      "Pulse Profile from max ADC",
				      11,-0.5,10.5);

  if (!readLUTfromTextFile()) {
    throw cms::Exception("Reading LUT text file failed");
  }
  //dumpLUT();
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
}

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

    if (tokens.size() != 52) {
      cerr << "# of entries in line " << nline << " = " << tokens.size() << " - incorrect" << endl;
      return false;
    }

    int itok = 0;
    for (int depth=1; depth<=2; depth++) {
      for (int ieta=-29; ieta>=-41; ieta--) insertLUTelement(ieta,depth,atoi(tokens[itok++].c_str()));
      for (int ieta= 29; ieta<= 41; ieta++) insertLUTelement(ieta,depth,atoi(tokens[itok++].c_str()));
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

TH2F *
HFtrigAnalAlgos::bookOccHisto(int depth, uint32_t runnum)
{
  edm::Service<TFileService> fs;

  char name[80];
  char title[128];

  sprintf(name,"run%dHF_MaxADC_occupancy_depth=%d",runnum,depth);
  sprintf(title,"Run# %d HF MaxADC occupancy, depth=%d",runnum,depth);

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

  m_hOccupancies_[depth] = h_occ;

  return h_occ;
}

//======================================================================

TH1F *
HFtrigAnalAlgos::bookSpectrumHisto(IetaDepth_t& id, uint32_t runnum)
{
  char name[80];
  char title[128];

  sprintf(name,"run%dHF_ieta=%d_depth=%d_Spectrum",
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

void HFtrigAnalAlgos::findMaxChannels(const HFDigiCollection& hfdigis,
				      std::vector<HFDataFrame>& maxdigis,
				      int& maxval)
{
  maxval = -INT_MAX;

  for (unsigned idig = 0; idig < hfdigis.size (); ++idig) {
    const HFDataFrame& frame = hfdigis[idig];
    HcalDetId detId = frame.id();
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

    if (max4thisdigi > maxval) {
      maxdigis.clear();
      maxval = max4thisdigi;
      maxdigis.push_back(frame);
    } else if (max4thisdigi == maxval) {
      maxdigis.push_back(frame);
    }
  } // loop over digi collection
}                                    // HFtrigAnalAlgos::findMaxChannels

//======================================================================

void HFtrigAnalAlgos::fillDigiSpectra(std::vector<HFDataFrame>& maxdigis,
				      int& maxadc,
				      uint32_t runnum)

{
  // Fill histos for all id's with maximum ADC
  //
  for (uint32_t idig=0; idig<maxdigis.size(); idig++) {
    HcalDetId detId = maxdigis[idig].id();
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
  }
}                                   // HFtrigAnalAlgos::fillDigiSpectra

//======================================================================

void HFtrigAnalAlgos::fillOccupancy(std::vector<HFDataFrame>& maxdigis,
				    uint32_t runnum)
{
  for (uint32_t idig=0; idig<maxdigis.size(); idig++) {
    HcalDetId id = maxdigis[idig].id();

    std::map<int,TH2F *>::iterator it = m_hOccupancies_.find(id.depth());
    TH2F *hp = 0;
    if (it == m_hOccupancies_.end()) hp = bookOccHisto(id.depth(),runnum);
    else                             hp = it->second;

    if (hp) {
      if      (id.ieta() >=  29 ) hp->Fill(id.ieta()-28,id.iphi()); 
      else if (id.ieta() <= -29 ) hp->Fill(id.ieta()+28,id.iphi());
      //cout << "Filling ieta=" << id.ieta() << ", iphi=" << id.iphi() << endl;
    } else {
      cout << "pointer is zero!";
      cout << " ieta=" << id.ieta() << ", iphi=" << id.iphi();
      cout << ", depth = " << id.depth() << endl;
    }
  }
}                                     //  HFtrigAnalAlgos::fillOccupancy

//======================================================================

void HFtrigAnalAlgos::fillPulseProfile(std::vector<HFDataFrame>& maxdigis)
{
  for (unsigned idig = 0; idig < maxdigis.size (); ++idig) {
    const HFDataFrame& frame = maxdigis[idig];

    for (int isample = 0; isample < std::min(10,frame.size()); ++isample) {
      int rawadc = frame[isample].adc();
      h_PulseProfileMax_->Fill(isample,rawadc);

    } // loop over samples in digi
  } // loop over digi collection
}                                    // HFtrigAnalAlgos::findMaxChannels

//======================================================================

void HFtrigAnalAlgos::fillBxNum(uint32_t bxnum)
{
  //bxhist_->Fill((float)bxnum);
}

//======================================================================

void HFtrigAnalAlgos::fillRhHistos(const HFRecHitCollection& hfrechits,
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

  for (unsigned ihit = 0; ihit < hfrechits.size (); ++ihit) {
    const HFRecHit& rh = hfrechits[ihit];
    if (rh.energy() > minGeVperRecHit_)
      totalE += rh.energy();
  }

  h1p->Fill(evtnum,totalE);

  h_totalE_->Fill(totalE);

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
  std::vector<HFDataFrame> maxdigis;
  int maxadc;

  findMaxChannels(hfdigis,maxdigis,maxadc);
  fillDigiSpectra(maxdigis,maxadc,runnum);
  fillPulseProfile(maxdigis);
  fillOccupancy(maxdigis,runnum);
  fillRhHistos(hfrechits,evtnum,runnum);
  //fillBxNum(bxnum);
}
