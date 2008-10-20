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
#include "TLatex.h"
#include "TText.h"

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
			bool logscale,
			bool saveplots)
{
  if (!v_histinfo.size()) return;

  cout <<"plotVecOnOneCanvas"<< endl;

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(optstats);
  gStyle->SetStatX(.98);
  gStyle->SetStatY(0.92);
  gStyle->SetStatW(0.25);
  gStyle->SetStatH(0.2);

  gStyle->SetPalette(1,0);

  uint32_t numplots = v_histinfo.size();

  TCanvas *c1 = new TCanvas(titlestr.c_str(),titlestr.c_str(),
			    numplots*500,500);

  c1->Divide(numplots);
  
  for (unsigned int i=0; i<numplots; i++) {
    TH1F *h1p = (TH1F *)v_histinfo[i].p;
    c1->cd(i+1);
    gPad->SetFillColor(10);
    gPad->SetTopMargin(0.08);
    gPad->SetRightMargin(0.02);
    gPad->SetLeftMargin(0.07);
    gPad->SetBottomMargin(.12);
    if (logscale) gPad->SetLogy();

    //h1p->GetXaxis()->SetTitle("ieta");
    //h1p->GetYaxis()->SetTitle("iphi");
    h1p->GetXaxis()->CenterTitle();
    h1p->GetYaxis()->CenterTitle();
    h1p->GetXaxis()->SetTitleOffset(1.2);

#if 0
    if (!i)  h1p->SetTitle("Tower Energy Avg, Beam-Gas Events, Run #62232");
    else     h1p->SetTitle("Tower Energy Max, Beam-Gas Events, Run #62232");
#endif
    h1p->Draw(drawstring);
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

void  plotLumi(HistInfo_t& hiAllBx,
	       HistInfo_t& hiGoodBx,
	       string titlestr,
	       int runnum,
	       bool saveplots)
{
  cout <<"plotLumi" << endl;

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

  hiAllBx.p->SetLineColor(1);
  hiAllBx.p->SetLineWidth(2);

  hiAllBx.p->SetXTitle("Lumi Section");
  hiAllBx.p->GetXaxis()->CenterTitle();
  hiAllBx.p->GetYaxis()->CenterTitle();
  hiAllBx.p->GetYaxis()->SetTitleOffset(1.2);
  hiAllBx.p->GetXaxis()->SetLabelSize(0.03);
  hiAllBx.p->GetYaxis()->SetLabelSize(0.03);
  hiAllBx.p->Draw();

  leg->AddEntry(hiAllBx.p,hiAllBx.descr.c_str(),"L");

  hiGoodBx.p->SetLineColor(2);
  hiGoodBx.p->SetLineWidth(2);
  hiGoodBx.p->Draw("SAME");

  leg->AddEntry(hiGoodBx.p,hiGoodBx.descr.c_str(),"L");
  leg->Draw();
  //leg->SetTextSize(20);

  TH1F h_frac = (*(TH1F *)(hiGoodBx.p))/(*(TH1F *)(hiAllBx.p));

  TCanvas *c2 = new TCanvas("Fraction Good BX","Fraction Good BX", 800,600);
  //c1->SetLogy();
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.1);
  gPad->SetFillColor(10);
  h_frac.SetLineWidth(2);
  cout << h_frac.GetEntries() << endl;
  c2->cd();
  h_frac.Draw();

  if (saveplots) {
    string plotstr("lumisections.png");
    c1->SaveAs(plotstr.c_str());
  }
}                                                            // plotLumi

//======================================================================

void plotPMThistos(vector<HistInfo_t>& v_histinfo,
		   string titlestr,
		   int runnum,
		   string plotopts)
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
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.1);
  gPad->SetFillColor(10);

  char name[80];
  sprintf(name,", Run #%d",runnum);
  titlestr += string(name);

  TLegend *leg = new TLegend(0.7,0.83,0.95,0.9); //, "Tech Trig 9, Lumi Segments 58-149");

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

  //TText *tt = new TText();
  //tt->SetTextSize(.025);
  //sprintf(name,"(Threshold=%4.1f GeV)",thresholdGeV);
  //tt->DrawTextNDC(0.75,0.81,name);
}                                                       // plotPMThistos

