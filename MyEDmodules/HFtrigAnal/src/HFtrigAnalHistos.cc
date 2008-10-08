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

//======================================================================

HFtrigAnalHistos::HFtrigAnalHistos(const edm::ParameterSet& iConfig)
{
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

}                                  // HFtrigAnalHistos::HFtrigAnalHistos

//======================================================================

void
HFtrigAnalHistos::beginJob(void)
{
  edm::Service<TFileService> fs;
  DigiSubDir_ = new TFileDirectory(fs->mkdir( "TPGET_spectra" ));
  RHsubDir_   = new TFileDirectory(fs->mkdir( "RecHitEnergies" ));
}

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

//======================================================================

void
HFtrigAnalHistos::bookPerRunHistos(uint32_t runnum)
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
  sprintf(title,"Average Summed Energy Per Event vs Eta, Run #%d;ieta;GeV", runnum);
  h_EvsIeta_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtaLT3h",runnum);
  sprintf(title,
	  "HF Tower Energy vs. Eta for <3 Towers with E>20GeV, Run #%d;ieta;GeV",
	  runnum);
  h_EvsIeta_nTlt3_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtahLT5h",runnum);
  sprintf(title,
	  "HF Tower Energy vs. Eta for <5 Towers with E>20GeV, Run #%d;ieta;GeV",
	  runnum);
  h_EvsIeta_nTlt5_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtahGE5h",runnum);
  sprintf(title,
	  "HF Tower Energy vs. Eta for >=5 Towers with E>20GeV, Run #%d;ieta;GeV",
	  runnum);
  h_EvsIeta_nTge5_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtaNonPMTh",runnum);
  sprintf(title,
	  "Average Summed Energy Per Event vs Eta for 'non-PMT' Events, Run #%d;ieta;GeV",
	  runnum);
  h_EvsIetaNonPMT_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  // ...but renumber the ticks so there's no confusion.
  for (int ibin=1; ibin<=13; ibin++) {
    ostringstream binlabel;
    binlabel<<(ibin-42);
    h_EvsIeta_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIeta_nTlt3_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIeta_nTlt5_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIeta_nTge5_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIetaNonPMT_->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    binlabel.str("");
    binlabel<<(ibin+28);
    h_EvsIeta_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIeta_nTlt3_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIeta_nTlt5_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIeta_nTge5_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIetaNonPMT_->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
  }

  sprintf(name,"run%dnLongVsShortLT3h",runnum);
  sprintf(title,
	  "Long vs. Short Energy for <3 Towers with E>20GeV, Run #%d; Short GeV; Long GeV",
	  runnum);
  h_LongVsShortE_nTlt3_ = RHsubDir_->make<TH2F>(name, title, 
						50,0.0,500.0,
						50,0.0,500.0);

  sprintf(name,"run%dnLongVsShortLT5h",runnum);
  sprintf(title,
	  "Long vs. Short Energy for <5 Towers with E>20GeV, Run #%d; Short GeV; Long GeV",
	  runnum);
  h_LongVsShortE_nTlt5_ = RHsubDir_->make<TH2F>(name, title,
						50,0.0,500.0,
						50,0.0,500.0);

  sprintf(name,"run%dnLongVsShortGE5h",runnum);
  sprintf(title,
	  "Long vs. Short Energy for >=5 Towers with E>20GeV, Run #%d; Short GeV; Long GeV",
	  runnum);
  h_LongVsShortE_nTge5_ = RHsubDir_->make<TH2F>(name, title,
						50,0.0,500.0,
						50,0.0,500.0);

  sprintf(name,"run%dnLongVsShorth",runnum);
  sprintf(title,
	  "Long vs. Short Energy for Towers with E>20GeV, Run #%d; Short GeV; Long GeV",
	  runnum);
  h_LongVsShortE_ = RHsubDir_->make<TH2F>(name, title,
					  50,0.0,500.0,
					  50,0.0,500.0);

  sprintf(name,"run%dEperPhih",runnum);
  sprintf(title,"HF RecHit Energy vs Phi, Run #%d;iphi;GeV", runnum);
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

  sprintf(name,"run%dTriggerEfficiencyVsThreshGoodBx",runnum);
  sprintf(title,"Trigger Efficiency vs. Threshold, Signal, Run #%d", runnum);
  h_trigEffvsThresh_      = fs->make<TH1F>(name, title, 101, -0.5,100.5);

  sprintf(name,"run%dTriggerEfficiencyVsThreshBadBx",runnum);
  sprintf(title,"Trigger Efficiency vs. Threshold, Noise, Run #%d", runnum);
  h_trigEffvsThreshBadBx_ = fs->make<TH1F>(name, title, 101, -0.5,100.5);

  sprintf(name,"run%dnPMThits",runnum);
  sprintf(title,"# PMT hits per event, Run #%d", runnum);
  h_nPMThits_ = fs->make<TH1F>(name, title, 51, -0.5,50.5);

  sprintf(name,"run%dnPMTsamesideHits",runnum);
  sprintf(title,"# PMT same-side hits per event, Run #%d", runnum);
  h_nPMTsamesideHits_ = fs->make<TH1F>(name, title, 51, -0.5,50.5);

  sprintf(name,"run%dnPMTopposideHits",runnum);
  sprintf(title,"# PMT opposite-side hits per event, Run #%d", runnum);
  h_nPMTopposideHits_ = fs->make<TH1F>(name, title, 51, -0.5,50.5);

  sprintf(name,"run%dnPMThitsDeltaIphi",runnum);
  sprintf(title,"#Delta i#phi(+HF,-HF) PMT hit pairs, Run #%d; #Delta i#phi", runnum);
  h_PMThitsDeltaIphi_ = fs->make<TH1F>(name, title, 37, -37.0,37.0);

  sprintf(name,"run%dnPMThitsDeltaIeta",runnum);
  sprintf(title,"#Delta i#eta(+HF,-HF) PMT hit pairs, Run #%d; #Delta i#eta", runnum);
  h_PMThitsDeltaIeta_ = fs->make<TH1F>(name, title, 27, -13.5,13.5);

  sprintf(name,"run%dnPMThitsAvgIphi",runnum);
  sprintf(title,"Avg i#phi(+HF,-HF) PMT hit pairs, Run #%d; Avg i#phi", runnum);
  h_PMThitsAvgIphi_ = fs->make<TH1F>(name, title, 72, -0.5,71.5);

  sprintf(name,"run%dnPMThitsAvgIeta",runnum);
  sprintf(title,"Avg i#eta(+HF,-HF) PMT hit pairs, Run #%d; Avg i#eta", runnum);
  h_PMThitsAvgIeta_ = fs->make<TH1F>(name, title, 13,28.5,41.5);

  sprintf(name,"run%dnPMThitsSameSideDeltaIphi",runnum);
  sprintf(title,
"#Delta i#phi(wedge1,wedge2) same-side PMT hit pairs, Run #%d; #Delta i#phi", runnum);
  h_samesidePMThitsDeltaIphi_ = fs->make<TH1F>(name, title, 37, -37.0,37.0);

  sprintf(name,"run%dnPMThitsSameSideDeltaIeta",runnum);
  sprintf(title,
"#Delta i#eta(wedge1,wedge2) same-side PMT hit pairs, Run #%d; #Delta i#eta", runnum);
  h_samesidePMThitsDeltaIeta_ = fs->make<TH1F>(name, title, 27, -13.5,13.5);

  sprintf(name,"run%dnPMThitsSameSideAvgIphi",runnum);
  sprintf(title,
"Avg i#phi(wedge1,wedge2) same-side PMT hit pairs, Run #%d; Avg i#phi", runnum);
  h_samesidePMThitsAvgIphi_ = fs->make<TH1F>(name, title, 37, -0.5,36.5);

  sprintf(name,"run%dnPMThitsSameSideAvgIeta",runnum);
  sprintf(title,
"Avg i#eta(wedge1,wedge2) same-side PMT hit pairs, Run #%d; Avg i#eta", runnum);
  h_samesidePMThitsAvgIeta_ = fs->make<TH1F>(name, title, 13, 28.5, 41.5);

}                                   // HFtrigAnalHistos::bookPerRunHistos

