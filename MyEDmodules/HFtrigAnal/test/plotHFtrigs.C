// Script takes output of HBHEHORecPulseAnal1.cc and reconstructs the HBHEHO
// pulse from successive TDC-indexed histograms.
//
//#include <boost/algorithm/string/split.hpp>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;

#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TLegend.h"
#include "TString.h"
#include "TROOT.h"

//extern TROOT *gROOT;

#ifdef MAIN
typedef unsigned long uint32_t;
#endif

typedef struct {
  TFile  *p;
  string  name;
}
FileInfo_t;

typedef struct {
  TH1    *p;
  string  descr;
  vector<int> args;
}
HistInfo_t;

//======================================================================

bool getOneHisto(FileInfo_t& file,
		 HistInfo_t& hi,
		 const char *hnamefmt,
		 ...)
{
  char hname[80];

  va_list args;
  va_start (args, hnamefmt);
  vsprintf (hname,hnamefmt, args);

  TH2 *h2p = (TH2 *)file.p->FindObjectAny(hname);
  hi.p     = h2p;

  if (!h2p) {
    cout << "\tcouldn't get " << hname;
    cout << " from " << file.name << endl;
    return false;
  }

  va_end (args);

  return true;
}

//======================================================================

void plotRHplots(std::vector<HistInfo_t>& v_histinfo,
		 int  runnum,
		 bool saveplots)
{
  if (!v_histinfo.size()) return;

  cout <<"plotRHplots"<< endl;

  gROOT->SetStyle("Plain");

  TCanvas *c1 = new TCanvas("totalE","totalE",
			    //v_histinfo.size()*400,400);
			    600,600);

  gStyle->SetOptStat(1111);

  TH1F *h1p = (TH1F *)v_histinfo[0].p;
  gPad->SetFillColor(10);
  gPad->SetTopMargin(0.04);
  gPad->SetRightMargin(0.12);
  gPad->SetLeftMargin(0.1);
  gPad->SetBottomMargin(.1);
  gPad->SetLogy(0);

  h1p->GetXaxis()->SetTitle("GeV"); h1p->GetXaxis()->CenterTitle();
  h1p->Draw();

  stringstream out;
  out << runnum;
  string runstr = out.str();

  if (saveplots) {
    string titlestr = "run"+runstr+string(h1p->GetName()) + ".png";
    c1->SaveAs(titlestr.c_str());
  }
}                                                         // plotRHplots

//======================================================================

void plotRHplot2(std::vector<HistInfo_t>& v_histinfo,
		 int  runnum,
		 bool saveplots)
{
  if (!v_histinfo.size()) return;

  cout <<"plotRHplot2"<< endl;

  gROOT->SetStyle("Plain");

  TCanvas *c1 = new TCanvas("totalE","totalE",
			    //v_histinfo.size()*400,400);
			    600,600);

  gStyle->SetOptStat(0);
  gStyle->SetPalette(1,0);

  TH2F *h2p = (TH2F *)v_histinfo[0].p;
  gPad->SetFillColor(10);
  gPad->SetTopMargin(0.04);
  gPad->SetRightMargin(0.15);
  gPad->SetLeftMargin(0.1);
  gPad->SetBottomMargin(.1);

  h2p->GetXaxis()->SetTitle("ieta"); h2p->GetXaxis()->CenterTitle();
  h2p->GetYaxis()->SetTitle("iphi"); h2p->GetYaxis()->CenterTitle();
  h2p->Draw("COLZ");

  stringstream out;
  out << runnum;
  string runstr = out.str();

  if (saveplots) {
    string titlestr = "run"+runstr+string(h2p->GetName()) + ".png";
    c1->SaveAs(titlestr.c_str());
  }
}                                                         // plotRHplot2

//======================================================================

