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
#include <map>

using namespace std;

#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TString.h"
#include "TObjString.h"
#include "TRegexp.h"
#include "TCollection.h"
#include "TKey.h"
#include "TObjArray.h"
#include "TClass.h"
#include "TCanvas.h"
#include "TGaxis.h"
#include "TLatex.h"

#include "/afs/cern.ch/user/d/dudero/private/root/utils.C"
#include "/afs/cern.ch/user/d/dudero/private/root/MyHistoWrapper.cc"


typedef unsigned long uint32_t;

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

static map<string, string>      glmap_objpath2id;  // keep track of objects read in
static map<string, string>      glmap_id2objpath;  // keep track of objects read in
static map<string, wTH1 *>      glmap_id2histo;

static string nullstr;

const bool gl_verbose=true;

//======================================================================
// Regex match a histo name in a directory
//
void regexMatchHisto( TObject    *obj,
		      TDirectory *dir,
		      TObjArray  *Args,   // list of regexes to match
		      TObjArray  *Matches)
{
  for (int i=0; i<Args->GetEntriesFast(); i++) {
    TObjString *sre = (TObjString *)(*Args)[i];
    TRegexp re(sre->GetString(),kFALSE);
    if (re.Status() != TRegexp::kOK) {
      cerr << "The regexp " << sre->GetString() << " is invalid, Status() = ";
      cerr << re.Status() << endl;
      exit(-1);
    }

    TString path( (char*)strstr( dir->GetPath(), ":" ) );
    path.Remove( 0, 2 ); // gets rid of ":/"

    TString fullspec = TString(dir->GetPath()) + "/" + obj->GetName();

    if ((fullspec.Index(re) != kNPOS) &&
	(obj->InheritsFrom("TH1"))) {
      // we have a match
      // Check to see if it's already in memory
      map<string,string>::const_iterator it = glmap_objpath2id.find(dir->GetPath());
      if (it != glmap_objpath2id.end()) {
	if (gl_verbose)
	  cout << "Object " << fullspec << " already read in, here it is" << endl;
	map<string,wTH1 *>::const_iterator hit = glmap_id2histo.find(it->second);

	// Is this okay? It's going to get wrapped again...
	TObjString *rpath = new TObjString(fullspec);
	Matches->AddLast(rpath);
	Matches->AddLast(hit->second->histo());
      } else {
	// success, record that you read it in.
	TObjString *rpath = new TObjString(fullspec);
	Matches->AddLast(rpath);
	Matches->AddLast(obj);
      }
      break; // don't let the object match more than one regex
    } // if we have a match
  } // Arg loop
}                                                     // regexMatchHisto

//======================================================================

void recurseDirs( TDirectory *thisdir,
		  void (*doFunc)(TObject *, TDirectory *,TObjArray *, TObjArray *),
		  TObjArray *Args,
		  TObjArray *Output)
{
  assert(doFunc);

  //thisdir->cd();

  // loop over all keys in this directory

  TIter nextkey( thisdir->GetListOfKeys() );
  TKey *key;
  while ( (key = (TKey*)nextkey())) {

    TObject *obj = key->ReadObj();

    if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      // it's a subdirectory, recurse
      //cout << "Checking path: " << ((TDirectory *)obj)->GetPath() << endl;
      recurseDirs( (TDirectory *)obj, doFunc, Args, Output );
    } else {
      doFunc(obj, thisdir, Args, Output);
    }
  } // key loop
}                                                         // recurseDirs

//======================================================================

