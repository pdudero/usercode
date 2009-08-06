#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm> // min,max
#include <iostream>
#include <sstream>
#include <ctype.h>     // isdigit
#include <stdlib.h>
#include "MyHistoWrapper.cc"
#include "inSet.hh"
#include "TFile.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TPaveStats.h"

#include "tdrstyle4timing.C"

using namespace std;

struct wPad_t {
  wPad_t(string name) : topmargin(0.),bottommargin(0.),rightmargin(0.),leftmargin(0.),
			fillcolor(10),logx(0),logy(0)
  { hframe = new wTH1D(name,name,100,0.0,1.0); }
  float topmargin, bottommargin, rightmargin, leftmargin;
  unsigned fillcolor;
  unsigned logx, logy;
  float titlexndc, titleyndc;
  wTH1D *hframe;           // the frame histo, holds lots of pad info
  vector<string> histo_ids;
  string legid;
  TVirtualPad *vp;
};

struct wCanvas_t {
  wCanvas_t(const string& intitle,
	    unsigned innpadsx=1, unsigned inpadxdim=600,
	    unsigned innpadsy=1, unsigned inpadydim=600) :
    title(intitle), npadsx(innpadsx),npadsy(innpadsy),
    padxdim(inpadxdim),padydim(inpadydim), optstat("nemr"),
    fillcolor(10) {}
  string   style;
  string   title;
  unsigned npadsx;
  unsigned npadsy;
  unsigned padxdim;
  unsigned padydim;
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
    fillcolor(infillclr),bordersize(1),linewidth(1) {}
  string   header;
  float    x1ndc, y1ndc;
  float    x2ndc, y2ndc;
  unsigned fillcolor;
  int      bordersize;
  unsigned linewidth;
  unsigned textfont;
  float    textsize;
  TLegend *leg;
};

static set<string> glset_histopathsReadIn;  // keep track of histos read in

static map<string, string>      glmap_fileid2path;
static map<string, wTH1D *>     glmap_id2histo;
static map<string, wLegend_t *> glmap_id2legend; // the map...of legends

//======================================================================

inline unsigned int str2int(const string& str) {
  return (unsigned int)strtoul(str.c_str(),NULL,0);
}

inline float str2flt(const string& str) {
  return (float)strtod(str.c_str(),NULL);
}

