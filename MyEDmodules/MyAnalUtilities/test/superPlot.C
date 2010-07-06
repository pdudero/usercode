#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm> // min,max
#include <iostream>
#include <sstream>
#include <ctype.h>     // isdigit
#include <stdlib.h>

using namespace std;

#include "MyHistoWrapper.cc"
#include "inSet.hh"
#include "TFile.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "THStack.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TPaveStats.h"
#include "TGraph.h"
#include "TGraphAsymmErrors.h"
#include "TGaxis.h"
#include "TVectorD.h"
#include "TArrayD.h"
#include "mystyle.C"
#include "tdrstyle4timing.C"
#include "drawStandardTexts.C"

#define LINELEN 512

struct wPad_t {
  wPad_t(string name) : topmargin(0.),bottommargin(0.),
			rightmargin(0.),leftmargin(0.),
			fillcolor(10),logx(0),logy(0),logz(0),
			stackem(false),legid("")
  { hframe = new wTH1(name,name,100,0.0,1.0); }
  float topmargin, bottommargin, rightmargin, leftmargin;
  unsigned fillcolor;
  unsigned logx, logy,logz;
  unsigned gridx, gridy;
  bool   stackem;
  string legid;
  float titlexndc, titleyndc;
  wTH1 *hframe;           // the frame histo, holds lots of pad info
  vector<string> histo_ids;
  vector<string> altyh_ids;
  vector<string> graph_ids;
  vector<string> label_ids;
  vector<string> line_ids;
  TVirtualPad *vp;
};

struct wCanvas_t {
  wCanvas_t(const string& intitle,
	    unsigned innpadsx=1, unsigned inpadxdim=600,
	    unsigned innpadsy=1, unsigned inpadydim=600,
	    float inpadxmarg=0.01, float inpadymarg=0.01) :
    title(intitle), npadsx(innpadsx),npadsy(innpadsy),
    padxdim(inpadxdim),padydim(inpadydim),
    padxmargin(inpadxmarg),padymargin(inpadymarg),
    optstat("nemr"), fillcolor(10) {}
  string   style;
  string   title;
  unsigned npadsx;
  unsigned npadsy;
  unsigned padxdim;
  unsigned padydim;
  float    padxmargin;
  float    padymargin;
  string   optstat;
  unsigned fillcolor;
  vector<wPad_t *> pads;
  TCanvas *c1;
};

struct wLegend_t {
  wLegend_t(const string& inhdr,
	    float inx1ndc=0., float iny1ndc=0.,
	    float inx2ndc=1., float iny2ndc=1.,
	    unsigned infillclr=10) :
    header(inhdr),
    x1ndc(inx1ndc),y1ndc(iny1ndc), x2ndc(inx2ndc),y2ndc(iny2ndc),
    fillcolor(infillclr),bordersize(1),ncolumns(1),linewidth(1) {}
  string   header;
  float    x1ndc, y1ndc;
  float    x2ndc, y2ndc;
  unsigned fillcolor;
  int      bordersize;
  int      ncolumns;
  unsigned linewidth;
  unsigned textfont;
  float    textsize;
  string   drawoption;
  TLegend *leg;
};

struct wLabel_t {
  wLabel_t(const string& intxt,
	   float inx1ndc=0., float iny1ndc=0.,
	   float inx2ndc=1., float iny2ndc=1.,
	   float intxtsz=0.035) :
    text(intxt),
    x1ndc(inx1ndc),y1ndc(iny1ndc), x2ndc(inx2ndc),y2ndc(iny2ndc), textsize(intxtsz) {}
  string   text;
  float    x1ndc, y1ndc;
  float    x2ndc, y2ndc;
  float    textsize;
  unsigned textfont;
};

static set<string> glset_graphFilesReadIn;  // keep track of graphs read in

static map<string, string>      glmap_objpaths2id;  // keep track of objects read in
static map<string, string>      glmap_aliii;        // better than aliases
static map<string, wTH1 *>      glmap_id2histo;
static map<string, TGraph *>    glmap_id2graph;
static map<string, TLine *>     glmap_id2line;
static map<string, wLegend_t *> glmap_id2legend;    // the map...of legends
static map<string, wLabel_t *>  glmap_id2label;
static map<string, TFile *>     glmap_id2rootfile;
static map<string, TF1 *>       glmap_id2tf1;

static string nullstr;

//======================================================================

inline unsigned int str2int(const string& str) {
  return (unsigned int)strtoul(str.c_str(),NULL,0);
}

inline float str2flt(const string& str) {
  return (float)strtod(str.c_str(),NULL);
}

inline string int2str(int i) {
  ostringstream ss;
  ss << i;
  return ss.str();
}

//======================================================================
// Got this from
// http://www.velocityreviews.com/forums/t286357-case-insensitive-stringfind.html
//
bool ci_equal(char ch1, char ch2)
{
  return (toupper((unsigned char)ch1) ==
          toupper((unsigned char)ch2));
}

size_t ci_find(const string& str1, const string& str2)
{
  string::const_iterator pos = search(str1.begin(), str1.end(),
				      str2.begin(), str2.end(), ci_equal);
  if (pos == str1.end())
    return string::npos;
  else
    return (pos-str1.begin());
}

//======================================================================
// Got this from
// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html

void Tokenize(const string& str,
	      vector<string>& tokens,
	      const string& delimiters = " ",
	      bool include_delimiters=false)
{
  tokens.clear();

  // Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  if (include_delimiters && lastPos>0)
    tokens.push_back(str.substr(0,lastPos));

  // Find first "non-delimiter".
  string::size_type pos = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos) {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));

    lastPos = str.find_first_not_of(delimiters, pos);

    if (include_delimiters && pos!=string::npos) {
      tokens.push_back(str.substr(pos, lastPos-pos));
    } //else skip delimiters.

    // Find next delimiter
    pos = str.find_first_of(delimiters, lastPos);
  }
}                                                            // Tokenize

//======================================================================

bool getLine(FILE *fp, string& theline, const string& callerid="")
{ 
  char linein[LINELEN];

  if (!feof(fp) && fgets(linein,LINELEN,fp)) {
    if (strlen(linein) && (linein[strlen(linein)-1] == '\n'))
      linein[strlen(linein)-1] = '\0';
    theline = string(linein);
  } else return false;

  //cout << theline <<  ", callerid = " << callerid << endl;

  return true;
}

//======================================================================

const string& extractAlias(const string& input)
{
  map<string,string>::const_iterator it;
  it = glmap_aliii.find(input);
  if (it == glmap_aliii.end()) {
    cerr << "alias " << input << " not found, define reference in ALIAS section first." << endl;
    return nullstr;
  }
  return it->second;
}

//======================================================================

void buildStringFromAliii(const string& input, 
			  const string& delimiters,
			  string& output)
{
  assert (delimiters.find('@') == string::npos);
  const bool include_delimiters = true;

  output.clear();
  vector<string> v_tokens;
  Tokenize(input,v_tokens, delimiters,include_delimiters);
  for (size_t i=0; i<v_tokens.size(); i++) {
    if (v_tokens[i][0] == '@')
      output += extractAlias(v_tokens[i].substr(1));
    else
      output += v_tokens[i];
  }
}                                                // buildStringFromAliii

//======================================================================

TH1 *findHisto(const string& hid, const std::string& errmsg="")
{
  map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid);
  if (it == glmap_id2histo.end()) {
    cerr << "Histo ID " << hid << " not found. " << errmsg << endl;
    return NULL;
  }
  return it->second->histo();
}

//======================================================================

TF1 *findTF1(const string& fid)
{
  cout << "looking for " << fid << endl;
  map<string,TF1 *>::const_iterator it = glmap_id2tf1.find(fid);
  if (it == glmap_id2tf1.end()) {
    cerr << "TF1 ID " << fid << " not found" << endl;
    return NULL;
  }
  return it->second;
}

//======================================================================

TH1 *IntegrateLeft(TH1 *h)
{
  string newname = string(h->GetName())+"_integleft";
  TH1 *hcum = (TH1 *)h->Clone(newname.c_str());
  hcum->Reset();

  int nbins = hcum->GetNbinsX();
  double htotal = h->Integral(1,nbins+1);

  // Include the overflow bin
  for (int i=1; i<=nbins+1 ; i++) { // includes overflow
    double integral = h->Integral(1,i);
    hcum->SetBinContent(i,integral/htotal);
  }

  return hcum;
}

//======================================================================

TH1 *IntegrateRight(TH1 *h,double skipbinatx=-9e99)
{
  string newname = string(h->GetName())+"_integright";
  TH1 *hcum = (TH1 *)h->Clone(newname.c_str());
  hcum->Reset();

  int nbins = hcum->GetNbinsX();
  double htotal = h->Integral(1,nbins+1);
  int   skipbin = nbins+2;
  float skipcontent = 0.0;

  if (skipbinatx > -9e99) {
    for (int i=1; i<=nbins+1 ; i++) { // includes overflow
      if ((h->GetXaxis()->GetBinLowEdge(i) < skipbinatx) &&
	  (h->GetXaxis()->GetBinUpEdge(i)  > skipbinatx)   ) {
	skipbin      = i;
	skipcontent  = h->GetBinContent(i);
	break;
      }
    }
  }

  // Include the overflow bin
  for (int i=1; i<=nbins+1 ; i++) { // includes overflow
    double integral = h->Integral(i,nbins+1);
    if (i > skipbin) skipcontent=0.0;
    hcum->SetBinContent(i,(integral-skipcontent)/htotal);
    //hcum->SetBinError(i,sqrt(integral-skipcontent)/htotal);
  }

  return hcum;
}

//======================================================================

