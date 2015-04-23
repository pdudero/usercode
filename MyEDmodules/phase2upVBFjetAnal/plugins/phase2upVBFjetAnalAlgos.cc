#include <cmath>

#include "FWCore/ServiceRegistry/interface/Service.h" 
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "phase2upVBFjetAnalAlgos.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

void phase2upVBFjetAnalAlgos::BOOK1D(const std::string& name,
				     const std::string& title,
				     int    nbinsx,
				     double minx,
				     double maxx) 
{
  myAnalHistos::HistoParams_t hpars1d(name,title,nbinsx,minx,maxx);
  v_hpars1d_.push_back(hpars1d);
} 

void phase2upVBFjetAnalAlgos::BOOK2D(const std::string& name,
				     const std::string& title,
				     int    nbinsx,
				     double minx,
				     double maxx,
				     int    nbinsy,
				     double miny,
				     double maxy) 
{
  myAnalHistos::HistoParams_t hpars2d(name,title,nbinsx,minx,maxx,nbinsy,miny,maxy);
  v_hpars2d_.push_back(hpars2d);

  //  h##name = fs->make<TH2F>(#name,title,nbinsx,lowx,highx,nbinsy,lowy,highy); h##name->Sumw2()

}

//macros for building barrel and endcap histos with one call
void phase2upVBFjetAnalAlgos::DBOOK1D(const std::string& name,
				     const std::string& title,
				     int    nbinsx,
				     double minx,
				     double maxx)
{
  BOOK1D("hB"+name,"Barrel "+title,nbinsx,minx,maxx);
  BOOK1D("hE"+name,"Endcap "+title,nbinsx,minx,maxx);
  BOOK1D("hF"+name,"Forward "+title,nbinsx,minx,maxx);
}

//macros for building barrel and endcap histos with one call
void phase2upVBFjetAnalAlgos::EBOOK1D(const std::string& name,
				     const std::string& title,
				     int    nbinsx,
				     double minx,
				     double maxx)
{
  BOOK1D("hBE"+name,"Barrel-Endcap "   +title,nbinsx,minx,maxx);
  BOOK1D("hBF"+name,"Barrel-Forward "  +title,nbinsx,minx,maxx);
  BOOK1D("hEE"+name,"Endcap-Endcap "   +title,nbinsx,minx,maxx);
  BOOK1D("hEF"+name,"Endcap-Forward "  +title,nbinsx,minx,maxx);
  BOOK1D("hFF"+name,"Forward-Forward " +title,nbinsx,minx,maxx);
}

void phase2upVBFjetAnalAlgos::DBOOK2D(const std::string& name,
				      const std::string& title,
				      int    nbinsx,
				      double minx,
				      double maxx,
				      int    nbinsy,
				      double miny,
				      double maxy) 
{
  BOOK2D("hB"+name,"Barrel "+title,nbinsx,minx,maxx,nbinsy,miny,maxy);
  BOOK2D("hE"+name,"Endcap "+title,nbinsx,minx,maxx,nbinsy,miny,maxy);
  BOOK2D("hF"+name,"Forward "+title,nbinsx,minx,maxx,nbinsy,miny,maxy);
}

#define PT (plotAgainstReco_)?std::string("reconstructed P_{T}") :std::string("generated P_{T}")
#define P (plotAgainstReco_)?std::string("generated P") :std::string("generated P")
#define SQ(_x) ((_x)*(_x))

using namespace reco;
using namespace std;

class MonitorElement;

phase2upVBFjetAnalAlgos::phase2upVBFjetAnalAlgos() : entry_(0) {}

phase2upVBFjetAnalAlgos::~phase2upVBFjetAnalAlgos() {
}

void phase2upVBFjetAnalAlgos::write() {
#if 0
   // Store the DAQ Histograms 
  if (outputFile_.size() != 0) {
    if (file_) {
      file_->Write(outputFile_.c_str());
      cout << "Benchmark output written to file " << outputFile_.c_str() << endl;
      file_->Close();
    }
  }
  else 
    cout << "No output file specified ("<<outputFile_<<"). Results will not be saved!" << endl;
#endif
} 

//======================================================================