//======================================================================

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

#if 0
    printf ("%2d %7.1lf %7.1lf %10.2e %10.2e\n",
	    i, r1, r2, areafactor, maxfactor);
#endif
  }
  for (uint32_t i = 0; i<v_areaCorr.size(); i++) 
    v_areaCorr[i] /= maxfactor;
}

//======================================================================

static const double theHFetaBounds[] = {
  -5.191, -4.889, -4.716, -4.538, -4.363, -4.191, -4.013,
  -3.839, -3.664, -3.489, -3.314, -3.139, -2.964, -2.853, 
  2.853, 2.964, 3.139, 3.314, 3.489, 3.664, 3.839, 
  4.013, 4.191, 4.363, 4.538, 4.716, 4.889, 5.191
};

TH1F *ieta2PhysEtaHisto(TH1F *h_ieta)
{
  string physname = string(h_ieta->GetName()) + "_phys";

  TH1F *h_physEta= new TH1F(physname.c_str(),h_ieta->GetTitle(),27,theHFetaBounds);
  for (int ibin=1; ibin<=27; ibin++) {  // -41 -> -29, X=empty bin, 29->41
    float binc = h_ieta->GetBinContent(ibin);
    h_physEta->SetBinContent(ibin,binc);
  }
  return h_physEta;
}

//======================================================================

TH1F *genAreaCorrectedEtaHisto(TH1F *h_uncorrected)
{
  TH1F *h_corrected = (TH1F *)h_uncorrected->Clone();
  h_corrected->SetName("physEtaCorrh");

  std::vector<double> v_areaCorr;
  calcAreaCorrection(v_areaCorr);

  for (int ibin=1; ibin<=27; ibin++) {  // -41 -> -29, X=empty bin, 29->41
    float binc = h_uncorrected->GetBinContent(ibin);
    int corrindex = abs(ibin-14);
    float corrfactor = (float)v_areaCorr[corrindex];
    h_corrected->SetBinContent(ibin, binc*corrfactor);
    //printf("%2d: %5.1f %7.5f\n", ibin, binc, corrfactor);
  }
  return h_corrected;
}

//======================================================================

void drawEtaHisto(TH1F *h_eta, const char *ctitle)
{
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetTitleW(.98);
  gStyle->SetTitleH(0.09);

  TCanvas *c1 = new TCanvas(ctitle, ctitle, 800,500);
  //c1->SetLogy();

  c1->Divide(2,1);
  c1->cd(1);
  gPad->SetRightMargin(0.01);
  gPad->SetTopMargin(0.05);
  gPad->SetLeftMargin(0.08);
  gPad->SetFillColor(10);

  h_eta->SetTitle("");
  //h_eta->SetXTitle("#eta");
  h_eta->SetYTitle("GeV");
  //h_eta->GetXaxis()->CenterTitle();
  h_eta->GetYaxis()->CenterTitle();
  h_eta->SetLineWidth(1);
  h_eta->GetXaxis()->SetRangeUser(-5.2,-2.854);
  h_eta->GetYaxis()->SetRangeUser(-1.0,35.0);
  h_eta->SetMarkerStyle(24);

  h_eta->Draw("E1");

  TH1F *h_etaClone = (TH1F *)h_eta->Clone();

  c1->cd(2);
  gPad->SetTopMargin(0.05);
  gPad->SetRightMargin(0.08);
  gPad->SetLeftMargin(0.0);
  gPad->SetFillColor(10);

  h_etaClone->GetXaxis()->SetRangeUser(2.853,5.2);
  h_etaClone->SetXTitle("");
  h_etaClone->GetYaxis()->SetTickLength(0);
  h_etaClone->GetYaxis()->Delete();
  h_etaClone->Draw("E1");

  c1->cd();

  TLatex *tl = new TLatex();
  tl->SetNDC();
  tl->DrawLatex(0.487,0.03,"#eta");
}                                                        // drawEtaHisto

