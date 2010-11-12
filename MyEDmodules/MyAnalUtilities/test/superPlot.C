#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm> // min,max
#include <iostream>
#include <sstream>
#include <ctype.h>     // isdigit
#include <stdlib.h>
#include <glob.h>

using namespace std;

#include "MyHistoWrapper.cc"
#include "inSet.hh"
#include "TFile.h"
#include "TROOT.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TCanvas.h"

struct wPad_t {
  wPad_t(string name) : topmargin(0.),bottommargin(0.),
			rightmargin(0.),leftmargin(0.),
			fillcolor(10),logx(0),logy(0),logz(0),
			legid("")
  { hframe = new wTH1(name,name,100,0.0,1.0); }
  wPad_t(const wPad_t& wp) {
    topmargin   = wp.topmargin;
    bottommargin= wp.bottommargin;
    rightmargin = wp.bottommargin;
    leftmargin  = wp.leftmargin;
    fillcolor   = wp.fillcolor;
    logx        = wp.logx;
    logy        = wp.logy;
    logz        = wp.logz;
    gridx       = wp.gridx;
    gridy       = wp.gridy;
    legid       = wp.legid;
    titlexndc   = wp.titlexndc;
    titleyndc   = wp.titleyndc;
    //hframe      = ;           // the frame histo, holds lots of pad info
  }
  float topmargin, bottommargin, rightmargin, leftmargin;
  unsigned fillcolor;
  unsigned logx, logy,logz;
  unsigned gridx, gridy;
  string legid;
  float titlexndc, titleyndc;
  wTH1 *hframe;           // the frame histo, holds lots of pad info
  vector<string> histo_ids;
  vector<string> stack_ids;
  vector<string> altyh_ids;
  vector<string> graph_ids;
  vector<string> macro_ids;
  vector<string> label_ids;
  vector<string> line_ids;
  vector<string> box_ids;
  TVirtualPad *vp;
};

struct wCanvas_t {
  wCanvas_t() {wCanvas_t("");}
  wCanvas_t(const string& intitle,
	    unsigned innpadsx=0, unsigned inpadxdim=600,
	    unsigned innpadsy=0, unsigned inpadydim=600,
	    float inpadxmarg=0.01, float inpadymarg=0.01,
	    float inleftmarg=0., float inrightmarg=0.,
	    float intopmarg=0.,  float inbottommarg=0.) :
    title(intitle),
    npadsx(innpadsx),npadsy(innpadsy),
    padxdim(inpadxdim),padydim(inpadydim),
    padxmargin(inpadxmarg),padymargin(inpadymarg),
    leftmargin(inleftmarg),rightmargin(inrightmarg),
    topmargin(intopmarg),bottommargin(inbottommarg),
    optstat("nemr"), fillcolor(10), multipad(NULL) {}
  wCanvas_t(const wCanvas_t& wc) :
    title(wc.title), npadsx(wc.npadsx),npadsy(wc.npadsy),
    padxdim(wc.padxdim),padydim(wc.padydim),
    padxmargin(wc.padxmargin),padymargin(wc.padymargin),
    leftmargin(wc.leftmargin),rightmargin(wc.rightmargin),
    topmargin(wc.topmargin),bottommargin(wc.bottommargin),
    optstat(wc.optstat), fillcolor(wc.fillcolor), multipad(NULL) {}
  string   style;
  string   title;
  unsigned npadsx;
  unsigned npadsy;
  unsigned padxdim;
  unsigned padydim;
  float    padxmargin;
  float    padymargin;
  float    leftmargin;    // These margins determine whether the motherpad
  float    rightmargin;   // overlays the canvas completely, or only partially
  float    topmargin;
  float    bottommargin;
  string   optstat;
  unsigned fillcolor;
  TPad    *motherpad;
  wPad_t  *multipad;
  vector<wPad_t *> pads;
  vector<string> latex_ids;
  vector<string> savenamefmts;
  TCanvas *c1;
};

struct canvasSet_t {
  canvasSet_t(const string& intitle, unsigned inncanvas=1):
    title(intitle), ncanvases(inncanvas) { canvases.clear(); }
  string   title;
  unsigned ncanvases;
  vector<wCanvas_t *> canvases;
};

static map<string, string>      glmap_objpaths2id;  // keep track of objects read in
static map<string, wTH1 *>      glmap_id2histo;
static map<string, TFile *>     glmap_id2rootfile;

static string nullstr;

#include "spUtils.C"
#include "spAlias.C"
#include "spSample.C"
#include "spGraph.C"
#include "spMacro.C"
#include "spStyle.C"
#include "spTF1.C"
#include "spHisto.C"
#include "spHmath.C"
#include "spLabelLatex.C"
#include "spLayout.C"
#include "spLegend.C"
#include "spLine.C"
#include "spBox.C"
#include "spMultiHist.C"
#include "spPad.C"
#include "spDraw.C"

//======================================================================