void phase2upVBFjetAnalAlgos::setup(const edm::ParameterSet& benchPars)
{
  debug_ = 
    benchPars.getParameter<bool>("pfjBenchmarkDebug");
  isSignal_ = 
    benchPars.getParameter<bool>("isSignalDoTruthMatch");
  plotAgainstReco_ = 
    benchPars.getParameter<bool>("PlotAgainstRecoQuantities");
  deltaRMax_ = 
    benchPars.getParameter<double>("deltaRMax");	  
  recPt_cut_ = 
    benchPars.getParameter<double>("minrecPt"); 
  minMjj_cut_ = 
    benchPars.getParameter<double>("minMjjGeV"); 
  minDeltaEtajj_cut_ = 
    benchPars.getParameter<double>("minDeltaEtajj"); 
  doHemisphere_cut_ = 
    benchPars.getParameter<bool>("doHemisphereCut");
  maxplotE_ = 
    benchPars.getParameter<double>("maxplotE"); 
  maxEta_cut_ = 
    benchPars.getParameter<double>("maxEta"); 

  edm::Service<TFileService> fs;

  //TFileDirectory td = fs->mkdir("phase2upVBFjetAnalAlgos");

  // print parameters
  cout<< "phase2upVBFjetAnalAlgos Setup parameters =============================================="<<endl;
  cout << "debug              " << debug_<< endl;
  cout << "isSignal           " << isSignal_ << endl;
  cout << "plotAgainstReco    " << plotAgainstReco_ << endl;
  cout << "deltaRMax          " << deltaRMax_ << endl;
  cout << "recPt_cut          " << recPt_cut_ << endl;
  cout << "maxEta_cut         " << maxEta_cut_ << endl;
  cout << "minDeltaEtajj_cut_ " << minDeltaEtajj_cut_ << endl;
  cout << "minMjj_cut_        " << minMjj_cut_ << endl;
  cout << "doHemisphere_cut_  " << doHemisphere_cut_ << endl;
  cout << "maxplotE           " << maxplotE_ << endl;
  
  // Book histogram
  m_cuts_["none"]               = new myAnalCut(0,"none",*fs,false);
  m_cuts_["vbf"]                = new myAnalCut(1,"vbf",*fs,false);
  m_cuts_["vbf+veto"]           = new myAnalCut(2,"vbf+veto",*fs,false);
  m_cuts_["vbf+dynveto"]        = new myAnalCut(3,"vbf+dynveto",*fs,false);
  if (isSignal_) {
    m_cuts_["matchQs"]               = new myAnalCut(4,"matchQs",*fs,false);
    m_cuts_["vbfANDmatchQs"]         = new myAnalCut(5,"vbfANDmatchQs",*fs,false);
    m_cuts_["vbf+vetoANDmatchQs"]    = new myAnalCut(6,"vbf+vetoANDmatchQs",*fs,false);
    m_cuts_["vbf+dynvetoANDmatchQs"] = new myAnalCut(6,"vbf+dynvetoANDmatchQs",*fs,false);
  }


  BOOK1D("hPassesCut","Cut Histo",m_cuts_.size(), 0, m_cuts_.size());

  BOOK1D("hNQmatchJets","Number of Quark-matched jets", 3, -0.5, 2.5);

  // Jets inclusive  distributions  (Pt > 20 or specified recPt GeV)

  BOOK1D("hrecojet1pt","Jet 1 P_{T} Distribution;  Jet 1 p_{T} (GeV)",njptbins_, 0, maxplotE_);
  BOOK1D("hrecojet2pt","Jet 2 P_{T} Distribution;  Jet 2 p_{T} (GeV)",njptbins_, 0, maxplotE_);

  BOOK1D("hrecojet1eta","Jet 1 #eta Distribution;  Jet 1 #eta",50,-5,5);
  BOOK1D("hrecojet2eta","Jet 2 #eta Distribution;  Jet 2 #eta",50,-5,5);

  //BOOK2D("hrecojet1EtavsPt","Jet 1 reco P_{T} (GeV);  Jet 1 reco #eta",200,0,maxplotE_,100,-5,5);
  //BOOK2D("hrecojet2EtavsPt","Jet 2 reco P_{T} (GeV);  Jet 2 reco #eta",200,0,maxplotE_,100,-5,5);

  BOOK2D("hrecojet1vsjet2eta","Jet 1 vs Jet2 #eta; Jet 2 #eta; Jet 1 #eta",100,-5,5,100,-5,5);

  EBOOK1D("recoMjj","M(Jet1+Jet2); M(Jet1+Jet2) (GeV)",200,0,2*maxplotE_);
  EBOOK1D("recoDeltaEtajj","#Delta#eta(Jet1,Jet2); #Delta#eta(Jet1,Jet2)",45,0,9);
  BOOK2D("hrecoJetsPtBetweenVBFtagjets", "PT for jets between Tag jets; Jet Number; Jet p_{T} (GeV)", 50,0,50,200,0,maxplotE_);
  BOOK2D("hrecoJetsEtaBetweenVBFtagjets", "Eta for jets between tag jets; Jet Number; Jet #eta", 50,0,50,100,-5,5);

  BOOK1D("hrecoJetNtracks","Number of tracks in jet",100,0,100);
  BOOK1D("hrecoJet1Ntracks","Number of tracks in jet 1",100,0,100);
  BOOK1D("hrecoJet2Ntracks","Number of tracks in jet 2",100,0,100);

  BOOK1D("hrecoJet12vertexZ","Vertex Z of Jets 1 & 2", 5000,-1,1);

  char cutString[35];
  sprintf(cutString,"Jet multiplicity P_{T}>%4.1f GeV; Multiplicity", recPt_cut_);
  BOOK1D("hNjets",cutString,50, 0, 50);
  BOOK1D("hgenrecodeltaR","#DeltaR(GenJet,RecoJet); #DeltaR(GenJet,RecoJet); Number of Jets; ",100, 0, 5);

  BOOK1D("hmatchedgenjetsPt","Jets P_{T} Distribution; generated P_{T}; Number of Jets",200, 0, 1000);

  BOOK1D("hrecojetsPt","Jets P_{T} Distribution; reconstructed P_{T}; Number of Jets",200, 0, maxplotE_);

  BOOK1D("hmgenjetsEndcapPt","Jets P_{T} Distribution, 1.6<|#eta|<2.7; generated P_{T}; Number of Jets",200, 0, 1000);
  BOOK1D("hrecojetsEndcapPt","Jets P_{T} Distribution, 1.6<|#eta|<2.7; reconstructed P_{T}; Number of Jets",200, 0, maxplotE_);

  DBOOK1D("CHE","E (charged had); E (charged had, GeV); Number of Jets",100,0,maxplotE_);
  DBOOK1D("NHE","E (neutral had); E (neutral had, GeV); Number of Jets",100,0,maxplotE_);
  DBOOK1D("NEE","E (neutral em); E (neutral em, GeV); Number of Jets",100,0,maxplotE_);
  DBOOK1D("CEE","E (charged em); E (charged em, GeV); Number of Jets",100,0,maxplotE_);
  DBOOK1D("TEE","E (Total em); E (Total em, GeV); Number of Jets",100,0,maxplotE_);
  DBOOK1D("THE","E (Total had); E (Total had, GeV); Number of Jets",100,0,maxplotE_);
  DBOOK1D("TNE","E (Total neutral); E (Total neutral, GeV); Number of Jets",100,0,maxplotE_);
  DBOOK1D("TE", "E (Total); E (Total, GeV); Number of Jets",100,0,maxplotE_);
  DBOOK1D("NEF","Neutral energy fraction; E_{neut}/E_{total}; Number of Jets",100,0,1);
  DBOOK1D("EoverH","E/H; Number of Jets",100,0,10);
  DBOOK1D("HADF","Had fraction; E_{Had}/E_{total}; Number of Jets",100,0,1);

  DBOOK2D("EMvsHad","EM vs Had; EM (GeV); Had (GeV)",100,0,maxplotE_,100,0,maxplotE_);

  DBOOK2D("EMFj1vsPt","EM fraction vs Jet1 p_{T}; Jet1 p_{T}; Jet1 E_{EM}/E_{total}",200,0,maxplotE_,100,0,1);
  DBOOK2D("EMFj2vsPt","EM fraction vs Jet2 p_{T}; Jet2 p_{T}; Jet2 E_{EM}/E_{total}",200,0,maxplotE_,100,0,1);
  DBOOK2D("RPtj1coreVsPt","P_{T, core}/P_{T, jet1} vs Jet1 p_{T}; Jet1 p_{T}; P_{T, core}/P_{T, jet1}",200,0,maxplotE_,140,-0.5,3);
  DBOOK2D("RPtj2coreVsPt","P_{T, core}/P_{T, jet2} vs Jet1 p_{T}; Jet2 p_{T}; P_{T, core}/P_{T, jet2}",200,0,maxplotE_,140,-0.5,3);
  DBOOK2D("dRj1coreVsPt","#DeltaR(core,jet1) vs Jet1 p_{T}; Jet1 p_{T}; #DeltaR(core,jet1)",200,0,maxplotE_,61,-0.1,0.6);
  DBOOK2D("dRj2coreVsPt","#DeltaR(core,jet2) vs Jet2 p_{T}; Jet2 p_{T}; #DeltaR(core,jet2)",200,0,maxplotE_,61,-0.1,0.6);

  sprintf(cutString,"Jets #eta Distribution |#eta|<%4.1f; generated #eta; Number of Jets", maxEta_cut_);
  BOOK1D("hmatchedgenjetsEta",cutString,100, -5, 5);

  sprintf(cutString,"Jets #eta Distribution |#eta|<%4.1f; reconstructed #eta; Number of Jets", maxEta_cut_);
  BOOK1D("hrecojetsEta",cutString,100, -5, 5);

  BOOK1D("hrecojetsPhi","Jets #phi Distribution; reconstructed #phi; Number of Jets",100, -3.2, 3.2);
  BOOK1D("hmatchedgenjetsPhi","Jets #phi Distribution; generated #phi; Number of Jets",100, -3.2, 3.2);

  BOOK2D("hrecojetsEtavsPt","reco #eta vs. P_{T}; reconstructed P_{T}; reconstructed #eta", 200,0,1000,100,-5,5);
  BOOK2D("hrecoPFConstituentsEtavsPt","Constituents #eta vs. P_{T}; Jet Constituents reco P_{T} (GeV); Jet Constituents reco #eta", 200,0,200,100,-5,5);
  BOOK1D("hrecoPFConstitPtJet30","Constituents P_{T} for PFJetPt=30GeV, #eta=2.8; Jet Constituents reco P_{T} (GeV); # Constituents", 200,0,200);
  BOOK1D("hrecoPFConstitPtJet50","Constituents P_{T} for PFJetPt=50GeV, #eta=2.8; Jet Constituents reco P_{T} (GeV); # Constituents", 200,0,200);
  BOOK1D("hrecoPFConstitPtJet100","Constituents P_{T} for PFJetPt=100GeV, #eta=2.8; Jet Constituents reco P_{T} (GeV); # Constituents", 200,0,200);
  BOOK2D("hRPtvsEta","#DeltaP_{T}/P_{T} vs #eta; #eta; #DeltaP_{T}/P_{T}",200, -5., 5., 400,-1,3); 
  BOOK2D("hRNeutvsEta","R_{Neutral} vs #eta; #eta; #DeltaE/E (Neutral)",200, -5., 5., 400,-1,3); 
  BOOK2D("hRNEUTvsEta","R_{HCAL+ECAL} vs #eta; #eta; #DeltaE/E (ECAL+HCAL)",200, -5., 5., 400,-1,3); 
  BOOK2D("hRNONLvsEta","R_{HCAL+ECAL - Hcal Only} vs #eta; #eta; #DeltaE/E (ECAL+HCAL-only)",200, -5., 5., 200,-1,1); 
  BOOK2D("hRHCALvsEta","R_{HCAL} vs #eta; #eta; #DeltaE/E (HCAL)",200, -5., 5., 200,-1,1); 
  BOOK2D("hRHONLvsEta","R_{HCAL only} vs #eta; #eta; #DeltaE/E (HCAL Only)",200, -5., 5., 200,-1,1); 
  BOOK2D("hRCHEvsEta","R_{Charged had} vs #eta; #eta; #DeltaE/E (Charged had)",200, -5., 5., 400,-1,3); 
  BOOK2D("hRCEEvsEta","R_{Charged em} vs #eta; #eta; #DeltaE/E (Charged em)",200, -5., 5., 400,-1,3); 
  BOOK2D("hRTHEvsEta","R_{Total had} vs #eta; #eta; #DeltaE/E (Total had)",200, -5., 5., 400,-1,3); 
  BOOK2D("hRTEEvsEta","R_{Total em} vs #eta; #eta; #DeltaE/E (Total em)",200, -5., 5., 400,-1,3); 
  BOOK2D("hNCHvsEta","N_{Charged} vs #eta",200, -5., 5., 100,0.,100.);
  BOOK2D("hNCH0vsEta","N_{Charged} vs #eta, iter 0",200, -5., 5., 100,0.,100.);
  BOOK2D("hNCH1vsEta","N_{Charged} vs #eta, iter 1",200, -5., 5., 100,0.,100.);
  BOOK2D("hNCH2vsEta","N_{Charged} vs #eta, iter 2",200, -5., 5., 100,0.,100.);
  BOOK2D("hNCH3vsEta","N_{Charged} vs #eta, iter 3",200, -5., 5., 100,0.,100.);
  BOOK2D("hNCH4vsEta","N_{Charged} vs #eta, iter 4",200, -5., 5., 100,0.,100.);
  BOOK2D("hNCH5vsEta","N_{Charged} vs #eta, iter 5",200, -5., 5., 100,0.,100.);
  BOOK2D("hNCH6vsEta","N_{Charged} vs #eta, iter 6",200, -5., 5., 100,0.,100.);
  BOOK2D("hNCH7vsEta","N_{Charged} vs #eta, iter 7",200, -5., 5., 100,0.,100.);
  // delta Pt or E quantities for Barrel
  DBOOK1D("RPt","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",160,-1,3);
  DBOOK1D("RCHE","#DeltaE/E (charged had); Jets",100,-2,3);
  DBOOK1D("RNHE","#DeltaE/E (neutral had); Jets",100,-2,3);
  DBOOK1D("RNEE","#DeltaE/E (neutral em); Jets",100,-2,3);
  DBOOK1D("RCEE","#DeltaE/E (charged em); Jets",100,-2,3);
  DBOOK1D("RTEE","#DeltaE/E (Total em); Jets",100,-2,3);
  DBOOK1D("RTHE","#DeltaE/E (Total had); Jets",100,-2,3);
  DBOOK1D("Rneut","#DeltaE/E (neutral); Jets",100,-2,3);
  DBOOK1D("NCH"," #N_{charged}",100,0.,100.);
  DBOOK2D("RPtvsPt","#DeltaP_{T}/P_{T} vs P_{T};"+PT+";#DeltaP_{T}/P_{T}",500,0,1000, 400,-1,3);       //used to be 50 bin for each in x-direction
  DBOOK2D("RCHEvsPt","#DeltaE/E (charged had) vs P_{T};"+PT+";#DeltaE/E (charged had)",500,0,1000, 160,-1,3);
  DBOOK2D("RNHEvsPt","#DeltaE/E (neutral had) vs P_{T};"+PT+";#DeltaE/E (neutral had)",500,0,1000, 160,-1,3);
  DBOOK2D("RNEEvsPt","#DeltaE/E (neutral em) vs P_{T};"+PT+";#DeltaE/E (neutral em)",500,0,1000, 160,-1,3);
  DBOOK2D("RCEEvsPt","#DeltaE/E (Charged em) vs P_{T};"+PT+";#DeltaE/E (Charged em)",500,0,1000, 160,-1,3);
  DBOOK2D("RTHEvsPt","#DeltaE/E (Total had) vs P_{T};"+PT+";#DeltaE/E (Total had)",500,0,1000, 160,-1,3);
  DBOOK2D("RTEEvsPt","#DeltaE/E (Total em) vs P_{T};"+PT+";#DeltaE/E (Total em)",500,0,1000, 160,-1,3);
  DBOOK2D("RneutvsPt","#DeltaE/E (neutral) vs P_{T};"+PT+";#DeltaE/E (neutral)",500,0,1000, 160,-1,3);
  DBOOK2D("NCHvsPt", "N_{charged} vs P_{T}"       +PT+";N_{charged}",500,0,1000,100,0.,100.);
  DBOOK2D("NCH0vsPt","N_{charged} vs P_{T} iter 0;"+PT+";N_{charged}",500,0,1000,100,0.,100.);
  DBOOK2D("NCH1vsPt","N_{charged} vs P_{T} iter 1;"+PT+";N_{charged}",500,0,1000,100,0.,100.);
  DBOOK2D("NCH2vsPt","N_{charged} vs P_{T} iter 2;"+PT+";N_{charged}",500,0,1000,100,0.,100.);
  DBOOK2D("NCH3vsPt","N_{charged} vs P_{T} iter 3;"+PT+";N_{charged}",500,0,1000,100,0.,100.);
  DBOOK2D("NCH4vsPt","N_{charged} vs P_{T} iter 4;"+PT+";N_{charged}",500,0,1000,100,0.,100.);
  DBOOK2D("NCH5vsPt","N_{charged} vs P_{T} iter 5;"+PT+";N_{charged}",500,0,1000,100,0.,100.);
  DBOOK2D("NCH6vsPt","N_{charged} vs P_{T} iter 6;"+PT+";N_{charged}",500,0,1000,100,0.,100.);
  DBOOK2D("NCH7vsPt","N_{charged} vs P_{T} iter 7;"+PT+";N_{charged}",500,0,1000,100,0.,100.);
  

  DBOOK2D("RNEUTvsP","#DeltaE/E (ECAL+HCAL) vs P;"+P+";#DeltaE/E (ECAL+HCAL)",250, 0, 1000, 150,-1.5,1.5);
  DBOOK2D("RNONLvsP","#DeltaE/E (ECAL+HCAL-only) vs P;"+P+";#DeltaE/E (ECAL+HCAL-only)",250, 0, 1000, 150,-1.5,1.5);
  DBOOK2D("RHCALvsP","#DeltaE/E (HCAL) vs P;"+P+";#DeltaE/E(HCAL)",250, 0, 1000, 150,-1.5,1.5);
  DBOOK2D("RHONLvsP","#DeltaE/E (HCAL only) vs P;"+P+";#DeltaE/E (HCAL only)",250, 0, 1000, 150,-1.5,1.5);
  DBOOK1D("RPt20_40", "#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",160,-1,3);
  DBOOK1D("RPt40_60","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",160,-1,3);
  DBOOK1D("RPt60_80","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",160,-1,3);
  DBOOK1D("RPt80_100","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",160,-1,3);
  DBOOK1D("RPt100_150","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",160,-1,3);
  DBOOK1D("RPt150_200","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",160,-1,3);
  DBOOK1D("RPt200_250","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",160,-1,3);
  DBOOK1D("RPt250_300","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",160,-1,3);
  DBOOK1D("RPt300_400","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",320,-1,3);
  DBOOK1D("RPt400_500","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",320,-1,3);
  DBOOK1D("RPt500_750","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",320,-1,3);
  DBOOK1D("RPt750_1250","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",320,-1,3);
  DBOOK1D("RPt1250_2000","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",320,-1,3);
  DBOOK1D("RPt2000_5000","#DeltaP_{T}/P_{T}; #DeltaP_{T}/P_{T}; Jets",320,-1,3);

  DBOOK2D("DEtavsPt","#Delta#eta vs P_{T};"+PT+";#Delta#eta",1000,0,2000,500,-0.5,0.5);
  DBOOK2D("DPhivsPt","#Delta#phi vs P_{T};"+PT+";#Delta#phi",1000,0,2000,500,-0.5,0.5);
  BOOK2D("hDEtavsEta","#Delta#eta vs #eta; #eta; #Delta#eta",1000,-5.,+5.,500,-0.5,0.5);
  BOOK2D("hDPhivsEta","#Delta#phi vs #eta; #eta; #Delta#phi",1000,-5.,+5.,500,-0.5,0.5);

  BOOK1D("hgenjetsPt","Jets P_{T} Distribution; generated P_{T}; Number of Jets",200, 0, 1000);

  BOOK1D("hgenjetsEndcapPt","Jets P_{T} Distribution, 1.6<|#eta|<2.7; generated P_{T}, 1.6<|#eta|<2.7; Number of Jets",200, 0, 1000);

  sprintf(cutString,"Jets #eta Distribution |#eta|<%4.1f; generated #eta; Number of Jets", maxEta_cut_);
  BOOK1D("hgenjetsEta",cutString,100, -5, 5);
  BOOK1D("hgenjetsPhi","Jets #phi Distribution; generated #phi; Number of Jets",100, -3.2, 3.2);

  BOOK2D("hsumdRvsSumDpt","min(#Sigma dR(2genjets,2recojets) vs #Sigma |dPt(2genjets,2recojets)|; #Sigma |dPt| (GeV); min(#Sigma dR)", 500,0,500,50,0,10);
 

  /*****************************************
   * BOOK 'EM, DANNO... *
   *****************************************/
  uint32_t total = v_hpars1d_.size()+v_hpars2d_.size();
  cout<<"Booking "<<total<<" histos for each of ";
  cout<<m_cuts_.size()<<" cuts..."<<std::endl;
  std::map<string, myAnalCut *>::const_iterator cutit;
  for (cutit = m_cuts_.begin(); cutit != m_cuts_.end(); cutit++) {
    cout << "Booking for cut " << cutit->first << endl;
    myAnalHistos *cutAH = cutit->second->cuthistos();
    cutAH->book1d<TH1F> (v_hpars1d_);
    cutAH->book2d<TH2F> (v_hpars2d_);

    double coarseptbins[]  = {0., 50., 100., 500.};
    double fineptbins[]    = {0.,30.,40.,60.,80.,100.,150.,200.,500.};
    double coarseetabins[] = {0.,1.4,1.6,2.7,3.0,5.0};

    cutAH->book2d<TH2F>("hrecojetsCoarsePtVsEta","Jet 1&2 reco #eta vs. P_{T}; Jet reco #eta;  Jet reco P_{T} (GeV)",20,-5,5,3,coarseptbins);
    cutAH->book2d<TH2F>("hrecojetsAbsEtavsPt","Jet 1&2 reco #eta vs. P_{T}; Jet reco P_{T} (GeV);  Jet reco |#eta|",8,fineptbins,4,coarseetabins);

    cutAH->book2d<TH2F>("hRPTvsCoarsePt","Jet 1&2 reco #DeltaP_{T}/P_{T}; Quark P_{T} (GeV); #DeltaP_{T}/P_{T}",3,coarseptbins,40,-1,3);
 }
}                                      // phase2upVBFjetAnalAlgos::setup