inline string int2str(int i) {
  std::ostringstream ss;
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
	      const string& delimiters = " ")
{
  tokens.clear();

  // Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos) {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}                                                            // Tokenize

//======================================================================

bool getLine(FILE *fp, string& theline, const string& callerid)
{ 
  char linein[256];


  if (!feof(fp) && fgets(linein,256,fp)) {
    if (strlen(linein) && (linein[strlen(linein)-1] == '\n'))
      linein[strlen(linein)-1] = '\0';
    theline = string(linein);
  } else return false;

  //cout << theline <<  ", callerid = " << callerid << endl;

  return true;
}

//======================================================================

TH1 *IntegrateLeft(TH1 *h)
{
  TH1 *hcum = (TH1 *)h->Clone();
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

TH1 *IntegrateRight(TH1 *h)
{
  TH1 *hcum = (TH1 *)h->Clone();
  hcum->Reset();

  int nbins = hcum->GetNbinsX();
  double htotal = h->Integral(1,nbins+1);

  // Include the overflow bin
  for (int i=1; i<=nbins+1 ; i++) { // includes overflow
    double integral = h->Integral(i,nbins+1);
    hcum->SetBinContent(i,integral/htotal);
  }

  return hcum;
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

    if (theline.find('[') != string::npos) {
      new_section=true;
      return true;
    }
    Tokenize(theline,v_tokens,"=");
    if (v_tokens.size() != 2) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key   = v_tokens[0];
    string value = v_tokens[1];

    if      (key == "style") {
      if (value == "TDR")
	setTDRStyle();
      else if (value == "Plain")
	gROOT->SetStyle("Plain");
    }
    else if (key == "optstat")   gStyle->SetOptStat(value.c_str());

    // Set the position/size of the title box
    else if (key == "title")     gStyle->SetOptTitle(str2int(value));
    else if (key == "titlexndc") gStyle->SetTitleX(str2flt(value));
    else if (key == "titleyndc") gStyle->SetTitleY(str2flt(value));
    else if (key == "titlewndc") gStyle->SetTitleW(str2flt(value));
    else if (key == "titlehndc") gStyle->SetTitleH(str2flt(value));
    else if (key == "titlefont") gStyle->SetTitleFont(str2int(value));

    // Set the position of the statbox
    else if (key == "statxndc")  gStyle->SetStatX(str2flt(value));
    else if (key == "statyndc")  gStyle->SetStatY(str2flt(value));
    else if (key == "statwndc")  gStyle->SetStatW(str2flt(value));
    else if (key == "stathndc")  gStyle->SetStatH(str2flt(value));

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

    if (theline.find('[') != string::npos) {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");
    if (v_tokens.size() != 2) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key   = v_tokens[0];
    string value = v_tokens[1];

    if (key == "npadsx") {
      if (isdigit(value[0])) {
	unsigned npadsx = value[0] - '0';
	if (npadsx > 1) wc.npadsx = npadsx;
      }
    }
    else if (key == "npadsy") {
      if (isdigit(value[0])) {
	unsigned npadsy = value[0] - '0';
	if (npadsy > 1) wc.npadsy = npadsy;
      }
    }
    else if (key == "padxdim") {
      unsigned long padxdim = str2int(value);
      if (padxdim > 0) wc.padxdim = padxdim;

    }
    else if (key == "padydim") {
      unsigned long padydim = str2int(value);
      if (padydim > 0) wc.padydim = padydim;
    }
    else if (key == "fillcolor")
      wc.fillcolor = str2int(value);

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

    if (theline.find('[') != string::npos) {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");
    if (v_tokens.size() != 2) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key   = v_tokens[0];
    string value = v_tokens[1];

    if      (key == "histos") {
      Tokenize(value,wpad->histo_ids," ,"); 
      if (!wpad->histo_ids.size()) wpad->histo_ids.push_back(value);
    }
    else if (key == "title")        wpad->hframe->histo()->SetTitle(value.c_str());
    else if (key == "xtitle")       wpad->hframe->histo()->SetXTitle(value.c_str());
    else if (key == "ytitle")       wpad->hframe->histo()->SetYTitle(value.c_str());
    else if (key == "rightmargin")  wpad->rightmargin  = str2flt(value);
    else if (key == "leftmargin")   wpad->leftmargin   = str2flt(value);
    else if (key == "topmargin")    wpad->topmargin    = str2flt(value);
    else if (key == "bottommargin") wpad->bottommargin = str2flt(value);
    else if (key == "fillcolor")    wpad->fillcolor    = str2int(value);
    else if (key == "legend")       wpad->legid        = value;
    else if (key == "logx")         wpad->logx         = str2int(value);
    else if (key == "logy")         wpad->logy         = str2int(value);
    else if (key == "xmin") {
      float xmin = str2flt(value);
      wpad->hframe->SetXaxis("",false,0.0,0.0,0.0,xmin,xmax);
    }
    else if (key == "xmax") {
      float xmax = str2flt(value);
      wpad->hframe->SetXaxis("",false,0.0,0.0,0.0,xmin,xmax);
    }
    else if (key == "ymin") {
      ymin = str2flt(value);
      wpad->hframe->SetYaxis("",false,0.0,0.0,0.0,ymin,ymax);
    }
    else if (key == "ymax") {
      ymax = str2flt(value);
      wpad->hframe->SetYaxis("",false,0.0,0.0,0.0,ymin,ymax);
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
			      wTH1D& wh)
{
  static float xmin=1e99, xmax=-1e99;
  static float ymin=1e99, ymax=-1e99;

  if      (key == "draw")        wh.SetDrawOption(value);
  else if (key == "title")       wh.histo()->SetTitle(value.c_str());

  else if (key == "markerstyle") wh.SetMarker(0,str2int(value));
  else if (key == "markercolor") wh.SetMarker(str2int(value));
  else if (key == "linecolor")   wh.SetLine(str2int(value));
  else if (key == "linestyle")   wh.SetLine(0,str2int(value));
  else if (key == "linewidth")   wh.SetLine(0,0,str2int(value));
  else if (key == "leglabel")    wh.SetLegendEntry(value);

  // for 2-d histos
  else if (key == "rebinx")      ((TH2 *)wh.histo())->RebinX(str2int(value));
  else if (key == "rebiny")      ((TH2 *)wh.histo())->RebinY(str2int(value));

  // axes
  else if (key == "xtitle")       wh.SetXaxis(value);
  else if (key == "ytitle")       wh.SetYaxis(value);
  else if (key == "xtitlesize")   wh.SetXaxis("",false,str2flt(value));
  else if (key == "ytitlesize")   wh.SetYaxis("",false,str2flt(value));
  else if (key == "xtitleoffset") wh.SetXaxis("",false,0.0,str2flt(value));
  else if (key == "ytitleoffset") wh.SetYaxis("",false,0.0,str2flt(value));
  else if (key == "xtitlefont")   wh.SetXaxis("",false,0.0,0.0,str2int(value));
  else if (key == "ytitlefont")   wh.SetYaxis("",false,0.0,0.0,str2int(value));
  else if (key == "xlabelsize")   wh.SetXaxis("",false,0.0,0.0,0,str2flt(value));
  else if (key == "ylabelsize")   wh.SetYaxis("",false,0.0,0.0,0,str2flt(value));
  else if (key == "xlabelfont")   wh.SetXaxis("",false,0.0,0.0,0,0.0,str2int(value));
  else if (key == "ylabelfont")   wh.SetYaxis("",false,0.0,0.0,0,0.0,str2int(value));
  else if (key == "xndiv")  wh.SetXaxis("",false,0.0,0.0,0,0.0,0,1e99,-1e99,str2int(value));
  else if (key == "yndiv")  wh.SetYaxis("",false,0.0,0.0,0,0.0,0,1e99,-1e99,str2int(value));

  // stats box
  else if (key == "statson")    wh.SetStats(str2int(value) != 0);
  else if (key == "statsx1ndc") wh.SetStats(wh.statsAreOn(),str2flt(value));
  else if (key == "statsy1ndc") wh.SetStats(wh.statsAreOn(),0.0,str2flt(value));
  else if (key == "statsx2ndc") wh.SetStats(wh.statsAreOn(),0.0,0.0,str2flt(value));
  else if (key == "statsy2ndc") wh.SetStats(wh.statsAreOn(),0.0,0.0,0.0,str2flt(value));

  else if (key == "xmin") {
    xmin = str2flt(value);
    wh.SetXaxis("",false,0.0,0.0,0,0.0,0,xmin,xmax);
  }
  else if (key == "xmax") {
    xmax = str2flt(value);
    wh.SetXaxis("",false,0.0,0.0,0,0.0,0,xmin,xmax);
  }
  else if (key == "ymin") {
    ymin = str2flt(value);
    wh.SetYaxis("",false,0.0,0.0,0,0.0,0,ymin,ymax);
  }
  else if (key == "ymax") {
    ymax = str2flt(value);
    wh.SetYaxis("",false,0.0,0.0,0,0.0,0,ymin,ymax);
  }
  else {
    cerr << "unknown key " << key << endl;
  }
}

//======================================================================

bool                              // returns true if success
processHistoSection(FILE *fp,
		    string& theline,
		    bool& new_section)
{
  vector<string> v_tokens;

  wTH1D *wh;
  string *hid = NULL;
  TFile *rootfile = NULL;
  TH1D  *h1d      = NULL;

  cout << "Processing histo section" << endl;

  new_section=false;

  while (getLine(fp,theline,"histo")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline.find('[') != string::npos) {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");

    if (v_tokens.size() != 2) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key   = v_tokens[0];
    string value = v_tokens[1];

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
	cerr << "id key must be defined before clone key" << endl; continue;
      }
      map<string,wTH1D *>::const_iterator it = glmap_id2histo.find(value);
      if (it == glmap_id2histo.end()) {
	cerr << "Histo ID " << value << " not found, clone must be defined after the clonee" << endl;break;
	break;
      }
      wh = it->second->Clone(string(it->second->histo()->GetName())+"_clone",
			     string(it->second->histo()->GetTitle()));
      h1d = wh->histo();
      glmap_id2histo.insert(std::pair<string,wTH1D *>(*hid,wh));

    //------------------------------
    } else if (key == "path") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined before path key" << endl; continue;
      }
      string path = value;
      if (h1d) {
	cerr << "histo already defined" << endl; continue;
      }
      if (inSet<string>(glset_histopathsReadIn,path)) {
	cerr << "histo " << path << " already read in. Use 'clone=someid' to duplicate it." << endl; break;
      }
      Tokenize(path,v_tokens,":");
      if (v_tokens.size() != 2) {
	cerr << "malformed root histo path file:folder/subfolder/.../histo " << path << endl; break;
      }
      string rootfn = v_tokens[0];
      if (rootfn[0] == '@') {  // reference to a file defined in FILES section
	map<string,string>::const_iterator it;
	string fileid=rootfn.substr(1);
	it = glmap_fileid2path.find(fileid);
	if (it == glmap_fileid2path.end()) {
	  cerr << "file id " << fileid << " not found, define reference in FILES section first." << endl;
	  continue;
	}
	rootfn = it->second;
      }
	
      rootfile = new TFile(rootfn.c_str());
      if (rootfile->IsZombie()) {
	cerr << "File failed to open, " << value << endl;
	break;
      }
      h1d = (TH1D *)rootfile->Get(v_tokens[1].c_str());
      if (!h1d) {
	cerr << "couldn't find " << v_tokens[1] << " in " << v_tokens[0] << endl;
	exit(-1);
      } else {
	wh = new wTH1D(h1d);
	glmap_id2histo.insert(std::pair<string,wTH1D *>(*hid,wh));
      }

    } else if (!h1d) {  // all other keys must have "path" defined
      cerr << "key 'path' or 'clone' must be defined before key " << key << endl;
      break;
    }

    else {
      processCommonHistoParams(key,value,*wh);
    }
  }
  return (h1d != NULL);
}                                                 // processHistoSection

//======================================================================

bool                              // returns true if success
processHmathSection(FILE *fp,
		    string& theline,
		    bool& new_section)
{
  vector<string> v_tokens;

  wTH1D *wh;
  string *hid = NULL;
  TH1D  *h1d      = NULL;

  cout << "Processing hmath section" << endl;

  new_section=false;

  while (getLine(fp,theline,"hmath")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline.find('[') != string::npos) {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");

    if (v_tokens.size() != 2) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key   = v_tokens[0];
    string value = v_tokens[1];

    //--------------------
    if (key == "id") {
    //--------------------
      if (hid != NULL) {
	cerr << "no more than one id per hmath section allowed " << value << endl;
	break;
      }

      hid = new string(value);

    //------------------------------
    } else if (key == "binaryop") {  // binary operation +-*/
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined before path key" << endl; continue;
      }
      if (h1d) {
	cerr << "histo already defined" << endl; continue;
      }
      string hmathexpr = value;

      Tokenize(hmathexpr,v_tokens,"-+*/");
      if (v_tokens.size() != 2) {
	cerr << "only simple math ops -+*/ supported between two operands, sorry!" << theline << endl;
	continue;
      }
      map<string,wTH1D *>::const_iterator op1 = glmap_id2histo.find(v_tokens[0]);
      if (op1 == glmap_id2histo.end()) {
	cerr << "Histo ID " << v_tokens[0] << " not found, math ops must be defined after the operands" << endl;
	continue;
      }
      map<string,wTH1D *>::const_iterator op2 = glmap_id2histo.find(v_tokens[1]);
      if (op2 == glmap_id2histo.end()) {
	cerr << "Histo ID " << v_tokens[1] << " not found, math ops must be defined after the operands" << endl;
	continue;
      }

      TH1D *h1 = op1->second->histo();
      TH1D *h2 = op2->second->histo();
      TH1D *hres = (TH1D *)h1->Clone();

      if      (theline.find('-') != string::npos) hres->Add(h2,-1.0);
      else if (theline.find('+') != string::npos) hres->Add(h2, 1.0);
      else if (theline.find('*') != string::npos) hres->Multiply(h2);
      else if (theline.find('/') != string::npos) hres->Divide(h2);

      h1d = hres;
      wh = new wTH1D(h1d);
      glmap_id2histo.insert(std::pair<string,wTH1D *>(*hid,wh));

    //------------------------------
    } else if (key == "integright") {  // sweep from low-high x-bins and integrate to the right
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined before path key" << endl; continue;
      }
      if (h1d) {
	cerr << "histo already defined" << endl; continue;
      }
      map<string,wTH1D *>::const_iterator op = glmap_id2histo.find(value);
      if (op == glmap_id2histo.end()) {
	cerr << "Histo ID " << value << " not found, histo operand must be defined before math ops" << endl;
	continue;
      }
      TH1D *h1 = op->second->histo();
      h1d = (TH1D *)IntegrateRight(h1);
      wh = new wTH1D(h1d);
      glmap_id2histo.insert(std::pair<string,wTH1D *>(*hid,wh));

    //------------------------------
    } else if (key == "integleft") {  // sweep from low-high x-bins and integrate to the left
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined before path key" << endl; continue;
      }
      if (h1d) {
	cerr << "histo already defined" << endl; continue;
      }
      map<string,wTH1D *>::const_iterator op = glmap_id2histo.find(value);
      if (op == glmap_id2histo.end()) {
	cerr << "Histo ID " << value << " not found, histo operand must be defined before math ops" << endl;
	continue;
      }
      TH1D *h1 = op->second->histo();
      h1d = (TH1D *)IntegrateLeft(h1);
      wh = new wTH1D(h1d);
      glmap_id2histo.insert(std::pair<string,wTH1D *>(*hid,wh));

    } else if (!h1d) {  // all other keys must have "path" defined
      cerr << "an operation key must be defined before key " << key << endl;
      break;
    }

    else {
      processCommonHistoParams(key,value,*wh);
    }
  } // while getline loop

  return (h1d != NULL);
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

    if (theline.find('[') != string::npos) {
      new_section=true;
      break;
    }

    Tokenize(theline,v_tokens,"=");

    if (v_tokens.size() != 2) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key   = v_tokens[0];
    string value = v_tokens[1];

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

    else if (key == "textsize")   wleg->textsize   = str2flt(value);
    else if (key == "textfont")   wleg->textfont   = str2int(value);
    else if (key == "linewidth")  wleg->linewidth  = str2int(value);
    else if (key == "fillcolor")  wleg->fillcolor  = str2int(value);
    else if (key == "bordersize") wleg->bordersize = str2int(value);
  }

  glmap_id2legend.insert(std::pair<string,wLegend_t *>(*lid,wleg));
  return true;
}                                                // processLegendSection

