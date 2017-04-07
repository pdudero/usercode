#include "WWptResummation.h"

//#include "TCanvas.h"
#include "TProfile.h"

// preprocessor macro for booking 1d histos with DQMStore -or- bare Root
#define BOOK1D(name,title,nbinsx,lowx,highx) \
  h##name = fs->make<TH1D>(#name,title,nbinsx,lowx,highx)

// all versions OK
// preprocesor macro for setting axis titles
#define SETAXES(name,xtitle,ytitle) \
  h##name->GetXaxis()->SetTitle(xtitle); h##name->GetYaxis()->SetTitle(ytitle)


using namespace reco;
using namespace std;


WWptResummation::WWptResummation() {}

WWptResummation::~WWptResummation() {}

//======================================================================
// takes table of "xval bincontent"
//
void WWptResummation::loadResumHisto(const TString& filename, 
				     TH1* th1,
				     double norm)
{
  char linein[256];

  FILE *fp = fopen(filename, "r");

  if (!fp) {
    cerr << "File failed to open, " << filename << endl;
    exit(-1);
  }

  int nrec=0;
  while (!feof(fp) && fgets(linein,256,fp)) {
    double x,binc;
    if (linein[0]=='#') continue; // comments are welcome
    int nscan= sscanf(linein, "%lf %lf", &x, &binc);

    if (nscan != 2) {
      cerr << "scan failed:";
      cerr << "  nscan  = " << nscan    << endl;
      cerr << "  file   = " << filename << endl;
      cerr << "  line   = " << linein;
      break;
    }
    nrec++;
    int ibin = 1 + (x-startval_)/binwidth_;

    th1->SetBinContent(ibin,binc);
    th1->SetBinError(ibin,0.);
  }

  cout << nrec << " records read, ";

  th1->Scale(norm/th1->Integral());

  cout << "Scaled " << filename << " to " << th1->Integral() << endl;

}                                                     //  loadResumHisto

//======================================================================

void WWptResummation::analyze( double xsec ) {

  hWWpt->Scale(xsec/hWWpt->Integral());

  hcentralRatio->Divide(hcentral,hWWpt);
  hQupRatio->Divide(hQup,hWWpt);
  hQdnRatio->Divide(hQdn,hWWpt);
  hRupRatio->Divide(hRup,hWWpt);
  hRdnRatio->Divide(hRdn,hWWpt);
} 

//======================================================================

void WWptResummation::write() {
   // Store the DAQ Histograms 
  hWWpt->Print();

  hWWpt->Write();
  hWppt->Write();
  hWmpt->Write();
  hWppthi->Write();
  hWmpthi->Write();

  hcentral->Write();
  hQup->Write();
  hQdn->Write();
  hRup->Write();
  hRdn->Write();
             
  hcentralRatio->Write();
  hQupRatio->Write();
  hQdnRatio->Write();
  hRupRatio->Write();
  hRdnRatio->Write();
    
} 

//======================================================================