void loadVectorsFromFile(const char *filename, 
			 TVectorD&   vx,
			 TVectorD&   vy)
{
  char linein[LINELEN];
  vector<double> v;

  FILE *fp = fopen(filename, "r");

  if (!fp) {
    cerr << "File failed to open, " << filename << endl;
    return;
  }

  cout << "Loading vectors from file " << filename;

  while (!feof(fp) && fgets(linein,LINELEN,fp)) {
    double x, y;
    if (sscanf(linein, "%lf %lf", &x, &y) != 2) {
      cerr << "scan failed, file " << filename << ", line = " << linein << endl;
      return;
    }
    else {
      v.push_back(x); v.push_back(y);
    }
  }

  int vecsize = v.size()/2;
  vx.ResizeTo(vecsize);
  vy.ResizeTo(vecsize);

  cout << "; read " << vecsize << " lines" << endl;

  for (int i=0; i<vecsize; i++) {
    vx[i] = v[2*i];
    vy[i] = v[2*i+1];
  }
}                                                 // loadVectorsFromFile

//======================================================================

void loadVectorsFromFile(const char *filename, 
			 TVectorD&   vx, TVectorD&   vy,
			 TVectorD&   exl,TVectorD&   exh,
			 TVectorD&   eyl,TVectorD&   eyh)
{
  char linein[LINELEN];
  vector<double> v;

  FILE *fp = fopen(filename, "r");

  if (!fp) {
    cerr << "File failed to open, " << filename << endl;
    return;
  }

  cout << "Loading vectors from file " << filename;

  while (!feof(fp) && fgets(linein,LINELEN,fp)) {
    double x, y, ymin,ymax;
    if (sscanf(linein, "%lf %lf %lf %lf", &x, &ymin, &ymax, &y) != 4) {
      cerr << "scan failed, file " << filename << ", line = " << linein << endl;
      return;
    }
    else {
      v.push_back(x); v.push_back(y);
      v.push_back(0); v.push_back(0);
      v.push_back(y-ymin); v.push_back(ymax-y);
    }
  }

  int vecsize = v.size()/6;
  vx.ResizeTo(vecsize);
  vy.ResizeTo(vecsize);
  exl.ResizeTo(vecsize);
  exh.ResizeTo(vecsize);
  eyl.ResizeTo(vecsize);
  eyh.ResizeTo(vecsize);

  cout << "; read " << vecsize << " lines" << endl;

  for (int i=0; i<vecsize; i++) {
    vx[i]  = v[6*i];
    vy[i]  = v[6*i+1];
    exl[i] = v[6*i+2];
    exh[i] = v[6*i+3];
    eyl[i] = v[6*i+4];
    eyh[i] = v[6*i+5];
  }
}                                                 // loadVectorsFromFile

//======================================================================

void printHisto2File(TH1 *histo, string filename)
{
  FILE *fp = fopen(filename.c_str(),"w");
  if (histo->InheritsFrom("TH3")) {
    TH3 *h3 = (TH3 *)histo;
    int totaln = h3->GetEntries();
    fprintf(fp,"#%s\t%s\t%s\tw\t%%\n",
	    h3->GetXaxis()->GetTitle(),
	    h3->GetYaxis()->GetTitle(),
	    h3->GetZaxis()->GetTitle());
    for (int ix=1; ix<=h3->GetNbinsX(); ix++)
      for (int iy=1; iy<=h3->GetNbinsY(); iy++)
	for (int iz=1; iz<=h3->GetNbinsZ(); iz++)
	  fprintf(fp,"%d\t%d\t%d\t%d\t%f\n",
		  (int)h3->GetXaxis()->GetBinCenter(ix),
		  (int)h3->GetYaxis()->GetBinCenter(iy),
		  (int)h3->GetZaxis()->GetBinCenter(iz),
		  (int)h3->GetBinContent(ix,iy,iz),
		  100.*h3->GetBinContent(ix,iy,iz)/(float)totaln);
    cout << "File " << filename << " written with contents of ";
    cout << h3->GetName() << endl;
  } else if (histo->InheritsFrom("TH2")) {
    TH2 *h2 = (TH2 *)histo;
    fprintf(fp,"#x\ty\tz\n");
    for (int ix=1; ix<=h2->GetNbinsX(); ix++)
      for (int iy=1; iy<=h2->GetNbinsY(); iy++)
	fprintf(fp,"%d\t%d\t%d\n",
		(int)h2->GetXaxis()->GetBinCenter(ix),
		(int)h2->GetYaxis()->GetBinCenter(iy),
		(int)h2->GetBinContent(ix,iy));
    cout << "File " << filename << " written with contents of ";
    cout << h2->GetName() << endl;
  } else {
    TH1 *h1 = (TH1 *)histo;
    fprintf(fp,"#x\ty\n");
    for (int ix=1; ix<=h1->GetNbinsX(); ix++)
      fprintf(fp,"%d\t%d\n",
	      (int)h1->GetBinCenter(ix),
	      (int)h1->GetBinContent(ix));
    cout << "File " << filename << " written with contents of ";
    cout << h1->GetName() << endl;
  }
  fclose(fp);
}

//======================================================================

bool                                          // returns true if success
processStyleSection(FILE *fp,string& theline, bool& new_section)
{
  vector<string> v_tokens;

  cout << "Processing style section" << endl;

  new_section=false;

  while (getLine(fp,theline,"style")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      return true;
    }
    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    if      (key == "style") {
      if (value == "TDR")
	setTDRStyle();
      else if (value == "Plain")
	gROOT->SetStyle("Plain");
    }
    else if (key == "optstat")  {
      cout << "OptStat = " << gStyle->GetOptStat() << endl;
      gStyle->SetOptStat(value.c_str());
      cout << "OptStat = " << gStyle->GetOptStat() << endl;
    }
    else if (key == "opttitle")  gStyle->SetOptTitle(str2int(value));

    // Set the position/size of the title box

    else if (key == "titlexndc") gStyle->SetTitleX(str2flt(value));
    else if (key == "titleyndc") gStyle->SetTitleY(str2flt(value));
    else if (key == "titlewndc") gStyle->SetTitleW(str2flt(value));
    else if (key == "titlehndc") gStyle->SetTitleH(str2flt(value));
    else if (key == "titlefont") gStyle->SetTitleFont(str2int(value));
    else if (key == "titlebordersize") gStyle->SetTitleBorderSize(str2int(value));

    // Set the position of the statbox
    else if (key == "statxndc")  gStyle->SetStatX(str2flt(value));
    else if (key == "statyndc")  gStyle->SetStatY(str2flt(value));
    else if (key == "statwndc")  gStyle->SetStatW(str2flt(value));
    else if (key == "stathndc")  gStyle->SetStatH(str2flt(value));

    else if (key == "painttextfmt") gStyle->SetPaintTextFormat(value.c_str());
    else if (key == "markersize") gStyle->SetMarkerSize(str2flt(value));
    else {
      cerr << "Unknown key " << key << endl;
    }

  } // while loop

  return true;
}                                                 // processStyleSection

//======================================================================

bool                                          // returns true if success
processLayoutSection(FILE      *fp,
		     string&    theline,
		     wCanvas_t& wc,
		     bool&      new_section)
{
  vector<string> v_tokens;

  cout << "Processing layout section" << endl;

  new_section=false;

  while (getLine(fp,theline,"layout")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    if (key == "npadsx") {
      unsigned long npadsx = str2int(value);
      if (npadsx > 1) wc.npadsx = npadsx;
    }
    else if (key == "npadsy") {
      unsigned long npadsy = str2int(value);
      if (npadsy > 1) wc.npadsy = npadsy;
    }
    else if (key == "padxdim") {
      unsigned long padxdim = str2int(value);
      if (padxdim > 0) wc.padxdim = padxdim;

    }
    else if (key == "padydim") {
      unsigned long padydim = str2int(value);
      if (padydim > 0) wc.padydim = padydim;
    }
    else if (key == "padxmargin") {
      float padxmargin = str2flt(value);
      if (padxmargin > 0.) wc.padxmargin = padxmargin;

    }
    else if (key == "padymargin") {
      float padymargin = str2flt(value);
      if (padymargin > 0.) wc.padymargin = padymargin;
    }
    else if (key == "fillcolor")
      wc.fillcolor = str2int(value);
    else {
      cerr << "Unknown key " << key << endl;
    }

  } // while loop

  return true;
}                                                // processLayoutSection

//======================================================================