//======================================================================

bool                                        // returns true if success
processFilesSection(FILE *fp,string& theline, bool& new_section)
{
  vector<string> v_tokens;

  cout << "Processing files section" << endl;

  new_section=false;

  while (getLine(fp,theline,"files")) {

    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline.find('[') != string::npos) {
      new_section=true;
      return true;
    }

    Tokenize(theline,v_tokens,"=");
    if (v_tokens.size() != 2) {
      cerr << "malformed key=value line " << theline << endl; continue;
    }

    string key   = v_tokens[0];
    string value = v_tokens[1];

    std::map<string,string>::const_iterator it;
    it = glmap_fileid2path.find(key);
    if (it != glmap_fileid2path.end()) {
      cerr << "Error in Files section: key " << key << " already defined." << endl;
    } else
      glmap_fileid2path.insert(std::pair<string,string>(key,value));

  } // while loop

  return true;
}                                                 // processFilesSection

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
    if (theline.find('[') != string::npos) {
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
    else if (section == "LEGEND") {
      processLegendSection(fp,theline,new_section);
    }
    else if (section == "FILES") {
      processFilesSection(fp,theline,new_section);
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

  cout << "Canvas " << cLayoutFile << " dimensions "
       << wc->npadsx << "x" << wc->npadsy << endl;

  return wc;
}

//======================================================================

void drawInPad(wPad_t *wp, wTH1D& myHisto,bool firstInPad,
	       const string& altdrawopt="")
{
  wp->vp->SetFillColor(wp->fillcolor);

  tdrStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);

  if (firstInPad) {
    if (wp->topmargin)    wp->vp->SetTopMargin   (wp->topmargin);
    if (wp->bottommargin) wp->vp->SetBottomMargin(wp->bottommargin);
    if (wp->rightmargin)  wp->vp->SetRightMargin (wp->rightmargin);
    if (wp->leftmargin)   wp->vp->SetLeftMargin  (wp->leftmargin);

    altdrawopt.size() ? myHisto.Draw(altdrawopt) : myHisto.Draw();
  }
  else {
    if (!myHisto.statsAreOn()) altdrawopt.size() ? myHisto.Draw(altdrawopt+ " same") : myHisto.DrawSame();
    else                       altdrawopt.size() ? myHisto.Draw(altdrawopt+ " sames") : myHisto.DrawSames();
  }

  wp->vp->Update();
}