void plotVecOnOneCanvas(std::vector<HistInfo_t>& v_histinfo,
			string titlestr,
			const char *drawstring,
			int optstats,
			bool saveplots)
{
  if (!v_histinfo.size()) return;

  cout <<"plotVecOnOneCanvas"<< endl;

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(optstats);
  gStyle->SetPalette(1,0);

  TCanvas *c1 = new TCanvas(titlestr.c_str(),titlestr.c_str(),
			    //v_histinfo.size()*400,400);
			    900,600);
  c1->Divide(2);
  
  for (unsigned int i=0; i<v_histinfo.size(); i++) {
    TH2F *h2p = (TH2F *)v_histinfo[i].p;
    c1->cd(i+1);
    gPad->SetFillColor(10);
    gPad->SetTopMargin(0.04);
    gPad->SetRightMargin(0.12);
    gPad->SetLeftMargin(0.1);
    gPad->SetBottomMargin(.1);

    h2p->GetXaxis()->SetTitle("ieta"); h2p->GetXaxis()->CenterTitle();
    h2p->GetYaxis()->SetTitle("iphi"); h2p->GetYaxis()->CenterTitle();
    h2p->Draw(drawstring);
  }

  c1->cd();

  titlestr += ".png";
  if (saveplots) c1->SaveAs(titlestr.c_str());
}                                                  // plotVecOnOneCanvas

//======================================================================

void plotDigiSpectra(vector<HistInfo_t>& v_histinfo,
		     const char *titlestr,
		     int runnum,
		     bool saveplots)
{
  cout <<"plotDigiSpectra, plotting/summing " << v_histinfo.size() << " histos" << endl;
  if (!v_histinfo.size()) {
    return;
  }

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(111111);
  gStyle->SetPalette(1,0);

  TCanvas *c1;
  TH1F *sumhp = 0;
  string sumname;

  stringstream out;
  out << runnum;
  string runstr = out.str();

  for (unsigned int i=0; i<v_histinfo.size(); i++) {
    TH1F *h1p = (TH1F *)v_histinfo[i].p;
    if (!h1p) continue;
    if (!sumhp) {
      sumname = "Summed " + string(titlestr) + ", Run " + runstr;
      sumhp = (TH1F *)h1p->Clone(sumname.c_str());
      sumhp->SetTitle(sumname.c_str());
      //c1 = new TCanvas(titlestr,titlestr,600,600);
      //c1->Divide(13,4);
    } else
      sumhp->Add(h1p);

#if 0
    c1->cd(i+1);
    gPad->SetFillColor(10);
    gPad->SetTopMargin(0.04);
    gPad->SetRightMargin(0.1);
    gPad->SetLeftMargin(0.1);
    gPad->SetBottomMargin(.1);
    gPad->SetLogy();
    gPad->SetLogx();

    //h1p->GetXaxis()->SetTitle("ieta"); h1p->GetXaxis()->CenterTitle();
    //h1p->GetYaxis()->SetTitle("iphi"); h1p->GetYaxis()->CenterTitle();
    h1p->Draw();
#endif
  }

  if (sumhp) {
    TCanvas *c2 = new TCanvas(sumname.c_str(),sumname.c_str(),
			      600,600);
    c2->SetLogy();
    sumhp->Draw();

    sumname += ".png";
    if (saveplots) c2->SaveAs(sumname.c_str());
  }
}                                                     // plotDigiSpectra

//======================================================================
// tuned for run 61186 right now! 09/05/2008

void getSpectrumHistos(FileInfo_t& file,
		       int runnum,
		       vector<HistInfo_t>& v_hi,
		       vector<HistInfo_t>& v_nohottowers)
{
  v_hi.clear();
  v_nohottowers.clear();

  int depth=1;
  for (int ieta=29; ieta <= 41; ieta++) {
    HistInfo_t hi;
    getOneHisto(file, hi,
		"run%d_HF_ieta=%d_depth=%d_Spectrum",
		runnum,ieta,depth);

    v_hi.push_back(hi);

    if (ieta != 35) v_nohottowers.push_back(hi);
  }
  for (int ieta=-29; ieta >= -41; ieta--) {
    HistInfo_t hi;
    getOneHisto(file, hi,
		"run%d_HF_ieta=%d_depth=%d_Spectrum",
		runnum,ieta,depth);

    v_hi.push_back(hi);

    if (ieta != -31) v_nohottowers.push_back(hi);
  }

  depth=2;

  for (int ieta=29; ieta <= 41; ieta++) {
    HistInfo_t hi;
    getOneHisto(file, hi,
		"run%d_HF_ieta=%d_depth=%d_Spectrum",
		runnum,ieta,depth);

    v_hi.push_back(hi);

    if ((ieta != 30) && (ieta != 36)) v_nohottowers.push_back(hi);
  }

  for (int ieta=-29; ieta >= -41; ieta--) {
    HistInfo_t hi;
    getOneHisto(file, hi,
		"run%d_HF_ieta=%d_depth=%d_Spectrum",
		runnum,ieta,depth);

    v_hi.push_back(hi);

    v_nohottowers.push_back(hi);
  }
}                                                   // getSpectrumHistos

