#include <iostream>
#include "MyEDmodules/HFtrigAnal/src/HFtrigAnalAlgos.hh"
#include "MyEDmodules/HFtrigAnal/src/inSet.hh"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/GeometryVector/interface/Phi.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "math.h"
#include "TMath.h"
#include "TF1.h"

using namespace std;

//======================================================================

void HFtrigAnalHistos::getEtaPhi(IetaIphi_t ieip, double& eta, double& phi)
{
  HcalDetId id(HcalForward,ieip.ieta(),ieip.iphi(),1);

  GlobalPoint gp = geo_->getPosition(id);

  eta = gp.eta();
  phi = gp.phi();
}

template <class T>
inline T sqr(T x) { return x*x; }

//======================================================================

HFtrigAnalHistos::HFtrigAnalHistos(const edm::ParameterSet& iConfig)
{
  digiSpectrumHp_.nbins = iConfig.getUntrackedParameter<int>   ("digiSpectrumNbins");
  digiSpectrumHp_.min   = iConfig.getUntrackedParameter<double>("digiSpectrumMinADC");
  digiSpectrumHp_.max   = iConfig.getUntrackedParameter<double>("digiSpectrumMaxADC");

  rhTotalEnergyHp_.nbins= iConfig.getUntrackedParameter<int>   ("rhTotalEnergyNbins");
  rhTotalEnergyHp_.min  = iConfig.getUntrackedParameter<double>("rhTotalEnergyMinGeV");
  rhTotalEnergyHp_.max  = iConfig.getUntrackedParameter<double>("rhTotalEnergyMaxGeV");

  towerEnergyHp_.nbins  = iConfig.getUntrackedParameter<int>   ("towerEnergyNbins");
  towerEnergyHp_.min    = iConfig.getUntrackedParameter<double>("towerEnergyMinGeV");
  towerEnergyHp_.max    = iConfig.getUntrackedParameter<double>("towerEnergyMaxGeV");

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
  BGsubDir_   = new TFileDirectory(fs->mkdir( "BeamGasPlots" ));
  PMTsubDir_  = new TFileDirectory(fs->mkdir( "PMThitPlots" ));
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

  // Beam-Gas Histos:
  map<string,BeamGasHistos_t>::iterator it;

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
	  "HF Tower Energy vs. Eta for <3 Towers Over Threshold, Run #%d;ieta;GeV",
	  runnum);
  h_EvsIeta_nTlt3_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtahLT5h",runnum);
  sprintf(title,
	  "HF Tower Energy vs. Eta for <5 Towers Over Threshold, Run #%d;ieta;GeV",
	  runnum);
  h_EvsIeta_nTlt5_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtahGE5h",runnum);
  sprintf(title,
	  "HF Tower Energy vs. Eta for >=5 Towers Over Threshold, Run #%d;ieta;GeV",
	  runnum);
  h_EvsIeta_nTge5_ = RHsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtaBGlooseh",runnum);
  sprintf(title,
  "Beam-Gas Events, Loose Criteria, Run #%d;ieta;Average Summed Energy Per Event (GeV)",
	  runnum);
  bgloose_.h_EvsIeta = BGsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtaBGmediumh",runnum);
  sprintf(title,
  "Beam-Gas Events, Medium Criteria, Run #%d;ieta;Average Summed Energy Per Event (GeV)",
	  runnum);
  bgmedium_.h_EvsIeta = BGsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  sprintf(name,"run%dEperEtaBGtighth",runnum);
  sprintf(title,
  "Beam-Gas Events, Tight Criteria, Run #%d;ieta;Average Summed Energy Per Event (GeV)",
	  runnum);
  bgtight_.h_EvsIeta = BGsubDir_->make<TH1F>(name, title, 27,-13.5,13.5);

  // ...but renumber the ticks so there's no confusion.
  for (int ibin=1; ibin<=13; ibin++) {
    ostringstream binlabel;
    binlabel<<(ibin-42);
    h_EvsIeta_            ->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIeta_nTlt3_      ->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIeta_nTlt5_      ->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    h_EvsIeta_nTge5_      ->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    bgloose_. h_EvsIeta   ->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    bgmedium_.h_EvsIeta   ->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    bgtight_. h_EvsIeta   ->GetXaxis()->SetBinLabel(ibin,binlabel.str().c_str());
    binlabel.str("");
    binlabel<<(ibin+28);
    h_EvsIeta_            ->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIeta_nTlt3_      ->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIeta_nTlt5_      ->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    h_EvsIeta_nTge5_      ->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    bgloose_ .h_EvsIeta   ->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    bgmedium_.h_EvsIeta   ->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
    bgtight_ .h_EvsIeta   ->GetXaxis()->SetBinLabel(ibin+14,binlabel.str().c_str());
  }

  sprintf(name,"run%dEspectrumBGlooseh",runnum);
  sprintf(title,
  "Tower Energy Spectrum, Beam-Gas Events, Loose Criteria, Run #%d;Tower Energy (GeV)",
	  runnum);
  bgloose_.h_Espectrum  = BGsubDir_->make<TH1F>(name, title, 
					       towerEnergyHp_.nbins,
					       towerEnergyHp_.min,
					       towerEnergyHp_.max);

  sprintf(name,"run%dEspectrumBGmediumh",runnum);
  sprintf(title,
  "Tower Energy Spectrum, Beam-Gas Events, Medium Criteria, Run #%d;Tower Energy (GeV)",
	  runnum);
  bgmedium_.h_Espectrum = BGsubDir_->make<TH1F>(name, title,
					       towerEnergyHp_.nbins,
					       towerEnergyHp_.min,
					       towerEnergyHp_.max);

  sprintf(name,"run%dEspectrumBGtighth",runnum);
  sprintf(title,
  "Tower Energy Spectrum, Beam-Gas Events, Tight Criteria, Run #%d;Tower Energy (GeV)",
	  runnum);
  bgtight_.h_Espectrum  = BGsubDir_->make<TH1F>(name, title, 
					       towerEnergyHp_.nbins,
					       towerEnergyHp_.min,
					       towerEnergyHp_.max);

  sprintf(name,"run%dEmaxBGlooseh",runnum);
  sprintf(title,
  "Tower Energy Max, Beam-Gas Events, Loose Criteria, Run #%d;Tower Energy (GeV)",
	  runnum);
  bgloose_.h_Emax       = BGsubDir_->make<TH1F>(name, title, 
					       towerEnergyHp_.nbins,
					       towerEnergyHp_.min,
					       towerEnergyHp_.max);

  sprintf(name,"run%dEmaxBGmediumh",runnum);
  sprintf(title,
  "Tower Energy Max, Beam-Gas Events, Medium Criteria, Run #%d;Tower Energy (GeV)",
	  runnum);
  bgmedium_.h_Emax      = BGsubDir_->make<TH1F>(name, title,
					       towerEnergyHp_.nbins,
					       towerEnergyHp_.min,
					       towerEnergyHp_.max);

  sprintf(name,"run%dEmaxBGtighth",runnum);
  sprintf(title,
  "Tower Energy Max, Beam-Gas Events, Tight Criteria, Run #%d;Tower Energy (GeV)",
	  runnum);
  bgtight_.h_Emax       = BGsubDir_->make<TH1F>(name, title, 
						towerEnergyHp_.nbins,
						towerEnergyHp_.min,
						towerEnergyHp_.max);

  sprintf(name,"run%dEavgBGlooseh",runnum);
  sprintf(title,
  "Tower Energy Avg, Beam-Gas Events, Loose Criteria, Run #%d;Tower Energy (GeV)",
	  runnum);
  bgloose_.h_Eavg       = BGsubDir_->make<TH1F>(name, title, 
						towerEnergyHp_.nbins,
						towerEnergyHp_.min,
						towerEnergyHp_.max);

  sprintf(name,"run%dEavgBGmediumh",runnum);
  sprintf(title,
  "Tower Energy Avg, Beam-Gas Events, Medium Criteria, Run #%d;Tower Energy (GeV)",
	  runnum);
  bgmedium_.h_Eavg      = BGsubDir_->make<TH1F>(name, title,
						towerEnergyHp_.nbins,
						towerEnergyHp_.min,
						towerEnergyHp_.max);

  sprintf(name,"run%dEavgBGtighth",runnum);
  sprintf(title,
  "Tower Energy Avg, Beam-Gas Events, Tight Criteria, Run #%d;Tower Energy (GeV)",
	  runnum);
  bgtight_.h_Eavg       = BGsubDir_->make<TH1F>(name, title, 
						towerEnergyHp_.nbins,
						towerEnergyHp_.min,
						towerEnergyHp_.max);

  sprintf(name,"run%dEweightedEtaBGlooseh",runnum);
  sprintf(title,
	  "Energy-weighted Eta, Beam-Gas Events, Loose Criteria, Run #%d;#eta",
	  runnum);
  bgloose_.h_EweightedEta = BGsubDir_->make<TH1F>(name, title, 30,2.5,5.5);

  sprintf(name,"run%dEweightedEtaBGmediumh",runnum);
  sprintf(title,
	  "Energy-weighted Eta, Beam-Gas Events, Medium Criteria, Run #%d;#eta",
	  runnum);
  bgmedium_.h_EweightedEta = BGsubDir_->make<TH1F>(name, title, 30,2.5,5.5);

  sprintf(name,"run%dEweightedEtaBGtighth",runnum);
  sprintf(title,
	  "Energy-weighted Eta, Beam-Gas Events, Tight Criteria, Run #%d;#eta",
	  runnum);
  bgtight_.h_EweightedEta = BGsubDir_->make<TH1F>(name, title, 30,2.5,5.5);

  sprintf(name,"run%dEweightedPhiBGlooseh",runnum);
  sprintf(title,
	  "Energy-weighted Phi, Beam-Gas Events, Loose Criteria, Run #%d;#phi",
	  runnum);
  bgloose_.h_EweightedPhi = BGsubDir_->make<TH1F>(name, title, 40,-0.5,3.5);

  sprintf(name,"run%dEweightedPhiBGmediumh",runnum);
  sprintf(title,
	  "Energy-weighted Phi, Beam-Gas Events, Medium Criteria, Run #%d;#phi",
	  runnum);
  bgmedium_.h_EweightedPhi = BGsubDir_->make<TH1F>(name, title, 40,-0.5,3.5);

  sprintf(name,"run%dEweightedPhiBGtighth",runnum);
  sprintf(title,
	  "Energy-weighted Phi, Beam-Gas Events, Tight Criteria, Run #%d;#phi",
	  runnum);
  bgtight_.h_EweightedPhi = BGsubDir_->make<TH1F>(name, title, 40,-0.5,3.5);

  sprintf(name,"run%d2ndMomEweightedEtaBGlooseh",runnum);
  sprintf(title,
"2nd Moment E-weighted Eta, Beam-Gas Events, Loose Criteria, Run #%d;<(#eta-<#eta>_{E})^{2}>_{E}",
	  runnum);
  bgloose_.h_2ndMomEweightedEta = BGsubDir_->make<TH1F>(name, title, 50,-0.5,4.5);

  sprintf(name,"run%d2ndMomEweightedEtaBGmediumh",runnum);
  sprintf(title,
"2nd Moment E-weighted Eta, Beam-Gas Events, Medium Criteria, Run #%d;<(#eta-<#eta>_{E})^{2}>_{E}",
	  runnum);
  bgmedium_.h_2ndMomEweightedEta = BGsubDir_->make<TH1F>(name, title, 50,-0.5,4.5);

  sprintf(name,"run%d2ndMomEweightedEtaBGtighth",runnum);
  sprintf(title,
"2nd Moment E-weighted Eta, Beam-Gas Events, Tight Criteria, Run #%d;<(#eta-<#eta>_{E})^{2}>_{E}",
	  runnum);
  bgtight_.h_2ndMomEweightedEta = BGsubDir_->make<TH1F>(name, title, 50,-0.5,4.5);

  sprintf(name,"run%d2ndMomEweightedPhiBGlooseh",runnum);
  sprintf(title,
"2nd Moment E-weighted Phi, Beam-Gas Events, Loose Criteria, Run #%d;<(#phi-<#phi>_{E})^{2}>_{E}",
	  runnum);
  bgloose_.h_2ndMomEweightedPhi = BGsubDir_->make<TH1F>(name, title, 44,-0.5,10.5);

  sprintf(name,"run%d2ndMomEweightedPhiBGmediumh",runnum);
  sprintf(title,
"2nd Moment E-weighted Phi, Beam-Gas Events, Medium Criteria, Run #%d;<(#phi-<#phi>_{E})^{2}>_{E}",
	  runnum);
  bgmedium_.h_2ndMomEweightedPhi = BGsubDir_->make<TH1F>(name, title, 44,-0.5,10.5);

  sprintf(name,"run%d2ndMomEweightedPhiBGtighth",runnum);
  sprintf(title,
"2nd Moment E-weighted Phi, Beam-Gas Events, Tight Criteria, Run #%d;<(#phi-<#phi>_{E})^{2}>_{E}",
	  runnum);
  bgtight_.h_2ndMomEweightedPhi = BGsubDir_->make<TH1F>(name, title, 44,-0.5,10.5);

  sprintf(name,"run%dnBGhitsLooseVsLS",runnum);
  sprintf(title,
  "# Beam-Gas hits vs. Lumi Section, Loose Criteria, Run #%d; Lumi Section; # BG hits/event",
	  runnum);
  bgloose_.h_nHitsVsLumiSection = BGsubDir_->make<TH2F>(name, title,
						       lumiSegHp_.nbins,
						       lumiSegHp_.min,
						       lumiSegHp_.max,
						       11, -0.5,10.5);

  sprintf(name,"run%dnBGhitsMediumVsLS",runnum);
  sprintf(title,
  "# Beam-Gas hits vs. Lumi Section, Medium Criteria, Run #%d; Lumi Section; # BG hits/event",
	  runnum);
  bgmedium_.h_nHitsVsLumiSection = BGsubDir_->make<TH2F>(name, title,
							lumiSegHp_.nbins,
							lumiSegHp_.min,
							lumiSegHp_.max,
							11, -0.5,10.5);

  sprintf(name,"run%dnBGhitsTightVsLS",runnum);
  sprintf(title,
  "# Beam-Gas hits vs. Lumi Section, Tight Criteria, Run #%d; Lumi Section; # BG hits/event",
	  runnum);
  bgtight_.h_nHitsVsLumiSection = BGsubDir_->make<TH2F>(name, title,
						       lumiSegHp_.nbins,
						       lumiSegHp_.min,
						       lumiSegHp_.max,
						       11, -0.5,10.5);

  sprintf(name,"run%dnLongVsShortLT3h",runnum);
  sprintf(title,
	  "Long vs. Short Energy for <3 Towers Over Threshold, Run #%d; Short GeV; Long GeV",
	  runnum);
  h_LongVsShortE_nTlt3_ = RHsubDir_->make<TH2F>(name, title, 
						50,0.0,500.0,
						50,0.0,500.0);

  sprintf(name,"run%dnLongVsShortLT5h",runnum);
  sprintf(title,
	  "Long vs. Short Energy for <5 Towers Over Threshold, Run #%d; Short GeV; Long GeV",
	  runnum);
  h_LongVsShortE_nTlt5_ = RHsubDir_->make<TH2F>(name, title,
						50,0.0,500.0,
						50,0.0,500.0);

  sprintf(name,"run%dnLongVsShortGE5h",runnum);
  sprintf(title,
	  "Long vs. Short Energy for >=5 Towers Over Threshold, Run #%d; Short GeV; Long GeV",
	  runnum);
  h_LongVsShortE_nTge5_ = RHsubDir_->make<TH2F>(name, title,
						50,0.0,500.0,
						50,0.0,500.0);

  sprintf(name,"run%dnLongVsShorth",runnum);
  sprintf(title,
	  "Long vs. Short Energy for Towers Over Threshold, Run #%d; Short GeV; Long GeV",
	  runnum);
  h_LongVsShortE_ = RHsubDir_->make<TH2F>(name, title,
					  100,0.0,200.0,
					  100,0.0,200.0);

  sprintf(name,"run%dnLongVsShortBadBxh",runnum);
  sprintf(title,
 "Long vs. Short Energy for Towers Over Threshold outside Bx Window, Run #%d; Short GeV; Long GeV",
	  runnum);
  h_LongVsShortEBadBx_ = RHsubDir_->make<TH2F>(name, title,
					       100,0.0,200.0,
					       100,0.0,200.0);

  sprintf(name,"run%dLongE",runnum);
  sprintf(title,
	  "Long Energy (for high Short Energy events), Run #%d; Long GeV",
	  runnum);
  h_longE_ = RHsubDir_->make<TH1F>(name,title,100,0.0,50.0);

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

  /*****************  PMT HISTOS *******************/

  sprintf(name,"run%dnPMThits",runnum);
  sprintf(title,"# PMT hits per event, Run #%d", runnum);
  h_nPMThits_ = PMTsubDir_->make<TH1F>(name, title, 51, -0.5,50.5);

  sprintf(name,"run%dnPMThitsVsLS",runnum);
  sprintf(title,
	  "# PMT hits per event vs. Lumi Section, Run #%d; Lumi Section; # PMT hits/event",
	  runnum);
  h_nPMThitsVsLumiSection_ = PMTsubDir_->make<TH2F>(name, title,
						    lumiSegHp_.nbins,
						    lumiSegHp_.min,
						    lumiSegHp_.max,
						    51, -0.5,50.5);

  sprintf(name,"run%dnPMThitsPlus",runnum);
  sprintf(title, "# PMT hits/event, HF+, Run #%d; # PMT hits/event", runnum);
  h_nPMTplus_  = PMTsubDir_->make<TH1F>(name,title,21,-0.5,20.5);

  sprintf(name,"run%dePMThitsPlus",runnum);
  sprintf(title, "PMT energy/event, HF+, Run #%d; GeV", runnum);
  h_ePMTplus_  = PMTsubDir_->make<TH1F>(name,title,
					towerEnergyHp_.nbins,
					towerEnergyHp_.min,
					towerEnergyHp_.max);

  sprintf(name,"run%dnPMThitsMinus",runnum);
  sprintf(title, "# PMT hits/event, HF-, Run #%d; # PMT hits/event", runnum);
  h_nPMTminus_ = PMTsubDir_->make<TH1F>(name,title,21,-0.5,20.5);

  sprintf(name,"run%dePMThitsMinus",runnum);
  sprintf(title, "PMT energy/event, HF-, Run #%d; GeV", runnum);
  h_ePMTminus_ = PMTsubDir_->make<TH1F>(name,title,
					towerEnergyHp_.nbins,
					towerEnergyHp_.min,
					towerEnergyHp_.max);

  sprintf(name,"run%dnPMTasym",runnum);
  sprintf(title,
  "#PMT hit asymmetry, Run #%d; #frac{nPMThitsPlus-nPMThitsMinus}{nPMThitsPlus+nPMThitsMinus}",
	  runnum);
  h_nPMTasym_  = PMTsubDir_->make<TH1F>(name,title,55,-1.1,1.1);

  sprintf(name,"run%dePMTasym",runnum);
  sprintf(title,
  "PMT energy asymmetry, Run #%d; #frac{ePMThitsPlus-ePMThitsMinus}{ePMThitsPlus+ePMThitsMinus}",
	  runnum);
  h_ePMTasym_  = PMTsubDir_->make<TH1F>(name,title,55,-1.1,1.1);

  /***************** SAME PMT HISTOS FOR BAD BX #: *******************/

  sprintf(name,"run%dnPMThitsBadBx",runnum);
  sprintf(title,"# PMT hits per event outside Bx Window, Run #%d", runnum);
  h_nPMThitsBadBx_ = PMTsubDir_->make<TH1F>(name, title, 51, -0.5,50.5);

  sprintf(name,"run%dnPMThitsVsLSbadBx",runnum);
  sprintf(title,
"# PMT hits per event outside Bx Window vs. Lumi Section, Run #%d; Lumi Section; # PMT hits/event",
	  runnum);
  h_nPMThitsVsLumiSectionBadBx_ = PMTsubDir_->make<TH2F>(name, title,
							 lumiSegHp_.nbins,
							 lumiSegHp_.min,
							 lumiSegHp_.max,
							 51, -0.5,50.5);

  sprintf(name,"run%dnPMThitsPlusBadBx",runnum);
  sprintf(title, "# PMT hits/event outside Bx Window, HF+, Run #%d; # PMT hits/event", runnum);
  h_nPMTplusBadBx_  = PMTsubDir_->make<TH1F>(name,title,21,-0.5,20.5);

  sprintf(name,"run%dePMThitsPlusBadBx",runnum);
  sprintf(title, "PMT energy/event outside Bx Window, HF+, Run #%d; GeV", runnum);
  h_ePMTplusBadBx_  = PMTsubDir_->make<TH1F>(name,title,
					     towerEnergyHp_.nbins,
					     towerEnergyHp_.min,
					     towerEnergyHp_.max);

  sprintf(name,"run%dnPMThitsMinusBadBx",runnum);
  sprintf(title, "# PMT hits/event outside Bx Window, HF-, Run #%d; # PMT hits/event", runnum);
  h_nPMTminusBadBx_ = PMTsubDir_->make<TH1F>(name,title,21,-0.5,20.5);

  sprintf(name,"run%dePMThitsMinusBadBx",runnum);
  sprintf(title, "PMT energy/event outside Bx Window, HF-, Run #%d; GeV", runnum);
  h_ePMTminusBadBx_ = PMTsubDir_->make<TH1F>(name,title,
					     towerEnergyHp_.nbins,
					     towerEnergyHp_.min,
					     towerEnergyHp_.max);

  sprintf(name,"run%dnPMTasymBadBx",runnum);
  sprintf(title,
  "#PMT hit asymmetry for events outside Bx Window, Run #%d; #frac{nPMThitsPlus-nPMThitsMinus}{nPMThitsPlus+nPMThitsMinus}",
	  runnum);
  h_nPMTasymBadBx_  = PMTsubDir_->make<TH1F>(name,title,55,-1.1,1.1);

  sprintf(name,"run%dePMTasymBadBx",runnum);
  sprintf(title,
  "PMT energy asymmetry for events outside Bx Window, Run #%d; #frac{ePMThitsPlus-ePMThitsMinus}{ePMThitsPlus+ePMThitsMinus}",
	  runnum);
  h_ePMTasymBadBx_  = PMTsubDir_->make<TH1F>(name,title,55,-1.1,1.1);

  /***************** END PMT HISTOS FOR BAD BX # *******************/

  sprintf(name,"run%dnPMTsamesideHits",runnum);
  sprintf(title,"# PMT same-side hits per event, Run #%d", runnum);
  h_nPMTsamesideHits_ = PMTsubDir_->make<TH1F>(name, title, 51, -0.5,50.5);

  sprintf(name,"run%dnPMTopposideHits",runnum);
  sprintf(title,"# PMT opposite-side hits per event, Run #%d", runnum);
  h_nPMTopposideHits_ = PMTsubDir_->make<TH1F>(name, title, 51, -0.5,50.5);

  sprintf(name,"run%dnPMThitsDeltaIphi",runnum);
  sprintf(title,"#Delta i#phi(+HF,-HF) PMT hit pairs, Run #%d; #Delta i#phi", runnum);
  h_PMThitsDeltaIphi_ = PMTsubDir_->make<TH1F>(name, title, 37, -37.0,37.0);

  sprintf(name,"run%dnPMThitsDeltaIeta",runnum);
  sprintf(title,"#Delta i#eta(+HF,-HF) PMT hit pairs, Run #%d; #Delta i#eta", runnum);
  h_PMThitsDeltaIeta_ = PMTsubDir_->make<TH1F>(name, title, 27, -13.5,13.5);

  sprintf(name,"run%dnPMThitsAvgIphi",runnum);
  sprintf(title,"Avg i#phi(+HF,-HF) PMT hit pairs, Run #%d; Avg i#phi", runnum);
  h_PMThitsAvgIphi_ = PMTsubDir_->make<TH1F>(name, title, 72, -0.5,71.5);

  sprintf(name,"run%dnPMThitsAvgIeta",runnum);
  sprintf(title,"Avg i#eta(+HF,-HF) PMT hit pairs, Run #%d; Avg i#eta", runnum);
  h_PMThitsAvgIeta_ = PMTsubDir_->make<TH1F>(name, title, 13,28.5,41.5);

  sprintf(name,"run%dnPMThitsSameSideDeltaIphi",runnum);
  sprintf(title,
"#Delta i#phi(wedge1,wedge2) same-side PMT hit pairs, Run #%d; #Delta i#phi", runnum);
  h_samesidePMThitsDeltaIphi_ = PMTsubDir_->make<TH1F>(name, title, 37, -37.0,37.0);

  sprintf(name,"run%dnPMThitsSameSideDeltaIeta",runnum);
  sprintf(title,
"#Delta i#eta(wedge1,wedge2) same-side PMT hit pairs, Run #%d; #Delta i#eta", runnum);
  h_samesidePMThitsDeltaIeta_ = PMTsubDir_->make<TH1F>(name, title, 27, -13.5,13.5);

  sprintf(name,"run%dnPMThitsSameSideAvgIphi",runnum);
  sprintf(title,
"Avg i#phi(wedge1,wedge2) same-side PMT hit pairs, Run #%d; Avg i#phi", runnum);
  h_samesidePMThitsAvgIphi_ = PMTsubDir_->make<TH1F>(name, title, 37, -0.5,36.5);

  sprintf(name,"run%dnPMThitsSameSideAvgIeta",runnum);
  sprintf(title,
"Avg i#eta(wedge1,wedge2) same-side PMT hit pairs, Run #%d; Avg i#eta", runnum);
  h_samesidePMThitsAvgIeta_ = PMTsubDir_->make<TH1F>(name, title, 13, 28.5, 41.5);

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
HFtrigAnalHistos::fillnTowersOverThresh(uint32_t nTowersOverThreshold)
{
  h_nTowersOverThresh_->Fill(nTowersOverThreshold);
}

