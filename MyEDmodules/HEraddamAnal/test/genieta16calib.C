#include <iostream>
#include <vector>
#include "TH1.h"
#include "TPad.h"
#include "TString.h"
#include "TVectorD.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "Math/Functor.h"
#include "Math/BrentMinimizer1D.h"
#include "/usr/lib/gcc/x86_64-redhat-linux/4.4.4/include/stddef.h"
#include <glob.h>
#include <cassert>
#include <cstdlib>

bool gl_verbose=false;

using namespace std;

#define LINELEN 80

//======================================================================

void loadVectorsFromFile(const char *filename, 
			 const char *scanfmt,
			 TVectorD&   vx,
			 TVectorD&   vy)
{
  char linein[LINELEN];
  vector<double> v;

  if (TString(scanfmt).Contains("%f")) {
    cerr << "Must use %lf format for doubles, sorry " << endl;
    exit(-1);
  }

  FILE *fp = fopen(filename, "r");

  if (!fp) {
    cerr << "File failed to open, " << filename << endl;
    exit(-1);
  }

  if (gl_verbose)
    cout << "Loading vectors from file " << filename
	 << " with scan format \"" << scanfmt << "\"" << endl;

  while (!feof(fp) && fgets(linein,LINELEN,fp)) {
    double x, y;
    if( linein[0]=='#' )
      continue;                // comments are welcome
    
    if( sscanf(linein, scanfmt, &x, &y) != 2 ) {
      cerr << "scan failed, file " << filename << ", line = " << linein ;
      cerr << ", scanfmt = " << scanfmt;
      cerr << ", skipping" << endl;
      //return;
      continue;
    }
    else {
      v.push_back(x); v.push_back(y);
    }
  }

  int vecsize = v.size()/2;
  vx.ResizeTo(vecsize);
  vy.ResizeTo(vecsize);

  if (gl_verbose) cout << "; read " << vecsize << " lines" << endl;

  for (int i=0; i<vecsize; i++) {
    vx[i] = v[2*i];
    vy[i] = v[2*i+1];
  }

  fclose(fp);
}                                                 // loadVectorsFromFile

//======================================================================

std::vector<TGraph *> v_graphs;

double mydistance(double p)
{
  assert(p!=0.0);
  double d2=0;
  size_t ngraphs = v_graphs.size();
  double *py1 = v_graphs[ngraphs-1]->GetY();
  for (size_t i=0; i<ngraphs-1; i++) {
    double *py2 = v_graphs[i]->GetY();
    for (int j=0; j<v_graphs[i]->GetN(); j++) {
      double d1 = (py1[j]/p - py2[j]);
      d2 += d1*d1;
    }
  }
  return d2;
}

//======================================================================

void optimizenorm(size_t i)
{
  if (v_graphs.size()==1) {
    double  *py = v_graphs[0]->GetY();
    int    npts = v_graphs[0]->GetN();
    double norm = 1./py[0];
    //cout << npts << endl;
    for (int j=0; j<npts; j++) py[j] = norm*py[j];
  } else if (v_graphs.size() > 1) {
    ROOT::Math::Functor1D func(& mydistance);
    ROOT::Math::BrentMinimizer1D bm;
    bm.SetFunction(func, 1,1500);
    bm.Minimize(1,0,0);

    cout << "f(" << bm.XMinimum() << ") = " <<bm.FValMinimum() << endl;
 
    double *py = v_graphs[i]->GetY();
    for (int j=0; j<v_graphs[0]->GetN(); j++) py[j] /= bm.XMinimum();
  }
}                                                        // optimizenorm

//======================================================================

void genieta16calib()
{
  glob_t globbuf;
  string fileglob = "_HE-16/*.txt";
  //string fileglob = "_HE16/*.txt";
  //string fileglob = "_HE-16/fcamps2012_114090194*.txt";

  int stat = glob (fileglob.c_str(), GLOB_MARK, NULL, &globbuf);
  if (stat) {
    switch (stat) {
    case GLOB_NOMATCH: cerr << "No file matching glob pattern "; break;
    case GLOB_NOSPACE: cerr << "glob ran out of memory "; break;
    case GLOB_ABORTED: cerr << "glob read error "; break;
    default: cerr << "unknown glob error stat=" << stat << " "; break;
    }
    cerr << fileglob << endl;
    exit(-1);
  }
  if (gl_verbose)
    cout<<globbuf.gl_pathc<<" files match the glob pattern"<<endl;
      
  for (size_t i=0; i<std::max((size_t)1,globbuf.gl_pathc); i++) {
    TVectorD vx,vy;

    string path = globbuf.gl_pathv[i];

    loadVectorsFromFile(path.c_str(),"%*lf %lf %*lf %*lf %*s %*s %lf",vx,vy);
    TGraph *pwg = new TGraph(vx,vy);
    pwg->SetNameTitle(path.c_str(),path.c_str());

    //pwg->Print();

    v_graphs.push_back(pwg);

    optimizenorm(i);

    //pwg->Print();
  }

  // calc average and rms
  int npts = v_graphs[0]->GetN();
  cout << npts << endl;
  int ngraphs = (int)v_graphs.size();
  TVectorD vx(npts),vyavg(npts),vyrms(npts), vxerr(npts);
  for (int i=0; i<npts; i++) {
    double x,y;
    double yavg = 0.0;
    v_graphs[0]->GetPoint(i,x,y);
    vx[i] = x;
    vxerr[i]=0;
    for (int j=0; j<ngraphs; j++) {
      v_graphs[j]->GetPoint(i,x,y);
      assert(x==vx[i]);
      yavg += y;
    }
    yavg /= (double)ngraphs;
    vyavg[i] = yavg;
    double var = 0.0;
    for (int j=0; j<ngraphs; j++) {
      v_graphs[j]->GetPoint(i,x,y);
      var += (y-yavg)*(y-yavg);
    }
    vyrms[i] = sqrt(var/(double)ngraphs);
  }

  TCanvas *c1 = new TCanvas("c1","c1",1500,600);
  v_graphs[0]->Draw("ALP");
  v_graphs[0]->GetHistogram()->GetYaxis()->SetRangeUser(0.8,1.1);
  v_graphs[0]->SetTitle("Average laser response from i#eta=-16 depth 3 over 2012, selected channels; Day # ; Arbitrary norm");
  for (int i=1; i<ngraphs; i++) {
    v_graphs[i]->Draw("LP same");
    v_graphs[i]->SetLineColor(15);
  }

  TGraphErrors *gravg = new TGraphErrors(vx,vyavg,vxerr,vyrms);

  gravg->Draw("LPE same");
  gravg->SetLineWidth(2);
  gPad->SetRightMargin(0.03);
  gPad->SetLeftMargin(0.08);
  gPad->Update();
  gPad->SaveAs("ieta-16calib.png");

  gravg->Print();
}