//======================================================================

void plotIntegratedWedges(vector<HistInfo_t>& v_histinfo,
			  string titlestr,
			  int runnum,
			  bool saveplots)
{
  cout <<"plotIntegratedWedges, plotting/summing ";
  cout << v_histinfo.size() << " histos" << endl;
  if (!v_histinfo.size()) {
    return;
  }

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetTitleW(0.95);

  TCanvas *c1 = new TCanvas("integWedges","integWedges", 800,600);
  c1->SetLogy();
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.1);
  gPad->SetFillColor(10);

  char name[80];
  sprintf(name,", Run #%d",runnum);
  titlestr += string(name);

  TLegend *leg = new TLegend(0.4,0.7,0.95,0.9, "Tech Trig 9, Lumi Segments 58-149");

  for (uint32_t i=0; i<v_histinfo.size(); i++) {
    TH1F  *hp     = (TH1F *)v_histinfo[i].p;
    int    nbinsx = hp->GetNbinsX();
    double minx   = hp->GetXaxis()->GetXmin();
    double maxx   = hp->GetXaxis()->GetXmax();
    sprintf (name,"%s_int",hp->GetName());
    TH1F *h_integ = new TH1F(name,titlestr.c_str(),nbinsx,minx,maxx);

    for (int j=1; j<=nbinsx; j++)
      h_integ->SetBinContent(j,hp->Integral(j,nbinsx+1));

    h_integ->Scale(1.0/hp->Integral(1,nbinsx+1));

    h_integ->SetMarkerStyle(20+(i*5));
    h_integ->SetLineColor(2*(i+1));
    h_integ->SetLineWidth(2);

    leg->AddEntry(h_integ,v_histinfo[i].descr.c_str(),"LP");

    if (!i) {
      h_integ->SetXTitle("HF Wedge Trigger Coincidence Multiplicity");
      h_integ->SetYTitle("Fraction of events passed");
      h_integ->GetXaxis()->CenterTitle();
      h_integ->GetYaxis()->CenterTitle();
      h_integ->GetYaxis()->SetTitleOffset(1.2);
      h_integ->GetXaxis()->SetLabelSize(0.03);
      h_integ->GetYaxis()->SetLabelSize(0.03);

      h_integ->GetXaxis()->SetRangeUser(1.5,20.5);

      h_integ->Draw("LP");
    }
    else
      h_integ->Draw("LP SAME");
  }
  leg->Draw();
  //leg->SetTextSize(20);

  if (saveplots) {
    string plotstr("integWedges.png");
    c1->SaveAs(plotstr.c_str());
  }
}                                                // plotIntegratedWedges

//======================================================================

void plotTriggerEffHists(vector<HistInfo_t>& v_histinfo,
			 string titlestr,
			 int runnum,
			 bool saveplots)
{
  cout <<"plotTriggerEffHists, plotting/summing ";
  cout << v_histinfo.size() << " histos" << endl;
  if (!v_histinfo.size()) {
    return;
  }

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetTitleW(0.95);

  TCanvas *c1 = new TCanvas("triggerEff","triggerEff", 800,600);
  //c1->SetLogy();
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.1);
  gPad->SetFillColor(10);

  char name[80];
  sprintf(name,", Run #%d",runnum);
  titlestr += string(name);

  TLegend *leg = new TLegend(0.4,0.7,0.95,0.9, "Tech Trig 9, Lumi Segments 58-149");

  for (uint32_t i=0; i<v_histinfo.size(); i++) {
    TH1F  *hp     = (TH1F *)v_histinfo[i].p;

    hp->SetLineColor(2*(i+1));
    hp->SetLineWidth(2);

    leg->AddEntry(hp,v_histinfo[i].descr.c_str(),"L");

    if (!i) {
      hp->SetXTitle("Threshold (LUT counts)");
      hp->SetYTitle("Fraction of events from run passed");
      hp->GetXaxis()->CenterTitle();
      hp->GetYaxis()->CenterTitle();
      hp->GetYaxis()->SetTitleOffset(1.2);
      hp->GetXaxis()->SetLabelSize(0.03);
      hp->GetYaxis()->SetLabelSize(0.03);

      //hp->GetXaxis()->SetRangeUser(1.5,20.5);

      hp->Draw("C");
    }
    else
      hp->Draw("C SAME");
  }
  leg->Draw();
  //leg->SetTextSize(20);

  if (saveplots) {
    string plotstr("triggerEff.png");
    c1->SaveAs(plotstr.c_str());
  }
}                                                 // plotTriggerEffHists