//======================================================================

void
HFtrigAnalHistos::fillTowerEhistos(uint32_t nTowersOverThreshold,
				   vector<TowerEnergies_t>& v_towers,
				   bool isGoodBx)
{
  for (uint32_t i=0; i<v_towers.size(); i++) {
    TowerEnergies_t& twr = v_towers[i];
    if (!isGoodBx) {
      h_LongVsShortEBadBx_->Fill(twr.shortE,twr.longE);
    } else {
      int ie2f = 0;
      int ieta = twr.ieip.ieta();
      if      (ieta >=  29 ) ie2f = ieta-28;
      else if (ieta <= -29 ) ie2f = ieta+28;

      if (nTowersOverThreshold < 5) {
	h_EvsIeta_nTlt5_->Fill(ie2f,twr.totalE);
	h_LongVsShortE_nTlt5_->Fill(twr.shortE,twr.longE);
	if (nTowersOverThreshold < 3) {
	  h_EvsIeta_nTlt3_->Fill(ie2f,twr.totalE);
	  h_LongVsShortE_nTlt3_->Fill(twr.shortE,twr.longE);
	}
      } else                  {
	h_EvsIeta_nTge5_->Fill(ie2f,twr.totalE);
	h_LongVsShortE_nTge5_->Fill(twr.shortE,twr.longE);
      }
      h_LongVsShortE_->Fill(twr.shortE,twr.longE);
    }
  }
}                                  // HFtrigAnalHistos::fillTowerEhistos