void parseCanvasLayout(const string& layoutFile,
		       canvasSet_t&  cs)
{
  cout << "Processing " << layoutFile << endl;

  FILE *fp = fopen(layoutFile.c_str(),"r");
  if (!fp) {
    cerr << "Error, couldn't open " << layoutFile << endl;
    exit(-1);
  }

  // Usually only need one canvas. Any other canvases will be
  // initialized with the parameters picked up for the first one here.
  //
  wCanvas_t *wc = new wCanvas_t(cs.title+"_1");
  cs.canvases.push_back(wc);

  bool   new_section = false;
  string section("");
  string theline;
  vector<string> v_tokens;
  bool keepgoing = getLine(fp,theline,"layoutintro");
  while (keepgoing) {
    if (!theline.size()) continue;
    if (theline[0] == '[') {
      Tokenize(theline,v_tokens," []");
      section = v_tokens[0];
    }

    if      (!section.size()) continue;
    if      (section == "STYLE")  processStyleSection(fp,theline,new_section);
    else if (section == "LAYOUT") processLayoutSection(fp,theline,cs,new_section);

    else if (section == "PAD") {
      string padname = string("pad")+int2str((int)(wc->pads.size()+1))+string("frame");
      wPad_t *wpad = new wPad_t(padname);
      processPadSection(fp,theline,wpad,new_section);
      wc->pads.push_back(wpad);
    }
    else if (section == "MULTIPAD") {
      // Store info for all pads in the 'multipad' member
      if (wc->multipad) {
	cerr << "Currently only one MULTIPAD section can be defined, sorry." << endl;
	continue;
      }
      wc->multipad = new wPad_t("multipad");
      processPadSection(fp,theline,wc->multipad,new_section);
      // Have to read in multihist before assigning histos to pads, so pended to drawPlots
    }
    else if (section == "HISTO")     processHistoSection    (fp,theline,new_section);
    else if (section == "MULTIHIST") processMultiHistSection(fp,theline,new_section);
    else if (section == "HMATH")     processHmathSection    (fp,theline,new_section);
    else if (section == "GRAPH")     processGraphSection    (fp,theline,new_section);
    else if (section == "LEGEND")    processLegendSection   (fp,theline,new_section);
    else if (section == "LABEL")     processLabelSection    (fp,theline,new_section);
    else if (section == "LATEX")     processLatexSection    (fp,theline,new_section);
    else if (section == "LINE")      processLineSection     (fp,theline,new_section);
    else if (section == "BOX")       processBoxSection      (fp,theline,new_section);
    else if (section == "ALIAS")     processAliasSection    (fp,theline,new_section);
    else if (section == "SAMPLE")    processSampleSection   (fp,theline,new_section);
    else if (section == "TF1")       processTF1Section      (fp,theline,new_section);
    else if (section == "MACRO")     processMacroSection    (fp,theline,new_section);
    else {
      cerr << "Unknown section " << section << " in " << layoutFile << endl;
    }

    if (new_section) {
      new_section = false;
      keepgoing = true;
    } else 
      keepgoing = getLine(fp,theline,"layoutelse");
  } // while (getline...
}                                                   // parseCanvasLayout

//======================================================================

canvasSet_t *initCanvasSet(const string& cLayoutFile)
{
  canvasSet_t *cs = new canvasSet_t(stripDirsAndSuffix(cLayoutFile));

  parseCanvasLayout(cLayoutFile, *cs);

  // Set Styles:
  gStyle->SetPalette(1,0); // always!

  wCanvas_t *wc = cs->canvases[0];
  unsigned npadsmin =  wc->npadsx*wc->npadsy;

  if (npadsmin) {
    wc->c1 = new TCanvas(wc->title.c_str(),wc->title.c_str(),
			 wc->padxdim*wc->npadsx,
			 wc->padydim*wc->npadsy);

    float left = wc->leftmargin;
    float bot  = wc->bottommargin;
    float rhgt = 1-wc->rightmargin;
    float top  = 1-wc->topmargin;
    wc->motherpad = new TPad("mother","",left,bot,rhgt,top);
    wc->c1->SetFillColor(wc->fillcolor);
    wc->motherpad->SetFillColor(wc->fillcolor);
    wc->motherpad->Draw();
    wc->motherpad->cd();

    wc->motherpad->Divide(wc->npadsx,wc->npadsy);
  			     // , wc->padxmargin,wc->padymargin);

    cout << "Canvas " << cLayoutFile << " dimensions "
	 << wc->npadsx << "x" << wc->npadsy << endl;
    cout << "Canvas " << cLayoutFile << " margins "
	 << wc->padxmargin << "x" << wc->padymargin << endl;

  }
  return cs;
}                                                       // initCanvasSet

//======================================================================

void superPlot(const string& canvasLayout="canvas.txt",
	       bool savePlot2file=false,
	       bool dotdrStyle=false)
{
  glmap_objpaths2id.clear();
  glmap_aliii.clear();
  glmap_id2graph.clear();
  glmap_id2histo.clear();
  glmap_id2label.clear();
  glmap_id2latex.clear();
  glmap_id2legend.clear();
  glmap_id2line.clear();
  glmap_id2box.clear();
  glmap_id2rootfile.clear();
  glmap_id2style.clear();
  glmap_id2tf1.clear();

  setPRDStyle();
  //gROOT->ForceStyle();

  if (dotdrStyle) {
    setTDRStyle();
    gROOT->ForceStyle();
    tdrStyle->SetOptTitle(1);
    tdrStyle->SetTitleFont(42);
    tdrStyle->SetTitleFontSize(0.05);
  // tdrStyle->SetTitleBorderSize(2);

  }

  drawPlots(*initCanvasSet(canvasLayout),savePlot2file);
}