//======================================================================

void  plotLumi(vector<HistInfo_t>& v_histinfo,
	       string titlestr,
	       int runnum,
	       bool saveplots)
{
  cout <<"plotLumi, plotting ";
  cout << v_histinfo.size() << " histos" << endl;
  if (!v_histinfo.size()) {
    return;
  }

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  //gStyle->SetTitleW(0.95);

  TCanvas *c1 = new TCanvas("LumiSections","LumiSections", 800,600);
  //c1->SetLogy();
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.1);
  gPad->SetFillColor(10);

  char name[80];
  sprintf(name,", Run #%d",runnum);
  titlestr += string(name);

  TLegend *leg = new TLegend(0.2,0.7,0.5,0.9); //, "Tech Trig 9, Lumi Segments 58-149");

  for (uint32_t i=0; i<v_histinfo.size(); i++) {
    TH1F  *hp     = (TH1F *)v_histinfo[i].p;

    hp->SetLineColor((i+1));
    hp->SetLineWidth(2);

    if (!i) {
      hp->SetXTitle("Lumi Section");
      hp->GetXaxis()->CenterTitle();
      hp->GetYaxis()->CenterTitle();
      hp->GetYaxis()->SetTitleOffset(1.2);
      hp->GetXaxis()->SetLabelSize(0.03);
      hp->GetYaxis()->SetLabelSize(0.03);

      hp->Draw();
    }
    else
      hp->Draw("SAME");

    leg->AddEntry(hp,v_histinfo[i].descr.c_str(),"L");
  }

  leg->Draw();
  //leg->SetTextSize(20);

  if (saveplots) {
    string plotstr("lumisections.png");
    c1->SaveAs(plotstr.c_str());
  }
}

//======================================================================

void plotPMThistos(vector<HistInfo_t>& v_histinfo,
		   string titlestr,
		   int runnum,
		   string plotopts,
		   bool saveplots)
{
  cout <<"plotPMThistos, plotting ";
  cout << v_histinfo.size() << " histos" << endl;
  if (!v_histinfo.size()) {
    return;
  }

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  //gStyle->SetTitleW(0.95);

  TCanvas *c1 = new TCanvas(titlestr.c_str(),titlestr.c_str(), 800,600);
  //c1->SetLogy();
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.1);
  gPad->SetFillColor(10);

  char name[80];
  sprintf(name,", Run #%d",runnum);
  titlestr += string(name);

  TLegend *leg = new TLegend(0.7,0.7,0.95,0.9); //, "Tech Trig 9, Lumi Segments 58-149");

  for (uint32_t i=0; i<v_histinfo.size(); i++) {
    TH1F  *hp     = (TH1F *)v_histinfo[i].p;

    hp->SetLineColor((i+1));
    hp->SetLineWidth(2);
    hp->SetMarkerStyle(20+(i*5));

    if (!i) {
      hp->SetTitle(titlestr.c_str());
      hp->GetXaxis()->CenterTitle();
      hp->GetYaxis()->CenterTitle();
      hp->GetYaxis()->SetTitleOffset(1.2);
      hp->GetXaxis()->SetLabelSize(0.03);
      hp->GetYaxis()->SetLabelSize(0.03);

      hp->Draw(plotopts.c_str());
    }
    else {
      string po(plotopts + "SAME");
      hp->Draw(po.c_str());
    }

    if (plotopts.length())
      leg->AddEntry(hp,v_histinfo[i].descr.c_str(),plotopts.c_str());
    else
      leg->AddEntry(hp,v_histinfo[i].descr.c_str(),"L");
  }

  leg->Draw();
  //leg->SetTextSize(20);

  if (saveplots) {
    string plotstr("pmt.png");
    c1->SaveAs(plotstr.c_str());
  }
}