void getHistosFromRE(const string&   mhid,
		     TFile         * rootfile,
		     const string&   sre,
		     vector<std::pair<string,wTH1*> >&  v_wth1)
{
  // allow for multiple regexes in OR combination
  //
  vector<string> v_regexes;
  Tokenize(sre,v_regexes,"|");
  if (!v_regexes.size())
    v_regexes.push_back(sre);

  // Build validated TRegexp arguments in preparation for directory recursion
  //
  TObjArray *Args = new TObjArray();
  for (size_t i=0; i<v_regexes.size(); i++) {
    TRegexp re(v_regexes[i].c_str(),kTRUE);
    if (re.Status() != TRegexp::kOK) {
      cerr << "The regexp " << v_regexes[i] << " is invalid, Status() = ";
      cerr << re.Status() << endl;
      exit(-1);
    }
    else {
      Args->AddLast(new TObjString(v_regexes[i].c_str()));
    }
  }

  // Do the recursion, collect matches
  //
  TObjArray *Matches = new TObjArray();
  recurseDirs(rootfile, &regexMatchHisto, Args, Matches);
  Args->Delete();
  delete Args;

  // Returns two objects per match: 
  // 1. the (string) path that was matched and
  // 2. the object whose path matched
  //
  int nx2matches = Matches->GetEntriesFast();
  if (gl_verbose) cout << "... " << nx2matches/2 << " match(es) found.";

  // Add the matches to the global map of histos
  //
  int istart = v_wth1.size();

  for (int i=0; i<nx2matches; i+=2) {
    TString fullspec = ((TObjString *)(*Matches)[i])->GetString();
    wTH1 *wth1 = new wTH1((TH1 *)((*Matches)[i+1]));
    wth1->histo()->UseCurrentStyle();
    string hidi= mhid+"_"+int2str(istart+(i/2));
    v_wth1.push_back(std::pair<string,wTH1 *>(hidi,wth1));

    //glmap_objpath2id.insert(pair<string,string>(fullspec,hidi));
    glmap_id2histo.insert(pair<string,wTH1 *>(hidi,wth1));
    glmap_id2objpath.insert(pair<string,string>(hidi,string(fullspec.Data())));
  }

  //Matches->Delete(); // need the histos!
  delete Matches;

  if (gl_verbose) cout << endl;
}                                                     // getHistosFromRE

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

  TH1 *h1p = (TH1 *)file.p->Get(hname);
  hi.p     = h1p;

  if (!h1p) {
    cout << "\tcouldn't get " << hname;
    cout << " from " << file.name << endl;
    return false;
  }

  va_end (args);

  return true;
}                                                         // getOneHisto

//======================================================================

//static int phasebegin = 1048;
//static int phaseend   = 1100;
static int phasebegin = 1090;
static int phaseend   = 1130;
static int nomwinwid  = 8;

void scanphase(wTH1   *inhist,
	       int&    phasepos,  int&    phasewid,
	       double& minmetric, double& optavgs2overs1)
{

  // scan position first with nom. width
  minmetric = 9e99;
  cout << "xpos\tratio" << endl;
  for (int xpos=phasebegin; xpos <= phaseend-nomwinwid; xpos++) {
    int ibinbegin=inhist->histo()->FindFixBin(xpos);
    double sumvals=0,sumdeltavals=0;
    int numentries=0;
    for (int ibin=ibinbegin; ibin < ibinbegin+nomwinwid; ibin++) {
      double binc = inhist->histo()->GetBinContent(ibin);
      int    binn = ((TProfile *)inhist->histo())->GetBinEntries(ibin);
      sumvals    += binn*binc;
      numentries += binn;
      if (ibin!= ibinbegin)
	sumdeltavals += fabs(inhist->histo()->GetBinContent(ibin+1)-binc);
    }
    if (sumvals==0.) continue;
    double avgs2overs1 = sumvals/numentries;
    if (avgs2overs1 > 3.) continue;

    double ratio = sumdeltavals/sumvals;
    if (ratio < minmetric) {
      minmetric = ratio;
      phasepos = xpos;
      optavgs2overs1 = avgs2overs1;
    }

    printf("%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\n",
	   xpos,ibinbegin,ibinbegin+nomwinwid,numentries,
	   sumdeltavals,sumvals,ratio,avgs2overs1);
  }

  // scan width with optimized position
  minmetric = 9e99;
  cout << "xpos\txwid\tratio" << endl;
  for (int xwidinc=0; xwidinc <= 10; xwidinc++) {
    int xpos=phasepos-(xwidinc/2);
    int ibinbegin=inhist->histo()->FindFixBin(xpos);
    if (ibinbegin < 1) break;
    double sumvals=0,sumdeltavals=0;
    int numentries=0;
    for (int ibin=ibinbegin; ibin < ibinbegin+nomwinwid+xwidinc; ibin++) {
      double binc = inhist->histo()->GetBinContent(ibin);
      int    binn = ((TProfile *)inhist->histo())->GetBinEntries(ibin);
      sumvals    += binn*binc;
      numentries += binn;
      if (ibin!= ibinbegin)
	sumdeltavals += fabs(inhist->histo()->GetBinContent(ibin+1)-
			     inhist->histo()->GetBinContent(ibin));
    }
    if (sumvals==0.) continue;
    double avgs2overs1 = sumvals/numentries;
    if (avgs2overs1 > 3.) continue;
    double ratio = sumdeltavals/sumvals;
    if (ratio < minmetric) {
      minmetric = ratio;
      phasewid = nomwinwid+xwidinc;
      optavgs2overs1 = avgs2overs1;
    }

    printf("%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\n",
	   xpos,ibinbegin,ibinbegin+nomwinwid+xwidinc,numentries,
	   sumdeltavals,sumvals,ratio,avgs2overs1);
  }
}                                                           // scanphase

//======================================================================