bool                                        // returns true if success
processPadSection(FILE *fp,string& theline, wPad_t * wpad, bool& new_section)
{
  vector<string> v_tokens;
  float xmin=1e99, xmax=-1e99;
  float ymin=1e99, ymax=-1e99;

  cout << "Processing pad section" << endl;

  new_section=false;

  while (getLine(fp,theline,"pad")) {

    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    if      (key == "histos") {
      Tokenize(value,wpad->histo_ids," ,"); 
      if (!wpad->histo_ids.size()) wpad->histo_ids.push_back(value);
    }
    else if (key == "altyhistos") { // histos on an alternate y-axis
      if (!wpad->histo_ids.size()) {
	cerr << "No 'histos' defined yet! Define alternates after them." << endl;
	continue;
      }
      Tokenize(value,wpad->altyh_ids," ,"); 
      if (!wpad->altyh_ids.size()) wpad->altyh_ids.push_back(value);
    }
    else if (key == "graphs") {
      Tokenize(value,wpad->graph_ids,","); 
      if (!wpad->graph_ids.size()) wpad->graph_ids.push_back(value);
    }
    else if (key == "lines") {
      Tokenize(value,wpad->line_ids,","); 
      if (!wpad->line_ids.size()) wpad->line_ids.push_back(value);
    }
    else if (key == "labels") {
      Tokenize(value,wpad->label_ids," ,"); 
      if (!wpad->label_ids.size())  wpad->label_ids.push_back(value);
    }
    else if (key == "title")        wpad->hframe->histo()->SetTitle(value.c_str());
    else if (key == "xtitle")       wpad->hframe->histo()->SetXTitle(value.c_str());
    else if (key == "ytitle")       wpad->hframe->histo()->SetYTitle(value.c_str());
    else if (key == "ztitle")       wpad->hframe->histo()->SetZTitle(value.c_str());
    else if (key == "rightmargin")  wpad->rightmargin  = str2flt(value);
    else if (key == "leftmargin")   wpad->leftmargin   = str2flt(value);
    else if (key == "topmargin")    wpad->topmargin    = str2flt(value);
    else if (key == "bottommargin") wpad->bottommargin = str2flt(value);
    else if (key == "fillcolor")    wpad->fillcolor    = str2int(value);
    else if (key == "legend")       wpad->legid        = value;
    else if (key == "logx")         wpad->logx         = str2int(value);
    else if (key == "logy")         wpad->logy         = str2int(value);
    else if (key == "logz")         wpad->logz         = str2int(value);
    else if (key == "gridx")        wpad->gridx        = str2int(value);
    else if (key == "gridy")        wpad->gridy        = str2int(value);
    else if (key == "stackem")      wpad->stackem      = (bool)str2int(value);
    else if (key == "xmin") {
      float xmin = str2flt(value);
      wpad->hframe->SetXaxis("",false,0,0,0,0,0,xmin,xmax);
    }
    else if (key == "xmax") {
      float xmax = str2flt(value);
      wpad->hframe->SetXaxis("",false,0,0,0,0,0,xmin,xmax);
    }
    else if (key == "ymin") {
      ymin = str2flt(value);
      wpad->hframe->SetYaxis("",false,0,0,0,0,0,ymin,ymax);
    }
    else if (key == "ymax") {
      ymax = str2flt(value);
      wpad->hframe->SetYaxis("",false,0,0,0,0,0,ymin,ymax);
    }
    else {
      cerr << "Unknown key " << key << endl;
    }


  } // while loop

  return true;
}                                                   // processPadSection

//======================================================================

void processCommonHistoParams(const string& key, 
			      const string& value,
			      wTH1& wh)
{
  static float xmin=1e99, xmax=-1e99;
  static float ymin=1e99, ymax=-1e99;
  static float zmin=1e99, zmax=-1e99;

  if      (key == "draw")        wh.SetDrawOption(value);
  else if (key == "title")       wh.histo()->SetTitle(value.c_str());

  else if (key == "markercolor") wh.SetMarker(str2int(value));
  else if (key == "markerstyle") wh.SetMarker(0,str2int(value));
  else if (key == "markersize")  wh.SetMarker(0,0,str2flt(value));
  else if (key == "linecolor")   wh.SetLine(str2int(value));
  else if (key == "linestyle")   wh.SetLine(0,str2int(value));
  else if (key == "linewidth")   wh.SetLine(0,0,str2int(value));
  else if (key == "fillcolor")   wh.histo()->SetFillColor(str2int(value));
  else if (key == "leglabel")    wh.SetLegendEntry(value);

  else if (key == "rebin")       ((TH1 *)wh.histo())->Rebin(str2int(value));

  else if (key == "rebinx") {
    if (wh.histo()->InheritsFrom("TH2")) ((TH2 *)wh.histo())->RebinX(str2int(value));
    else                                 ((TH1 *)wh.histo())->Rebin(str2int(value));
  }
  else if (key == "rebiny") {
    if (wh.histo()->InheritsFrom("TH2")) ((TH2 *)wh.histo())->RebinY(str2int(value));
    else {
      cerr << "\trebiny: not defined for 1-d histograms" << endl;
    }
  } 

  // axes
  else if (key == "xtitle")       wh.SetXaxis(value);
  else if (key == "ytitle")       wh.SetYaxis(value);
  else if (key == "ztitle")       wh.SetZaxis(value);
  else if (key == "xtitlesize")   wh.SetXaxis("",false,str2flt(value));
  else if (key == "ytitlesize")   wh.SetYaxis("",false,str2flt(value));
  else if (key == "ztitlesize")   wh.SetZaxis("",false,str2flt(value));
  else if (key == "xtitleoffset") wh.SetXaxis("",false,0,str2flt(value));
  else if (key == "ytitleoffset") wh.SetYaxis("",false,0,str2flt(value));
  else if (key == "ztitleoffset") wh.SetZaxis("",false,0,str2flt(value));
  else if (key == "xtitlefont")   wh.SetXaxis("",false,0,0,str2int(value));
  else if (key == "ytitlefont")   wh.SetYaxis("",false,0,0,str2int(value));
  else if (key == "ztitlefont")   wh.SetZaxis("",false,0,0,str2int(value));
  else if (key == "xlabelsize")   wh.SetXaxis("",false,0,0,0,str2flt(value));
  else if (key == "ylabelsize")   wh.SetYaxis("",false,0,0,0,str2flt(value));
  else if (key == "zlabelsize")   wh.SetZaxis("",false,0,0,0,str2flt(value));
  else if (key == "xlabelfont")   wh.SetXaxis("",false,0,0,0,0,str2int(value));
  else if (key == "ylabelfont")   wh.SetYaxis("",false,0,0,0,0,str2int(value));
  else if (key == "zlabelfont")   wh.SetZaxis("",false,0,0,0,0,str2int(value));
  else if (key == "xndiv")  wh.SetXaxis("",false,0,0,0,0,0,1e99,-1e99,str2int(value));
  else if (key == "yndiv")  wh.SetYaxis("",false,0,0,0,0,0,1e99,-1e99,str2int(value));
  else if (key == "zndiv")  wh.SetZaxis("",false,0,0,0,0,0,1e99,-1e99,str2int(value));

  else if (key == "xmin") {
    xmin = str2flt(value);
    wh.SetXaxis("",false,0,0,0,0,0,xmin,xmax);
  }
  else if (key == "xmax") {
    xmax = str2flt(value);
    wh.SetXaxis("",false,0,0,0,0,0,xmin,xmax);
  }
  else if (key == "ymin") {
    ymin = str2flt(value);
    wh.SetYaxis("",false,0,0,0,0,0,ymin,ymax);
  }
  else if (key == "ymax") {
    ymax = str2flt(value);
    wh.SetYaxis("",false,0,0,0,0,0,ymin,ymax);
  }
  else if (key == "zmin") {
    zmin = str2flt(value);
    wh.SetZaxis("",false,0,0,0,0,0,zmin,zmax);
  }
  else if (key == "zmax") {
    zmax = str2flt(value);
    wh.SetZaxis("",false,0,0,0,0,0,zmin,zmax);
  }

  else if (key == "xbinlabels") {
    vector<string> v_tokens;
    Tokenize(value,v_tokens,",");
    if (!v_tokens.size()) {
      cerr << "expect comma-separated list of bin labels ";
      cerr << value << endl;
    }
    cout << "Loaded " << v_tokens.size() << " x-axis bin labels" << endl;
    for (int ibin=1; ibin<=std::min((int)v_tokens.size(),wh.histo()->GetNbinsX()); ibin++)
      wh.histo()->GetXaxis()->SetBinLabel(ibin,v_tokens[ibin-1].c_str());
  }

  else if (key == "ybinlabels") {
    vector<string> v_tokens;
    Tokenize(value,v_tokens,",");
    if (!v_tokens.size()) {
      cerr << "expect comma-separated list of bin labels ";
      cerr << value << endl;
    }
    for (int ibin=1; ibin<=std::min((int)v_tokens.size(),wh.histo()->GetNbinsY()); ibin++)
      wh.histo()->GetYaxis()->SetBinLabel(ibin,v_tokens[ibin-1].c_str());
  }

  // stats box
  else if (key == "statson")    wh.SetStats(str2int(value) != 0);
  else if (key == "statsx1ndc") wh.SetStats(wh.statsAreOn(),str2flt(value));
  else if (key == "statsy1ndc") wh.SetStats(wh.statsAreOn(),0.0,str2flt(value));
  else if (key == "statsx2ndc") wh.SetStats(wh.statsAreOn(),0.0,0.0,str2flt(value));
  else if (key == "statsy2ndc") wh.SetStats(wh.statsAreOn(),0.0,0.0,0.0,str2flt(value));

  else if (key == "erroroption") {
    // only for TProfiles.
    // allowed options are
    // ' ' (default, e=spread/sqrt(n)
    // 's' (e=spread)
    // others -> see ROOT documentation!
    //
    if (wh.histo()->InheritsFrom("TProfile")) {
      ((TProfile *)wh.histo())->SetErrorOption(value.c_str());
    }
  }
  else if (key == "print2file") {
    printHisto2File(wh.histo(),value);
  }
  else if ((key == "normalize") &&
	   str2int(value)) {
    TH1 *h1 = (TH1 *)wh.histo();
    if (h1->Integral() > 0.0)
      h1->Scale(1./h1->Integral());
    else
      cerr << h1->GetName() << " integral is ZERO, cannot normalize." << endl;
  }
  else if (key == "fits") {
    vector<string> v_tokens;
    Tokenize(value,v_tokens,","); 
    for (size_t i=0; i<v_tokens.size(); i++) {
      TF1 *tf1 = findTF1(v_tokens[i]);
      if (!tf1) {
	cerr << "TF1 " << v_tokens[i] << "must be defined first" << endl;
      }
      wh.loadFitFunction(tf1);
    }
  }
  else {
    cerr << "unknown key " << key << endl;
  }
}                                            // processCommonHistoParams

//======================================================================