//======================================================================

TH2F *
HFtrigAnalHistos::bookOccHisto(int depth, uint32_t runnum, bool ismaxval)
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
}                                       // HFtrigAnalHistos::bookOccHisto

//======================================================================

TH1F *
HFtrigAnalHistos::bookSpectrumHisto(IetaDepth_t& id, uint32_t runnum)
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
}                                  // HFtrigAnalHistos::bookSpectrumHisto

//======================================================================

TH1F *
HFtrigAnalHistos::bookEperEventHisto(uint32_t nkevent,uint32_t runnum)
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
}                                 // HFtrigAnalHistos::bookEperEventHisto

//======================================================================

TH2F *
HFtrigAnalHistos::book2dEnergyHisto(uint32_t evtnum, uint32_t runnum)
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
}                                  // HFtrigAnalHistos::book2dEnergyHisto

//======================================================================

void
HFtrigAnalHistos::fillNwedges(bool goodBx,
			      int  nWedgesOverThresh,
			      int  nWedgesOverThreshPlus,
			      int  nWedgesOverThreshMinus)
{
  if (goodBx) {
    h_nWedgesOverThreshGoodBx_->Fill(nWedgesOverThresh);

    if (nWedgesOverThreshPlus  > 1)  h_PlusMinusTrigger_->Fill(1);
    if (nWedgesOverThreshMinus > 1)  h_PlusMinusTrigger_->Fill(-1);
    if (nWedgesOverThreshPlus &&
	nWedgesOverThreshMinus   )   h_PlusMinusTrigger_->Fill(0);
  } else {
    h_nWedgesOverThreshBadBx_->Fill(nWedgesOverThresh);

    if (nWedgesOverThreshPlus  > 1)  h_PlusMinusTriggerBadBx_->Fill(1);
    if (nWedgesOverThreshMinus > 1)  h_PlusMinusTriggerBadBx_->Fill(-1);
    if (nWedgesOverThreshPlus &&
	nWedgesOverThreshMinus   )   h_PlusMinusTriggerBadBx_->Fill(0);
  }
}                                       // HFtrigAnalHistos::fillNwedges