//======================================================================

void
HFtrigAnalHistos::fillBeamGasHistos(const HFtrigAnalEvent_t& ev,
				    BeamGasHistos_t& BG)
{
  double sumE            = 0.0;
  double sumEplus        = 0.0;
  double sumEweightedPhi = 0.0;
  double sumEweightedEta = 0.0;

  BeamGasInfo_t bgi(ev.runnum,ev.lsnum,ev.evtnum,ev.bxnum);

  bgi.ntowOverThresh = ev.towersOverThresh.size();

  for (uint32_t i=0; i<ev.towersOverThresh.size(); i++) {
    const TowerEnergies_t& twr = ev.towersOverThresh[i];

    if (twr.ieip.ieta() > 0) {
      double eta,phi;
      getEtaPhi(twr.ieip,eta,phi);

      sumEweightedEta += eta*twr.totalE;
      sumEweightedPhi += phi*twr.totalE;
      sumEplus += twr.totalE;
    }

    // Energy spectrum
    BG.h_Espectrum->Fill(twr.totalE);

    // max E and average E
    if (twr.totalE > bgi.Emax) bgi.Emax = twr.totalE;
    sumE += twr.totalE;

    // E vs. ieta
    int ie2f = 0;
    int ieta = twr.ieip.ieta();
    if      (ieta >=  29 ) ie2f = ieta-28;
    else if (ieta <= -29 ) ie2f = ieta+28;

    BG.h_EvsIeta->Fill(ie2f,twr.totalE);
  }

  bgi.Eavg = sumE/(double)bgi.ntowOverThresh;
  BG.h_nHitsVsLumiSection->Fill(bgi.lsnum,bgi.ntowOverThresh);
  BG.h_Eavg->Fill(bgi.Eavg);
  BG.h_Emax->Fill(bgi.Emax);

  if (sumEplus > 0.0) {
    bgi.EweightedEta = sumEweightedEta/sumEplus;
    bgi.EweightedPhi = sumEweightedPhi/sumEplus;
    BG.h_EweightedEta->Fill(bgi.EweightedEta);
    BG.h_EweightedPhi->Fill(bgi.EweightedPhi);

    // Now for the 2nd moment histos
    //
    double sum2ndMomPhi = 0.0;
    double sum2ndMomEta = 0.0;

    for (uint32_t i=0; i<ev.towersOverThresh.size(); i++) {
      const TowerEnergies_t& twr = ev.towersOverThresh[i];

      if (twr.ieip.ieta() > 0) {
	double eta,phi;
	getEtaPhi(twr.ieip,eta,phi);

	sum2ndMomEta += twr.totalE * sqr<double>(eta-bgi.EweightedEta);
	sum2ndMomPhi += twr.totalE * sqr<double>(phi-bgi.EweightedPhi);
      }
    }
    bgi.EweightedEta2ndMom = sum2ndMomEta/sumEplus;
    bgi.EweightedPhi2ndMom = sum2ndMomPhi/sumEplus;

    BG.h_2ndMomEweightedEta->Fill(bgi.EweightedEta2ndMom);
    BG.h_2ndMomEweightedPhi->Fill(bgi.EweightedPhi2ndMom);
  }

  BG.v_bginfo.push_back(bgi);
}                                 // HFtrigAnalHistos::fillBeamGasHistos

