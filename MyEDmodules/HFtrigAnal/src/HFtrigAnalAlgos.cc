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

class compareADC {
public:
  bool operator()(const HFtrigAnalAlgos::triggerWedge_t& w1,
		  const HFtrigAnalAlgos::triggerWedge_t& w2) const {
    return w1.maxadc>w2.maxadc;
  }
};//needed for sorting by maxadc

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
  vector<int> v_maskidnumbers;
  vector<int> v_validBXnumbers;

  lutFileName_         = iConfig.getUntrackedParameter<string>("lutFileName");

  sampleWindowLeft_    = iConfig.getParameter<int>("digiSampleWindowMin");
  sampleWindowRight_   = iConfig.getParameter<int>("digiSampleWindowMax");

  minGeVperRecHit_     = iConfig.getParameter<double>("minGeVperRecHit");

  totalRHenergyThresh4Plotting_ = iConfig.getParameter<double>("totalEthresh4eventPlotting");

  //eventNumberMin_       = (uint32_t)iConfig.getParameter<int>("eventNumberMin");
  //eventNumberMax_       = (uint32_t)iConfig.getParameter<int>("eventNumberMax");

  adcTrigThreshold_       = (uint32_t)iConfig.getParameter<int>("adcTrigThreshold");
  v_maskidnumbers         = iConfig.getParameter<vector<int> > ("detIds2Mask");
  v_validBXnumbers        = iConfig.getParameter<vector<int> > ("validBxNumbers");

  digiSpectrumHp_.nbins = iConfig.getUntrackedParameter<int>   ("digiSpectrumNbins");
  digiSpectrumHp_.min   = iConfig.getUntrackedParameter<double>("digiSpectrumMinADC");
  digiSpectrumHp_.max   = iConfig.getUntrackedParameter<double>("digiSpectrumMaxADC");

  rhTotalEnergyHp_.nbins= iConfig.getUntrackedParameter<int>   ("rhTotalEnergyNbins");
  rhTotalEnergyHp_.min  = iConfig.getUntrackedParameter<double>("rhTotalEnergyMinGeV");
  rhTotalEnergyHp_.max  = iConfig.getUntrackedParameter<double>("rhTotalEnergyMaxGeV");

  nWedgesHp_.nbins      = iConfig.getUntrackedParameter<int>   ("nWedgesPlotNbins");
  nWedgesHp_.min        = iConfig.getUntrackedParameter<double>("nWedgesPlotMin");
  nWedgesHp_.max        = iConfig.getUntrackedParameter<double>("nWedgesPlotMax");

  lumiSegHp_.nbins      = iConfig.getUntrackedParameter<int>   ("lumiSegPlotNbins");
  lumiSegHp_.min        = iConfig.getUntrackedParameter<double>("lumiSegPlotMin");
  lumiSegHp_.max        = iConfig.getUntrackedParameter<double>("lumiSegPlotMax");

  if (!convertIdNumbers(v_maskidnumbers, detIds2mask_))
    throw cms::Exception("Invalid detID vector");

  for (uint32_t i=0; i<v_validBXnumbers.size(); i++)
    s_validBxNums_.insert((uint16_t)v_validBXnumbers[i]);

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

void HFtrigAnalAlgos::insertLUTelement(int ieta,int depth, uint32_t lutval)
{
  IetaDepth_t id(ieta,depth);

  map<int,vector<uint32_t> >::iterator it = m_LUT_.find(id.toCode());
  if (it == m_LUT_.end()) {
    vector<uint32_t> v(1,lutval);
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
      cerr << "# of entries in line " << nline << " = ";
      cerr << tokens.size() << " - incorrect" << endl;
      return false;
    }

    int i=0;
    for (int dp=1; dp<=2; dp++) {
      for (int ie=-29; ie>=-41; ie--) insertLUTelement(ie,dp,atoi(tokens[i].c_str())); i++;
      for (int ie= 29; ie<= 41; ie++) insertLUTelement(ie,dp,atoi(tokens[i].c_str())); i++;
    }
  } // loop over lines in text file

  return true;
}                                // HFtrigAnalAlgos::readLUTfromTextFile