wTH1 *getHistoFromSpec(const string& hid,
		       const string& spec)
{
  wTH1  *wth1     = NULL;
  TFile *rootfile = NULL;
  vector<string> v_tokens;
  string fullspec;     // potentially expanded from aliases.

  cout << "processing " << spec << endl;

  Tokenize(spec,v_tokens,":");
  if ((v_tokens.size() != 2) ||
      (!v_tokens[0].size())  ||
      (!v_tokens[1].size())    ) {
    cerr << "malformed root histo path file:folder/subfolder/.../histo " << spec << endl;
    return NULL;
  }

  // Check for file alias
  string rootfn = v_tokens[0];
  if (rootfn[0] == '@') {  // reference to an alias defined in ALIAS section
    rootfn = extractAlias(rootfn.substr(1));
    if (!rootfn.size()) return NULL;
  }

  // Check for histo alias, which can consist of multiple aliii...
  //
  string hspec = v_tokens[1];
  if (hspec.find('@') != string::npos) {
    string temp=hspec;
    buildStringFromAliii(temp,"/",hspec);
    if (!hspec.size()) return NULL;
  }

  fullspec = rootfn + ":" + hspec;

  map<string,string>::const_iterator it = glmap_objpaths2id.find(fullspec);
  if (it != glmap_objpaths2id.end()) {
    // Allow the possibility to run the script a second time in root
    cout << "Object " << fullspec << " already read in, here it is" << endl;
    map<string,wTH1 *>::const_iterator hit = glmap_id2histo.find(it->second);
    if (hit == glmap_id2histo.end()) {
      cout << "oops, sorry, I lied." << endl;
      return NULL;
    }
    wth1 = hit->second;
  } else {
    // Now check to see if this file has already been opened...
    map<string,TFile*>::const_iterator it = glmap_id2rootfile.find(rootfn);
    if (it != glmap_id2rootfile.end())
      rootfile = it->second;
    else
      rootfile = new TFile(rootfn.c_str());

    if (rootfile->IsZombie()) {
      cerr << "File failed to open, " << rootfn << endl;
    } else {
      glmap_id2rootfile.insert(pair<string,TFile*>(rootfn,rootfile));
      TH1 *h1 = (TH1 *)rootfile->Get(hspec.c_str());
      if (!h1) {
	cerr << "couldn't find " << hspec << " in " << rootfn << endl;
      } else {
	// success, record that you read it in.
	cerr << "Found " << fullspec << endl;
	glmap_objpaths2id.insert(pair<string,string>(fullspec,hid));
	wth1 = new wTH1(h1);
	glmap_id2histo.insert(pair<string,wTH1 *>(hid,wth1));
      }
    }
  }
  return wth1;
}                                                    // getHistoFromSpec

//======================================================================

TGraph *getGraphFromSpec(const string& gid,
			 const string& spec)
{
  TGraph  *gr     = NULL;
  TFile *rootfile = NULL;
  vector<string> v_tokens;
  string fullspec;     // potentially expanded from aliases.

  cout << "processing " << spec << endl;

  Tokenize(spec,v_tokens,":");
  if ((v_tokens.size() != 2) ||
      (!v_tokens[0].size())  ||
      (!v_tokens[1].size())    ) {
    cerr << "malformed root graph path file:folder/subfolder/.../graph " << spec << endl;
    return NULL;
  }

  // Check for file alias
  string rootfn = v_tokens[0];
  if (rootfn[0] == '@') {  // reference to an alias defined in ALIAS section
    rootfn = extractAlias(rootfn.substr(1));
    if (!rootfn.size()) return NULL;
  }

  // Check for graph alias
  string gspec = v_tokens[1];
  if (gspec.find('@') != string::npos) {
    string temp=gspec;
    buildStringFromAliii(temp,"/",gspec);
    if (!gspec.size()) return NULL;
  }

  fullspec = rootfn + gspec;

  map<string,string>::const_iterator it = glmap_objpaths2id.find(fullspec);
  if (it != glmap_objpaths2id.end()) {
    // Allow the possibility to run the script a second time in root
    cout << "Object " << fullspec << " already read in, here it is" << endl;
    map<string,TGraph *>::const_iterator git = glmap_id2graph.find(it->second);
    if (git == glmap_id2graph.end()) {
      cout << "oops, sorry, I lied." << endl;
      return NULL;
    }
    gr = git->second;
  } else {
    // Now check to see if this file has already been opened...
    map<string,TFile*>::const_iterator it = glmap_id2rootfile.find(rootfn);
    if (it != glmap_id2rootfile.end())
      rootfile = it->second;
    else
      rootfile = new TFile(rootfn.c_str());

    if (rootfile->IsZombie()) {
      cerr << "File failed to open, " << rootfn << endl;
    } else {
      glmap_id2rootfile.insert(pair<string,TFile*>(rootfn,rootfile));
      gr = (TGraph *)rootfile->Get(gspec.c_str());
      if (!gr) {
	cerr << "couldn't find " << gspec << " in " << rootfn << endl;
      } else {
	// success, record that you read it in.
	glmap_objpaths2id.insert(pair<string,string>(fullspec,gid));
	glmap_id2graph.insert(pair<string,TGraph *>(gid,gr));
      }
    }
  }
  return gr;
}                                                    // getGraphFromSpec

//======================================================================

bool                              // returns true if success
processHistoSection(FILE *fp,
		    string& theline,
		    bool& new_section)
{
  vector<string> v_tokens;

  wTH1   *wth1 = NULL;
  string *hid  = NULL;

  cout << "Processing histo section" << endl;

  new_section=false;

  while (getLine(fp,theline,"histo")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    //--------------------
    if (key == "id") {
    //--------------------
      if (hid != NULL) {
	cerr << "no more than one id per histo section allowed " << value << endl;
	break;
      }

      hid = new string(value);

    //------------------------------
    } else if (key == "clone") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(value);
      if (it == glmap_id2histo.end()) {
	cerr << "Histo ID " << value << " not found,";
	cerr << "clone must be defined after the clonee" << endl;
	break;
      }
      wth1 = it->second->Clone(string(it->second->histo()->GetName())+"_clone",
			       string(it->second->histo()->GetTitle()));
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wth1));

    //------------------------------
    } else if (key == "path") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (wth1) {
	cerr << "histo already defined" << endl; continue;
      }
      wth1  = getHistoFromSpec(*hid,value);
      if (!wth1) continue;

    //------------------------------
    } else if (key == "hprop") {    // fill a histogram with a per-bin property of another
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (wth1) {
	cerr << "histo already defined" << endl; continue;
      }

      // Tokenize again to get the histo ID and the desired property
      string hprop = value;
      Tokenize(hprop,v_tokens,":");
      if ((v_tokens.size() != 2) ||
	  (!v_tokens[0].size())  ||
	  (!v_tokens[1].size())    ) {
	cerr << "malformed hid:property specification " << hprop << endl; continue;
      }

      string tgthid = v_tokens[0];
      string prop   = v_tokens[1];

      map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(tgthid);
      if (it == glmap_id2histo.end()) {
	cerr << "Histo ID " << tgthid << " not found, histo must be defined first" << endl;
	break;
      }
      TH1 *tgth1 = it->second->histo();
      wth1       = it->second->Clone(string(tgth1->GetName())+"_"+prop,
				     string(tgth1->GetTitle())+" ("+prop+")");
      TH1 *h1=wth1->histo();
      h1->Clear();
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wth1));
      
      if (!prop.compare("errors")) {
	int nbins = h1->GetNbinsX()*h1->GetNbinsY()*h1->GetNbinsZ();
	for (int ibin=1; ibin<=nbins; ibin++)
	  h1->SetBinContent(ibin,tgth1->GetBinError(ibin));
      }
      else {
	cerr << "Unrecognized property: " << prop << endl;
	break;
      }

    } else if (!wth1) {  // all other keys must have "path" defined
      cerr << "key 'path' or 'clone' must be defined before key " << key << endl;
      break;
    }

    else {
      processCommonHistoParams(key,value,*wth1);
    }
  }
  return (wth1 != NULL);
}                                                 // processHistoSection

//======================================================================

bool                              // returns true if success
processTF1Section(FILE *fp,
		  string& theline,
		  bool& new_section)
{
  string *fid  = NULL;
  string *form = NULL;
  TF1    *f1   = NULL;
  double xmin=0.0, xmax=0.0;
  vector<string> parstrs;

  cout << "Processing TF1 section" << endl;

  new_section=false;

  while (getLine(fp,theline,"TF1")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      break;
    }

    vector<string> v_tokens;
    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() != 2) ||
	(!v_tokens[0].size())  ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key   = v_tokens[0];
    string value = v_tokens[1];

    //--------------------
    if (key == "id") {
    //--------------------
      if (fid != NULL) {
	cerr << "no more than one id per F1 section allowed " << value << endl;
	break;
      }
      fid = new string(value);
      
      map<string, TF1 *>::const_iterator fit = glmap_id2tf1.find(*fid);
      if (fit != glmap_id2tf1.end()) {
	cerr << "Function id " << *fid << " already defined" << endl;
	break;
      }

    //------------------------------
    } else if (key == "formula") {
    //------------------------------
      if (!fid) {
	cerr << "id key must be defined before formula key" << endl; continue;
      }
      if (form) {
	cerr << "Formula for " << *fid << " already defined" << endl; continue;
      }
      form = new string (value);

    //------------------------------
    }
    else if (key == "initpars") {
      if (!fid) {
	cerr << "id key must be defined before formula key" << endl; continue;
      }
      Tokenize(value,parstrs,",");
    }

    else if (key == "xmin") xmin = str2flt(value);
    else if (key == "xmax") xmax = str2flt(value);
  }

  if (fid && form && (xmax > xmin)) {
    f1 = new TF1(fid->c_str(),form->c_str(),xmin,xmax);
    for (size_t i=0; i<parstrs.size(); i++) {
      f1->SetParameter(i,str2flt(parstrs[i]));
    }
    glmap_id2tf1.insert(pair<string,TF1 *>(*fid,f1));
    //delete fid;
    //delete form;
  }

  if (fid) delete fid;
  return (f1 != NULL);
}                                                   // processTF1section