//======================================================================

void  prettifyEtaPlot(HistInfo_t& hi)
  //		      string titlestr,
  //		      int runnum,
  //		      bool saveplots)
{
  cout <<"prettifyEtaPlot" << endl;

  TH1F *h_ieta    = (TH1F *)hi.p;

  TH1F *h_physEta     = ieta2PhysEtaHisto(h_ieta);
  TH1F *h_physEtaCorr = genAreaCorrectedEtaHisto(h_physEta);

  string titlestr = string(h_physEta->GetName()) + "_uncor";
  drawEtaHisto(h_physEta, titlestr.c_str());
  titlestr = string(h_physEta->GetName()) + "_cor";
  drawEtaHisto(h_physEtaCorr, titlestr.c_str());
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

  plotVecOnOneCanvas(v_hi,"HF MaxADC Occupancies", "COLZ",0,false,saveplots);

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
#endif
#if 0
  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dlumisegh",runnum);
  getOneHisto(file,v_hi[1],"run%dlumisegGoodBxh",runnum);
  v_hi[0].descr = string("All");
  v_hi[1].descr = string("In Bx Window");
  plotLumi(v_hi[0], v_hi[1], "#Events per Lumi Section", runnum, saveplots);

  v_hi.clear();
  v_hi.resize(3);
  getOneHisto(file,v_hi[0],"run%dnPMThits",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMTsamesideHits",runnum);
  getOneHisto(file,v_hi[2],"run%dnPMTopposideHits",runnum);
  v_hi[0].descr = string("Total Individual hits");
  v_hi[1].descr = string("Same Side Pairings");
  v_hi[2].descr = string("Opposite Side Pairings");
  plotPMThistos(v_hi, "# PMT hits per event", runnum,"P",50.0);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMThitsSameSideDeltaIeta",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMThitsDeltaIeta",runnum);
  v_hi[0].descr = string("Same Side");
  v_hi[1].descr = string("Opposite Side");
  plotPMThistos(v_hi, "#Delta i#eta(wedge1,wedge2) PMT hit pairs", runnum, "",50.0);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMThitsSameSideDeltaIphi",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMThitsDeltaIphi",runnum);
  v_hi[0].descr = string("Same Side");
  v_hi[1].descr = string("Opposite Side");
  plotPMThistos(v_hi, "#Delta i#phi(wedge1,wedge2) PMT hit pairs", runnum,"", 50.0);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMThitsSameSideAvgIeta",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMThitsAvgIeta",runnum);
  v_hi[0].descr = string("Same Side");
  v_hi[1].descr = string("Opposite Side");
  plotPMThistos(v_hi, "Avg i#eta(wedge1,wedge2) PMT hit pairs", runnum, "",50.0);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMThitsSameSideAvgIphi",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMThitsAvgIphi",runnum);
  v_hi[0].descr = string("Same Side");
  v_hi[1].descr = string("Opposite Side");
  plotPMThistos(v_hi, "Avg i#phi(wedge1,wedge2) PMT hit pairs", runnum,"", 50.0);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMThitsPlus",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMThitsMinus",runnum);
  v_hi[0].descr = string("HF+");
  v_hi[1].descr = string("HF-");
  plotPMThistos(v_hi, "#PMT hits on each side", runnum,"");

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dePMThitsPlus",runnum);
  getOneHisto(file,v_hi[1],"run%dePMThitsMinus",runnum);
  v_hi[0].descr = string("HF+");
  v_hi[1].descr = string("HF-");
  plotPMThistos(v_hi, "PMT Energy on each side", runnum,"");
#endif

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dnPMTasym",runnum);
  getOneHisto(file,v_hi[1],"run%dnPMTasymBadBx",runnum);
  plotPMThistos(v_hi, "PMT hit # asymmetry",runnum,"");

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dePMTasym",runnum);
  getOneHisto(file,v_hi[1],"run%dePMTasymBadBx",runnum);
  plotPMThistos(v_hi, "PMT hit Energy asymmetry",runnum,"");

