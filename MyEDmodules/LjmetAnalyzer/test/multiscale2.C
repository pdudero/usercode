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
#include "TKey.h"

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
    } else
      cout << xsec << " " << integluminvpb << " " << nev << endl;

    weight = (xsec*integluminvpb)/((float)nev);
    cout << "calculated weight for file " << path << " = " << weight << endl;
    FileInfo_t fileinfo(tfile,path,xsec,nev,weight);
    v_rootfiles.push_back(fileinfo);
  }
  return 1;
}                                                         // getFileInfo

//======================================================================
// most code borrowed from hadd.C tutorial

void ScaleAll1file( TDirectory *target, FileInfo_t& source ) {

  cout << "Target path: " << target->GetPath() << endl;
  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  source.fp->cd( path );
  TDirectory *current_sourcedir = gDirectory;

  // loop over all keys in this directory

  bool newdir = true;

  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key;
  while ( (key = (TKey*)nextkey())) {

    // read object from first source file
    source.fp->cd( path );
    TObject *obj = key->ReadObj();

    if ( obj->IsA()->InheritsFrom( "TH1" ) ) {
      // descendant of TH1 -> scale it

      if (newdir) {
	newdir=false;
	cout << "Scaling histograms: " << endl;
      }

      cout << obj->GetName() << " ";
      TH1 *h1 = (TH1*)obj;

      h1->Scale(source.weight);

    } else if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      // it's a subdirectory

      newdir = true;

      cout << "\n=====> Found subdirectory " << obj->GetName();
      cout << "<=====\n" << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      ScaleAll1file( newdir, source );

    } else {

      // object is of no type that we know or can handle
      cout << "\n======> Unknown object type, name: " 
           << obj->GetName() << " title: " << obj->GetTitle();
      cout << "<======\n" << endl;
    }

    // now write the scaledd histogram (which is "in" obj) to the target file
    // note that this will just store obj in the current directory level,
    // which is not persistent until the complete directory itself is stored
    // by "target->Write()" below
    if ( obj ) {
      target->cd();

      obj->Write( key->GetName() );
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  cout << endl;

  // save modifications to target file
  target->Write();

}

//======================================================================

void multiscale2(const char* filewithpaths,
		 float integluminvpb)
{
  vector<FileInfo_t> v_rootfiles;

  if (!getFileInfo(filewithpaths, v_rootfiles,integluminvpb))
    return;

  // EXTRACT HISTOGRAMS and SCALE EACH
  for (unsigned int i=0; i<v_rootfiles.size(); i++) {

    char scalename[80];
    FileInfo_t file = v_rootfiles[i];
    char *fn = strrchr(file.path.c_str(),'/');
    if (!fn) fn = (char *)file.path.c_str();
    else fn++;
    char *ptr = strstr(fn, ".root");
    *ptr = 0;
    sprintf (scalename, "%s_%dinvpb.root", fn, (int)integluminvpb);
    cout << "Writing to " << scalename << endl;
    TFile *scaledfp = new TFile(scalename,"RECREATE");

    ScaleAll1file(scaledfp, file);
    scaledfp->Write();
    delete scaledfp;
    delete file.fp;
  }
}