//======================================================================
static const double theHFEtaBounds[] = {
  -5.191, -4.889, -4.716, -4.538, -4.363, -4.191, -4.013,
  -3.839, -3.664, -3.489, -3.314, -3.139, -2.964, -2.853, 
  2.853, 2.964, 3.139, 3.314, 3.489, 3.664, 3.839, 
  4.013, 4.191, 4.363, 4.538, 4.716, 4.889, 5.191
};

static const double theHFradii_mm[] = {
  9999.9, // this first entry is bogus
  1300.0, 1162.0, 975.0, 818.0, 686.0, 576.0, 483.0,
  406.0,340.0, 286.0, 240.0, 201.0, 169.0, 125.0
};

void calcAreaCorrection(std::vector<double>& v_areaCorr)
{
  double maxfactor =0.0;
  for (int i=0; i<14; i++) {
    double r1 = theHFradii_mm[i];
    double r2 = theHFradii_mm[i+1];
    double areafactor = 1.0/(r1*r1 - r2*r2);
    v_areaCorr.push_back(areafactor);
    if (areafactor > maxfactor) maxfactor = areafactor;

    printf ("%2d %7.1lf %7.1lf %10.2e %10.2e\n", i, r1, r2, areafactor, maxfactor);
  }
  for (uint32_t i = 0; i<v_areaCorr.size(); i++) 
    v_areaCorr[i] /= maxfactor;
}

void  prettifyEtaPlot(vector<HistInfo_t>& v_histinfo,
		      string titlestr,
		      int runnum,
		      bool saveplots)
{
  cout <<"prettifyEtaPlot" << endl;

  TH1F *h_ieta    = (TH1F *)v_histinfo[0].p;
  TH1F *h_physEta     = new TH1F("physEtah",h_ieta->GetTitle(),27,theHFEtaBounds);
  TH1F *h_physEtaCorr = new TH1F("physEtaCorrh",h_ieta->GetTitle(),27,theHFEtaBounds);

  std::vector<double> v_areaCorr;
  calcAreaCorrection(v_areaCorr);

  for (int ibin=1; ibin<=27; ibin++) {  // -41 -> -29, X=empty bin, 29->41
    float binc = h_ieta->GetBinContent(ibin);
    h_physEta->SetBinContent(ibin,binc);
    int corrindex = abs(ibin-14);
    float corrfactor = (float)v_areaCorr[corrindex];
    h_physEtaCorr->SetBinContent(ibin, binc*corrfactor);
    printf("%2d: %5.1f %7.5f\n", ibin, binc, corrfactor);
  }
			     
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  //gStyle->SetTitleW(0.95);

  TCanvas *c1 = new TCanvas("Uncorrected","Uncorrected", 800,600);
  //c1->SetLogy();
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.1);
  gPad->SetFillColor(10);

  h_physEta->SetXTitle("#eta");
  h_physEta->SetYTitle("GeV");
  h_physEta->GetXaxis()->CenterTitle();
  h_physEta->GetYaxis()->CenterTitle();
  h_physEta->SetLineWidth(2);
  h_physEta->GetXaxis()->SetRangeUser(-5.0,5.0);

  h_physEta->Draw();

  TCanvas *c2 = new TCanvas("Corrected","Corrected", 800,600);
  //c1->SetLogy();
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.1);
  gPad->SetFillColor(10);

  h_physEtaCorr->SetXTitle("#eta");
  h_physEtaCorr->SetYTitle("GeV");
  h_physEtaCorr->GetXaxis()->CenterTitle();
  h_physEtaCorr->GetYaxis()->CenterTitle();
  h_physEtaCorr->SetLineWidth(2);
  h_physEtaCorr->GetXaxis()->SetRangeUser(-5.0,5.0);

  h_physEtaCorr->Draw();
}                                                    //  prettifyEtaPlot

//======================================================================

