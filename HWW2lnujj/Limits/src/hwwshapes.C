#include <iostream>
#include <stdio.h>
#include <map>
#include "TH1D.h"
#include "TGraph.h"
#include "TFile.h"

using namespace std;

struct HdataPerMassPt {
  HdataPerMassPt() :
    massgev(0),cspb(0),unchi(0),unclo(0),br2lnujj(0) {}
  double massgev;
  double cspb;
  double unchi;
  double unclo;
  double br2lnujj;
};

std::map<int,HdataPerMassPt> m_signals; // signal (Higgs) data per mass point

struct FourGraphs_t {
  TGraph *data;
  TGraph *backnm;
  TGraph *backup;
  TGraph *backdn;
};

FourGraphs_t gr2j150to230quad;
FourGraphs_t gr2j200to400quad;
FourGraphs_t gr2j360to500quad;
FourGraphs_t gr2j450to800quad;
FourGraphs_t gr3j150to230quad;
FourGraphs_t gr3j200to400quad;
FourGraphs_t gr3j360to800quad;

const double intlumipbinv = 2100.;

const int NUMMASSPTS=13;
const int masspts[NUMMASSPTS] = {
  160,170,180,190,200,250,300,350,400,450,500,550,600
};
//================================================================================

void readHxsTable(const std::string& fname)
{
  cout << "Reading cross sections table from " << fname << endl;

  FILE *mapf = fopen(fname.c_str(),"r");
  if (!mapf) {
    cerr << "Couldn't open " << fname << endl;
    exit(-1);
  }

  char str[128];
  while (!feof(mapf) && fgets(str,128,mapf)) {
    if (str[0] == '#') continue;
    int mHgev;
    int n = sscanf(str,"%d",&mHgev);
    HdataPerMassPt& hd = m_signals[mHgev];
    n += sscanf(str,"%*d %lf %lf %lf",&hd.cspb,&hd.unchi,&hd.unclo);
    if (n != 4) {
      cerr << "Error reading from file " << fname << endl;
      exit(-1);
    }
    hd.massgev = (double)mHgev;
  }
  fclose(mapf);
}                                                                  // readHxsTable

//================================================================================

void readBRtable(const std::string& fname)
{
  cout << "Reading branching ratios table from " << fname << endl;

  FILE *brf = fopen(fname.c_str(),"r");
  if (!brf) {
    cerr << "Couldn't open " << fname << endl;
    exit(-1);
  }

  char str[128];
  while (!feof(brf) && fgets(str,128,brf)) {
    if (str[0] == '#') continue;
    int mHgev;
    double br;
    int n = sscanf(str,"%d %lf",&mHgev,&br);
    if (n != 2) {
      cerr << "Error reading from file " << fname << endl;
      exit(-1);
    }
    m_signals[mHgev].br2lnujj = 2*br; // listed as for only one charge lepton
  }
  fclose(brf);
}                                                                  // readBRtable

//================================================================================

void writesig(TFile *allHistFile,
	      TH1   *hin,
	      int    imass,
	      int    numjets,
	      double xmin,
	      double xmax,
	      int    binwidth)
{
  int rebin = binwidth/5; // assumes 5GeV binning on higgs histos
  int nbins = (int)(xmax-xmin)/binwidth;

  assert(nbins);

  TString name = Form("Signal_bn%djet%dto%d_Mass_%d",numjets,(int)xmin,(int)xmax,imass);
  TH1D    *hnm = new TH1D(name,name,nbins,xmin,xmax);
  TH1D    *hup = new TH1D(name+"_SigxsecUp",name+"_SigxsecUp",nbins,xmin,xmax);
  TH1D    *hdn = new TH1D(name+"_SigxsecDown",name+"_SigxsecDown",nbins,xmin,xmax);

  cout
    <<"M="<<imass<<",xmin="<<xmin<<",xmax="<<xmax<<",binw="<<binwidth
    <<",rebin="<<rebin<<",nbins="<<nbins<<endl;

  HdataPerMassPt& hd = m_signals[imass];
  
  // make copies of the histograms that are restricted to the bin range lobin-hibin
  int lobin = hin->FindFixBin(xmin);
  int hibin = hin->FindFixBin(xmax); // one higher than we want
  for (int ibin=lobin; ibin<hibin; ) {
    double sum=0;
    int newbin = 1+((ibin-lobin)/rebin);
    for (int j=0; j<rebin; j++)
      sum += hin->GetBinContent(ibin++);
    hnm->SetBinContent(newbin,sum);
    hup->SetBinContent(newbin,(1.0+(hd.unchi/100.0))*sum);
    hdn->SetBinContent(newbin,(1.0+(hd.unclo/100.0))*sum);
  }

  //double ovflwcor = hin->Integral(1,hin->GetNbinsX())/hin->Integral(0,hin->GetNbinsX()+1);
  double norm     = 
    hd.cspb*
    //ovflwcor*
    intlumipbinv*
    hd.br2lnujj;
    
  hnm->Scale(norm);
  hup->Scale(norm);
  hdn->Scale(norm);
  
  allHistFile->WriteTObject(hnm);
  allHistFile->WriteTObject(hup);
  allHistFile->WriteTObject(hdn);
}                                                                      // writesig

