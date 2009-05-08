// Script takes output of HBHEHORecPulseAnal1.cc and reconstructs the HBHEHO
// pulse from successive TDC-indexed histograms.
//

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "TH1.h"
#include "THStack.h"
#include "TF1.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TGraph.h"

#ifdef MAIN
typedef unsigned long uint32_t;
#endif

struct FileInfo_t {
  FileInfo_t(TFile *infp,string inpath,float inxs, int innev, float inwt):
    fp(infp),path(inpath),xsec(inxs),nev(innev),weight(inwt) {}
  TFile *fp;
  string path;
  float  xsec;
  int    nev;
  float  weight;
};

typedef struct {
  TH1 *p;
  string path;
  string type;
  string descr;
}
HistInfo_t;

#define NUM_COLORS 26
static const int colors[NUM_COLORS] = {
  1,2,3,4,5,6,7,8,9,10,
  11,13,25,28,29,30,33,35,38,40,
  41,42,43,46,47,49
};

//======================================================================

int getFileInfo(const char *filewithpaths,
		vector<FileInfo_t>& v_rootfiles,
		float integluminvpb)
{
  char line[256];

  FILE *pathfp = fopen(filewithpaths, "r");

  if (!pathfp) {
    cerr << "File not found, " << filewithpaths << endl;
    return 0;
  }

  while (!feof(pathfp) && fgets(line, 256, pathfp)) {
    char path[256];
    int nev;
    float xsec,weight;

    if (line[0] == '#') continue;

    int nscanned = sscanf(line, "%s %f %d", path,&xsec,&nev);

    TFile *tfile =  new TFile(path);
    
    if (tfile->IsZombie()) {
      cerr << "File failed to open, " << path << endl;
      return 0;
    }
    if (nscanned != 3) {
      cerr << "pathfile requires <pathstring> <xsec> <nevents>\n";
      return 0;
    }
    weight = (xsec*integluminvpb)/((float)nev);
    cout << "calculated weight for file " << path << " = " << weight << endl;
    FileInfo_t fileinfo(tfile,path,xsec,nev,weight);
    v_rootfiles.push_back(fileinfo);
  }
  return 1;
}                                                         // getFileInfo

//======================================================================

int getHisto(FileInfo_t& file,
	     const char *hname,
	     HistInfo_t& histinfo)
{
  file.fp->GetObject(hname,histinfo.p);
  if (!histinfo.p) {
    cout << "couldn't get " << hname;
    cout << " from " << file.path << endl;
    return 0;
  }

  char *fn = strrchr(file.path.c_str(),'/');
  if (!fn) fn = (char *)file.path.c_str();
  else fn++;

  histinfo.path  = std::string(hname);
  histinfo.descr = std::string(fn);

  histinfo.p->Scale(file.weight);

  return 1;
}                                                            // getHisto

//======================================================================

void plotHistsStacked(std::vector<HistInfo_t>& v_phist,
		      THStack *hs,
		      const char *titlestr,
		      bool logscale)
{
  TLegend *leg = new TLegend(0.60,0.60,0.85,0.85);
  leg->SetFillColor(10);

  for (unsigned int i=0; i<v_phist.size(); i++) {
    unsigned int j=v_phist.size()-i-1;
    TH1 *phist = v_phist[j].p;
    phist->SetFillColor(colors[j%NUM_COLORS]);
    hs->Add(phist);

    if (i==0) {
//    phist->SetXTitle(xtitle);
      phist->GetXaxis()->CenterTitle(true);
      phist->GetXaxis()->SetLabelSize(.04);
      phist->GetXaxis()->SetTitleSize(.04);
      phist->GetXaxis()->SetTitleOffset(1.1);
      phist->GetYaxis()->SetLabelSize(.04);
      //phist->GetYaxis()->SetRangeUser(0.00001,1.0);
      phist->SetTitle(titlestr);
      phist->SetLineColor(1);
      phist->SetLineWidth(2);
    }

    leg->AddEntry((TH1F *)v_phist[i].p,v_phist[i].descr.c_str(),"F");
  }

  hs->SetMinimum(0.1);

#if 0
  gStyle->SetStatX(.97);
  gStyle->SetStatY(.97);
#endif
  gStyle->SetOptStat(0);

  gROOT->SetStyle("Plain");

  TCanvas *c1 = new TCanvas(titlestr,titlestr,600,400);

  c1->SetFillColor(10);
  c1->SetTopMargin(0.04);
  c1->SetRightMargin(0.02);
  c1->SetLeftMargin(0.09);
  c1->SetBottomMargin(.1);

  if (logscale) c1->SetLogy();

  hs->Draw();
  leg->Draw();
}                                                    // plotHistsStacked

//======================================================================

void multistack(const char* filewithpaths,
		const char* filewithhistnames,
		float integluminvpb)
{
  char line[256];

  vector<FileInfo_t> v_rootfiles;

  FILE *hnamefp = fopen(filewithhistnames, "r");

  if (!hnamefp) {
    cerr << "File not found, " << filewithhistnames << endl;
    return;
  }

  if (!getFileInfo(filewithpaths, v_rootfiles,integluminvpb))
    return;

  // EXTRACT HISTOGRAMS, SCALE AND STACK

  while (!feof(hnamefp) && fgets(line, 256, hnamefp)) {
    if (char *newline = strchr(line,'\n')) *newline=0;

    if (line[0] == '#') continue;

    THStack *hs = new THStack(line,"");

    vector<HistInfo_t> v_phist;
    for (unsigned int i=0; i<v_rootfiles.size(); i++) {
      HistInfo_t histinfo;
      if (!getHisto(v_rootfiles[i],line,histinfo))
	continue;
      v_phist.push_back(histinfo);
      if (!i) hs->SetTitle(histinfo.p->GetTitle());
    }
    plotHistsStacked(v_phist, hs, line, true);
  }
}