void plotHFtrigs(const char* rootfile,
		 int runnum,
		 bool saveplots=false)
{
  FileInfo_t file;
  file.name = string(rootfile);
  file.p    = new TFile(rootfile);

  if (file.p->IsZombie()) {
    cerr << "File failed to open, " << file.name << endl;
    return;
  }

  vector<HistInfo_t> v_hi(2);
#if 0
  getOneHisto(file, v_hi[0], "run%dHF_MaxADC_occupancy_depth=%d",runnum,1);
  getOneHisto(file, v_hi[1], "run%dHF_MaxADC_occupancy_depth=%d",runnum,2);

  plotVecOnOneCanvas(v_hi,"HF MaxADC Occupancies", "COLZ",0,saveplots);

  return;

  v_hi.clear();
  vector<HistInfo_t> v_nohottowers;
  getSpectrumHistos(file, runnum, v_hi, v_nohottowers);
  plotDigiSpectra(v_hi,"HF LUTout Spectra",runnum,saveplots);
  plotDigiSpectra(v_nohottowers,"HF LUTout Spectra, Hot Towers Masked",runnum,saveplots);

  v_hi.clear();
  v_hi.resize(1);
  //plotRHplots(v_hi,runnum,saveplots);

  //getOneHisto(file,v_hi[0],"run57662rhEforEvent00521");
  //plotRHplot2(v_hi,runnum,saveplots);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnWedgesOverThreshGoodBx",runnum);
  getOneHisto(file,v_hi[1],"run%dnWedgesOverThreshBadBx",runnum);
  v_hi[0].descr = string("In Bx Window 900,901,904");
  v_hi[1].descr = string("Outside Bx Window");
  plotIntegratedWedges(v_hi, "Trigger Efficiency vs. Coincidence Multiplicity", runnum, saveplots);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dTriggerEfficiencyVsThreshGoodBx",runnum);
  getOneHisto(file,v_hi[1],"run%dTriggerEfficiencyVsThreshBadBx",runnum);
  v_hi[0].descr = string("In Bx Window 900,901,904");
  v_hi[1].descr = string("Outside Bx Window");
  plotTriggerEffHists(v_hi, "Trigger Efficiency vs. Threshold", runnum, saveplots);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dlumisegh",runnum);
  getOneHisto(file,v_hi[1],"run%dlumisegGoodBxh",runnum);
  v_hi[0].descr = string("All");
  v_hi[1].descr = string("In Bx Window");
  plotLumi(v_hi, "#Events per Lumi Section", runnum, saveplots);

  v_hi.clear();
  v_hi.resize(3);
  getOneHisto(file,v_hi[0],"run%dnPMThits",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMTsamesideHits",runnum);
  getOneHisto(file,v_hi[2],"run%dnPMTopposideHits",runnum);
  v_hi[0].descr = string("Total Individual hits");
  v_hi[1].descr = string("Same Side Pairings");
  v_hi[2].descr = string("Opposite Side Pairings");
  plotPMThistos(v_hi, "# PMT hits per event", runnum,"P",saveplots);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMThitsSameSideDeltaIeta",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMThitsDeltaIeta",runnum);
  v_hi[0].descr = string("Same Side");
  v_hi[1].descr = string("Opposite Side");
  plotPMThistos(v_hi, "#Delta i#eta(wedge1,wedge2) PMT hit pairs", runnum, "",saveplots);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMThitsSameSideDeltaIphi",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMThitsDeltaIphi",runnum);
  v_hi[0].descr = string("Same Side");
  v_hi[1].descr = string("Opposite Side");
  plotPMThistos(v_hi, "#Delta i#phi(wedge1,wedge2) PMT hit pairs", runnum,"", saveplots);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMThitsSameSideAvgIeta",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMThitsAvgIeta",runnum);
  v_hi[0].descr = string("Same Side");
  v_hi[1].descr = string("Opposite Side");
  plotPMThistos(v_hi, "Avg i#eta(wedge1,wedge2) PMT hit pairs", runnum, "",saveplots);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMThitsSameSideAvgIphi",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMThitsAvgIphi",runnum);
  v_hi[0].descr = string("Same Side");
  v_hi[1].descr = string("Opposite Side");
  plotPMThistos(v_hi, "Avg i#phi(wedge1,wedge2) PMT hit pairs", runnum,"", saveplots);
#endif
  v_hi.clear();
  v_hi.resize(1);
  getOneHisto(file,v_hi[0],"run%dEperEtaNonPMTh",runnum);
  prettifyEtaPlot(v_hi,"c1",runnum,saveplots);
}
