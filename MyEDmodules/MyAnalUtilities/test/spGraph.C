#include "TVectorD.h"
#include "TGraph.h"
#include "TGraphAsymmErrors.h"

#ifndef LINELEN
#define LINELEN 512
#endif

static set<string> glset_graphFilesReadIn;  // keep track of graphs read in

static map<string, TGraph *>    glmap_id2graph;

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

TGraph *getGraphFromSpec(const string& gid,
			 const string& spec)
{
  TGraph  *gr     = NULL;
  TFile *rootfile = NULL;
  vector<string> v_tokens;

  string fullspec;     // potentially expanded from aliases.
  string gspec;
  string rootfn;

  cout << "processing " << spec << endl;

  // Expand aliii first
  if (spec.find('@') != string::npos) {
    assert(0);
    string temp=spec;
    expandAliii(temp,fullspec);
    if (!fullspec.size()) return NULL;
  } else {
    fullspec = spec;
  }

  // process the (expanded) specification
  Tokenize(fullspec,v_tokens,":");
  if ((v_tokens.size() != 2) ||
      (!v_tokens[0].size())  ||
      (!v_tokens[1].size())    ) {
    cerr << "malformed root graph path file:folder/subfolder/.../graph " << fullspec << endl;
    return NULL;
  } else {
    rootfn = v_tokens[0];
    gspec  = v_tokens[1];
  }

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

    string key, value;
    if (!getKeyValue(theline,key,value)) continue;

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
	assert(0);
	string temp=path;
	expandAliii(temp,path);
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
