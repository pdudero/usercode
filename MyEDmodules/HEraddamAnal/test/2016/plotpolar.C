#include <iostream>
#include <vector>
#include "TH2D.h"
#include "TPad.h"
#include "TString.h"
#include "TVectorD.h"
#include "TVector3.h"
#include "TCanvas.h"
#include <glob.h>
#include <cassert>
#include <cstdlib>

//string fileglob = "CLEAN_NODUPEDAYS/_HE16/*.txt";
//string fileglob = "_HE16/*.txt";
//string fileglob = "_HE-16/fcamps2012_114090194*.txt";

string basedir = "CLEAN_NODUPE_NOPROBDAYS";
string scanfmt = "%*lf %*lf %*lf %*lf %*s %d,%d,%*s %lf";

bool gl_verbose=false;

using namespace std;

#define LINELEN 128

TH2D *hem, *hep;
TH2D *hemL0, *hepL0;

static const double hetwrEtas[] = { 
  1.305,  // 16
  1.392,  // 17
  1.479,  // 18
  1.566,  // 19
  1.653,  // 20
  1.740,  // 21
  1.830,  // 22
  1.930,  // 23
  2.043,  // 24
  2.172,  // 25
  2.322,  // 26
  2.500,  // 27
  2.650,  // 28
  2.868,  // 29
  3.000
};

static const double hepcalibL40=5.67572; // =1/.176189
static const double hemcalibL40=5.87865; // =1/.170107

//======================================================================

void bookPolarHists()
{
  TVectorD sintheta(15);

  for (int i=0; i<=14; i++) {
    TVector3 v3;
    v3.SetPtEtaPhi(1, hetwrEtas[i], 0);
    sintheta[14-i] = sin(v3.Theta());
    cout << sintheta[14-i] << endl;
  }
  hem   = new TH2D("hem","HEM",72,0,6.28,14,sintheta.GetMatrixArray());
  hep   = new TH2D("hep","HEP",72,0,6.28,14,sintheta.GetMatrixArray());
  hemL0 = new TH2D("hemL0","HEM",72,0,6.28,14,sintheta.GetMatrixArray());
  hepL0 = new TH2D("hepL0","HEP",72,0,6.28,14,sintheta.GetMatrixArray());
}

//======================================================================

void expandGlob(const string& globstr,
		vector<string>& outpaths)
{
  glob_t globbuf;

  int stat = glob (globstr.c_str(), GLOB_MARK, NULL, &globbuf);
  if (stat) {
    switch (stat) {
    case GLOB_NOMATCH: cerr << "No file matching glob pattern "; break;
    case GLOB_NOSPACE: cerr << "glob ran out of memory "; break;
    case GLOB_ABORTED: cerr << "glob read error "; break;
    default: cerr << "unknown glob error stat=" << stat << " "; break;
    }
    cerr << globstr << endl;
    exit(-1);
  }

  outpaths.clear();
  for (size_t i=0; i<globbuf.gl_pathc; i++)
    outpaths.push_back(string(globbuf.gl_pathv[i]));

  if (globbuf.gl_pathc) 
    globfree(&globbuf);
}                                                          // expandGlob

//======================================================================
// takes table of "xbincenter ybincenter bincontent"
//
void load2DHistoContentsFromTextFile(const char *filename, 
				     TH2 *h2) // must be prebooked
{
  char linein[LINELEN];

  FILE *fp = fopen(filename, "r");

  if (!fp) {
    cerr << "File failed to open, " << filename << endl;
    return;
  }

  if (gl_verbose)
    cout << "Loading bin contents from file " << filename << endl;

  int nrec=0;
  //cout<<"nrec\txbc\tybc\tibin\tbinc"<<endl;
  while (!feof(fp) && fgets(linein,LINELEN,fp)) {
    int ieta, iphi;
    double binc;
    if (linein[0]=='#') continue; // comments are welcome
    int nscan= sscanf(linein, scanfmt.c_str(), &ieta, &iphi, &binc);

    double dphi = TMath::Pi() * (iphi-1)/36.;
    double deta = hetwrEtas[abs(ieta)-16];
    TVector3 v3;
    v3.SetPtEtaPhi(1, deta, 0);
    double sintheta = sin(v3.Theta());

    if (nscan != 3) {
      cerr << "scan failed:";
      cerr << "  nscan  = " << nscan    << endl;
      cerr << "  file   = " << filename << endl;
      cerr << "  line   = " << linein;
      break;
    }
    nrec++;
    int ibin = h2->FindFixBin(dphi,sintheta);
    //cout<<nrec<<"\t"<<xbc<<"\t"<<ybc<<"\t"<<ibin<<"\t"<<binc<<endl;
    h2->SetBinContent(ibin,binc);

    if (abs(ieta) > 20) {
      // double-wide 10 degree cells
      dphi = TMath::Pi() * iphi/36.;
      ibin = h2->FindFixBin(dphi,sintheta);
      h2->SetBinContent(ibin,binc);
    }
  }
  cout << nrec << " records read in" << endl;

}                                    //  load2DHistoContentsFromTextFile

//======================================================================

void loadHistos(TH2 *hem, TH2 *hep, int hemday, int hepday)
{
  vector<string> vpaths;

  expandGlob(Form("%s/_HE-*d1/*day%d.txt",basedir.c_str(),hemday), vpaths);
  for (size_t i=0; i<vpaths.size(); i++)
    load2DHistoContentsFromTextFile(vpaths[i].c_str(),hem);

  expandGlob(Form("%s/_HE[12]*d1/*day%d.txt",basedir.c_str(),hepday), vpaths);
  for (size_t i=0; i<vpaths.size(); i++)
    load2DHistoContentsFromTextFile(vpaths[i].c_str(),hep);
}

//======================================================================

void plotpolar()
{
  bookPolarHists();

  gStyle->SetOptStat(0);

  TCanvas *c1 = new TCanvas("c1","c1",1200,600);

  loadHistos(hem,   hep,   302,302);
  loadHistos(hemL0, hepL0, 104,113);

  hem->Divide(hemL0);
  hep->Divide(hepL0);

  // Calibrate for laser amplitude changes:
  hem->Scale(hemcalibL40);
  hep->Scale(hepcalibL40);

  hem->GetZaxis()->SetRangeUser(0.,1.);
  hep->GetZaxis()->SetRangeUser(0.,1.);

  c1->Divide(2);

  c1->cd(1);
  TH2D *framehem = new TH2D("framehem","HEM",10,-0.6,0.6,10,-0.6,0.6);
  framehem->Draw();
  hem->Draw("POL COLZ same");

  c1->cd(2);
  TH2D *framehep = new TH2D("framehep","HEP",10,-0.6,0.6,10,-0.6,0.6);
  framehep->Draw();
  hep->Draw("POL COLZ same");

  c1->SaveAs("polar_day302.png");
}