//======================================================================

myAnalCut *
phase2upVBFjetAnalAlgos::getCut(const std::string& cutstr)
{
  std::map<std::string,myAnalCut *>::const_iterator it = m_cuts_.find(cutstr);
  if (it == m_cuts_.end()) {
    throw cms::Exception("Cut not found: ") << cutstr << endl;
  }
  return it->second;

}                                    // phase2upVBFjetAnalAlgos::getCut

//======================================================================

void
phase2upVBFjetAnalAlgos::calcQGdiscriminators(const reco::PFJetCollection& pfJets,
					      const reco::PFJetCollection& pfJetCores)
{
  dRjet1core_ = -1;
  dRjet1core_ = -1;
  RPTjet1core_ = -1;
  RPTjet2core_ = -1;

  if (pfJets.size()     < 2) return;
  if (pfJetCores.size() < 2) return;

  assert (pfJets[0].pt() >= pfJets[1].pt());

  double mindRsq1=1e99, mindRsq2=1e99;
  unsigned mindRj1=99, mindRj2=99;
  for( unsigned i = 0; i < pfJetCores.size(); i++ ) {
    const reco::PFJet& pfjc = pfJetCores[i];

    double dRsq1 = SQ(pfJets[0].eta()-pfjc.eta()) + SQ(pfJets[0].phi()-pfjc.phi());
    double dRsq2 = SQ(pfJets[1].eta()-pfjc.eta()) + SQ(pfJets[1].phi()-pfjc.phi());

    if (dRsq1 < mindRsq1) { mindRsq1 = dRsq1; mindRj1 = i; }
    if (dRsq2 < mindRsq2) { mindRsq2 = dRsq2; mindRj2 = i; }
  }

  dRjet1core_ = sqrt(mindRsq1);
  dRjet2core_ = sqrt(mindRsq2);

  if (pfJets[0].pt()) RPTjet1core_ = pfJetCores[mindRj1].pt()/pfJets[0].pt();
  if (pfJets[1].pt()) RPTjet2core_ = pfJetCores[mindRj2].pt()/pfJets[1].pt();

}                       // phase2upVBFjetAnalAlgos::calcQGdiscriminators

//======================================================================

bool
phase2upVBFjetAnalAlgos::calcVBFCut(const reco::PFJetCollection& pfJets)
{
  if (pfJets.size() < 2) return false;

  assert (pfJets[0].pt() >= pfJets[1].pt());

  // both jets above the pT threshold?
  if (pfJets[1].pt() < recPt_cut_) return false;

  // both jets in acceptance?
  if (abs(pfJets[0].eta()) > 4.7 ||
      abs(pfJets[1].eta()) > 4.7    ) return false;

  // jets in opposite hemispheres?
  if (doHemisphere_cut_ &&
      pfJets[0].eta() * pfJets[1].eta() > 0) return false;

  // jets separated by at least 4.0 in eta?
  if (abs(pfJets[0].eta()-pfJets[1].eta()) < minDeltaEtajj_cut_) return false;

  // is dijet invariant mass big enough?
  double mjj = (pfJets[0].p4()+pfJets[1].p4()).M();
  if (mjj < minMjj_cut_)
    return false;

  return true;
}                                 // phase2upVBFjetAnalAlgos::calcVBFCut

//======================================================================

bool
phase2upVBFjetAnalAlgos::calcVBFgapjetveto(const reco::PFJetCollection& pfJets,
					   bool dynamicveto)
{
  if (pfJets.size() < 2) return false;

  double gapjetptthreshold = recPt_cut_;
  if (dynamicveto) {
    double binwidth= maxplotE_/njptbins_;
    gapjetptthreshold = binwidth * floor(pfJets[1].pt()/binwidth);
    if (debug_)
      cout << "gapjetpthreshold = " << gapjetptthreshold << endl;
  }

  // no jets of substance in between them?
  for (unsigned i = 2; i < pfJets.size() && pfJets[i].pt() > gapjetptthreshold; i++)
    if ((pfJets[i].eta() > pfJets[0].eta() && pfJets[1].eta() > pfJets[i].eta()) ||
	(pfJets[i].eta() < pfJets[0].eta() && pfJets[1].eta() < pfJets[i].eta())    )
      return false;

  return true;

}                          // phase2upVBFjetAnalAlgos::calcVBFgapjetveto

//======================================================================

static int lev=0;

void printDaughters(const reco::Candidate *c)
{
  lev++;

  int nglus=0;

  double ceta = c->eta(), cphi=c->phi();

  // Loop over daughters
  size_t ndau = c->numberOfDaughters();
  if (abs(c->pdgId()) <=6) {
    for(size_t j = 0; j < ndau; ++j) {
      double cdeta=c->daughter(j)->eta();
      double cdphi=c->daughter(j)->phi();
      double deltarsq=((ceta-cdeta)*(ceta-cdeta) + (cphi-cdphi)*(cphi-cdphi));

      if (c->daughter(j)->pdgId() == 21) nglus++;
      else {
	for (int i=0; i<lev; i++) cout<<"   ";
	cout<<"id="<<c->daughter(j)->pdgId()<<" st="<<c->daughter(j)->status()<<" pt="<<c->daughter(j)->pt();
	cout<<" nmth="<<c->numberOfMothers()<<" dRfromMother="<<sqrt(deltarsq)<<endl;
	if (c->daughter(j)->pdgId()==c->pdgId())
	  printDaughters(c->daughter(j));
      }


    } // end ndaughter loop

    if (nglus) { for (int i=0; i<lev; i++) cout<<"   "; cout << "+"<<nglus<<" gluons"<<endl; }
  }

  lev--;
}


//======================================================================

bool
phase2upVBFjetAnalAlgos::calcMatchQuarkCut(const reco::GenParticleCollection& genParts,
					   const reco::PFJetCollection& pfJets)
{
  nQmatchJets_=0;

  if (pfJets.size() < 2) return false;

  assert (pfJets[0].pt() >= pfJets[1].pt());

  cout << "=============================================================" << endl;      

  int nvbfq=0;

  for( unsigned i = 0; i < genParts.size(); i++ ) {
    const reco::Candidate *p = &genParts[i];

    if (!p)  { cout << "Null particle pointer for index "<< i << "!" << endl; continue; }

    // Find the VBF quarks
    if ( p->status() == 3 ) {
      if (p->numberOfMothers()==2 ) {
	cout<<"id="<<p->pdgId()<<" st="<<p->status()<<" pt="<<p->pt()<<" nmth="<<p->numberOfMothers();
	cout<<" eta,phi="<<p->eta()<<","<<p->phi()<<endl;
	printDaughters(p);
	if (abs(p->pdgId()) <=6) {
	  vbfq_[nvbfq++]=(reco::Candidate *)p;
	}
	if (nvbfq==2) break;
      }
    }
#if 0
    else
      break; // all done with the hard interaction constituents
#endif
  }

  if (nvbfq < 2) { cout << "Couldn't find VBF quarks!" << endl; return false; }

  double dRsq00 = SQ(pfJets[0].eta()-vbfq_[0]->eta()) + SQ(pfJets[0].phi()-vbfq_[0]->phi());
  double dRsq11 = SQ(pfJets[1].eta()-vbfq_[1]->eta()) + SQ(pfJets[1].phi()-vbfq_[1]->phi());
  double dRsq10 = SQ(pfJets[1].eta()-vbfq_[0]->eta()) + SQ(pfJets[1].phi()-vbfq_[0]->phi());
  double dRsq01 = SQ(pfJets[0].eta()-vbfq_[1]->eta()) + SQ(pfJets[0].phi()-vbfq_[1]->phi());

  nQmatchJets_ = (dRsq00<0.04) + (dRsq11<0.04);

  if (nQmatchJets_ == 2) {
    q1pt_ = vbfq_[0]->pt();
    q2pt_ = vbfq_[1]->pt();
    j1q1deltaptoverpt_=(pfJets[0].pt()-q1pt_)/q1pt_;
    j2q2deltaptoverpt_=(pfJets[1].pt()-q2pt_)/q2pt_;
  } else {
    nQmatchJets_ = std::max<int>(nQmatchJets_,(dRsq10<0.04) + (dRsq01<0.04) );
    if (nQmatchJets_ == 2) {
      q1pt_ = vbfq_[1]->pt();
      q2pt_ = vbfq_[0]->pt();
      j1q1deltaptoverpt_=(pfJets[0].pt()-q1pt_)/q1pt_;
      j2q2deltaptoverpt_=(pfJets[1].pt()-q2pt_)/q2pt_;
    }
  }
  return (nQmatchJets_==2);
  //return true;
}                          // phase2upVBFjetAnalAlgos::calcMatchQuarkCut

