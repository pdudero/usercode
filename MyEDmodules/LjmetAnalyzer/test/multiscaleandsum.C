// Script takes output of HBHEHORecPulseAnal1.cc and reconstructs the HBHEHO
// pulse from successive TDC-indexed histograms.
//

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "TH1.h"
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
	     const char *type,
	     const char *hname,
	     HistInfo_t& histinfo)
{
  file.fp->GetObject(hname,histinfo.p);
  if (!histinfo.p) {
    cout << "WARNING: couldn't get " << hname;
    cout << " from " << file.path << endl;
    return 0;
  }

  char *fn = strrchr(file.path.c_str(),'/');
  if (!fn) fn = (char *)file.path.c_str();
  else fn++;

  histinfo.path  = std::string(hname);
  histinfo.descr = std::string(fn);
  histinfo.type  = string(type);

  histinfo.p->Scale(file.weight);

  return 1;
}                                                            // getHisto

//======================================================================

void addHisto(int i,HistInfo_t& addend,HistInfo_t& sum)
{
  if (!i) {
    sum.path  = addend.path;
    sum.type  = addend.type;
    sum.descr = addend.descr;
    sum.p     = (TH1 *)addend.p->Clone();
  }
  else
    sum.p->Add(addend.p);
}                                                            // addHisto

//======================================================================

void multiscale(const char* filewithpaths,
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

  // EXTRACT HISTOGRAMS, SCALE AND SUM
  char sumname[80];
  sprintf (sumname, "sum_%dinvpb.root", (int)integluminvpb);
  TFile *sumfp = new TFile(sumname,"RECREATE");

  while (!feof(hnamefp) && fgets(line, 256, hnamefp)) {
    if (char *newline = strchr(line,'\n')) *newline=0;

    if (line[0] == '#') continue;

    char *ptype = strtok(line,",");
    char *pname = strtok(NULL,",");

    if (!pname) continue;

    HistInfo_t sum;

    for (unsigned int i=0; i<v_rootfiles.size(); i++) {
      HistInfo_t histinfo;
      if (!getHisto(v_rootfiles[i],ptype,pname,histinfo))
	continue;
      addHisto(i,histinfo,sum);
    }
  }
  sumfp->Write();
}