//======================================================================

void
HFtrigAnalHistos::fillLooseBeamGasHistos(const HFtrigAnalEvent_t& ev)
{
  fillBeamGasHistos(ev,bgloose_);
}

//======================================================================

void
HFtrigAnalHistos::fillMediumBeamGasHistos(const HFtrigAnalEvent_t& ev)
{
  fillBeamGasHistos(ev,bgmedium_);
}

//======================================================================

void
HFtrigAnalHistos::fillTightBeamGasHistos(const HFtrigAnalEvent_t& ev)
{
  fillBeamGasHistos(ev,bgtight_);
}

//======================================================================

void
HFtrigAnalHistos::fillPMTeventHistos (const std::vector<TowerEnergies_t>& v_PMThits,
				      const std::vector<deltaAvg_t>& sameSidePMTpairs,
				      const std::vector<deltaAvg_t>& oppoSidePMTpairs,
				      uint32_t lsnum,
				      bool     isGoodBx)
{
  uint32_t nPMThits = v_PMThits.size();
  int nPMTminus=0,nPMTplus=0;
  float ePMTminus=0.,ePMTplus=0.;
  for (uint32_t i=0; i<nPMThits; i++) {
    const TowerEnergies_t& twr = v_PMThits[i];
    if (twr.ieip.ieta() > 0) { nPMTplus++; ePMTplus +=twr.totalE; }
    else                     { nPMTminus++;ePMTminus+=twr.totalE; }
  }

  if (isGoodBx) {
    h_nPMThits_->Fill(nPMThits);
    h_nPMThitsVsLumiSection_->Fill(lsnum,nPMThits);
    h_nPMTplus_->Fill(nPMTplus);
    h_ePMTplus_->Fill(ePMTplus);
    h_nPMTminus_->Fill(nPMTminus);
    h_ePMTminus_->Fill(ePMTminus);
    if (nPMThits) {
      h_nPMTasym_->Fill((nPMTplus-nPMTminus)/(nPMTplus+nPMTminus));
      h_ePMTasym_->Fill((ePMTplus-ePMTminus)/(ePMTplus+ePMTminus));
    }
    
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
  } else {                               // BAD BX #
    h_nPMThitsBadBx_->Fill(nPMThits);
    h_nPMThitsVsLumiSectionBadBx_->Fill(lsnum,nPMThits);
    h_nPMTplusBadBx_->Fill(nPMTplus);
    h_ePMTplusBadBx_->Fill(ePMTplus);
    h_nPMTminusBadBx_->Fill(nPMTminus);
    h_ePMTminusBadBx_->Fill(ePMTminus);
    if (nPMThits) {
      h_nPMTasymBadBx_->Fill((nPMTplus-nPMTminus)/(nPMTplus+nPMTminus));
      h_ePMTasymBadBx_->Fill((ePMTplus-ePMTminus)/(ePMTplus+ePMTminus));
    }
  }
}                                // HFtrigAnalHistos::fillPMTeventHistos