//======================================================================

void
phase2upVBFjetAnalAlgos::fillHistos4cut(const reco::PFJetCollection& pfJets,
					const reco::GenJetCollection& genJets,
					myAnalCut& thisCut)
{
  myAnalHistos *myAH;
  if (thisCut.isActive()) {
    if (thisCut.doInverted()) {
      myAH = thisCut.invhistos();
    } else
      return;
  } else {
    myAH = thisCut.cuthistos();
  }

  const double smallval = 0.0001;

  // loop over reco  pf  jets
  resPtMax_ = 0.;
  resChargedHadEnergyMax_ = 0.;
  resNeutralHadEnergyMax_ = 0.;
  resNeutralEmEnergyMax_ = 0.; 
  int NPFJets = 0;

  unsigned i1,i2;

  myAH->fill1d<TH1F>("hPassesCut",thisCut.getNum());

  myAH->fill1d<TH1F>("hNQmatchJets",nQmatchJets_);

  // FILL j1/j2 HISTOS FIRST

  if (pfJets.size()>1) {
    double j1pt = pfJets[0].pt();
    double j2pt = pfJets[1].pt();
    double j1eta = pfJets[0].eta();
    double j2eta = pfJets[1].eta();
    double recomjj = (pfJets[0].p4()+pfJets[1].p4()).M();
    double detajj  = abs(j1eta-j2eta);

    myAH->fill1d<TH1F>("hrecojet1pt",j1pt);
    myAH->fill1d<TH1F>("hrecojet1eta",j1eta);
    myAH->fill1d<TH1F>("hrecoJet1Ntracks",pfJets[0].getTrackRefs().size());


    myAH->fill1d<TH1F>("hrecojet2pt",j2pt);
    myAH->fill1d<TH1F>("hrecojet2eta",j2eta);
    myAH->fill1d<TH1F>("hrecoJet2Ntracks",pfJets[1].getTrackRefs().size());

    myAH->fill2d<TH2F>("hrecojetsCoarsePtVsEta",j1eta,j1pt);
    myAH->fill2d<TH2F>("hrecojetsCoarsePtVsEta",j2eta,j2pt);
    myAH->fill2d<TH2F>("hrecojetsAbsEtavsPt",   j1pt,abs(j1eta));
    myAH->fill2d<TH2F>("hrecojetsAbsEtavsPt",   j2pt,abs(j2eta));

    myAH->fill2d<TH2F>("hrecojet1vsjet2eta",j1eta,j2eta);

    myAH->fill1d<TH1F>("hrecoJet12vertexZ",pfJets[0].vertex().Z());
    myAH->fill1d<TH1F>("hrecoJet12vertexZ",pfJets[1].vertex().Z());

    if (matchQs_) {
      myAH->fill2d<TH2F>("hRPTvsCoarsePt",q1pt_,j1q1deltaptoverpt_);
      myAH->fill2d<TH2F>("hRPTvsCoarsePt",q2pt_,j2q2deltaptoverpt_);
    }

    bool j1Barrel = false;
    bool j1Endcap = false;
    bool j1Forward = false;
    double rec_ChargedHadEnergy    = pfJets[0].chargedHadronEnergy();
    double rec_NeutralHadEnergy    = pfJets[0].neutralHadronEnergy();
    double rec_NeutralEmEnergy     = pfJets[0].neutralEmEnergy();
    double rec_ChargedEmEnergy     = pfJets[0].chargedEmEnergy();;
    //double rec_ChargedMultiplicity = pfJets[0].chargedMultiplicity();
    double rec_TotalHadEnergy      = rec_ChargedHadEnergy + rec_NeutralHadEnergy;
    double rec_TotalEmEnergy       = rec_ChargedEmEnergy  + rec_NeutralEmEnergy;
    //double rec_NeutralEnergy       = rec_NeutralHadEnergy + rec_NeutralEmEnergy;
    double rec_TotalEnergy         = rec_TotalHadEnergy   + rec_TotalEmEnergy;

    if (std::abs (j1eta) < 1.4 ) j1Barrel = true;
    if (std::abs (j1eta) > 1.6 && std::abs (j1eta) < 2.7 ) j1Endcap = true;
    if (std::abs (j1eta) > 3.0 && std::abs (j1eta) < 4.7 ) j1Forward = true;

    if (j1Barrel) {
      myAH->fill2d<TH2F>("hBdRj1coreVsPt",j1pt,dRjet1core_);
      myAH->fill2d<TH2F>("hBRPtj1coreVsPt",j1pt,RPTjet1core_);
      if (rec_TotalEnergy > smallval)
	myAH->fill2d<TH2F>("hBEMFj1vsPt",j1pt,rec_TotalEmEnergy/rec_TotalEnergy);
    } else if (j1Endcap) {
      myAH->fill2d<TH2F>("hEdRj1coreVsPt",j1pt,dRjet1core_);
      myAH->fill2d<TH2F>("hERPtj1coreVsPt",j1pt,RPTjet1core_);
      if (rec_TotalEnergy > smallval)
	myAH->fill2d<TH2F>("hEEMFj1vsPt",j1pt,rec_TotalEmEnergy/rec_TotalEnergy);
    } else if (j1Forward) {
      myAH->fill2d<TH2F>("hFdRj1coreVsPt",j1pt,dRjet1core_);
      myAH->fill2d<TH2F>("hFRPtj1coreVsPt",j1pt,RPTjet1core_);
      if (rec_TotalEnergy > smallval)
	myAH->fill2d<TH2F>("hFEMFj1vsPt",j1pt,rec_TotalEmEnergy/rec_TotalEnergy);
    }

    bool j2Barrel = false;
    bool j2Endcap = false;
    bool j2Forward = false;

    if (std::abs (j2eta) < 1.4 ) j2Barrel = true;
    if (std::abs (j2eta) > 1.6 && std::abs (j2eta) < 2.7 ) j2Endcap = true;
    if (std::abs (j2eta) > 3.0 && std::abs (j2eta) < 4.7 ) j2Forward = true;

    rec_ChargedHadEnergy    = pfJets[1].chargedHadronEnergy();
    rec_NeutralHadEnergy    = pfJets[1].neutralHadronEnergy();
    rec_NeutralEmEnergy     = pfJets[1].neutralEmEnergy();
    rec_ChargedEmEnergy     = pfJets[1].chargedEmEnergy();;
    //rec_ChargedMultiplicity = pfJets[1].chargedMultiplicity();
    rec_TotalHadEnergy      = rec_ChargedHadEnergy + rec_NeutralHadEnergy;
    rec_TotalEmEnergy       = rec_ChargedEmEnergy  + rec_NeutralEmEnergy;
    //rec_NeutralEnergy       = rec_NeutralHadEnergy + rec_NeutralEmEnergy;
    rec_TotalEnergy         = rec_TotalHadEnergy   + rec_TotalEmEnergy;

    if (j2Barrel) {
      myAH->fill2d<TH2F>("hBdRj2coreVsPt",j2pt,dRjet2core_);
      myAH->fill2d<TH2F>("hBRPtj2coreVsPt",j2pt,RPTjet2core_);
      if (rec_TotalEnergy > smallval)
	myAH->fill2d<TH2F>("hBEMFj2vsPt",j2pt,rec_TotalEmEnergy/rec_TotalEnergy);
    } else if (j2Endcap) {
      myAH->fill2d<TH2F>("hEdRj2coreVsPt",j2pt,dRjet2core_);
      myAH->fill2d<TH2F>("hERPtj2coreVsPt",j2pt,RPTjet2core_);
      if (rec_TotalEnergy > smallval)
	myAH->fill2d<TH2F>("hEEMFj2vsPt",j2pt,rec_TotalEmEnergy/rec_TotalEnergy);
    } else if (j2Forward) {
      myAH->fill2d<TH2F>("hFdRj2coreVsPt",j2pt,dRjet2core_);
      myAH->fill2d<TH2F>("hFRPtj2coreVsPt",j2pt,RPTjet2core_);
      if (rec_TotalEnergy > smallval)
	myAH->fill2d<TH2F>("hFEMFj2vsPt",j2pt,rec_TotalEmEnergy/rec_TotalEnergy);
    }

    if ((j1Barrel && j2Endcap) ||
	(j2Barrel && j1Endcap)) {
      myAH->fill1d<TH1F>("hBErecoDeltaEtajj",detajj);
      myAH->fill1d<TH1F>("hBErecoMjj",recomjj);
    } else if ((j1Barrel && j2Forward) ||
	       (j2Barrel && j1Forward)) {
      myAH->fill1d<TH1F>("hBFrecoDeltaEtajj",detajj);
      myAH->fill1d<TH1F>("hBFrecoMjj",recomjj);
    } else if  (j1Endcap && j2Endcap) {
      myAH->fill1d<TH1F>("hEErecoDeltaEtajj",detajj);
      myAH->fill1d<TH1F>("hEErecoMjj",recomjj);
    } else if ((j1Endcap && j2Forward) ||
	       (j2Endcap && j1Forward)) {
      myAH->fill1d<TH1F>("hEFrecoDeltaEtajj",detajj);
      myAH->fill1d<TH1F>("hEFrecoMjj",recomjj);
    } else if  (j1Forward && j2Forward) {
      myAH->fill1d<TH1F>("hFFrecoDeltaEtajj",detajj);
      myAH->fill1d<TH1F>("hFFrecoMjj",recomjj);
    }
  }

  // no jets of substance in between them?
  for (unsigned i = 2; i < pfJets.size() && pfJets[i].pt() > recPt_cut_; i++)
    if ((pfJets[1].eta() > pfJets[i].eta() && pfJets[i].eta() > pfJets[0].eta()) ||
	(pfJets[1].eta() < pfJets[i].eta() && pfJets[i].eta() < pfJets[0].eta())   ) {
      myAH->fill2d<TH2F>("hrecoJetsPtBetweenVBFtagjets", i, pfJets[i].pt());
      myAH->fill2d<TH2F>("hrecoJetsEtaBetweenVBFtagjets",i, pfJets[i].eta());
    }

  //for(unsigned i=0; i<std::min<size_t>(2,pfJets.size()); i++) {   

  // FILL HISTOS FOR ALL JETS in our acceptance

  for(unsigned i=0; i<pfJets.size(); i++) {   

    
    const reco::PFJet& pfj = pfJets[i];
    double rec_pt  = pfj.pt();
    double rec_eta = pfj.eta();
    double rec_phi = pfj.phi();

    // skip PFjets with pt < recPt_cut GeV
    if (rec_pt<recPt_cut_ and recPt_cut_ != -1.) break; // all the jets after this will be lower in Pt
    // skip PFjets with eta > maxEta_cut
    if (fabs(rec_eta)>maxEta_cut_ and maxEta_cut_ != -1.) continue;

    double rec_ChargedHadEnergy    = pfj.chargedHadronEnergy();
    double rec_NeutralHadEnergy    = pfj.neutralHadronEnergy();
    double rec_NeutralEmEnergy     = pfj.neutralEmEnergy();
    double rec_ChargedEmEnergy     = pfj.chargedEmEnergy();;
    double rec_ChargedMultiplicity = pfj.chargedMultiplicity();
    double rec_TotalHadEnergy      = rec_ChargedHadEnergy + rec_NeutralHadEnergy;
    double rec_TotalEmEnergy       = rec_ChargedEmEnergy  + rec_NeutralEmEnergy;
    double rec_NeutralEnergy       = rec_NeutralHadEnergy + rec_NeutralEmEnergy;
    double rec_TotalEnergy         = rec_TotalHadEnergy   + rec_TotalEmEnergy;

    // separate Barrel PFJets from Endcap PFJets
    bool Barrel = false;
    bool Endcap = false;
    bool Forward = false;
    if (std::abs (rec_eta) < 1.4 ) Barrel = true;
    if (std::abs (rec_eta) > 1.6 && std::abs (rec_eta) < 2.7 ) Endcap = true;
    if (std::abs (rec_eta) > 3.0 && std::abs (rec_eta) < 4.7 ) Forward = true;

    myAH->fill1d<TH1F>("hrecojetsPt",rec_pt);
    if (Barrel)
      myAH->fill2d<TH2F>("hBEMvsHad",rec_TotalHadEnergy,rec_TotalEmEnergy);
    else if (Endcap) {
      myAH->fill1d<TH1F>("hrecojetsEndcapPt",rec_pt);
      myAH->fill2d<TH2F>("hEEMvsHad",rec_TotalHadEnergy,rec_TotalEmEnergy);
    } else if (Forward)
      myAH->fill2d<TH2F>("hFEMvsHad",rec_TotalHadEnergy,rec_TotalEmEnergy);

    myAH->fill1d<TH1F>("hrecojetsEta",rec_eta);
    myAH->fill1d<TH1F>("hrecojetsPhi",rec_phi);
    myAH->fill1d<TH1F>("hrecojetsEtavsPt",rec_pt,rec_eta);

    myAH->fill1d<TH1F>("hrecoJetNtracks",pfj.getTrackRefs().size());

    if (Barrel) {
      myAH->fill1d<TH1F>("hBCHE",rec_ChargedHadEnergy);
      myAH->fill1d<TH1F>("hBNHE",rec_NeutralHadEnergy);
      myAH->fill1d<TH1F>("hBNEE",rec_NeutralEmEnergy);
      myAH->fill1d<TH1F>("hBCEE",rec_ChargedEmEnergy);
      myAH->fill1d<TH1F>("hBTEE",rec_TotalEmEnergy);
      myAH->fill1d<TH1F>("hBTHE",rec_TotalHadEnergy);
      myAH->fill1d<TH1F>("hBTNE",rec_NeutralEnergy);
      myAH ->fill1d<TH1F>("hBTE",rec_TotalEnergy);
      if (rec_TotalEnergy > smallval) {
	myAH->fill1d<TH1F>("hBNEF",rec_NeutralEnergy/rec_TotalEnergy);
	myAH->fill1d<TH1F>("hBHADF",rec_TotalHadEnergy/rec_TotalEnergy);
      }
      if (rec_TotalHadEnergy > smallval)
	myAH->fill1d<TH1F>("hBEoverH",rec_TotalEmEnergy/rec_TotalHadEnergy);
    } else if (Endcap) {
      myAH->fill1d<TH1F>("hECHE",rec_ChargedHadEnergy);
      myAH->fill1d<TH1F>("hENHE",rec_NeutralHadEnergy);
      myAH->fill1d<TH1F>("hENEE",rec_NeutralEmEnergy);
      myAH->fill1d<TH1F>("hECEE",rec_ChargedEmEnergy);
      myAH->fill1d<TH1F>("hETEE",rec_TotalEmEnergy);
      myAH->fill1d<TH1F>("hETHE",rec_TotalHadEnergy);
      myAH->fill1d<TH1F>("hETNE",rec_NeutralEnergy);
      myAH ->fill1d<TH1F>("hETE",rec_TotalEnergy);
      if (rec_TotalEnergy > smallval) {
	myAH->fill1d<TH1F>("hENEF",rec_NeutralEnergy/rec_TotalEnergy);
	myAH->fill1d<TH1F>("hEHADF",rec_TotalHadEnergy/rec_TotalEnergy);
      }
      if (rec_TotalHadEnergy > smallval)
	myAH->fill1d<TH1F>("hEEoverH",rec_TotalEmEnergy/rec_TotalHadEnergy);
    } else if (Forward) {
      myAH->fill1d<TH1F>("hFCHE",rec_ChargedHadEnergy);
      myAH->fill1d<TH1F>("hFNHE",rec_NeutralHadEnergy);
      myAH->fill1d<TH1F>("hFNEE",rec_NeutralEmEnergy);
      myAH->fill1d<TH1F>("hFCEE",rec_ChargedEmEnergy);
      myAH->fill1d<TH1F>("hFTEE",rec_TotalEmEnergy);
      myAH->fill1d<TH1F>("hFTHE",rec_TotalHadEnergy);
      myAH->fill1d<TH1F>("hFTNE",rec_NeutralEnergy);
      myAH ->fill1d<TH1F>("hFTE",rec_TotalEnergy);
      if (rec_TotalEnergy > smallval) {
	myAH->fill1d<TH1F>("hFNEF",rec_NeutralEnergy/rec_TotalEnergy);
	myAH->fill1d<TH1F>("hFHADF",rec_TotalHadEnergy/rec_TotalEnergy);
      }
      if (rec_TotalHadEnergy > smallval)
	myAH->fill1d<TH1F>("hFEoverH",rec_TotalEmEnergy/rec_TotalHadEnergy);
    }

    if (!NPFJets)        i1=i;
    else if (NPFJets==1) i2=i;

    NPFJets++;

    // fill inclusive PFjet distribution pt > 20 GeV
    myAH->fill1d<TH1F>("hNjets",NPFJets);

    std::vector <PFCandidatePtr> constituents = pfj.getPFConstituents ();

    for (unsigned ic = 0; ic < constituents.size (); ++ic) {

      double cpt = constituents[ic]->pt();

      myAH->fill2d<TH2F>("hrecoPFConstituentsEtavsPt",cpt,constituents[ic]->eta());

      // Look at constituents near |eta|=3
      if (abs(rec_eta)>2.7 && abs(rec_eta)<2.9) {
	if      (abs(rec_pt- 30.)< 3.) myAH->fill1d<TH1F>("hrecoPFConstitPtJet30",cpt);
	else if (abs(rec_pt- 50.)< 5.) myAH->fill1d<TH1F>("hrecoPFConstitPtJet50",cpt);
	else if (abs(rec_pt-100.)<10.) myAH->fill1d<TH1F>("hrecoPFConstitPtJet100",cpt);
      }
    }

    // look for the closets gen Jet : truth
    const GenJet *truth = jetalgo_->matchByDeltaR(&pfj,&genJets);

    double deltaR = 9999; // represents no gen jets!
    if(truth) 
      deltaR = jetalgo_->deltaR(&pfj, truth);

    myAH->fill1d<TH1F>("hgenrecodeltaR",deltaR);

    // check deltaR is small enough
    if( (deltaR < deltaRMax_) ||
       /* (abs(rec_eta)>2.5 && deltaR < 0.2) || */
	(deltaRMax_ == -1.0 ) ) {                    //start case deltaR < deltaRMax

      // generate histograms comparing the reco and truth candidate (truth = closest in delta-R) 
      // get the quantities to place on the denominator and/or divide by
      double pt_denom;
      double true_E = truth->p();
      double true_pt = truth->pt();
      double true_eta = truth->eta();
      double true_phi = truth->phi();

      myAH->fill1d<TH1F>("hmatchedgenjetsPt",true_pt);
      myAH->fill1d<TH1F>("hmatchedgenjetsEta",true_eta);
      myAH->fill1d<TH1F>("hmatchedgenjetsPhi",true_phi);

      if (Endcap)
	myAH->fill1d<TH1F>("hmgenjetsEndcapPt",true_pt);

      if (plotAgainstReco_) {pt_denom = rec_pt;}
      else {pt_denom = true_pt;}
      // get true specific quantities
      double true_ChargedHadEnergy;
      double true_NeutralHadEnergy;
      double true_ChargedEmEnergy;
      double true_NeutralEmEnergy;
      double true_TotalEmEnergy = truth->emEnergy();
      double true_TotalHadEnergy = truth->hadEnergy();
      gettrue (truth, true_ChargedHadEnergy, true_NeutralHadEnergy, true_ChargedEmEnergy, true_NeutralEmEnergy);
      double true_NeutralEnergy = true_NeutralHadEnergy + true_NeutralEmEnergy;
      std::vector <unsigned int> chMult(9, static_cast<unsigned int>(0)); 
      for (unsigned ic = 0; ic < constituents.size (); ++ic) {
	if ( constituents[ic]->particleId() > 3 ) continue;

	reco::TrackRef trackRef = constituents[ic]->trackRef();
	if ( trackRef.isNull() ) {
	  //std::cout << "Warning in entry " << entry_ 
	  //	    << " : a track with Id " << constituents[ic]->particleId() 
	  //	    << " has no track ref.." << std::endl;
	  continue;
	}
	unsigned int iter = 0; 
	switch (trackRef->algo()) {
	case TrackBase::ctf:
	case TrackBase::iter0:
	  iter = 0;
	  break;
	case TrackBase::iter1:
	  iter = 1;
	  break;
	case TrackBase::iter2:
	  iter = 2;
	  break;
	case TrackBase::iter3:
	  iter = 3;
	  break;
	case TrackBase::iter4:
	  iter = 4;
	  break;
	case TrackBase::iter5:
	  iter = 5;
	  break;
	case TrackBase::iter6:
	  iter = 6;
	  break;
	case TrackBase::iter8:
	  iter = 7;
	  //std::cout << "Warning in entry " << entry_ << " : iter = " << trackRef->algo() << std::endl;
	  //std::cout << ic << " " << *(constituents[ic]) << std::endl;
	  break;
	default:
	  iter = 8;
	  std::cout << "Warning in entry " << entry_ << " : iter = " << trackRef->algo() << std::endl;
	  std::cout << ic << " " << *(constituents[ic]) << std::endl;
	  break;
	}
	++(chMult[iter]);
      }

      bool plot1 = false;
      bool plot2 = false;
      bool plot3 = false;
      bool plot4 = false;
      bool plot5 = false;
      bool plot6 = false;
      bool plot7 = false;
      bool plot8 = false;
      bool plot9 = false;
      bool plot10 = false;
      double resPt =0.;
      double resChargedHadEnergy= 0.;
      double resNeutralHadEnergy= 0.;
      double resTotalHadEnergy= 0.;
      double resChargedEmEnergy= 0.;
      double resNeutralEmEnergy= 0.;
      double resTotalEmEnergy= 0.;
      double resNeutralEnergy= 0.;

      double resHCALEnergy = 0.;
      double resNEUTEnergy = 0.;
      if ( rec_NeutralHadEnergy > smallval && rec_ChargedHadEnergy < smallval ) { 
	double true_NEUTEnergy = true_NeutralHadEnergy + true_NeutralEmEnergy;
	double true_HCALEnergy = true_NEUTEnergy - rec_NeutralEmEnergy;
	double rec_NEUTEnergy = rec_NeutralHadEnergy+rec_NeutralEmEnergy; 
	double rec_HCALEnergy = rec_NeutralHadEnergy; 
	resHCALEnergy = (rec_HCALEnergy-true_HCALEnergy)/rec_HCALEnergy;
	resNEUTEnergy = (rec_NEUTEnergy-true_NEUTEnergy)/rec_NEUTEnergy;
	if ( rec_NeutralEmEnergy > smallval ) {
	  plot7 = true;
	} else {
	  plot8 = true;
	}
      }

      // get relative delta quantities (protect against division by zero!)
      if (true_pt > 0.0001){
	resPt = (rec_pt -true_pt)/true_pt ; 
	plot1 = true;}
      if (true_ChargedHadEnergy > smallval){
	resChargedHadEnergy = (rec_ChargedHadEnergy- true_ChargedHadEnergy)/true_ChargedHadEnergy;
	plot2 = true;}
      if (true_NeutralHadEnergy > smallval){
	resNeutralHadEnergy = (rec_NeutralHadEnergy- true_NeutralHadEnergy)/true_NeutralHadEnergy;
	plot3 = true;}
      else 
	if (rec_NeutralHadEnergy > smallval){
	  resNeutralHadEnergy = (rec_NeutralHadEnergy- true_NeutralHadEnergy)/rec_NeutralHadEnergy;
	  plot3 = true;}
      if (true_NeutralEmEnergy > smallval){
	resNeutralEmEnergy = (rec_NeutralEmEnergy- true_NeutralEmEnergy)/true_NeutralEmEnergy;
	plot4 = true;}
      if (true_ChargedEmEnergy > smallval){
	resChargedEmEnergy = (rec_ChargedEmEnergy- true_ChargedEmEnergy)/true_ChargedEmEnergy;
	plot5 = true;}
      if (true_NeutralEnergy > smallval){
	resNeutralEnergy = (rec_NeutralEnergy- true_NeutralEnergy)/true_NeutralEnergy;
	plot6 = true;}

      if (true_TotalHadEnergy > smallval) {
	resTotalHadEnergy = (rec_TotalHadEnergy- true_TotalHadEnergy)/true_TotalHadEnergy;
	plot9 = true;
      }
      if (true_TotalEmEnergy > smallval) {
	resTotalEmEnergy = (rec_TotalEmEnergy- true_TotalEmEnergy)/true_TotalEmEnergy;
	plot10 = true;
      }

      //double deltaEta = jetalgo_->deltaEta(&pfj, truth);
      //double deltaPhi = jetalgo_->deltaPhi(&pfj, truth);

      // Print outliers for further debugging
      if ( ( resPt > 0.2 && true_pt > 100. ) || 
	   ( resPt < -0.5 && true_pt > 100. ) ) {
	//if ( ( true_pt > 50. && 
	//     ( ( truth->eta()>3.0 && rec_eta-truth->eta() < -0.1 ) || 
	//       ( truth->eta()<-3.0 && rec_eta-truth->eta() > 0.1 ) ))) {
	std::cout << "Entry " << entry_ 
		  << " resPt = " << resPt
		  <<" resCharged  " << resChargedHadEnergy
		  <<" resNeutralHad  " << resNeutralHadEnergy
		  << " resNeutralEm  " << resNeutralEmEnergy
		  << " pT (T/R) " << true_pt << "/" << rec_pt 
		  << " Eta (T/R) " << truth->eta() << "/" << rec_eta 
		  << " Phi (T/R) " << truth->phi() << "/" << rec_phi 
		  << std::endl;

	// check overlapping PF jets
	const reco::PFJet* pfoj = 0; 
	double dRo = 1E9;
	for(unsigned j=0; j<pfJets.size(); j++) { 
	  const reco::PFJet& pfo = pfJets[j];
	  if ( j != i &&  jetalgo_->deltaR(&pfj,&pfo) < dRo && pfo.pt() > 0.25*pfj.pt()) { 
	    dRo = jetalgo_->deltaR(&pfj,&pfo);	
	    pfoj = &pfo;
	  }
	}
	
	// Check overlapping Gen Jet 
	math::XYZTLorentzVector overlappinGenJet(0.,0.,0.,0.);
	const reco::GenJet* genoj = 0;
	double dRgo = 1E9;
	for(unsigned j=0; j<genJets.size(); j++) { 
	  const reco::GenJet* gjo = &(genJets[j]);
	  if ( gjo != truth && jetalgo_->deltaR(truth,gjo) < dRgo && gjo->pt() > 0.25*truth->pt() ) { 
	    dRgo = jetalgo_->deltaR(truth,gjo);
	    genoj = gjo;
	  }
	}
	
	if ( dRo < 0.8 && dRgo < 0.8 && jetalgo_->deltaR(genoj,pfoj) < 2.*deltaRMax_ ) 
	  std::cout << "Excess probably due to overlapping jets (DR = " <<   jetalgo_->deltaR(genoj,pfoj) << "),"
		    << " at DeltaR(T/R) = " << dRgo << "/" << dRo  
		    << " with pT(T/R) " << genoj->pt() << "/" << pfoj->pt()
		    << " and Eta (T/R) " << genoj->eta() << "/" << pfoj->eta()
		    << " and Phi (T/R) " << genoj->phi() << "/" << pfoj->phi()
		    << std::endl;
      }

      if(std::abs(resPt) > std::abs(resPtMax_)) resPtMax_ = resPt;
      if(std::abs(resChargedHadEnergy) > std::abs(resChargedHadEnergyMax_) ) resChargedHadEnergyMax_ = resChargedHadEnergy;
      if(std::abs(resNeutralHadEnergy) > std::abs(resNeutralHadEnergyMax_) ) resNeutralHadEnergyMax_ = resNeutralHadEnergy;
      if(std::abs(resNeutralEmEnergy) > std::abs(resNeutralEmEnergyMax_) ) resNeutralEmEnergyMax_ = resNeutralEmEnergy;
      if (debug_) {
	cout << i <<"  =========PFJet Pt "<< rec_pt
	     << " eta " << rec_eta
	     << " phi " << rec_phi
	     << " Charged Had Energy " << rec_ChargedHadEnergy
	     << " Neutral Had Energy " << rec_NeutralHadEnergy
	     << " Charged Em Energy " << rec_ChargedEmEnergy
	     << " Neutral em Energy " << rec_NeutralEmEnergy << endl;
	cout << " matching Gen Jet Pt " << true_pt
	     << " eta " << truth->eta()
	     << " phi " << truth->phi()
	     << " Charged Had Energy " << true_ChargedHadEnergy
	     << " Neutral Had Energy " << true_NeutralHadEnergy
	     << " Charged Em Energy " << true_ChargedEmEnergy
	     << " Neutral em Energy " << true_NeutralEmEnergy << endl;
	printPFJet(&pfj);
	//      cout<<pfj.print()<<endl;
	printGenJet(truth);
	//cout <<truth->print()<<endl;
				
	cout << "==============deltaR " << deltaR << "  resPt " << resPt
	     << " resChargedHadEnergy " << resChargedHadEnergy
	     << " resNeutralHadEnergy " << resNeutralHadEnergy
	     << " resChargedEmEnergy " << resChargedEmEnergy
	     << " resNeutralEmEnergy " << resNeutralEmEnergy
	     << endl;
      }
			

      if(plot1) {
	if ( rec_eta > 0. ) 
	  myAH->fill2d<TH2F>("hDEtavsEta",true_eta,rec_eta-true_eta);
	else
	  myAH->fill2d<TH2F>("hDEtavsEta",true_eta,-rec_eta+true_eta);
	myAH->fill2d<TH2F>("hDPhivsEta",true_eta,rec_phi-true_phi);

	myAH->fill2d<TH2F>("hRPtvsEta",true_eta, resPt);
	myAH->fill2d<TH2F>("hNCHvsEta",true_eta, rec_ChargedMultiplicity);
	myAH->fill2d<TH2F>("hNCH0vsEta",true_eta,chMult[0]);
	myAH->fill2d<TH2F>("hNCH2vsEta",true_eta,chMult[1]);
	myAH->fill2d<TH2F>("hNCH2vsEta",true_eta,chMult[2]);
	myAH->fill2d<TH2F>("hNCH3vsEta",true_eta,chMult[3]);
	myAH->fill2d<TH2F>("hNCH4vsEta",true_eta,chMult[4]);
	myAH->fill2d<TH2F>("hNCH5vsEta",true_eta,chMult[5]);
	myAH->fill2d<TH2F>("hNCH6vsEta",true_eta,chMult[6]);
	myAH->fill2d<TH2F>("hNCH7vsEta",true_eta,chMult[7]);
      }
      if(plot2)myAH->fill2d<TH2F>("hRCHEvsEta",true_eta, resChargedHadEnergy);
      if(plot6)myAH->fill2d<TH2F>("hRNeutvsEta",true_eta, resNeutralEnergy);
      if(plot7) { 
	myAH->fill2d<TH2F>("hRHCALvsEta",true_eta, resHCALEnergy);
	myAH->fill2d<TH2F>("hRNEUTvsEta",true_eta, resNEUTEnergy);
      }
      if(plot8) {  
	myAH->fill2d<TH2F>("hRHONLvsEta",true_eta, resHCALEnergy);
	myAH->fill2d<TH2F>("hRNONLvsEta",true_eta, resNEUTEnergy);
      }
      if(plot9)myAH->fill2d<TH2F>("hRTHEvsEta",true_eta, resTotalHadEnergy);
      if(plot10)myAH->fill2d<TH2F>("hRTEEvsEta",true_eta, resTotalEmEnergy);

      // fill histograms for relative delta quantitites of matched jets
      // delta Pt or E quantities for Barrel
      if (Barrel){
	if(plot1) { 
	  myAH->fill1d<TH1F> ("hBRPt",resPt);
	  if ( pt_denom >  20. && pt_denom <  40. ) myAH->fill1d<TH1F>("hBRPt20_40",resPt);
	  if ( pt_denom >  40. && pt_denom <  60. ) myAH->fill1d<TH1F>("hBRPt40_60",resPt);
	  if ( pt_denom >  60. && pt_denom <  80. ) myAH->fill1d<TH1F>("hBRPt60_80",resPt);
	  if ( pt_denom >  80. && pt_denom < 100. ) myAH->fill1d<TH1F>("hBRPt80_100",resPt);
	  if ( pt_denom > 100. && pt_denom < 150. ) myAH->fill1d<TH1F>("hBRPt100_150",resPt);
	  if ( pt_denom > 150. && pt_denom < 200. ) myAH->fill1d<TH1F>("hBRPt150_200",resPt);
	  if ( pt_denom > 200. && pt_denom < 250. ) myAH->fill1d<TH1F>("hBRPt200_250",resPt);
	  if ( pt_denom > 250. && pt_denom < 300. ) myAH->fill1d<TH1F>("hBRPt250_300",resPt);
	  if ( pt_denom > 300. && pt_denom < 400. ) myAH->fill1d<TH1F>("hBRPt300_400",resPt);
	  if ( pt_denom > 400. && pt_denom < 500. ) myAH->fill1d<TH1F>("hBRPt400_500",resPt);
	  if ( pt_denom > 500. && pt_denom < 750. ) myAH->fill1d<TH1F>("hBRPt500_750",resPt);
	  if ( pt_denom > 750. && pt_denom < 1250. ) myAH->fill1d<TH1F>("hBRPt750_1250",resPt);
	  if ( pt_denom > 1250. && pt_denom < 2000. ) myAH->fill1d<TH1F>("hBRPt1250_2000",resPt);
	  if ( pt_denom > 2000. && pt_denom < 5000. ) myAH->fill1d<TH1F>("hBRPt2000_5000",resPt);
	  myAH->fill1d<TH1F>("hBNCH",rec_ChargedMultiplicity);
	  myAH->fill2d<TH2F>("hBNCH0vsPt",pt_denom,chMult[0]);
	  myAH->fill2d<TH2F>("hBNCH2vsPt",pt_denom,chMult[1]);
	  myAH->fill2d<TH2F>("hBNCH2vsPt",pt_denom,chMult[2]);
	  myAH->fill2d<TH2F>("hBNCH3vsPt",pt_denom,chMult[3]);
	  myAH->fill2d<TH2F>("hBNCH4vsPt",pt_denom,chMult[4]);
	  myAH->fill2d<TH2F>("hBNCH5vsPt",pt_denom,chMult[5]);
	  myAH->fill2d<TH2F>("hBNCH6vsPt",pt_denom,chMult[6]);
	  myAH->fill2d<TH2F>("hBNCH7vsPt",pt_denom,chMult[7]);
	  myAH->fill2d<TH2F>("hBNCHvsPt",pt_denom,rec_ChargedMultiplicity);
	  if ( rec_eta > 0. ) 
	    myAH->fill2d<TH2F>("hBDEtavsPt",pt_denom,rec_eta-true_eta);
	  else
	    myAH->fill2d<TH2F>("hBDEtavsPt",pt_denom,-rec_eta+true_eta);
	  myAH->fill2d<TH2F>("hBDPhivsPt",pt_denom,rec_phi-true_phi);
	}
	if(plot2)myAH->fill1d<TH1F>("hBRCHE",resChargedHadEnergy);
	if(plot3)myAH->fill1d<TH1F>("hBRNHE",resNeutralHadEnergy);
	if(plot4)myAH->fill1d<TH1F>("hBRNEE",resNeutralEmEnergy);
	if(plot5)myAH->fill1d<TH1F>("hBRCEE",resChargedEmEnergy);
	if(plot6)myAH->fill1d<TH1F>("hBRneut",resNeutralEnergy);
	if(plot9)myAH->fill1d<TH1F>("hBRTHE",resTotalHadEnergy);
	if(plot10)myAH->fill1d<TH1F>("hBRTEE",resTotalEmEnergy);
	if(plot1)myAH->fill2d<TH2F>("hBRPtvsPt",pt_denom, resPt);
	if(plot2)myAH->fill2d<TH2F>("hBRCHEvsPt",pt_denom, resChargedHadEnergy);
	if(plot3)myAH->fill2d<TH2F>("hBRNHEvsPt",pt_denom, resNeutralHadEnergy);
	if(plot4)myAH->fill2d<TH2F>("hBRNEEvsPt",pt_denom, resNeutralEmEnergy);
	if(plot5)myAH->fill2d<TH2F>("hBRCEEvsPt",pt_denom, resChargedEmEnergy);
	if(plot6)myAH->fill2d<TH2F>("hBRneutvsPt",pt_denom, resNeutralEnergy);
	if(plot9)myAH->fill2d<TH2F>("hBRTHEvsPt",pt_denom, resTotalHadEnergy);
	if(plot10)myAH->fill2d<TH2F>("hBRTEEvsPt",pt_denom, resTotalEmEnergy);
	if(plot7) { 
	  myAH->fill2d<TH2F>("hBRHCALvsP",true_E, resHCALEnergy);
	  myAH->fill2d<TH2F>("hBRNEUTvsP",true_E, resNEUTEnergy);
	}
	if(plot8) { 
	  myAH->fill2d<TH2F>("hBRHONLvsP",true_E, resHCALEnergy);
	  myAH->fill2d<TH2F>("hBRNONLvsP",true_E, resNEUTEnergy);
	}

      }
      // delta Pt or E quantities for Endcap
      if (Endcap){
	if(plot1) {
	  myAH->fill1d<TH1F> ("hERPt",resPt);
	  if ( pt_denom >  20. && pt_denom <  40. ) myAH->fill1d<TH1F>("hERPt20_40",resPt);
	  if ( pt_denom >  40. && pt_denom <  60. ) myAH->fill1d<TH1F>("hERPt40_60",resPt);
	  if ( pt_denom >  60. && pt_denom <  80. ) myAH->fill1d<TH1F>("hERPt60_80",resPt);
	  if ( pt_denom >  80. && pt_denom < 100. ) myAH->fill1d<TH1F>("hERPt80_100",resPt);
	  if ( pt_denom > 100. && pt_denom < 150. ) myAH->fill1d<TH1F>("hERPt100_150",resPt);
	  if ( pt_denom > 150. && pt_denom < 200. ) myAH->fill1d<TH1F>("hERPt150_200",resPt);
	  if ( pt_denom > 200. && pt_denom < 250. ) myAH->fill1d<TH1F>("hERPt200_250",resPt);
	  if ( pt_denom > 250. && pt_denom < 300. ) myAH->fill1d<TH1F>("hERPt250_300",resPt);
	  if ( pt_denom > 300. && pt_denom < 400. ) myAH->fill1d<TH1F>("hERPt300_400",resPt);
	  if ( pt_denom > 400. && pt_denom < 500. ) myAH->fill1d<TH1F>("hERPt400_500",resPt);
	  if ( pt_denom > 500. && pt_denom < 750. ) myAH->fill1d<TH1F>("hERPt500_750",resPt);
	  if ( pt_denom > 750. && pt_denom < 1250. ) myAH->fill1d<TH1F>("hERPt750_1250",resPt);
	  if ( pt_denom > 1250. && pt_denom < 2000. ) myAH->fill1d<TH1F>("hERPt1250_2000",resPt);
	  if ( pt_denom > 2000. && pt_denom < 5000. ) myAH->fill1d<TH1F>("hERPt2000_5000",resPt);
	  myAH->fill1d<TH1F>("hENCH",rec_ChargedMultiplicity);
	  myAH->fill2d<TH2F>("hENCHvsPt",pt_denom,rec_ChargedMultiplicity);
	  myAH->fill2d<TH2F>("hENCH0vsPt",pt_denom,chMult[0]);
	  myAH->fill2d<TH2F>("hENCH1vsPt",pt_denom,chMult[1]);
	  myAH->fill2d<TH2F>("hENCH2vsPt",pt_denom,chMult[2]);
	  myAH->fill2d<TH2F>("hENCH3vsPt",pt_denom,chMult[3]);
	  myAH->fill2d<TH2F>("hENCH4vsPt",pt_denom,chMult[4]);
	  myAH->fill2d<TH2F>("hENCH5vsPt",pt_denom,chMult[5]);
	  myAH->fill2d<TH2F>("hENCH6vsPt",pt_denom,chMult[6]);
	  myAH->fill2d<TH2F>("hENCH7vsPt",pt_denom,chMult[7]);
	  if ( rec_eta > 0. ) 
	    myAH->fill2d<TH2F>("hEDEtavsPt",pt_denom,rec_eta-true_eta);
	  else
	    myAH->fill2d<TH2F>("hEDEtavsPt",pt_denom,-rec_eta+true_eta);
	  myAH->fill2d<TH2F>("hEDPhivsPt",pt_denom,rec_phi-true_phi);
	}
	if(plot2)myAH->fill1d<TH1F>("hERCHE",resChargedHadEnergy);
	if(plot3)myAH->fill1d<TH1F>("hERNHE",resNeutralHadEnergy);
	if(plot4)myAH->fill1d<TH1F>("hERNEE",resNeutralEmEnergy);
	if(plot5)myAH->fill1d<TH1F>("hERCEE",resChargedEmEnergy);
	if(plot6)myAH->fill1d<TH1F>("hERneut",resNeutralEnergy);
	if(plot9)myAH->fill1d<TH1F>("hERTHE",resTotalHadEnergy);
	if(plot10)myAH->fill1d<TH1F>("hERTEE",resTotalEmEnergy);
	if(plot1)myAH->fill2d<TH2F>("hERPtvsPt",pt_denom, resPt);
	if(plot2)myAH->fill2d<TH2F>("hERCHEvsPt",pt_denom, resChargedHadEnergy);
	if(plot3)myAH->fill2d<TH2F>("hERNHEvsPt",pt_denom, resNeutralHadEnergy);
	if(plot4)myAH->fill2d<TH2F>("hERNEEvsPt",pt_denom, resNeutralEmEnergy);
	if(plot5)myAH->fill2d<TH2F>("hERCEEvsPt",pt_denom, resChargedEmEnergy);
	if(plot9)myAH->fill2d<TH2F>("hERTHEvsPt",pt_denom, resTotalHadEnergy);
	if(plot10)myAH->fill2d<TH2F>("hERTEEvsPt",pt_denom, resTotalEmEnergy);
	if(plot6)myAH->fill2d<TH2F>("hERneutvsPt",pt_denom, resNeutralEnergy);
	if(plot7) {
	  myAH->fill2d<TH2F>("hERHCALvsP",true_E, resHCALEnergy);
	  myAH->fill2d<TH2F>("hERNEUTvsP",true_E, resNEUTEnergy);
	}
	if(plot8) {
	  myAH->fill2d<TH2F>("hERHONLvsP",true_E, resHCALEnergy);
	  myAH->fill2d<TH2F>("hERNONLvsP",true_E, resNEUTEnergy);
	}
      }						
      // delta Pt or E quantities for Forward
      if (Forward){
	if(plot1) {
	  myAH->fill1d<TH1F> ("hFRPt",resPt);
	  if ( pt_denom >  20. && pt_denom <  40. ) myAH->fill1d<TH1F>("hFRPt20_40",resPt);
	  if ( pt_denom >  40. && pt_denom <  60. ) myAH->fill1d<TH1F>("hFRPt40_60",resPt);
	  if ( pt_denom >  60. && pt_denom <  80. ) myAH->fill1d<TH1F>("hFRPt60_80",resPt);
	  if ( pt_denom >  80. && pt_denom < 100. ) myAH->fill1d<TH1F>("hFRPt80_100",resPt);
	  if ( pt_denom > 100. && pt_denom < 150. ) myAH->fill1d<TH1F>("hFRPt100_150",resPt);
	  if ( pt_denom > 150. && pt_denom < 200. ) myAH->fill1d<TH1F>("hFRPt150_200",resPt);
	  if ( pt_denom > 200. && pt_denom < 250. ) myAH->fill1d<TH1F>("hFRPt200_250",resPt);
	  if ( pt_denom > 250. && pt_denom < 300. ) myAH->fill1d<TH1F>("hFRPt250_300",resPt);
	  if ( pt_denom > 300. && pt_denom < 400. ) myAH->fill1d<TH1F>("hFRPt300_400",resPt);
	  if ( pt_denom > 400. && pt_denom < 500. ) myAH->fill1d<TH1F>("hFRPt400_500",resPt);
	  if ( pt_denom > 500. && pt_denom < 750. ) myAH->fill1d<TH1F>("hFRPt500_750",resPt);
	  if ( pt_denom > 750. && pt_denom < 1250. ) myAH->fill1d<TH1F>("hFRPt750_1250",resPt);
	  if ( pt_denom > 1250. && pt_denom < 2000. ) myAH->fill1d<TH1F>("hFRPt1250_2000",resPt);
	  if ( pt_denom > 2000. && pt_denom < 5000. ) myAH->fill1d<TH1F>("hFRPt2000_5000",resPt);
	  if ( rec_eta > 0. ) 
	    myAH->fill2d<TH2F>("hFDEtavsPt",pt_denom,rec_eta-true_eta);
	  else
	    myAH->fill2d<TH2F>("hFDEtavsPt",pt_denom,-rec_eta+true_eta);
	  myAH->fill2d<TH2F>("hFDPhivsPt",pt_denom,rec_phi-true_phi);
	}
	if(plot2)myAH->fill1d<TH1F>("hFRCHE",resChargedHadEnergy);
	if(plot3)myAH->fill1d<TH1F>("hFRNHE",resNeutralHadEnergy);
	if(plot4)myAH->fill1d<TH1F>("hFRNEE",resNeutralEmEnergy);
	if(plot5)myAH->fill1d<TH1F>("hFRCEE",resChargedEmEnergy);
	if(plot6)myAH->fill1d<TH1F>("hFRneut",resNeutralEnergy);
	if(plot9)myAH->fill1d<TH1F>("hFRTHE",resTotalHadEnergy);
	if(plot10)myAH->fill1d<TH1F>("hFRTEE",resTotalEmEnergy);

	if(plot1)myAH->fill2d<TH2F>("hFRPtvsPt",pt_denom, resPt);
	if(plot2)myAH->fill2d<TH2F>("hFRCHEvsPt",pt_denom, resChargedHadEnergy);
	if(plot3)myAH->fill2d<TH2F>("hFRNHEvsPt",pt_denom, resNeutralHadEnergy);
	if(plot4)myAH->fill2d<TH2F>("hFRNEEvsPt",pt_denom, resNeutralEmEnergy);
	if(plot5)myAH->fill2d<TH2F>("hFRCEEvsPt",pt_denom, resChargedEmEnergy);
	if(plot6)myAH->fill2d<TH2F>("hFRneutvsPt",pt_denom, resNeutralEnergy);
	if(plot9)myAH->fill2d<TH2F>("hFRTHEvsPt",pt_denom, resTotalHadEnergy);
	if(plot10)myAH->fill2d<TH2F>("hFRTEEvsPt",pt_denom, resTotalEmEnergy);
	if(plot7) {
	  myAH->fill2d<TH2F>("hFRHCALvsP",true_E, resHCALEnergy);
	  myAH->fill2d<TH2F>("hFRNEUTvsP",true_E, resNEUTEnergy);
	}
	if(plot8) {
	  myAH->fill2d<TH2F>("hFRHONLvsP",true_E, resHCALEnergy);
	  myAH->fill2d<TH2F>("hFRNONLvsP",true_E, resNEUTEnergy);
	}
      }						
    } // end case deltaR < deltaRMax
		
  } // i loop on pf Jets	

  // Now look at gen jets on their own merit

  for(unsigned i=0; i<std::min(size_t(2),genJets.size()); i++) {   
		
    const reco::GenJet& genj = genJets[i];
    double gen_pt = genj.pt();
    double gen_eta = genj.eta();
    double gen_phi = genj.phi();
		
    myAH->fill1d<TH1F>("hgenjetsPt",gen_pt);
    if ((abs(gen_eta)<=2.7 &&
	 abs(gen_eta)>=1.6) )
      myAH->fill1d<TH1F>("hgenjetsEndcapPt",gen_pt);

    myAH->fill1d<TH1F>("hgenjetsEta",gen_eta);
    myAH->fill1d<TH1F>("hgenjetsPhi",gen_phi);
  }

  if (NPFJets >= 2) {
    double dr11 = sqrt(
		       (pfJets[i1].eta()-genJets[0].eta())*(pfJets[i1].eta()-genJets[0].eta()) +
		       (pfJets[i1].phi()-genJets[0].phi())*(pfJets[i1].phi()-genJets[0].phi())
		       );
    
    double dr12 = sqrt(
		       (pfJets[i1].eta()-genJets[1].eta())*(pfJets[0].eta()-genJets[1].eta()) +
		       (pfJets[i1].phi()-genJets[1].phi())*(pfJets[0].phi()-genJets[1].phi())
		       );
    
    double dr21 = sqrt(
		       (pfJets[i2].eta()-genJets[i1].eta())*(pfJets[i2].eta()-genJets[0].eta()) +
		       (pfJets[i2].phi()-genJets[i1].phi())*(pfJets[i2].phi()-genJets[0].phi())
		       );
    
    double dr22 = sqrt(
		       (pfJets[i2].eta()-genJets[1].eta())*(pfJets[i2].eta()-genJets[1].eta()) +
		       (pfJets[i2].phi()-genJets[1].phi())*(pfJets[i2].phi()-genJets[1].phi())
		       );
    
    if (dr11+dr22 < dr12+dr21) {
      double sumdrs = dr11+dr22;
      double sumdpt = abs(pfJets[i1].pt()-genJets[0].pt())+abs(pfJets[i2].pt()-genJets[1].pt());
      myAH->fill2d<TH2F>("hsumdRvsSumDpt",sumdpt,sumdrs);
    } else {
      double sumdrs = dr12+dr21;
      double sumdpt = abs(pfJets[i2].pt()-genJets[0].pt())+abs(pfJets[i1].pt()-genJets[1].pt());
      myAH->fill2d<TH2F>("hsumdRvsSumDpt",sumdpt,sumdrs);
    }

  }
}                             // phase2upVBFjetAnalAlgos::fillHistos4cut