void plot(TProfile *histo,
	  int phasepos,
	  int phasewid,
	  bool notfound,
	  const HistInfo_t& tdchi)
{
  //gPad->SetLogy(1);
  gPad->SetGridy(1);

  histo->GetXaxis()->SetRangeUser(1040,1140);
  histo->GetXaxis()->SetNdivisions(505);
  histo->GetXaxis()->SetLabelSize(.06);
  histo->GetXaxis()->SetTitleSize(.06);
  histo->GetXaxis()->SetTitleOffset(.8);
  histo->GetXaxis()->SetTitle("Laser TDC phase");

  histo->GetYaxis()->SetRangeUser(.1,5);
  histo->GetYaxis()->SetNdivisions(505);
  histo->GetYaxis()->SetLabelSize(.06);
  histo->GetYaxis()->SetTitleSize(.06);
  histo->GetYaxis()->SetTitleOffset(.75);
  histo->GetYaxis()->SetTitle("S2/S1");
  histo->SetStats(0);
  histo->Draw();

  gPad->Update();

  // Draw tdc phase dist on alternate y axis:

  //scale second set of histos to the pad coordinates
  Float_t rightmin = tdchi.p->GetMinimum();
  Float_t rightmax = 1.1*tdchi.p->GetMaximum();
  Float_t scale    = gPad->GetUymax()/rightmax;

  TH1 *scaled=(TH1 *)tdchi.p->Clone();

  scaled->Scale(scale);
  scaled->Draw("same");
   
  //draw an axis on the right side
  TGaxis *axis = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(),
			    gPad->GetUxmax(), gPad->GetUymax(),
			    rightmin,rightmax,505,"+L");
  axis->Draw();
  gPad->Update();

  // Draw Window

  TLine *left, *right;

  if (notfound) {
    // "X" it out
    left  = new TLine(1040,.1,1140,5.);
    right = new TLine(1040,5.,1140,.1);
  } else {
    left  = new TLine(phasepos,.1,phasepos,5);
    right = new TLine(phasepos+phasewid,.1,phasepos+phasewid,5);
  }
  left->SetLineColor(kRed);
  right->SetLineColor(kRed);
  left->Draw("same");
  right->Draw("same");
}                                                                // plot

//======================================================================

void scanphases(const char* rootfile, int runnum)
{
  FileInfo_t file;
  file.name = string(rootfile);
  file.p    = new TFile(rootfile);

  if (file.p->IsZombie()) {
    cerr << "File failed to open, " << file.name << endl;
    return;
  }

  vector<std::pair<string, wTH1 *> > v_histos;
  
  getHistosFromRE("phases",file.p,"/hfraddam/cutNone/_S2overS1perID/.*", v_histos);

  if (!v_histos.size()) exit(-1);

  HistInfo_t tdchi;

  getOneHisto(file, tdchi, "/hfraddam/cutNone/TDClaserFireTime");
  assert (tdchi.p);

  int ican=0;
  int ipad=0;

  TCanvas *c=NULL;
  TPad *motherpad=NULL;

  TString cname;

  for (size_t i=0; i<v_histos.size(); i++,ipad++) {
    int phasepos=-1, phasewid=-1;
    double minmetric=9e99;
    double avgs2overs1=9e99;
    scanphase(v_histos[i].second,phasepos,phasewid,minmetric,avgs2overs1);

    cout <<v_histos[i].second->histo()->GetName()<<" Optimal position, width = ";

    bool notfound = (avgs2overs1>3) || (minmetric > 1e-04);
    if (notfound)
      cout<< "Not found."<<endl;
    else
      cout<<phasepos<<", "<<phasewid<< "; minmetric, avgs2/s1 = "<<minmetric<<", "<<avgs2overs1<<endl;

    if (!(ipad%28))
    {

      if (c) c->SaveAs(TString(rootfile)+"_"+cname+".png");

      cname = Form("c%d",++ican);
      c = new TCanvas(cname,cname,180*7,180*4);

      assert(c);

      c->SetFillColor(10);

      motherpad = new TPad(Form("mother%d",ican),"",0.,0.,1.,0.96);
      motherpad->SetFillColor(10);
      motherpad->Draw();
      motherpad->cd();

      motherpad->Divide(7,4);
      ipad=0;

      c->cd();

      TLatex *ltx = new TLatex();
      ltx->SetNDC();
      ltx->SetText(.45,.97,Form("Run # %d",runnum));
      ltx->SetTextSize(.03);
      ltx->Draw();

      c->Update();
    }

    assert(motherpad);

    motherpad->cd(ipad+1);

    plot((TProfile *)(v_histos[i].second->histo()),phasepos,phasewid,notfound,tdchi);

    c->Update();
  }
  c->SaveAs(TString(rootfile)+"_"+cname+".png");
  
}
