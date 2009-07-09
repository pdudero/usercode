
#include <iostream>
using namespace std;

#include "TH1.h"
#include "TFile.h"
#include "TLegend.h"

#define NUM_CUTS 6

void plotOne(TFile *sigrootf, TFile *bgrootf, const char *histname) 
{
  TH1D *sighist = 0, *bghist = 0;
  char s1[50];

  // Remove yellow border and off-white color
  //
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetPadColor(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetOptStat(1110);

  c1 = new TCanvas(histname,histname,800,600);
  c1->Divide(2,NUM_CUTS-4);

  // gPad->SetLogy(1);

  c1->SetFillColor(10);

  int j;
  for (int i=0; i<(NUM_CUTS-1)*2; i++) {
    switch (i) {
    case 0:
    case 1: // j=i; break;
    case 2:
    case 3: continue;  // skip HLT cut
    case 4: 
    case 5: continue;  // skip 2nd ET cut
    default: j=i-6; break;
    }

    sprintf (s1, "%s%d", histname, i);

    // Check for existence of signal histogram
    //
    sigrootf->GetObject(s1, sighist);
    
    if (!sighist) {
      cerr << "Signal histogram " << s1 << " not found." << endl;
      return;
    }

    // Check for existence of background histogram
    //
    bgrootf->GetObject(s1, bghist);

    if (!bghist) {
      cerr << "Background histogram " << s1 << " not found." << endl;
      return;
    }
    
    c1->cd(j+1);

    TLegend *leg = new TLegend(0.7,0.7,0.9,0.9);
    leg->SetFillColor(10);

    // cout << "Sig integral is = " << sighist->Integral() << endl;

    sighist->Scale(1.0/sighist->Integral());
    //  sighist->GetYaxis()->SetRangeUser(0.0, 0.4);

    sighist->SetTitle(histname);

    sighist->GetXaxis()->CenterTitle(true);
    sighist->GetXaxis()->SetLabelSize(.05);
    sighist->GetXaxis()->SetTitleSize(.05);
    sighist->GetXaxis()->SetTitleOffset(0.9);

    sighist->GetYaxis()->SetLabelSize(.05);
    sighist->GetYaxis()->SetTitleSize(.05);
  
    sighist->SetLineColor(kRed);
    sighist->SetStats(0);          // turn off stats box

    leg->AddEntry(sighist,"Signal","l");

    // cout << "BG integral is = " << bghist->Integral() << endl;
    bghist->Scale(1.0/bghist->Integral());
    //  bghist->GetYaxis()->SetRangeUser(0.0, 0.4);

    bghist->GetXaxis()->CenterTitle(true);
    bghist->GetXaxis()->SetLabelSize(.05);
    bghist->GetXaxis()->SetTitleSize(.05);
    bghist->GetXaxis()->SetTitleOffset(0.9);

    bghist->GetYaxis()->SetLabelSize(.05);
    bghist->GetYaxis()->SetTitleSize(.05);

    bghist->SetLineColor(kBlue);
    bghist->SetStats(0);          // turn off stats box

    leg->AddEntry(bghist,"Background","l");
    
    bghist->Draw();
    sighist->Draw("same");

    leg->Draw();
  }
  c1->cd();
}

//======================================================================

void plotMasses(TFile *sigrootf, TFile *bgrootf, const char *whist, const char *thist )
{
  TH1D *whistsig = 0, *whistbg = 0;
  TH1D *thistsig = 0, *thistbg = 0;
  char s1[50];

  // Remove yellow border and off-white color
  //
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetPadColor(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetFrameBorderMode(0);

  gStyle->SetOptStat(0);

  c1 = new TCanvas("Masses","Masses",1000,400);
  c1->Divide(2,1);

  // gPad->SetLogy(1);

  c1->SetFillColor(10);

  // Check for existence of signal histogram
  //
  sigrootf->GetObject(whist, whistsig);
    
  if (!whistsig) {
      cerr << "Signal histogram " << whist << " not found." << endl;
      return;
    }

  // Check for existence of background histogram
  //
  bgrootf->GetObject(whist, whistbg);

  if (!whistbg) {
    cerr << "Background histogram " << whist << " not found." << endl;
    return;
  }

  // Check for existence of signal histogram
  //
  sigrootf->GetObject(thist, thistsig);
    
  if (!thistsig) {
      cerr << "Signal histogram " << thist << " not found." << endl;
      return;
    }

  // Check for existence of background histogram
  //
  bgrootf->GetObject(thist, thistbg);

  if (!thistbg) {
    cerr << "Background histogram " << thist << " not found." << endl;
    return;
  }
    
  c1->cd(1);


  gPad->SetTopMargin(0.03);
  gPad->SetRightMargin(0.03);

  TLegend *leg = new TLegend(0.7,0.7,0.9,0.9);
  leg->SetFillColor(10);
  leg->SetX1(.7);  leg->SetY1(.5);
  leg->SetX2(.95); leg->SetY2(.75);

  whistsig->Scale(1.0/whistsig->Integral());
  //  whistsig->GetYaxis()->SetRangeUser(0.0, 0.4);

  whistsig->SetTitle();

  whistsig->GetXaxis()->SetTitle("W Mass, GeV");
  whistsig->GetXaxis()->CenterTitle(true);
  whistsig->GetXaxis()->SetLabelSize(.05);
  whistsig->GetXaxis()->SetTitleSize(.05);
  whistsig->GetXaxis()->SetTitleOffset(0.9);

  whistsig->GetYaxis()->SetLabelSize(.05);
  whistsig->GetYaxis()->SetTitleSize(.05);
  
  whistsig->SetLineColor(kRed);
  whistsig->SetStats(0);          // turn off stats box

  leg->AddEntry(whistsig,"Signal","l");
  
  whistbg->Scale(1.0/whistbg->Integral());
  //  whistbg->GetYaxis()->SetRangeUser(0.0, 0.4);

  whistbg->SetTitle();

  whistbg->GetXaxis()->CenterTitle(true);
  whistbg->GetXaxis()->SetLabelSize(.05);
  whistbg->GetXaxis()->SetTitleSize(.05);
  whistbg->GetXaxis()->SetTitleOffset(0.9);

  whistbg->GetYaxis()->SetLabelSize(.05);
  whistbg->GetYaxis()->SetTitleSize(.05);

  whistbg->SetLineColor(kBlue);
  whistbg->SetStats(0);          // turn off stats box

  leg->AddEntry(whistbg,"Background","l");
    
  whistsig->Draw();
  whistbg->Draw("same");

  leg->Draw();
    
  c1->cd(2);

  gPad->SetTopMargin(0.03);
  gPad->SetRightMargin(0.03);

  TLegend *leg2 = new TLegend(0.7,0.7,0.9,0.9);
  leg2->SetFillColor(10);

  leg2->SetFillColor(10);
  leg2->SetX1(.7);  leg2->SetY1(.5);
  leg2->SetX2(.95); leg2->SetY2(.75);

  thistsig->Scale(1.0/thistsig->Integral());
  //  thistsig->GetYaxis()->SetRangeUser(0.0, 0.4);

  thistsig->SetTitle();

  thistsig->GetXaxis()->SetTitle("Top Mass, GeV");
  thistsig->GetXaxis()->CenterTitle(true);
  thistsig->GetXaxis()->SetLabelSize(.05);
  thistsig->GetXaxis()->SetTitleSize(.05);
  thistsig->GetXaxis()->SetTitleOffset(0.9);

  thistsig->GetYaxis()->SetLabelSize(.05);
  thistsig->GetYaxis()->SetTitleSize(.05);
  
  thistsig->SetLineColor(kRed);
  thistsig->SetStats(0);          // turn off stats box

  leg2->AddEntry(thistsig,"Signal","l");
  
  thistbg->Scale(1.0/thistbg->Integral());
  //  thistbg->GetYaxis()->SetRangeUser(0.0, 0.4);

  thistbg->SetTitle();

  thistbg->GetXaxis()->CenterTitle(true);
  thistbg->GetXaxis()->SetLabelSize(.05);
  thistbg->GetXaxis()->SetTitleSize(.05);
  thistbg->GetXaxis()->SetTitleOffset(0.9);

  thistbg->GetYaxis()->SetLabelSize(.05);
  thistbg->GetYaxis()->SetTitleSize(.05);

  thistbg->SetLineColor(kBlue);
  thistbg->SetStats(0);          // turn off stats box

  leg2->AddEntry(thistbg,"Background","l");
    
  thistsig->Draw();
  thistbg->Draw("same");

  leg2->Draw();

  c1->cd();
}

//======================================================================

static const int cutnumber = 8;

// From Pythia
static const double signal_cross_section_pb  = 0.15712 + 0.70302;
static const double bckgrnd_cross_section_pb =  919.86 + 5964.2;
static const double integrated_luminosity_inverse_pb = 1E4;

static const double scale_factor =
  signal_cross_section_pb*sqrt(integrated_luminosity_inverse_pb)/sqrt(bckgrnd_cross_section_pb);


void plotSignificance(TFile *sigrootf, TFile *bgrootf, const char *histname)
{
  TH1D *sighist1=0, *sighist2=0, *bghist1=0, *bghist2=0;
  char name[50], title[50];

  sprintf (name, "%s%d", histname, cutnumber);

  // Check for existence of signal histogram
  //
  sigrootf->GetObject(name, sighist1);
    
  if (!sighist1) {
    cerr << "Signal histogram " << name << " not found." << endl;
    return;
  }

  // Check for existence of background histogram
  //
  bgrootf->GetObject(name, bghist1);

  if (!bghist1) {
    cerr << "Signal histogram " << name << " not found." << endl;
    return;
  }

  sprintf (name, "%s%d", histname, cutnumber+1);

  // Check for existence of signal histogram
  //
  sigrootf->GetObject(name, sighist2);
    
  if (!sighist2) {
    cerr << "Signal histogram " << name << " not found." << endl;
    return;
  }

  // Check for existence of background histogram
  //
  bgrootf->GetObject(name, bghist2);

  if (!bghist2) {
    cerr << "Signal histogram " << name << " not found." << endl;
    return;
  }

  int nbins;
  double sigtotal, bgtotal;
  if (cutnumber != 0) {
    TH1D *sigh1=0, *sigh2=0, *bgh1=0, *bgh2=0;
    sprintf (name, "%s0", histname);

    // Check for existence of signal histogram
    //
    sigrootf->GetObject(name, sigh1);
    
    if (!sigh1) {
      cerr << "Signal histogram " << name << " not found." << endl;
      return;
    }

    // Check for existence of background histogram
    //
    bgrootf->GetObject(name, bgh1);

    if (!bgh1) {
      cerr << "Signal histogram " << name << " not found." << endl;
      return;
    }

    sprintf (name, "%s1", histname);

    // Check for existence of signal histogram
    //
    sigrootf->GetObject(name, sigh2);
    
    if (!sigh2) {
      cerr << "Signal histogram " << name << " not found." << endl;
      return;
    }

    // Check for existence of background histogram
    //
    bgrootf->GetObject(name, bgh2);
    
    if (!bgh2) {
      cerr << "Signal histogram " << name << " not found." << endl;
      return;
    }

    nbins = sigh1->GetNbinsX();
    assert(nbins == sigh2->GetNbinsX());
    assert(nbins == bgh1->GetNbinsX());
    assert(nbins == bgh2->GetNbinsX());

    sigtotal = sigh1->Integral(1,nbins+1) + sigh2->Integral(1,nbins+1);
    bgtotal  = bgh1->Integral(1,nbins+1) + bgh2->Integral(1,nbins+1);
  }
  else {
    nbins = sighist1->GetNbinsX();
    assert(nbins == sighist2->GetNbinsX());
    assert(nbins == bghist1->GetNbinsX());
    assert(nbins == bghist2->GetNbinsX());

    sigtotal = sighist1->Integral(1,nbins+1) + sighist2->Integral(1,nbins+1);
    bgtotal  = bghist1->Integral(1,nbins+1) + bghist2->Integral(1,nbins+1);
  }


  double sigintegral = 0.0;
  double bgintegral  = 0.0;
  double significance;
  double xmax = 0.0;

  sprintf (name, "signifh%d", cutnumber);
  sprintf (title, "Significance, %s", histname); 
  h1d_significance = new TH1D(name, title, nbins+1,
			      sighist1->GetXaxis()->GetXmin(),
			      sighist1->GetXaxis()->GetXmax());

  // Include the overflow bin
  for (int i=nbins+1; i> 0; i--) {
    sigintegral += sighist1->GetBinContent(i)+sighist2->GetBinContent(i);
    bgintegral  +=  bghist1->GetBinContent(i)+bghist2->GetBinContent(i);

    if (bgintegral != 0) {
      if (xmax==0.0) xmax = sighist1->GetXaxis()->GetBinLowEdge(i);

      significance = sigintegral/sqrt(bgintegral);

      significance *= sqrt(bgtotal);
      significance /= sigtotal;
      significance *= scale_factor;

      h1d_significance->SetBinContent(i,significance);
#if 0
      printf("%d sigint=%6.2f bgint=%6.2f snce=%5.3f\n",
	     i, sigintegral, bgintegral,significance);
#endif
    }
  }

  // Remove yellow border and off-white color
  //
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetPadColor(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetFrameBorderMode(0);

  c1 = new TCanvas(name,name,600,400);

  // gPad->SetLogy(1);

  c1->SetFillColor(10);

  gPad->SetTopMargin(0.13);
  gPad->SetRightMargin(0.05);

  h1d_significance->SetXTitle("GeV");
  h1d_significance->GetXaxis()->CenterTitle(true);
  h1d_significance->GetXaxis()->SetRangeUser(0.0, xmax);
  h1d_significance->SetStats(0);
  h1d_significance->Draw();

  double maxsignif = h1d_significance->GetMaximum();
  double minsignif = h1d_significance->GetMinimum();
  double maxsignifx = h1d_significance->GetXaxis()->GetBinCenter(h1d_significance->GetMaximumBin());

  double newscalefactor = 3.0/maxsignif;

  cout << "scale_factor = " << scale_factor << endl;
  cout << "Need es/root(eb) to be = " << 3.0/scale_factor << endl;
  cout << "   (or ~" << newscalefactor << " bigger than currently)" << endl;

  //draw a line at the peak
  TGaxis *axis = new TGaxis(maxsignifx,0.96*minsignif,maxsignifx,
                            1.025*maxsignif,0,0,0,"+L");
  axis->SetLineColor(kRed);
  axis->Draw();

}

void plotSigvsBG(const char* sigrootfname, const char *bgrootfname)
{
  char s[50];

#if 0
  gStyle->SetStatX(.97);
  gStyle->SetStatY(.97);
#endif

  TFile *sigrootf = new TFile(sigrootfname);
  TFile *bgrootf  = new TFile(bgrootfname);

  if (sigrootf->IsZombie()) {
    cerr << "File failed to open, " << sigrootfname << endl;
    return;
  }

  if (bgrootf->IsZombie()) {
    cerr << "File failed to open, " << bgrootfname << endl;
    return;
  }

  plotSignificance(sigrootf, bgrootf, "HT+METh");

#if 1

  plotOne(sigrootf, bgrootf, "rTe+bjetdPhih");
  plotOne(sigrootf, bgrootf, "HT+METh");
#else
  plotMasses(sigrootf, bgrootf, "rWmassh6", "rTmassh6");
  plotOne(sigrootf, bgrootf, "e+bjetmassh");

  plotOne(sigrootf, bgrootf, "HTh");
  plotOne(sigrootf, bgrootf, "METh");
  plotOne(sigrootf, bgrootf, "metsigh");

  plotOne(sigrootf, bgrootf, "rWmassh");
  plotOne(sigrootf, bgrootf, "rTmassh");
  plotOne(sigrootf, bgrootf, "rTopEth");
  plotOne(sigrootf, bgrootf, "rTopPth");
  plotOne(sigrootf, bgrootf, "rTedRh");
  plotOne(sigrootf, bgrootf, "rTedPhih");
  plotOne(sigrootf, bgrootf, "rmassChi2h");
  plotOne(sigrootf, bgrootf, "rTboosth");
  plotOne(sigrootf, bgrootf, "eEThist");
  plotOne(sigrootf, bgrootf, "ljEThist");
#endif
}