void  WWptResummation::setup(const edm::ParameterSet& pars)
{
  finalstatus_ = pars.getParameter<int>("finalParticleStatus");

  // print parameters
  //cout<< "WWptResummation Setup parameters =============================================="<<endl;
  cout << "final particle status to latch pt values =  " << finalstatus_ <<endl;
  
  // Book histogram
  edm::Service<TFileService> fs;

//    TTree * tr = new TTree("PFTast");
//    tr->Branch("Benchmarks/ParticleFlow")


  int nbins = (int)((endval_-startval_)/binwidth_);
  BOOK1D(WWpt,    "WWpt; p_{T}(WW) (GeV)", nbins, startval_, endval_);
  BOOK1D(Wppt,    "Wppt; p_{T}(W+) (GeV)", 100, 0., 1000.);
  BOOK1D(Wmpt,    "Wmpt; p_{T}(W-) (GeV)", 100, 0., 1000.);
  BOOK1D(Wppthi,  "Wppthi; p_{T}(W+) (GeV)", 100, 0., 1000.);
  BOOK1D(Wmpthi,  "Wmpthi; p_{T}(W-) (GeV)", 100, 0., 1000.);

  BOOK1D(central, "central; p_{T}(WW) (GeV)", nbins, startval_, endval_);
  BOOK1D(Qup,     "Qup; p_{T}(WW) (GeV)", nbins, startval_, endval_);
  BOOK1D(Qdn,     "Qdn; p_{T}(WW) (GeV)", nbins, startval_, endval_);
  BOOK1D(Rup,     "Rup; p_{T}(WW) (GeV)", nbins, startval_, endval_);
  BOOK1D(Rdn,     "Rdn; p_{T}(WW) (GeV)", nbins, startval_, endval_);

  BOOK1D(centralRatio, "central; p_{T}(WW) (GeV)", nbins, startval_, endval_);
  BOOK1D(QupRatio,"QupRatio; p_{T}(WW) (GeV)", nbins, startval_, endval_);
  BOOK1D(QdnRatio,"QdnRatio; p_{T}(WW) (GeV)", nbins, startval_, endval_);
  BOOK1D(RupRatio,"RupRatio; p_{T}(WW) (GeV)", nbins, startval_, endval_);
  BOOK1D(RdnRatio,"RdnRatio; p_{T}(WW) (GeV)", nbins, startval_, endval_);

  hWWpt->Sumw2();
  hWppt->Sumw2();
  hWmpt->Sumw2();
  hWppthi->Sumw2();
  hWmpthi->Sumw2();

  string centralFilename  =  pars.getParameter<string>("centralFile");
  string QupFilename      =  pars.getParameter<string>("QupFile");
  string QdnFilename      =  pars.getParameter<string>("QdnFile");
  string RupFilename      =  pars.getParameter<string>("RupFile");
  string RdnFilename      =  pars.getParameter<string>("RdnFile");
  double centralXsecPb    =  pars.getParameter<double>("centralXsecPb");
  double QupXsecPb        =  pars.getParameter<double>("QupXsecPb");
  double QdnXsecPb        =  pars.getParameter<double>("QdnXsecPb");
  double RupXsecPb        =  pars.getParameter<double>("RupXsecPb");
  double RdnXsecPb        =  pars.getParameter<double>("RdnXsecPb");

  loadResumHisto(centralFilename, hcentral, centralXsecPb);
  loadResumHisto(QupFilename, hQup, QupXsecPb);
  loadResumHisto(QdnFilename, hQdn, QdnXsecPb);
  loadResumHisto(RupFilename, hRup, RupXsecPb);
  loadResumHisto(RdnFilename, hRdn, RdnXsecPb);

}


void WWptResummation::process(const reco::GenParticleCollection& genParticleList) {
  calculateQuantities(genParticleList);

  hWWpt->Fill(wwpt_);
  hWppt->Fill(wppt_);
  hWmpt->Fill(wmpt_);

  if (wwpt_ > 160.) {
    hWppthi->Fill(wppt_);
    hWmpthi->Fill(wmpt_);
  }
}

void WWptResummation::calculateQuantities(const reco::GenParticleCollection& genParticleList) 
{

  //  for( genParticle = genParticleList.begin(); genParticle != genParticleList.end(); genParticle++ )
  math::XYZTLorentzVector WWp4(0,0,0,0);
  for( unsigned i = 0; i < genParticleList.size(); i++ ) {
    const GenParticle& p = genParticleList[i];

    if ( p.status() == 3 ) {
      TString outstr = TString::Format("%i\t%i\t%i\tpx=%g\tpy=%g\tpt=%g",
				       i, p.pdgId(), p.status(),
				       p.px(), p.py(), p.pt());
      cout << outstr << endl;
    }

    if ( abs(p.pdgId()) == 24 ) { // W

      if( p.status() == finalstatus_ ) { // "decayed"
	TString outstr = TString::Format("%i\t%i\t%i\tpt=%g",i, p.pdgId(), p.status(), p.pt());
	cout << outstr << endl;

	WWp4 += p.p4();

	if ( p.charge() > 0 ) wppt_ = p.pt();
	else                  wmpt_ = p.pt();
      }
    }
  }
  wwpt_ = WWp4.pt();
  cout << "WWpt = " << wwpt_ << endl;
}

double   
WWptResummation::mpi_pi(double angle) {

  const double pi = 3.14159265358979323;
  const double pi2 = pi*2.;
  while(angle>pi) angle -= pi2;
  while(angle<-pi) angle += pi2;
  return angle;

}