#if 0
  v_hi.clear();
  v_hi.resize(1);
  getOneHisto(file,v_hi[0],"run%dEperEtaBGmediumh",runnum);
  prettifyEtaPlot(v_hi[0]); // ,"c1",runnum,saveplots);

  v_hi.clear();
  v_hi.resize(3);
  getOneHisto(file,v_hi[0],"run%dnBGhitsLooseVsLS",runnum);
  getOneHisto(file,v_hi[1],"run%dnBGhitsMediumVsLS",runnum);
  getOneHisto(file,v_hi[2],"run%dnBGhitsTightVsLS",runnum);
  plotVecOnOneCanvas(v_hi,"Beam-GasHitsvsLumiSection", "BOX",1111,false,saveplots);

  v_hi.clear();
  v_hi.resize(3);
  getOneHisto(file,v_hi[0],"run%dEperEtaBGlooseh",runnum);
  getOneHisto(file,v_hi[1],"run%dEperEtaBGmediumh",runnum);
  getOneHisto(file,v_hi[2],"run%dEperEtaBGtighth",runnum);
  plotVecOnOneCanvas(v_hi,"Beam-GasHitsvsLumiSection", "",111111,false,saveplots);

  v_hi.clear();
  v_hi.resize(3);
  getOneHisto(file,v_hi[0],"run%dEspectrumBGlooseh",runnum);
  getOneHisto(file,v_hi[1],"run%dEspectrumBGmediumh",runnum);
  getOneHisto(file,v_hi[2],"run%dEspectrumBGtighth",runnum);
  plotVecOnOneCanvas(v_hi,"Beam-GasEnergySpectrum", "",111111,false,saveplots);

  v_hi.clear();
  v_hi.resize(3);
  getOneHisto(file,v_hi[0],"run%dEmaxBGlooseh",runnum);
  getOneHisto(file,v_hi[1],"run%dEmaxBGmediumh",runnum);
  getOneHisto(file,v_hi[2],"run%dEmaxBGtighth",runnum);
  plotVecOnOneCanvas(v_hi,"Beam-GasMaxTowerEnergy", "",111111,false,saveplots);

  v_hi.clear();
  v_hi.resize(3);
  getOneHisto(file,v_hi[0],"run%dEavgBGlooseh",runnum);
  getOneHisto(file,v_hi[1],"run%dEavgBGmediumh",runnum);
  getOneHisto(file,v_hi[2],"run%dEavgBGtighth",runnum);
  plotVecOnOneCanvas(v_hi,"Beam-GasTowerAverageEnergy", "",111111,false,saveplots);

  v_hi.clear();
  v_hi.resize(3);
  getOneHisto(file,v_hi[0],"run%dEweightedEtaBGlooseh",runnum);
  getOneHisto(file,v_hi[1],"run%dEweightedEtaBGmediumh",runnum);
  getOneHisto(file,v_hi[2],"run%dEweightedEtaBGtighth",runnum);
  plotVecOnOneCanvas(v_hi,"Beam-GasEnergyWeightedEta", "",111111,false,saveplots);

  v_hi.clear();
  v_hi.resize(3);
  getOneHisto(file,v_hi[0],"run%d2ndMomEweightedEtaBGlooseh",runnum);
  getOneHisto(file,v_hi[1],"run%d2ndMomEweightedEtaBGmediumh",runnum);
  getOneHisto(file,v_hi[2],"run%d2ndMomEweightedEtaBGtighth",runnum);
  plotVecOnOneCanvas(v_hi,"2ndMomentEnergyWeightedEta", "",111111,false,saveplots);

  v_hi.clear();
  v_hi.resize(2);
  getOneHisto(file,v_hi[0],"run%dEavgBGmediumh",runnum);
  getOneHisto(file,v_hi[1],"run%dEmaxBGmediumh",runnum);
  plotVecOnOneCanvas(v_hi,"Beam-GasTowerEnergy", "",1110,false,saveplots);
#endif
}