//======================================================================

void  drawPlots(wCanvas_t& wc)
{
  unsigned npads = wc.pads.size();

  cout << "Drawing on " << npads << " pad(s)" << endl;

  bool drawlegend = false;
  wLegend_t *wl;

  vector<vector<string> >::const_iterator it;
  for (unsigned i = 0; i< npads; i++) {

    wPad_t *& wp = wc.pads[i];
    wp->vp = wc.c1->cd(i+1);

    if (!wp->histo_ids.size()) {
      cerr << "ERROR: pad #" << i+1 << " has no ids defined for it";
      cerr << ", continuing to the next" << endl;
      continue;
    }

    /***************************************************
     * Draw the frame first:
     * (Fix up frame since it can't be auto-scaled:)
     ***************************************************/

    string& hid0 = wp->histo_ids[0];
    map<string,wTH1D *>::const_iterator it = glmap_id2histo.find(hid0);
    if (it == glmap_id2histo.end()) {
      cerr << "ERROR: id0 " << hid0 << " never defined in layout" << endl;
      return;
    }
    wTH1D *myHisto = it->second;
    TH1D  *h = myHisto->histo();

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

    for (unsigned j = 0; j < wp->histo_ids.size(); j++) {
      string& hid = wp->histo_ids[j];
      map<string,wTH1D *>::const_iterator it = glmap_id2histo.find(hid);
      if (it == glmap_id2histo.end()) {
	cerr << "ERROR: id " << hid << " never defined in layout" << endl;
	exit (-1);
      }

      wTH1D *myHisto = it->second;

      if (myHisto) {
	cout << "Drawing " << hid << " => " << myHisto->histo()->GetName() << endl;
	drawInPad(wp,*myHisto,!j);
	if (myHisto->statsAreOn()) {
	  myHisto->DrawStats();
	  wp->vp->Update();
	}
	if (drawlegend)
	  myHisto->Add2Legend(wl->leg);
      }
    }

    if (drawlegend) {
      if (wl->header != "FillMe") wl->leg->SetHeader(wl->header.c_str());
      wl->leg->SetTextSize(wl->textsize);
      wl->leg->SetTextFont(wl->textfont);
      wl->leg->SetBorderSize(wl->bordersize);
      wl->leg->SetFillColor(wl->fillcolor);
      wl->leg->SetLineWidth(wl->linewidth);
      wl->leg->Draw();
    }

    wc.c1->Update();
  }
  wc.c1->cd();
}                                                           // drawPlots

//======================================================================

void superPlot(const string& canvasLayout="canvas.txt",
	       bool dotdrStyle=false)
{
  glmap_fileid2path.clear();
  glmap_id2histo.clear();
  glmap_id2legend.clear();

  if (dotdrStyle) {
    setTDRStyle();
    gROOT->ForceStyle();
    tdrStyle->SetOptTitle(1);
    tdrStyle->SetTitleFont(42);
    tdrStyle->SetTitleFontSize(0.05);
  // tdrStyle->SetTitleBorderSize(2);

  }

  drawPlots(*initCanvas(canvasLayout));
}