//======================================================================

void HFtrigAnalAlgos::dumpLUT(void)
{
  map<int,vector<uint32_t> >::iterator it;
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
  h_bx_ = fs->make<TH1S>(name,title, 3601, -0.5, 3600.5);

  sprintf(name,"run%dlumisegh",runnum);
  sprintf(title,"Run #%d Lumi Segment #s", runnum);
  h_lumiseg_ = fs->make<TH1S>(name,title, 
			      lumiSegHp_.nbins,
			      lumiSegHp_.min,
			      lumiSegHp_.max);

  sprintf(name,"run%dlumisegGoodBxh",runnum);
  sprintf(title,"Run #%d Lumi Segment #s, Good BX only", runnum);
  h_lumisegGoodBx_ = fs->make<TH1S>(name,title, 
				    lumiSegHp_.nbins,
				    lumiSegHp_.min,
				    lumiSegHp_.max);

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
  h_totalE_ = RHsubDir_->make<TH1F>(name, title, NUMEBINS,logEbins);

  sprintf(name,"run%dEperEtah",runnum);
  sprintf(title,"HF RecHit Energy vs Eta, Run #%d;ieta", runnum);
  h_EvsIeta_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtaLT3h; ieta; GeV",runnum);
  sprintf(title,"HF Tower Energy vs. Eta for <3 Towers with E>20GeV, Run #%d;ieta", runnum);
  h_EvsIeta_nTlt3_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtahLT5h; ieta; GeV",runnum);
  sprintf(title,"HF Tower Energy vs. Eta for <5 Towers with E>20GeV, Run #%d;ieta", runnum);
  h_EvsIeta_nTlt5_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtahGE5h; ieta; GeV",runnum);
  sprintf(title,"HF Tower Energy vs. Eta for >=5 Towers with E>20GeV, Run #%d;ieta", runnum);
  h_EvsIeta_nTge5_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtaNonPMTh; ieta; GeV",runnum);
  sprintf(title,"HF RecHit Energy vs Eta for 'non-PMT' Events, , Run #%d;ieta", runnum);
  h_EvsIetaNonPMT_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  // ...but renumber the ticks so there's no confusion.
  for (int ibin=1; ibin<=13; ibin++) {
    ostringstream binlabel;
    binlabel<<(ibin-42);
    h_EvsIeta_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIeta_nTlt3_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIeta_nTlt5_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIeta_nTge5_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    binlabel.str("");
    binlabel<<(ibin+28);
    h_EvsIeta_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIeta_nTlt3_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIeta_nTlt5_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIeta_nTge5_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
  }

  sprintf(name,"run%dnLongVsShortLT3h; Short GeV; Long GeV",runnum);
  sprintf(title,"Long vs. Short Energy for <3 Towers with E>20GeV, Run #%d", runnum);
  h_LongVsShortE_nTlt3_ = RHsubDir_->make<TH2F>(name, title, 
						50,0.0,500.0,
						50,0.0,500.0);

  sprintf(name,"run%dnLongVsShortLT5h; Short GeV; Long GeV",runnum);
  sprintf(title,"Long vs. Short Energy for <5 Towers with E>20GeV, Run #%d", runnum);
  h_LongVsShortE_nTlt5_ = RHsubDir_->make<TH2F>(name, title,
						50,0.0,500.0,
						50,0.0,500.0);

  sprintf(name,"run%dnLongVsShortGE5h; Short GeV; Long GeV",runnum);
  sprintf(title,"Long vs. Short Energy for >=5 Towers with E>20GeV, Run #%d", runnum);
  h_LongVsShortE_nTge5_ = RHsubDir_->make<TH2F>(name, title,
						50,0.0,500.0,
						50,0.0,500.0);

  sprintf(name,"run%dnLongVsShorth; Short GeV; Long GeV",runnum);
  sprintf(title,"Long vs. Short Energy for Towers with E>20GeV, Run #%d", runnum);
  h_LongVsShortE_ = RHsubDir_->make<TH2F>(name, title,
					  50,0.0,500.0,
					  50,0.0,500.0);

  sprintf(name,"run%dEperPhih; iphi; GeV",runnum);
  sprintf(title,"HF RecHit Energy vs Phi, Run #%d;iphi", runnum);
  h_EvsIphi_ = RHsubDir_->make<TH1F>(name,title, 72,-0.5,71.5);

  sprintf(name,"run%dpulseProfileMaxADC",runnum);
  sprintf(title,"HF Pulse Profile from max ADC, Run #%d", runnum);
  h_PulseProfileMax_ = fs->make<TH1F>(name,title,11,-0.5,10.5);

  sprintf(name,"run%dnWedgesOverThreshGoodBx",runnum);
  sprintf(title,"HF #Wedges Over Threshold, Signal, Run #%d", runnum);
  h_nWedgesOverThreshGoodBx_ = fs->make<TH1F>(name,title,
					      nWedgesHp_.nbins,
					      nWedgesHp_.min,
					      nWedgesHp_.max);

  sprintf(name,"run%dnWedgesOverThreshBadBx",runnum);
  sprintf(title,"HF #Wedges Over Threshold, Noise, Run #%d", runnum);
  h_nWedgesOverThreshBadBx_  = fs->make<TH1F>(name,title,
					      nWedgesHp_.nbins,
					      nWedgesHp_.min,
					      nWedgesHp_.max);

  sprintf(name,"run%dnPlusMinusTriggerGoodBx",runnum);
  sprintf(title,"Two HF Wedges Coincidence Classification, Run #%d", runnum);
  h_PlusMinusTrigger_ = fs->make<TH1F>(name, title, 5, -2.5,2.5);
  h_PlusMinusTrigger_->GetXaxis()->SetBinLabel(2,"Minus-Minus");
  h_PlusMinusTrigger_->GetXaxis()->SetBinLabel(3,"Minus-Plus");
  h_PlusMinusTrigger_->GetXaxis()->SetBinLabel(4,"Plus-Plus");

  sprintf(name,"run%dnTowersOverThreshGoodBx",runnum);
  sprintf(title,"HF #Towers Over Threshold, Run #%d", runnum);
  h_nTowersOverThresh_  = fs->make<TH1F>(name,title,
					 nWedgesHp_.nbins,
					 nWedgesHp_.min,
					 nWedgesHp_.max);

  sprintf(name,"run%dnPlusMinusTriggerBadBx",runnum);
  sprintf(title,"Two HF Wedges Coincidence Classification, Bad BX#, Run #%d", runnum);
  h_PlusMinusTriggerBadBx_ = fs->make<TH1F>(name, title, 5, -2.5,2.5);
  h_PlusMinusTriggerBadBx_->GetXaxis()->SetBinLabel(2,"Minus-Minus");
  h_PlusMinusTriggerBadBx_->GetXaxis()->SetBinLabel(3,"Minus-Plus");
  h_PlusMinusTriggerBadBx_->GetXaxis()->SetBinLabel(4,"Plus-Plus");


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
  for (int i=1; i<=h_totalE_->GetNbinsX(); i++) {
    h_totalE_->SetBinContent(i, h_totalE_->GetBinContent(i)/h_totalE_->GetBinWidth(i));
  }
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

uint32_t HFtrigAnalAlgos::lookupLinearizerLUTval(IetaDepth_t& id, int rawadc)
{
#if 0
  int capid = frame[isample].capid ();
  double linear_ADC = frame[isample].nominal_fC();
  double nominal_fC = detId.subdet () == HcalForward ? 2.6 *  linear_ADC : linear_ADC;
#endif

  map<int,vector<uint32_t> >::const_iterator it = m_LUT_.find(id.toCode());
  if (it == m_LUT_.end()) {
    cout << "LUT entry for id " << id.ieta() << " " << id.depth();
    cout << " not found." << endl;
    throw cms::Exception("LUT entry for id not found");
  }

  return (it->second[rawadc]);
}                             // HFtrigAnalAlgos::lookupLinearizerLUTval

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

    if (maskedId(detId)) continue;

    int ieta  = detId.ieta();
    int zside = detId.zside();
    int iphi  = detId.iphi();
    int depth = detId.depth();
    IetaDepth_t id(ieta,depth); // for LUT lookup
    HFwedgeID_t wid(iphi,zside);

    if (verbose_) std::cout <<"HF digi # " <<idig<<": eta/phi/depth: "
			   <<ieta<<'/'<<iphi<<'/'<< depth << std::endl;
    int isample;
    uint32_t max4thisdigi=0;
    int maxsamplenum = -1;
    for (isample = std::max(0,sampleWindowLeft_);
	 isample <= std::min(sampleWindowRight_,frame.size()-1);
	 ++isample) {

      uint32_t lutval = lookupLinearizerLUTval(id,frame[isample].adc());

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

void HFtrigAnalAlgos::fillNwedges(std::vector<triggerWedge_t>& sortedWedges,
				  uint16_t bxnum)
{
  int nWedgesOverThresh = 0;
  int nWedgesOverThreshPlus = 0;
  int nWedgesOverThreshMinus = 0;
  for (uint32_t i = 0; i<sortedWedges.size(); i++) {
    if (sortedWedges[i].maxadc < adcTrigThreshold_) break;
    nWedgesOverThresh++;
    if (sortedWedges[i].wid.id() < 0) nWedgesOverThreshMinus++;
    else                              nWedgesOverThreshPlus++;
  }
  if (s_validBxNums_.find(bxnum) == s_validBxNums_.end()) {
    h_nWedgesOverThreshBadBx_->Fill(nWedgesOverThresh);

    if (nWedgesOverThreshPlus  > 1)  h_PlusMinusTriggerBadBx_->Fill(1);
    if (nWedgesOverThreshMinus > 1)  h_PlusMinusTriggerBadBx_->Fill(-1);
    if (nWedgesOverThreshPlus &&
	nWedgesOverThreshMinus   )   h_PlusMinusTriggerBadBx_->Fill(0);
  } else {
    h_nWedgesOverThreshGoodBx_->Fill(nWedgesOverThresh);

    if (nWedgesOverThreshPlus  > 1)  h_PlusMinusTrigger_->Fill(1);
    if (nWedgesOverThreshMinus > 1)  h_PlusMinusTrigger_->Fill(-1);
    if (nWedgesOverThreshPlus &&
	nWedgesOverThreshMinus   )   h_PlusMinusTrigger_->Fill(0);
  }

}                                       //  HFtrigAnalAlgos::fillNwedges

//======================================================================

void HFtrigAnalAlgos::fillDigiSpectra(const triggerWedge_t& maxwedge,
				      uint32_t runnum)
{
  // Fill histos with maximum ADC
  //
  HcalDetId detId = maxwedge.frame.id();
  IetaDepth_t id(detId.ieta(),detId.depth());
  std::map<int,TH1F *>::iterator it = m_hSpectra_.find(id.toCode());
  TH1F *hp = 0;

  if (it == m_hSpectra_.end()) hp = bookSpectrumHisto(id,runnum);
  else                         hp = it->second;

  if (hp) {
    //cout << "Filling ieta=" << id.ieta() << ", depth=" << id.depth();
    //cout << ", maxadc = " << maxadc << endl;
    hp->Fill(maxwedge.maxadc);
  }
}                                   // HFtrigAnalAlgos::fillDigiSpectra

//======================================================================

void HFtrigAnalAlgos::fillOccupancy(const std::vector<triggerWedge_t>& sortedWedges,
				    uint32_t runnum)
{
  HcalDetId id = sortedWedges[0].frame.id();

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

  if (sortedWedges.size() <= 1) return;

  id = sortedWedges[1].frame.id();

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

  //if (totalE < 500.0) return;
  //cout << "evtnum = " << evtnum << endl;

  h1p->Fill(evtnum,totalE);
  h_totalE_->Fill(totalE);

  map<int,TowerEnergies_t> m_TowerEnergies;

  for (unsigned ihit = 0; ihit < hfrechits.size (); ++ihit) {
    const HFRecHit& rh  = hfrechits[ihit];
    const HcalDetId& id = rh.id();
    int   ieta          = id.ieta();
    int   iphi          = id.iphi();
    float rhenergy      = rh.energy();

    if      (ieta >=  29 ) h_EvsIeta_->Fill(ieta-28,rhenergy); 
    else if (ieta <= -29 ) h_EvsIeta_->Fill(ieta+28,rhenergy);
    h_EvsIphi_->Fill(iphi,rhenergy);

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

  // Tower Energy plots, count up # towers over threshold
  map<int,TowerEnergies_t>::const_iterator it;
  int ntowers = 0;
  for (it  = m_TowerEnergies.begin();
       it != m_TowerEnergies.end(); it++) {
    if (it->second.totalE > 20.0) ntowers++;
  }

  h_nTowersOverThresh_->Fill(ntowers);

  for (it  = m_TowerEnergies.begin();
       it != m_TowerEnergies.end(); it++) {
    IetaIphi_t ieip(it->first);
    int ie2f = 0;
    if      (ieip.ieta() >=  29 ) ie2f = ieip.ieta()-28;
    else if (ieip.ieta() <= -29 ) ie2f = ieip.ieta()+28;

    TowerEnergies_t t = it->second;
    if (it->second.totalE > 20.0) {
      if      (ntowers < 3) {h_EvsIeta_nTlt3_->Fill(ie2f,t.totalE);h_LongVsShortE_nTlt3_->Fill(t.shortE,t.longE);}
      else if (ntowers < 5) {h_EvsIeta_nTlt5_->Fill(ie2f,t.totalE);h_LongVsShortE_nTlt5_->Fill(t.shortE,t.longE);}
      else                  {h_EvsIeta_nTge5_->Fill(ie2f,t.totalE);h_LongVsShortE_nTge5_->Fill(t.shortE,t.longE);}
      h_LongVsShortE_->Fill(t.shortE,t.longE);

      if ((t.shortE > 10.0) && (t.longE> 10.0)) {
	h_EvsIetaNonPMT_->Fill(ie2f,t.totalE);
      }
    }
  }

}                                       // HFtrigAnalAlgos::fillRhHistos

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

  if (s_runs_.find(runnum) == s_runs_.end()) {
    bookPerRunHistos(runnum);
    s_runs_.insert(runnum);
  }

  std::vector<triggerWedge_t> sortedWedges;

  findMaxWedges(hfdigis, sortedWedges);
  //dumpWedges(sortedWedges);

  maxadc = sortedWedges[0].maxadc;

  if (maxadc < adcTrigThreshold_) {
    cout << " maxadc = " << maxadc << " less than adcTrigThreshold_ " << adcTrigThreshold_;
    cout << "; HF Digi Collection size = " << hfdigis.size() << endl;
  } else if (evtnum == 1) 
    cout << "HF Digi Collection size = " << hfdigis.size() << endl;

  if (maxadc < 5) return;

  h_bx_->Fill(bxnum);
  h_lumiseg_->Fill(lsnum);

  if (s_validBxNums_.find(bxnum) != s_validBxNums_.end())
    h_lumisegGoodBx_->Fill(lsnum);

  fillNwedges(sortedWedges,bxnum);

  fillDigiSpectra(sortedWedges[0],runnum);
  fillPulseProfile(sortedWedges[0].frame);
  fillOccupancy(sortedWedges,runnum);

  std::vector<HFRecHit> filtrh;
  filterRHs(hfrechits, filtrh);
  if (s_validBxNums_.find(bxnum) != s_validBxNums_.end())
    fillRhHistos(filtrh,evtnum,runnum);
}                                            // HFtrigAnalAlgos::analyze

//======================================================================