//================================================================================

void writeGraph2TH1(TGraph *ingr, TH1 *outh, TFile *fout)
{
  for (int ibin=1; ibin <= outh->GetNbinsX(); ibin++)
    outh->SetBinContent(ibin,ingr->Eval(outh->GetBinCenter(ibin)));

  fout->WriteTObject(outh);
}

//================================================================================

void writedataback(TFile *fout,
		   FourGraphs_t& quadgr,
		   int massgev,
		   int numjets,
		   double xmin,
		   double xmax,
		   int binwidth)
{
  int nbins = (int)(xmax-xmin)/binwidth;
  assert(nbins);

  TString name = Form("data_obs_bn%djet%dto%d_Mass_%d",numjets,(int)xmin,(int)xmax,massgev);
  TH1D *data   = new TH1D(name,name, nbins,xmin,xmax);

  name = Form("Bkgrdtot_bn%djet%dto%d_Mass_%d",numjets,(int)xmin,(int)xmax,massgev);
  TH1D *backnm = new TH1D(name,name, nbins,xmin,xmax);
  TH1D *backup = new TH1D(name+"_WjetshapeUp",name+"_WjetshapeUp",nbins,xmin,xmax);
  TH1D *backdn = new TH1D(name+"_WjetshapeDown",name+"_WjetshapeDown",nbins,xmin,xmax);
  
  writeGraph2TH1(quadgr.data,   data,   fout);
  writeGraph2TH1(quadgr.backnm, backnm, fout);
  writeGraph2TH1(quadgr.backup, backup, fout);
  writeGraph2TH1(quadgr.backdn, backdn, fout);
}                                                                 // writedataback

//================================================================================

void writeHistosPerMass(int massgev,
			TFile *fp)
{
  TH1 *higgs2jHist = (TH1 *)fp->Get(Form("2jet_HiggsTemplate_Mass_%d",massgev));
  TH1 *higgs3jHist = (TH1 *)fp->Get(Form("3jet_HiggsTemplate_Mass_%d",massgev));

  TFile *fout = new TFile(Form("kal-histo-shapes-M=%d.root",massgev),"RECREATE");

  writesig      (fout,higgs2jHist,massgev,2,150,200,10);
  writesig      (fout,higgs2jHist,massgev,2,200,360,20);
  writesig      (fout,higgs2jHist,massgev,2,360,500,20);
  writesig      (fout,higgs2jHist,massgev,2,500,800,25);

  writedataback (fout,gr2j150to230quad,massgev,2,150,200,10);
  writedataback (fout,gr2j200to400quad,massgev,2,200,360,20);
  writedataback (fout,gr2j360to500quad,massgev,2,360,500,20);
  writedataback (fout,gr2j450to800quad,massgev,2,500,800,25);

  writesig      (fout,higgs3jHist,massgev,3,150,200,10);
  writesig      (fout,higgs3jHist,massgev,3,200,360,20);
  writesig      (fout,higgs3jHist,massgev,3,360,800,40);

  writedataback (fout,gr3j150to230quad,massgev,3,150,200,10);
  writedataback (fout,gr3j200to400quad,massgev,3,200,360,20);
  writedataback (fout,gr3j360to800quad,massgev,3,360,800,40);

  fout->Close();

}                                                            // writeHistosPerMass