//======================================================================

bool                              // returns true if success
processGraphSection(FILE *fp,
		    string& theline,
		    bool& new_section)
{
  vector<string> v_tokens;

  string *gid = NULL;
  TGraph *gr  = NULL;
  float xoffset=0.0,yoffset=0.0, yscale=1.0;
  int  lcolor=1,lstyle=1,lwidth=1;
  int  mcolor=1,mstyle=3,msize=1;
  int  yndiv=510;
  TVectorD vx,vy,exl,exh,eyl,eyh;
  bool asymerrors = false;

  cout << "Processing graph section" << endl;

  new_section=false;

  while (getLine(fp,theline,"graph")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      break;
    }

    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    //--------------------
    if (key == "id") {
    //--------------------
      if (gid != NULL) {
	cerr << "no more than one id per graph section allowed " << value << endl;
	break;
      }

      gid = new string(value);

    //------------------------------
    } else if (key == "vectorfile") {
    //------------------------------
      if (!gid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      string path = value;
      if (gr) {
	cerr << "graph already defined" << endl; continue;
      }
      if (inSet<string>(glset_graphFilesReadIn,path)) {
	cerr << "vector file " << path << " already read in" << endl; break;
      }
      if (path.find('@') != string::npos) {
	string temp=path;
	buildStringFromAliii(temp,"/",path);
	if (!path.size()) continue;
      }

      if (asymerrors)
	loadVectorsFromFile(path.c_str(),vx,vy,exl,exh,eyl,eyh);
      else
	loadVectorsFromFile(path.c_str(),vx,vy);

    //------------------------------
    } else if (key == "path") {
    //------------------------------

      if (!gid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (gr) {
	cerr << "graph already defined" << endl; continue;
      }
      gr  = getGraphFromSpec(*gid,value);
      if (!gr) continue;

    } else {
      // "gr" is not defined yet!
      if      (key == "xoffset")     xoffset = str2flt(value);
      else if (key == "yoffset")     yoffset = str2flt(value);
      else if (key == "yscale")      yscale  = str2flt(value);
      else if (key == "linecolor")   lcolor  = str2int(value);
      else if (key == "linestyle")   lstyle  = str2int(value);
      else if (key == "linewidth")   lwidth  = str2int(value);
      else if (key == "markercolor") mcolor  = str2int(value);
      else if (key == "markerstyle") mstyle  = str2int(value);
      else if (key == "markersize")  msize   = str2int(value);
      else if (key == "asymerrors")  asymerrors = (bool)str2int(value);
      else if (key == "yndiv")       yndiv   = str2int(value);
      else {
	cerr << "unknown key " << key << endl;
      }
#if 0
      processCommonHistoParams(key,value,*wh);
#endif
    }
  }

  if (vx.GetNoElements()) { // load utility guarantees the same size for both
    if (yscale  != 1.0) vy *= yscale;
    if (xoffset != 0.0) vx += xoffset;
    if (yoffset != 0.0) vy += yoffset;
    if (asymerrors) 
      gr = new TGraphAsymmErrors(vx,vy,exl,exh,eyl,eyh);
    else
      gr = new TGraph(vx,vy);

    gr->SetLineStyle(lstyle);
    gr->SetLineColor(lcolor);
    gr->SetLineWidth(lwidth);
    gr->SetMarkerColor(mcolor);
    gr->SetMarkerStyle(mstyle);
    gr->SetMarkerSize(msize);
    gr->GetYaxis()->SetNdivisions(yndiv);

    glmap_id2graph.insert(pair<string,TGraph *>(*gid,gr));
  }

  return (gr != NULL);
}                                                 // processGraphSection

//======================================================================

bool                              // returns true if success
processLineSection(FILE *fp,
		   string& theline, // the text line, that is
		   bool& new_section)
{
  vector<string> v_tokens;

  string *lid = NULL;
  TLine *line = NULL;

  cout << "Processing line section" << endl;

  new_section=false;

  while (getLine(fp,theline,"graph")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      break;
    }

    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    //--------------------
    if (key == "id") {
    //--------------------
      if (lid != NULL) {
	cerr << "no more than one id per line section allowed " << value << endl;
	break;
      }

      lid = new string(value);

    //------------------------------
    } else if (key == "x1x2y1y2") {
    //------------------------------
      if (!lid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }

      if (line) {
	cerr << "line already defined" << endl; continue;
      }

      Tokenize(value,v_tokens,",");

      if (v_tokens.size() != 4) {
	cerr << "expecting four coordinates x1,x2,y1,y2: " << theline << endl; continue;
      }

      float x1=str2flt(v_tokens[0]);
      float x2=str2flt(v_tokens[1]);
      float y1=str2flt(v_tokens[2]);
      float y2=str2flt(v_tokens[3]);

      line = new TLine(x1,y1,x2,y2);

    } else if (!line) {
      cerr << "key x1x2y1y2 must appear before this key: " << key << endl; continue;
    } else {
      if      (key == "linecolor")   line->SetLineColor(str2int(value));
      else if (key == "linestyle")   line->SetLineStyle(str2int(value));
      else if (key == "linewidth")   line->SetLineWidth(str2int(value));
      else {
	cerr << "unknown key " << key << endl;
      }
    }
  }

  if (line)
    glmap_id2line.insert(pair<string,TLine *>(*lid,line));

  return (line != NULL);
}                                                  // processLineSection

//======================================================================