//======================================================================

void
HFtrigAnalHistos::fillEvtInfoHistos(const HFtrigAnalEvent_t& ev)
{
  h_bx_->Fill(ev.bxnum);
  h_lumiseg_->Fill(ev.lsnum);
  if (ev.isGoodBx)
    h_lumisegGoodBx_->Fill(ev.lsnum);
}

//======================================================================

void
HFtrigAnalHistos::fillLongEhisto(double longE)
{
  h_longE_->Fill(longE);
}

//======================================================================

void
HFtrigAnalHistos::endJob(void)
{
  h_EvsIeta_->Scale(1.0/h_totalE_->GetEntries());

  for (int i=1; i<=h_totalE_->GetNbinsX(); i++) {
    h_totalE_->SetBinContent(i, h_totalE_->GetBinContent(i)/h_totalE_->GetBinWidth(i));
  }

  h_trigEffvsThresh_->Scale(1.0/h_bx_->GetEntries());
  h_trigEffvsThreshBadBx_->Scale(1.0/h_bx_->GetEntries());

  h_nPMTasym_->Scale(1.0/h_nPMTasym_->GetEntries());
  h_ePMTasym_->Scale(1.0/h_ePMTasym_->GetEntries());

  h_nPMTasymBadBx_->Scale(1.0/h_nPMTasymBadBx_->GetEntries());
  h_ePMTasymBadBx_->Scale(1.0/h_ePMTasymBadBx_->GetEntries());

  bgloose_.h_EvsIeta->Scale(1.0/(double)bgloose_.h_nHitsVsLumiSection->GetEntries());
  bgmedium_.h_EvsIeta->Scale(1.0/(double)bgmedium_.h_nHitsVsLumiSection->GetEntries());
  bgtight_.h_EvsIeta->Scale(1.0/(double)bgtight_.h_nHitsVsLumiSection->GetEntries());

  char s[128];
  cout << "============================ Loose Beam-Gas Events ============================" << endl;
  cout << "Run # LS# Event #  BX# Nt Emax(GeV) Eavg(GeV) <Eta>E <Phi>E 2ndMomEta 2ndMomPhi" << endl;
  for (uint32_t i=0; i<bgloose_.v_bginfo.size(); i++) {
    BeamGasInfo_t& bg = bgloose_.v_bginfo[i];
    sprintf (s,"%d %3d %7d %4d %2d %9.1f %9.1f %6.3f %6.3f %9.3f %9.3f\n",
	     bg.runnum,bg.lsnum,bg.evnum,bg.bxnum,bg.ntowOverThresh,bg.Emax,bg.Eavg,
	     bg.EweightedEta,bg.EweightedPhi,bg.EweightedEta2ndMom,bg.EweightedPhi2ndMom);
    cout << s;
  }

  cout << "=========================== Medium Beam-Gas Events ============================" << endl;
  cout << "Run # LS# Event #  BX# Nt Emax(GeV) Eavg(GeV) <Eta>E <Phi>E 2ndMomEta 2ndMomPhi" << endl;
  for (uint32_t i=0; i<bgmedium_.v_bginfo.size(); i++) {
    BeamGasInfo_t& bg = bgmedium_.v_bginfo[i];
    sprintf (s,"%d %3d %7d %4d %2d %9.1f %9.1f %6.3f %6.3f %9.3f %9.3f\n",
	     bg.runnum,bg.lsnum,bg.evnum,bg.bxnum,bg.ntowOverThresh,bg.Emax,bg.Eavg,
	     bg.EweightedEta,bg.EweightedPhi,bg.EweightedEta2ndMom,bg.EweightedPhi2ndMom);
    cout << s;
  }

  cout << "============================ Tight Beam-Gas Events ============================" << endl;
  cout << "Run # LS# Event #  BX# Nt Emax(GeV) Eavg(GeV) <Eta>E <Phi>E 2ndMomEta 2ndMomPhi" << endl;
  for (uint32_t i=0; i<bgtight_.v_bginfo.size(); i++) {
    BeamGasInfo_t& bg = bgtight_.v_bginfo[i];
    sprintf (s,"%d %3d %7d %4d %2d %9.1f %9.1f %6.3f %6.3f %9.3f %9.3f\n",
	     bg.runnum,bg.lsnum,bg.evnum,bg.bxnum,bg.ntowOverThresh,bg.Emax,bg.Eavg,
	     bg.EweightedEta,bg.EweightedPhi,bg.EweightedEta2ndMom,bg.EweightedPhi2ndMom);
    cout << s;
  }
}