//======================================================================

void
phase2upVBFjetAnalAlgos::process(const reco::GenParticleCollection& genParts,
				 const reco::PFJetCollection& pfJets,
				 const reco::PFJetCollection& pfJetCores,
				 const reco::GenJetCollection& genJets)
{
  calcQGdiscriminators(pfJets, pfJetCores);

  isVBF_     = calcVBFCut(pfJets);
  matchQs_   = isSignal_ && calcMatchQuarkCut(genParts,pfJets);
  nogapjets_ = calcVBFgapjetveto(pfJets);
  nogapjetsdynamic_ = calcVBFgapjetveto(pfJets,true);

  getCut("none")->Activate (false);                       fillHistos4cut(pfJets, genJets, *getCut("none"));
  getCut("vbf")->Activate(!isVBF_);                       fillHistos4cut(pfJets, genJets, *getCut("vbf"));
  getCut("vbf+veto")->Activate(!isVBF_ || !nogapjets_);   fillHistos4cut(pfJets, genJets, *getCut("vbf+veto"));
  getCut("vbf+dynveto")->Activate(!isVBF_ ||
				  !nogapjetsdynamic_);    fillHistos4cut(pfJets, genJets, *getCut("vbf+dynveto"));
  if (isSignal_) {
    getCut("matchQs")->Activate (!matchQs_);              fillHistos4cut(pfJets, genJets, *getCut("matchQs"));
    getCut("vbfANDmatchQs")->Activate(!isVBF_ ||
				      !matchQs_);         fillHistos4cut(pfJets, genJets, *getCut("vbfANDmatchQs"));
    getCut("vbf+vetoANDmatchQs")->Activate(!isVBF_ ||
					   !matchQs_ ||
					   !nogapjets_);  fillHistos4cut(pfJets, genJets, *getCut("vbf+vetoANDmatchQs"));
    getCut("vbf+dynvetoANDmatchQs")->Activate(!isVBF_ ||
					      !matchQs_ ||
					      !nogapjetsdynamic_);  fillHistos4cut(pfJets, genJets,
										   *getCut("vbf+dynvetoANDmatchQs"));
  }

  // Increment counter
  entry_++;
}