bool                              // returns true if success
processHmathSection(FILE *fp,
		    string& theline,
		    bool& new_section)
{
  vector<string> v_tokens;
  vector<TH1*>   v_histos2dize;

  wTH1  *wh  = NULL;
  string *hid = NULL;
  TH1   *h1 = NULL;
  float skipbinatx=-9e99;

  cout << "Processing hmath section" << endl;

  new_section=false;

  while (getLine(fp,theline,"hmath")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    //--------------------
    if (key == "id") {
    //--------------------
      if (hid != NULL) {
	cerr << "no more than one id per hmath section allowed " << value << endl;
	break;
      }

      hid = new string(value);

    //------------------------------
    } else if (key == "unaryop") {  // unary operation /+-* with a constant
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string hmathexpr = value;

      Tokenize(hmathexpr,v_tokens,"-+*/");
      if (v_tokens.size() != 2) {
	cerr << "only simple math ops -+*/ supported between two operands, sorry!";
	cerr << theline << endl;
	continue;
      }

      TH1 *histop = 0;
      double dummynum=0.0;
      TF1 *f1 = 0;

      string& arg1=v_tokens[0];
      string& arg2=v_tokens[1];
      if (arg1[0] == '@') {
	arg1= extractAlias(arg1.substr(1));
	if (!arg1.size()) continue;
      }
      if (arg2[0] == '@') {
	arg2= extractAlias(arg2.substr(1));
	if (!arg2.size()) continue;
      }

      TH1 *hres;

      histop = findHisto(arg1,"Checking:is this a histo?");
      if (!histop) {	// trying scanning a double
	if(!sscanf(arg1.c_str(),"%lg",&dummynum)) {
	  cerr << arg1 << ": it's not a known histo, and it's not a number.";
	  cerr << "I'm outta here." << endl;
	  continue;
	} else {
	  // arg1 is a constant, so
	  // arg2 must be a histogram
	  histop = findHisto(arg2,"Checking:is this a histo?");
	  if (!histop) continue;
	  if (theline.find('-') != string::npos) {
	    // the histo operand is being negated
	    //
	    // WARNING: not handling negative constants
	    //
	    f1 = new TF1("minus1","-1",
			 histop->GetXaxis()->GetXmin(),
			 histop->GetXaxis()->GetXmax());
	    histop->Multiply(f1);
	    string newname=string(histop->GetName())+"_"+arg1+"-this";
	    hres = (TH1 *)histop->Clone(newname.c_str());
	    f1 = new TF1("someconst",arg1.c_str(),
			 histop->GetXaxis()->GetXmin(),
			 histop->GetXaxis()->GetXmax());
	    hres->Add(f1);
	  }
	}
      } else {
	// arg1 is a histo, so
	// arg2 must be a number
	cout << "made it here..." << endl;
	if(!sscanf(arg2.c_str(),"%lg",&dummynum)) {
	  cerr << arg2 << " must be a number, since " << arg1 << " is a histogram. ";
	  cerr << "Use 'binaryop' otherwise." << endl;
	  continue;
	}
	f1 = new TF1("myfunc",arg2.c_str(),
		     histop->GetXaxis()->GetXmin(),
		     histop->GetXaxis()->GetXmax());
	if (theline.find('-') != string::npos) {
	  string newname=string(histop->GetName())+"_-"+arg1;
	  hres = (TH1 *)histop->Clone(newname.c_str());
	  hres->Add(f1,-1.0);
	} else {
	  string newname=string(histop->GetName())+"_?"+arg1;
	  hres = (TH1 *)histop->Clone(newname.c_str());
	}
      }

      if      (theline.find('*') != string::npos) hres->Multiply(f1);
      //else if (theline.find('/') != string::npos) hres->Divide(h1,h2,1.0,1.0,"C");
      else if (theline.find('/') != string::npos) hres->Divide(f1);
      else if (theline.find('+') != string::npos) hres->Add(f1);

      h1 = hres;
      wh = new wTH1(h1);
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key == "binaryop") {  // binary operation +-*/
    //------------------------------

      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string hmathexpr = value;

      Tokenize(hmathexpr,v_tokens,"-+*/");
      if (v_tokens.size() != 2) {
	cerr << "only simple math ops -+*/ supported between two operands, sorry!" << theline << endl;
	continue;
      }

      TH1 *tmph1 = findHisto(v_tokens[0]); if (!tmph1) continue;
      TH1 *tmph2 = findHisto(v_tokens[1]); if (!tmph2) continue;
      TH1 *hres = (TH1 *)tmph1->Clone();
      //hres->Reset();

      if      (theline.find('-') != string::npos) hres->Add(tmph2,-1.0);
      else if (theline.find('+') != string::npos) hres->Add(tmph2);
      else if (theline.find('*') != string::npos) hres->Multiply(tmph2);
      //else if (theline.find('/') != string::npos) hres->Divide(tmph1,tmph2,1.0,1.0,"C");
      else if (theline.find('/') != string::npos) hres->Divide(tmph2);

      h1 = (TH1 *)hres;
      wh = new wTH1(h1);
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key.find("sum") != string::npos) {
    //------------------------------
      Tokenize(value,v_tokens,",");
      if (v_tokens.size() < 2) {
	cerr << "expect comma-separated list of at least two histo specs to sum!" << theline << endl;
	continue;
      }
      TH1 *firstone = (TH1 *)findHisto(v_tokens[0]);
      if (!firstone) exit(-1);
      h1 = (TH1 *)firstone->Clone();
      if (key == "weightsum")
	h1->SetBit(TH1::kIsAverage);  // <========== Addends also have to have this set.
      for (unsigned i=1; i<v_tokens.size(); i++) {
	TH1 *addend = (TH1 *)findHisto(v_tokens[i]);
	if (!addend) exit(-1);
	h1->Add(addend,1.0);
      }
      wh = new wTH1(h1);
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key == "integright") {  // sweep from low-high x-bins and integrate to the right
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      TH1 *tmph1 = findHisto(value,"histo operand must be defined before math ops");
      if (!tmph1) exit(-1);
      h1 = (TH1 *)IntegrateRight(tmph1,skipbinatx);
      wh = new wTH1(h1);
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key == "integleft") {  // sweep from low-high x-bins and integrate to the left
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      TH1 *tmph1 = findHisto(value,"histo operand must be defined before math ops");
      if (!tmph1) exit(-1);
      h1 = (TH1 *)IntegrateLeft(tmph1);
      wh = new wTH1(h1);
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    } else if (key == "skipbinatx")
      skipbinatx = str2flt(value);

    //------------------------------
    else if (key == "projectx") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string binspec = value; // range of bins to project

      // binspec of form
      //
      Tokenize(binspec,v_tokens,":");
      if (v_tokens.size() != 2) {
	cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
	cerr << theline << endl;
	continue;
      }
      TH2 *tmph2 = (TH2 *)findHisto(v_tokens[0],"histo operand must be defined before math ops");
      if (!tmph2) exit(-1);
      binspec = v_tokens[1];
      Tokenize(binspec,v_tokens,"-");
      if (v_tokens.size() != 2) {
	cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
	cerr << theline << endl;
	continue;
      }
      string newname = string(tmph2->GetName())+"_Ybins"+binspec;
      int lobin=str2int(v_tokens[0]);
      int hibin=str2int(v_tokens[1]);
      if (lobin > hibin) {
	cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
	cerr << theline << endl;
	continue;
      }
      h1 = (TH1 *)tmph2->ProjectionX(newname.c_str(),lobin,hibin);
      wh = new wTH1(h1);
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key == "projecty") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string binspec = value; // range of bins to project

      // binspec of form
      //
      Tokenize(binspec,v_tokens,":");
      if (v_tokens.size() != 2) {
	cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
	cerr << theline << endl;
	continue;
      }
      TH2 *tmph2 = (TH2 *)findHisto(v_tokens[0],"histo operand must be defined before math ops");
      if (!tmph2) exit(-1);
      binspec = v_tokens[1];
      Tokenize(binspec,v_tokens,"-");
      if (v_tokens.size() != 2) {
	cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
	cerr << theline << endl;
	continue;
      }
      string newname = string(tmph2->GetName())+"_Xbins"+binspec;
      int lobin=str2int(v_tokens[0]);
      int hibin=str2int(v_tokens[1]);
      if (lobin > hibin) {
	cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
	cerr << theline << endl;
	continue;
      }
      h1 = (TH1 *)tmph2->ProjectionY(newname.c_str(),lobin,hibin);
      wh = new wTH1(h1);
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key == "projectyx") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined, " << hid << endl; continue;
      }

      string binspec = value; // range of bins to project

      // binspec of form
      //
      Tokenize(binspec,v_tokens,":");
      if (v_tokens.size() != 2) {
	cerr << "Error, expecting binspec of form 'histo_id:zlobin-zhibin'";
	cerr << theline << endl;
	continue;
      }
      TH1 *tmph = findHisto(v_tokens[0],"histo operand must be defined before math ops");
      if (!tmph) continue;
      if (!tmph->InheritsFrom("TH3")) {
	cerr << "operation projectyx only defined for 3D histograms, " << hid << endl; continue;
      }
      TH3 *h3 = (TH3 *)tmph;

      binspec = v_tokens[1];
      Tokenize(binspec,v_tokens,"-");
      if (v_tokens.size() != 2) {
	cerr << "Error, expecting binspec of form 'histo_id:zlobin-zhibin'";
	cerr << theline << endl;
	continue;
      }
      int lobin=str2int(v_tokens[0]);
      int hibin=str2int(v_tokens[1]);
      if (lobin > hibin) {
	cerr << "Error, expecting binspec of form 'histo_id:zlobin-zhibin'";
	cerr << theline << endl;
	continue;
      }
      string newname = string(h3->GetName())+"_Zbins"+binspec;

      h3->GetZaxis()->SetRange(lobin,hibin);
      tmph = (TH1 *)h3->Project3D("yx");
      h1 = (TH1 *)tmph->Clone(newname.c_str()); // for multiple projections, otherwise root overwrites
      wh = new wTH1(h1);
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));
    }
    //------------------------------
    else if (key.find("slice") != string::npos) {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string binspec = value; // range of bins to project

      // binspec of form
      //
      Tokenize(binspec,v_tokens,":");
      if (v_tokens.size() != 2) {
	cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
	cerr << theline << endl;
	continue;
      }
      TH2 *tmph2 = (TH2 *)findHisto(v_tokens[0],"histo operand must be defined before math ops");
      if (!tmph2) exit(-1);
      binspec = v_tokens[1];
      Tokenize(binspec,v_tokens,"-");
      if (v_tokens.size() != 2) {
	cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
	cerr << theline << endl;
	continue;
      }

      int lobin=str2int(v_tokens[0]);
      int hibin=str2int(v_tokens[1]);
      if (lobin > hibin) {
	cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
	cerr << theline << endl;
	continue;
      }

      string newname = string(tmph2->GetName())+"_Ybins"+binspec;
      TObjArray *aSlices = new TObjArray();
      if (key.find("x")!=string::npos) tmph2->FitSlicesX(0,lobin,hibin,0, "QNR",aSlices);
      else if (key.find("y")!=string::npos) tmph2->FitSlicesY(0,lobin,hibin,0,"QNR",aSlices);
      else continue;
      
      if (key.find("mean")!=string::npos) {
	h1 = (TH1 *)(*aSlices)[1];
      } else if (key.find("rms")!=string::npos) {
	h1 = (TH1 *)(*aSlices)[2];
      } else if (key.find("chi2")!=string::npos) {
	h1 = (TH1 *)(*aSlices)[aSlices->GetEntriesFast()-1];
      }
      if (h1) {
	wh = new wTH1((TH1 *)h1->Clone()); // h1 points to a "dead histo walking"
	glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));
      } else {
	cerr << "Unknown key '" << key << "'" << endl;
      }

    //------------------------------
    } else if (key == "x2Dize") {  // means to take a collection of 1d plots and make a
    //------------------------------  2d plot by laying them sequentially along the x-axis

      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      Tokenize(value,v_tokens,",");
      if (!v_tokens.size()) {
	cerr << "expect comma-separated list of at least two histo ids to 2D-ize!";
	cerr << theline << endl;
	continue;
      }

      for (unsigned i=0; i<v_tokens.size(); i++) {
	TH1 *tmph1 = findHisto(v_tokens[i],"histo operand must be defined before math ops");
	if (!tmph1) exit(-1);
	v_histos2dize.push_back(tmph1);
      }
      cout << "x-2d-izing " << v_histos2dize.size() << " histos" << endl;
      int nbinsx = v_histos2dize.size();
      if (nbinsx) {
	TAxis   *xax = v_histos2dize[0]->GetXaxis();
	int   nbinsy = xax->GetNbins();
	h1 = new TH2D(hid->c_str(),hid->c_str(),
		      v_histos2dize.size(),1.0,(float)v_histos2dize.size(),
		      nbinsy,xax->GetXmin(),xax->GetXmax());
	for (int i=1; i<=nbinsx; i++) {
	  TH1 *src = v_histos2dize[i-1];
	  for (int j=1; j<=xax->GetNbins(); j++)
	    ((TH2D *)h1)->SetBinContent(i,j,src->GetBinContent(j));
	}
	wh = new wTH1(h1);
	glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));
      }

    } else if (!h1) {  // all other keys must have one of the above ops defined 1st
      cerr << "an operation key must be defined before key " << key << endl;
      break;
    }

    else {
      processCommonHistoParams(key,value,*wh);
    }
  } // while getline loop

  return (h1 != NULL);
}                                                 // processHmathSection

//======================================================================