//======================================================================

void
HFtrigAnalHistos::fillTrigEffHistos(bool goodBx, uint32_t thresh)
{
  if (goodBx) h_trigEffvsThresh_->Fill(thresh);
  else        h_trigEffvsThreshBadBx_->Fill(thresh);
}

//======================================================================

void HFtrigAnalHistos::fillDigiSpectra(int ieta, int depth,
				       uint32_t maxadc, uint32_t runnum)
{
  // Fill histos with maximum ADC
  //
  IetaDepth_t id(ieta,depth);
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

void
HFtrigAnalHistos::fillOccupancyHistos(HcalDetId idA,
				      HcalDetId idB,
				      uint32_t runnum)
{
  std::map<int,TH2F *>::iterator it = m_hOccupancies1_.find(idA.depth());
  TH2F *hp = 0;
  if (it == m_hOccupancies1_.end()) hp = bookOccHisto(idA.depth(),runnum);
  else                              hp = it->second;

  if (hp) {
    if      (idA.ieta() >=  29 ) hp->Fill(idA.ieta()-28,idA.iphi()); 
    else if (idA.ieta() <= -29 ) hp->Fill(idA.ieta()+28,idA.iphi());
    //cout << "Filling ieta=" << idA.ieta() << ", iphi=" << idA.iphi() << endl;
  } else {
    cout << "pointer is zero!";
    cout << " ieta=" << idA.ieta() << ", iphi=" << idA.iphi();
    cout << ", depth = " << idA.depth() << endl;
  }

  if (idB == HcalDetId::Undefined) return;

  it = m_hOccupancies2_.find(idB.depth());
  hp = 0;
  if (it == m_hOccupancies2_.end()) hp = bookOccHisto(idB.depth(),runnum,false);
  else                              hp = it->second;

  if (hp) {
    if      (idB.ieta() >=  29 ) hp->Fill(idB.ieta()-28,idB.iphi()); 
    else if (idB.ieta() <= -29 ) hp->Fill(idB.ieta()+28,idB.iphi());
    //cout << "Filling ieta=" << idB.ieta() << ", iphi=" << idB.iphi() << endl;
  } else {
    cout << "pointer is zero!";
    cout << " ieta=" << idB.ieta() << ", iphi=" << idB.iphi();
    cout << ", depth = " << idB.depth() << endl;
  }
}                              //  HFtrigAnalHistos::fillOccupancyHistos

//======================================================================

void
HFtrigAnalHistos::fillPulseProfileHisto(vector<int>& v_samples)
{
  for (uint32_t i = 0; i < v_samples.size(); ++i)
    h_PulseProfileMax_->Fill(i, v_samples[i]);
}                             // HFtrigAnalHistos::fillPulseProfileHisto

//======================================================================

void
HFtrigAnalHistos::fillRHetaPhiDistHistos (int ieta, int iphi, float rhenergy)
{
  if      (ieta >=  29 ) h_EvsIeta_->Fill(ieta-28,rhenergy); 
  else if (ieta <= -29 ) h_EvsIeta_->Fill(ieta+28,rhenergy);
  h_EvsIphi_->Fill(iphi,rhenergy);
}

//======================================================================

void
HFtrigAnalHistos::fillTotalEnergyHistos(uint32_t evtnum,
					uint32_t runnum,
					float    totalE)
{
  uint32_t nkevents = (evtnum-1)/1000;
  TH1F *h1p;

  if ((nkevents+1) > v_ePerEventHistos_.size())
    h1p = bookEperEventHisto(nkevents,runnum);
  else if (!v_ePerEventHistos_[nkevents])
    h1p = bookEperEventHisto(nkevents,runnum);
  else
    h1p = v_ePerEventHistos_[nkevents];

  h1p->Fill(evtnum,totalE);
  h_totalE_->Fill(totalE);
}

//======================================================================

void
HFtrigAnalHistos::fillCenterOfEnergyHistos(float coEinPhiMinus,
					   float coEinPhiPlus,
					   float coEinEtaMinus,
					   float coEinEtaPlus)
{
  if (coEinPhiMinus >  0.0) h_CoEinPhiMinus_->Fill(coEinPhiMinus);
  if (coEinPhiPlus  >  0.0) h_CoEinPhiPlus_->Fill(coEinPhiPlus);
  if (coEinEtaMinus != 0.0) h_CoEinEtaMinus_->Fill(coEinPhiMinus);
  if (coEinEtaPlus  != 0.0) h_CoEinEtaPlus_->Fill(coEinPhiPlus);
}

//======================================================================

void
HFtrigAnalHistos::fillTowerEhistos(int   ntowers,
				   int   ieta,
				   float twrTotalE,
				   float twrShortE,
				   float twrLongE)
{
  int ie2f = 0;
  if      (ieta >=  29 ) ie2f = ieta-28;
  else if (ieta <= -29 ) ie2f = ieta+28;

  if (twrTotalE > 20.0) {
    if      (ntowers < 3) {h_EvsIeta_nTlt3_->Fill(ie2f,twrTotalE);h_LongVsShortE_nTlt3_->Fill(twrShortE,twrLongE);}
    else if (ntowers < 5) {h_EvsIeta_nTlt5_->Fill(ie2f,twrTotalE);h_LongVsShortE_nTlt5_->Fill(twrShortE,twrLongE);}
    else                  {h_EvsIeta_nTge5_->Fill(ie2f,twrTotalE);h_LongVsShortE_nTge5_->Fill(twrShortE,twrLongE);}
    h_LongVsShortE_->Fill(twrShortE,twrLongE);

    if ((twrShortE > 10.0) && (twrLongE> 10.0)) { // beam-gas event
      h_EvsIetaNonPMT_->Fill(ie2f,twrTotalE);
    }
  }
}                                  // HFtrigAnalHistos::fillTowerEhistos

//======================================================================

void
HFtrigAnalHistos::fillNtowersHisto(int ntowers)
{
  h_nTowersOverThresh_->Fill(ntowers);
}

//======================================================================

void
HFtrigAnalHistos::fillPMTeventHistos (std::vector<deltaAvg_t>& sameSidePMTpairs,
				      std::vector<deltaAvg_t>& oppoSidePMTpairs,
				      uint32_t nPMThits)
{
  h_nPMThits_->Fill(nPMThits);
  h_nPMTsamesideHits_->Fill(sameSidePMTpairs.size());
  h_nPMTopposideHits_->Fill(oppoSidePMTpairs.size());

  for (uint32_t i=0; i<oppoSidePMTpairs.size(); i++) {
    h_PMThitsDeltaIphi_->Fill(oppoSidePMTpairs[i].deltaIphi);
    h_PMThitsDeltaIeta_->Fill(oppoSidePMTpairs[i].deltaIeta);
    h_PMThitsAvgIphi_->Fill(oppoSidePMTpairs[i].avgIphi);
    h_PMThitsAvgIeta_->Fill(oppoSidePMTpairs[i].avgIeta);
  }

  for (uint32_t i=0; i<sameSidePMTpairs.size(); i++) {
    h_samesidePMThitsDeltaIphi_->Fill(sameSidePMTpairs[i].deltaIphi);
    h_samesidePMThitsDeltaIeta_->Fill(sameSidePMTpairs[i].deltaIeta);
    h_samesidePMThitsAvgIphi_->Fill(sameSidePMTpairs[i].avgIphi);
    h_samesidePMThitsAvgIeta_->Fill(sameSidePMTpairs[i].avgIeta);
  }
}                                // HFtrigAnalHistos::fillPMTeventHistos

//======================================================================

void
HFtrigAnalHistos::fillEvtInfoHistos(uint16_t bxnum,
				    uint32_t lsnum,
				    bool     goodBx)
{
  h_bx_->Fill(bxnum);
  h_lumiseg_->Fill(lsnum);
  if (goodBx)
    h_lumisegGoodBx_->Fill(lsnum);
}

//======================================================================

void
HFtrigAnalHistos::endJob(int nTotalNonPMTevents)
{
  h_EvsIeta_->Scale(1.0/h_totalE_->GetEntries());

  for (int i=1; i<=h_totalE_->GetNbinsX(); i++) {
    h_totalE_->SetBinContent(i, h_totalE_->GetBinContent(i)/h_totalE_->GetBinWidth(i));
  }

  h_trigEffvsThresh_->Scale(1.0/h_bx_->GetEntries());
  h_trigEffvsThreshBadBx_->Scale(1.0/h_bx_->GetEntries());

  h_EvsIetaNonPMT_->Scale(1.0/(double)nTotalNonPMTevents);

}