//======================================================================

void phase2upVBFjetAnalAlgos::gettrue (const reco::GenJet* truth,
			      double& true_ChargedHadEnergy, double& true_NeutralHadEnergy,
			      double& true_ChargedEmEnergy,  double& true_NeutralEmEnergy)
{
  std::vector <const GenParticle*> mcparts = truth->getGenConstituents ();
  true_ChargedEmEnergy = 0.;
  true_NeutralEmEnergy = 0.;
  true_ChargedHadEnergy = 0.;
  true_NeutralHadEnergy = 0.;
  // for each MC particle in turn  
  for (unsigned i = 0; i < mcparts.size (); i++) {
    const GenParticle* mcpart = mcparts[i];
    int PDG = std::abs( mcpart->pdgId());
    double e = mcpart->energy(); 
    switch(PDG){  // start PDG switch
    case 11: //electrons (until recognised)
      true_ChargedEmEnergy += e;
      break;
    case 22: // photon
      true_NeutralEmEnergy += e;
      break;
    case 211: // pi
    case 321: // K
    case 2212: // p
      true_ChargedHadEnergy += e;
      break;
    case 310: // K_S0
    case 130: // K_L0
    case 3122: // Lambda0
    case 2112: // n0
      true_NeutralHadEnergy += e;
    default:
      break;
    }  // end PDG switch		
  }  // end loop on constituents.
}