//================================================================================

void hwwshapes()
{
  readHxsTable   ("ggHtable.txt");
  readBRtable    ("twikiBRtable.txt");

  // Get all inputs

  TFile *fp = new TFile("Histograms_data_and_template.root");

  gr2j150to230quad.data   = (TGraph *)fp->Get("2jet_MassRange_150-230_hist_data");
  gr2j150to230quad.backnm = (TGraph *)fp->Get("2jet_MassRange_150-230_curve_fitTotal");
  gr2j150to230quad.backup = (TGraph *)fp->Get("2jet_MassRange_150-230_curve_fitTotal_SystUp");
  gr2j150to230quad.backdn = (TGraph *)fp->Get("2jet_MassRange_150-230_curve_fitTotal_SystDown");

  gr2j200to400quad.data   = (TGraph *)fp->Get("2jet_MassRange_200-400_hist_data");
  gr2j200to400quad.backnm = (TGraph *)fp->Get("2jet_MassRange_200-400_curve_fitTotal");
  gr2j200to400quad.backup = (TGraph *)fp->Get("2jet_MassRange_200-400_curve_fitTotal_SystUp");
  gr2j200to400quad.backdn = (TGraph *)fp->Get("2jet_MassRange_200-400_curve_fitTotal_SystDown");

  gr2j360to500quad.data   = (TGraph *)fp->Get("2jet_MassRange_360-500_hist_data");
  gr2j360to500quad.backnm = (TGraph *)fp->Get("2jet_MassRange_360-500_curve_fitTotal");
  gr2j360to500quad.backup = (TGraph *)fp->Get("2jet_MassRange_360-500_curve_fitTotal_SystUp");
  gr2j360to500quad.backdn = (TGraph *)fp->Get("2jet_MassRange_360-500_curve_fitTotal_SystDown");

  gr2j450to800quad.data   = (TGraph *)fp->Get("2jet_MassRange_450-800_hist_data");
  gr2j450to800quad.backnm = (TGraph *)fp->Get("2jet_MassRange_450-800_curve_fitTotal");
  gr2j450to800quad.backup = (TGraph *)fp->Get("2jet_MassRange_450-800_curve_fitTotal_SystUp");
  gr2j450to800quad.backdn = (TGraph *)fp->Get("2jet_MassRange_450-800_curve_fitTotal_SystDown");

  gr3j150to230quad.data   = (TGraph *)fp->Get("3jet_MassRange_150-230_hist_data");
  gr3j150to230quad.backnm = (TGraph *)fp->Get("3jet_MassRange_150-230_curve_fitTotal");
  gr3j150to230quad.backup = (TGraph *)fp->Get("3jet_MassRange_150-230_curve_fitTotal_SystUp");
  gr3j150to230quad.backdn = (TGraph *)fp->Get("3jet_MassRange_150-230_curve_fitTotal_SystDown");

  gr3j200to400quad.data   = (TGraph *)fp->Get("3jet_MassRange_200-400_hist_data");
  gr3j200to400quad.backnm = (TGraph *)fp->Get("3jet_MassRange_200-400_curve_fitTotal");
  gr3j200to400quad.backup = (TGraph *)fp->Get("3jet_MassRange_200-400_curve_fitTotal_SystUp");
  gr3j200to400quad.backdn = (TGraph *)fp->Get("3jet_MassRange_200-400_curve_fitTotal_SystDown");

  gr3j360to800quad.data   = (TGraph *)fp->Get("3jet_MassRange_360-800_hist_data");
  gr3j360to800quad.backnm = (TGraph *)fp->Get("3jet_MassRange_360-800_curve_fitTotal");
  gr3j360to800quad.backup = (TGraph *)fp->Get("3jet_MassRange_360-800_curve_fitTotal_SystUp");
  gr3j360to800quad.backdn = (TGraph *)fp->Get("3jet_MassRange_360-800_curve_fitTotal_SystDown");

  for (int i=0; i<NUMMASSPTS; i++)
    writeHistosPerMass(masspts[i],fp);
}