bool                              // returns true if success
processLegendSection(FILE *fp,
		     string& theline,
		     bool& new_section)
{
  vector<string> v_tokens;

  string  *lid  = NULL;
  wLegend_t *wleg = new wLegend_t("FillMe");

  cout << "Processing legend section" << endl;

  new_section=false;

  while (getLine(fp,theline,"legend")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      break;
    }

    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    //--------------------
    if (key == "id") {
    //--------------------
      if (lid != NULL) {
	cerr << "no more than one id per legend section allowed " << *lid << endl;
	break;
      }
      lid = new string(value);
      if (glmap_id2legend.find(*lid) != glmap_id2legend.end()) { // legend id's cannot be redefined
	cerr << "Legend ID " << *lid << " already defined, cannot be redefined." << endl;
	break;
      }
    }
    else if (key == "header")     wleg->header = value;
    else if (key == "x1ndc")      wleg->x1ndc  = str2flt(value);
    else if (key == "y1ndc")      wleg->y1ndc  = str2flt(value);
    else if (key == "x2ndc")      wleg->x2ndc  = str2flt(value);
    else if (key == "y2ndc")      wleg->y2ndc  = str2flt(value);
    else if (key == "ncol")       wleg->ncolumns   = str2int(value);
    else if (key == "textsize")   wleg->textsize   = str2flt(value);
    else if (key == "textfont")   wleg->textfont   = str2int(value);
    else if (key == "linewidth")  wleg->linewidth  = str2int(value);
    else if (key == "fillcolor")  wleg->fillcolor  = str2int(value);
    else if (key == "bordersize") wleg->bordersize = str2int(value);
    else if (key == "draw")       wleg->drawoption = value;
    else {
      cerr << "unknown key " << key << endl;
    }
  }

  glmap_id2legend.insert(pair<string,wLegend_t *>(*lid,wleg));
  return true;
}                                                // processLegendSection

//======================================================================

bool                                          // returns true if success
processLabelSection(FILE   *fp,
		    string& theline,
		    bool&   new_section)
{
  vector<string> v_tokens;

  cout << "Processing label section" << endl;

  string  *lid  = NULL;
  wLabel_t *wlab = new wLabel_t("FillMe");

  new_section=false;

  while (getLine(fp,theline,"label")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    //--------------------
    if (key == "id") {
    //--------------------
      if (lid != NULL) {
	cerr << "no more than one id per label section allowed " << *lid << endl;
	break;
      }
      lid = new string(value);
      if (glmap_id2label.find(*lid) != glmap_id2label.end()) { // label id's cannot be redefined
	cerr << "Label ID " << *lid << " already defined, cannot be redefined." << endl;
	break;
      }
    }
    else if (key == "text")       wlab->text   = value;
    else if (key == "x1ndc")      wlab->x1ndc  = str2flt(value);
    else if (key == "y1ndc")      wlab->y1ndc  = str2flt(value);

    else if (key == "textsize")   wlab->textsize   = str2flt(value);
    else if (key == "textfont")   wlab->textfont   = str2int(value);
    else {
      cerr << "unknown key " << key << endl;
    }
  } // while loop

  glmap_id2label.insert(pair<string,wLabel_t *>(*lid,wlab));
  return true;
}                                                 // processLabelSection

//======================================================================

bool                                        // returns true if success
processAliasSection(FILE *fp,string& theline, bool& new_section)
{
  vector<string> v_tokens;

  cout << "Processing alias section" << endl;

  new_section=false;

  while (getLine(fp,theline,"alias")) {

    if (!theline.size())   continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");

    if ((v_tokens.size() < 2) ||
	(!v_tokens[0].size()) ||
	(!v_tokens[1].size())    ) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key = v_tokens[0];
    string value;
    for (unsigned i=1; i<v_tokens.size(); i++) {
      if (value.size()) value += "=";
      value+=v_tokens[i];
    }

    if (key=="includefile") { // reserved for putting aliii in a separate file
      FILE *fp = fopen(value.c_str(),"r");
      if (!fp) {
	cerr<<"Error, couldn't open alias include file '"<<value<<"'"<<endl;
	exit(-1);
      }
      cout << "Loading include file '" << value << "'" << endl;
      processAliasSection(fp,theline,new_section);
    } else {
      map<string,string>::const_iterator it;
      it = glmap_aliii.find(key);
      if (it != glmap_aliii.end()) {
	cerr << "Error in ALIAS section: key '" << key;
	cerr << "' already defined." << endl;
      } else {
	// Alias can include other aliii, but must be delimited by '/'
	string aspec = value;
	if (aspec.find('@') != string::npos) {
	  string temp=aspec;
	  buildStringFromAliii(temp,"/",aspec);
	  if (!aspec.size()) continue;
	}

	glmap_aliii.insert(pair<string,string>(key,aspec));
	cout << "alias '" << key << "' added" << endl;
      }
    }
  } // while loop

  return true;
}                                                 // processAliasSection

//======================================================================

void parseCanvasLayout(const string& layoutFile,
		       wCanvas_t&  wc)
{
  cout << "Processing " << layoutFile << endl;

  FILE *fp = fopen(layoutFile.c_str(),"r");
  if (!fp) {
    cerr << "Error, couldn't open " << layoutFile << endl;
    exit(-1);
  }

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
    else if (section == "LAYOUT") processLayoutSection(fp,theline,wc,new_section);

    else if (section == "PAD") {
      string padname = string("pad")+int2str((int)(wc.pads.size()+1))+string("frame");
      wPad_t *wpad = new wPad_t(padname);
      processPadSection(fp,theline,wpad,new_section);
      wc.pads.push_back(wpad);
    }
    else if (section == "HISTO") {
      processHistoSection(fp,theline,new_section);
    }
    else if (section == "HMATH") {
      processHmathSection(fp,theline,new_section);
    }
    else if (section == "GRAPH") {
      processGraphSection(fp,theline,new_section);
    }
    else if (section == "LEGEND") {
      processLegendSection(fp,theline,new_section);
    }
    else if (section == "LABEL") {
      processLabelSection(fp,theline,new_section);
    }
    else if (section == "LINE") {
      processLineSection(fp,theline,new_section);
    }
    else if (section == "ALIAS") {
      processAliasSection(fp,theline,new_section);
    }
    else if (section == "TF1") {
      processTF1Section(fp,theline,new_section);
    }
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

wCanvas_t *initCanvas(const string& cLayoutFile)
{
  wCanvas_t *wc = new wCanvas_t(cLayoutFile);

  parseCanvasLayout(cLayoutFile, *wc);

  // Set Styles:
  gStyle->SetPalette(1,0); // always!

  wc->c1 = new TCanvas(cLayoutFile.c_str(),cLayoutFile.c_str(),
		       wc->padxdim*wc->npadsx,
		       wc->padydim*wc->npadsy);

  wc->c1->SetFillColor(wc->fillcolor);

  unsigned npads = wc->npadsx*wc->npadsy;
  if (npads>1) wc->c1->Divide(wc->npadsx,wc->npadsy);
			      // , wc->padxmargin,wc->padymargin);

  cout << "Canvas " << cLayoutFile << " dimensions "
       << wc->npadsx << "x" << wc->npadsy << endl;
  cout << "Canvas " << cLayoutFile << " margins "
       << wc->padxmargin << "x" << wc->padymargin << endl;

  return wc;
}                                                          // initCanvas

//======================================================================

void drawInPad(wPad_t *wp, wTH1& myHisto,bool firstInPad,
	       const string& altdrawopt="")
{
  wp->vp->SetFillColor(wp->fillcolor);

  //tdrStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);
  wp->vp->SetGridx(wp->gridx);
  wp->vp->SetGridy(wp->gridy);

  if (firstInPad) {
    if (wp->topmargin)    wp->vp->SetTopMargin   (wp->topmargin);
    if (wp->bottommargin) wp->vp->SetBottomMargin(wp->bottommargin);
    if (wp->rightmargin)  wp->vp->SetRightMargin (wp->rightmargin);
    if (wp->leftmargin)   wp->vp->SetLeftMargin  (wp->leftmargin);

    altdrawopt.size() ? myHisto.Draw(altdrawopt) : myHisto.Draw();
  }
  else {
    if (!myHisto.statsAreOn()) altdrawopt.size() ?
				 myHisto.Draw(altdrawopt+ " same") :
				 myHisto.DrawSame();
    else                       altdrawopt.size() ?
				 myHisto.Draw(altdrawopt+ " sames") :
				 myHisto.DrawSames();
  }

#if 0
  myHisto.histo()->Sumw2();
  TArrayD *sumw2 = myHisto.histo()->GetSumw2();
  cout << sumw2->GetSize() << endl;
  for (int i=0; i<sumw2->GetSize(); i++)
    printf ("%g ", (*sumw2)[i]);
  cout << endl;
#endif

  wp->vp->Update();
}                                                           // drawInPad

//======================================================================

void drawInPad(wPad_t *wp, TGraph *gr, const string& drawopt)
{
  wp->vp->SetFillColor(wp->fillcolor);

  //tdrStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);

  cout<<"Drawing "<<gr->GetName()<<" with option(s) "<<drawopt<<endl;

  gr->Draw(drawopt.c_str());

  wp->vp->Update();
}                                                           // drawInPad

//======================================================================

void drawInPad(wPad_t *wp, TLine *line, const string& drawopt)
{
  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);

  cout<<"Drawing line with option(s) "<<drawopt<<endl;

  line->Draw(drawopt.c_str());

  wp->vp->Update();
}                                                           // drawInPad

//======================================================================

void drawInPad(wPad_t *wp, THStack *stack)
{
  wp->vp->SetFillColor(wp->fillcolor);

  //tdrStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);

  if (wp->topmargin)    wp->vp->SetTopMargin   (wp->topmargin);
  if (wp->bottommargin) wp->vp->SetBottomMargin(wp->bottommargin);
  if (wp->rightmargin)  wp->vp->SetRightMargin (wp->rightmargin);
  if (wp->leftmargin)   wp->vp->SetLeftMargin  (wp->leftmargin);

  stack->Draw();

  wp->vp->Update();
}                                                           // drawInPad

//======================================================================