//======================================================================

void phase2upVBFjetAnalAlgos::printPFJet(const reco::PFJet* pfj){
  cout<<setiosflags(ios::right);
  cout<<setiosflags(ios::fixed);
  cout<<setprecision(3);

  cout << "PFJet  p/px/py/pz/pt: " << pfj->p() << "/" << pfj->px () 
       << "/" << pfj->py() << "/" << pfj->pz() << "/" << pfj->pt() << endl
       << "    eta/phi: " << pfj->eta () << "/" << pfj->phi () << endl   		
       << "    PFJet specific:" << std::endl
       << "      charged/neutral hadrons energy: " << pfj->chargedHadronEnergy () << '/' << pfj->neutralHadronEnergy () << endl
       << "      charged/neutral em energy: " << pfj->chargedEmEnergy () << '/' << pfj->neutralEmEnergy () << endl
       << "      charged muon energy: " << pfj->chargedMuEnergy () << '/' << endl
       << "      charged/neutral multiplicity: " << pfj->chargedMultiplicity () << '/' << pfj->neutralMultiplicity () << endl;
  
  // And print the constituents
  std::cout << pfj->print() << std::endl;

  cout<<resetiosflags(ios::right|ios::fixed);
}

//======================================================================

void phase2upVBFjetAnalAlgos::printGenJet (const reco::GenJet* truth){
  std::vector <const GenParticle*> mcparts = truth->getGenConstituents ();
  cout << "GenJet p/px/py/pz/pt: " << truth->p() << '/' << truth->px () 
       << '/' << truth->py() << '/' << truth->pz() << '/' << truth->pt() << endl
       << "    eta/phi: " << truth->eta () << '/' << truth->phi () << endl
       << "    # of constituents: " << mcparts.size() << endl;
  cout << "    constituents:" << endl;
  for (unsigned i = 0; i < mcparts.size (); i++) {
    const GenParticle* mcpart = mcparts[i];
    cout << "      #" << i << "  PDG code:" << mcpart->pdgId() 
	 << ", p/pt/eta/phi: " << mcpart->p() << '/' << mcpart->pt() 
	 << '/' << mcpart->eta() << '/' << mcpart->phi() << endl;	
  }    
}