void  drawPlots(wCanvas_t& wc,bool savePlot2file)
{
  unsigned npads = std::min(wc.npadsx*wc.npadsy, wc.pads.size());

  cout << "Drawing on " << npads << " pad(s)" << endl;

  wLegend_t *wl = NULL;

  /***************************************************
   * LOOP OVER PADS...
   ***************************************************/

  vector<vector<string> >::const_iterator it;
  for (unsigned i = 0; i< npads; i++) {

    bool drawlegend = false;

    wPad_t *& wp = wc.pads[i];
    wp->vp = wc.c1->cd(i+1);

    if (!wp->histo_ids.size() && !wp->graph_ids.size()) {
      cerr << "ERROR: pad #" << i+1 << " has no ids defined for it";
      cerr << ", continuing to the next" << endl;
      continue;
    }

    /***************************************************
     * Draw the frame first:
     * (Fix up frame since it can't be auto-scaled:)
     ***************************************************/

#if 0
    string& hid0 = wp->histo_ids[0];
    map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid0);
    if (it == glmap_id2histo.end()) {
      cerr << "ERROR: id0 " << hid0 << " never defined in layout" << endl;
      return;
    }
    wTH1 *myHisto = it->second;
    TH1  *h = myHisto->histo();

    if (wp->hframe->histo()->GetXaxis()->GetXmin() <=
	wp->hframe->histo()->GetXaxis()->GetXmax())
      wp->hframe->histo()->GetXaxis()->SetRangeUser(h->GetXaxis()->GetXmin(),
						    h->GetXaxis()->GetXmax());
    if (wp->hframe->histo()->GetYaxis()->GetXmin() <= 
	wp->hframe->histo()->GetYaxis()->GetXmax())
      wp->hframe->histo()->GetYaxis()->SetRangeUser(h->GetYaxis()->GetXmin(),
						    h->GetYaxis()->GetXmax());

    wp->hframe->SetStats(0);
    //wp->hframe->Draw("AXIS");
#endif

    /***************************************************
     * Check for existence of a legend, create it
     ***************************************************/

    if (wp->legid.size()) {
      map<string,wLegend_t *>::const_iterator it=glmap_id2legend.find(wp->legid);
      if (it != glmap_id2legend.end()) {
	drawlegend = true;
	wl = it->second;
	wl->leg = new TLegend(wl->x1ndc,wl->y1ndc,
			      wl->x2ndc,wl->y2ndc);
      } else {
	cerr << "ERROR: legend id " << wp->legid;
	cerr << " never defined in layout" << endl;
      }
    }

    /***************************************************
     * Draw each histo
     ***************************************************/

    THStack *stack=NULL;
    if (wp->stackem)
      stack = new THStack(wp->hframe->histo()->GetName(),
			  wp->hframe->histo()->GetTitle());

    /***************************************************
     * LOOP OVER HISTOS DEFINED FOR PAD...
     ***************************************************/

    for (unsigned j = 0; j < wp->histo_ids.size(); j++) {
      string& hid = wp->histo_ids[j];
      map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid);
      if (it == glmap_id2histo.end()) {
	cerr << "ERROR: histo id " << hid << " never defined in layout" << endl;
	exit (-1);
      }

      wTH1 *myHisto = it->second;
      
      if (myHisto) {
	if (wp->stackem) {
	  stack->Add(myHisto->histo());
	} else {
	  cout << "Drawing " << hid << " => ";
	  cout << myHisto->histo()->GetName() << endl;
	  drawInPad(wp,*myHisto,!j);
	  if (myHisto->statsAreOn()) {
	    cout << "OptStat = " <<  gStyle->GetOptStat() << endl;
	    myHisto->DrawStats();
	    wp->vp->Update();
	  }
	}
	myHisto->DrawFits();
	if (drawlegend && myHisto->GetLegendEntry().size()) {
	  if (wl->drawoption.size()) myHisto->SetDrawOption(wl->drawoption);
	  myHisto->Add2Legend(wl->leg);
	}
      }
    } // histos loop

    Float_t rightmax=0.0,rightmin=0.0;
    Float_t scale=0.0;
    for (unsigned j = 0; j < wp->altyh_ids.size(); j++) {
      string& hid = wp->altyh_ids[j];
      map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid);
      if (it == glmap_id2histo.end()) {
	cerr << "ERROR: histo id " << hid << " never defined in layout" << endl;
	exit (-1);
      }

      wTH1 *myHisto = it->second;

      if (!j) {
	//scale second set of histos to the pad coordinates
	rightmin = myHisto->histo()->GetMinimum();
	rightmax = 1.1*myHisto->histo()->GetMaximum();
	scale    = gPad->GetUymax()/rightmax;
      }
      myHisto->histo()->Scale(scale);
      myHisto->histo()->Draw("same");
   
      //draw an axis on the right side
      TGaxis *axis = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(),
				gPad->GetUxmax(), gPad->GetUymax(),
				rightmin,rightmax,510,"+L");
      axis->Draw();
      gPad->Update();
      if (drawlegend && myHisto->GetLegendEntry().size()) {
	if (wl->drawoption.size()) myHisto->SetDrawOption(wl->drawoption);
	myHisto->Add2Legend(wl->leg);
      }
    }

    if (wp->stackem) {
      cout << "Drawing stack" << endl;
      drawInPad(wp,stack);
    }

    /***************************************************
     * LOOP OVER GRAPHS DEFINED FOR PAD...
     ***************************************************/

    for (unsigned j = 0; j < wp->graph_ids.size(); j++) {
      //string drawopt("CP");
      string drawopt("L");
      string& gid = wp->graph_ids[j];
      map<string,TGraph *>::const_iterator it = glmap_id2graph.find(gid);
      if (it == glmap_id2graph.end()) {
	cerr << "ERROR: graph id " << gid << " never defined in layout" << endl;
	exit (-1);
      }

      TGraph *gr = it->second;

      if (!j && !wp->histo_ids.size())
	drawopt += string("A"); // no histos drawn, need to draw the frame ourselves.

      if (gr) {
	drawInPad(wp,gr,drawopt.c_str());
	wp->vp->Update();

	if (drawlegend)
	  wl->leg->AddEntry(gr,gid.c_str(),"L");
      }
    }

    /***************************************************
     * LOOP OVER LINES DEFINED FOR PAD...
     ***************************************************/

    for (unsigned j = 0; j < wp->line_ids.size(); j++) {
      string drawopt("L");
      string& lid = wp->line_ids[j];
      map<string,TLine *>::const_iterator it = glmap_id2line.find(lid);
      if (it == glmap_id2line.end()) {
	cerr << "ERROR: line id " << lid << " never defined in layout" << endl;
	exit (-1);
      }

      TLine *line = it->second;

      if (!j && !wp->histo_ids.size())
	drawopt += string("A"); // no histos drawn, need to draw the frame ourselves.

      if (line) {
	drawInPad(wp,line,drawopt.c_str());
	wp->vp->Update();

	if (drawlegend)
	  wl->leg->AddEntry(line,lid.c_str(),"L");
      }
    }

    /***************************************************
     * Draw the legend
     ***************************************************/

    if (drawlegend) {
      if (wl->header != "FillMe") wl->leg->SetHeader(wl->header.c_str());
      wl->leg->SetTextSize(wl->textsize);
      wl->leg->SetTextFont(wl->textfont);
      wl->leg->SetBorderSize(wl->bordersize);
      wl->leg->SetFillColor(wl->fillcolor);
      wl->leg->SetLineWidth(wl->linewidth);
      wl->leg->SetNColumns(wl->ncolumns);
      wl->leg->Draw("same");
      wp->vp->Update();
    }

    /***************************************************
     * Draw each label
     ***************************************************/

    for (unsigned j = 0; j < wp->label_ids.size(); j++) {
      string& lid = wp->label_ids[j];
      map<string,wLabel_t *>::const_iterator it = glmap_id2label.find(lid);
      if (it == glmap_id2label.end()) {
	cerr << "ERROR: label id " << lid << " never defined in layout" << endl;
	exit (-1);
      }
      wLabel_t *wl = it->second;
      drawStandardText(wl->text, wl->x1ndc, wl->y1ndc,-1,-1,wl->textsize);

      wp->vp->Update();
    }
    wc.c1->Update();

  } // pad loop

  //prdFixOverlay();

  if (savePlot2file) {
    wc.c1->cd();
    // Use the first file read in and the config file name to make
    // the filename.
    //
    std::string datafile,cfgfile;
    map<string,TFile*>::const_iterator it = glmap_id2rootfile.begin();
    if (it != glmap_id2rootfile.end())
      datafile = it->first.substr(0,it->first.find_last_of('.'));
    size_t startpos=wc.title.find_last_of('/');
    if (startpos==string::npos) startpos = 0;
    else startpos++;
    cfgfile=wc.title.substr(startpos,wc.title.find_last_of('.')-startpos);
    string picfile = datafile+"_"+cfgfile+".png";
    cout << "saving to..." << picfile << endl;
    wc.c1->SaveAs(picfile.c_str());
  }
}                                                           // drawPlots

//======================================================================

void superPlot(const string& canvasLayout="canvas.txt",
	       bool savePlot2file=false,
	       bool dotdrStyle=false)
{
  glmap_objpaths2id.clear();
  glmap_aliii.clear();
  glmap_id2histo.clear();
  glmap_id2graph.clear();
  glmap_id2legend.clear();
  glmap_id2label.clear();
  glmap_id2line.clear();
  glmap_id2tf1.clear();

  setPRDStyle();
  gROOT->ForceStyle();

  if (dotdrStyle) {
    setTDRStyle();
    gROOT->ForceStyle();
    tdrStyle->SetOptTitle(1);
    tdrStyle->SetTitleFont(42);
    tdrStyle->SetTitleFontSize(0.05);
  // tdrStyle->SetTitleBorderSize(2);

  }

  drawPlots(*initCanvas(canvasLayout),savePlot2file);
}
