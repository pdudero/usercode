#include <fstream>
#include "TVectorD.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TGraphAsymmErrors.h"
#include "TF1.h"

#ifndef LINELEN
#define LINELEN 512
#endif

void dumpAxis(const TAxis *axis);

struct wGraph_t {
  wGraph_t() :
    lcolor(1),lstyle(1),lwidth(1),
    mcolor(1),mstyle(3),msize(1),
    fcolor(0),fstyle(1001),
    leglabel(""),drawopt(""),legdrawopt(""),
    xax(new TAxis(1,0,0)),yax(new TAxis(1,0,0)),zax(new TAxis(1,0,0)),
    gr(NULL),gr2d(NULL),fitfn(NULL),contours(NULL) {
    xax->ResetAttAxis("x"); // dumpAxis(xax);
    yax->ResetAttAxis("y");
    zax->ResetAttAxis("z");
  }
  wGraph_t(const wGraph_t& wg,const string& newname) :
    lcolor(wg.lcolor),lstyle(wg.lstyle),lwidth(wg.lwidth),
    mcolor(wg.mcolor),mstyle(wg.mstyle),msize(wg.msize),
    leglabel(wg.leglabel),drawopt(wg.drawopt),legdrawopt(wg.legdrawopt),
    xax(new TAxis(1,0,0)),yax(new TAxis(1,0,0)),zax(new TAxis(1,0,0)),
    gr(NULL), gr2d(NULL),fitfn(NULL),contours(NULL) {
    xax->ResetAttAxis("x");
    yax->ResetAttAxis("y");
    zax->ResetAttAxis("z");
    if (wg.gr)
      gr = (TGraph *)wg.gr->Clone(newname.c_str());
    else if (wg.gr2d)
      gr2d = (TGraph2D *)wg.gr2d->Clone(newname.c_str());
    }
  int  lcolor,lstyle,lwidth;
  int  mcolor,mstyle,msize;
  int  fcolor,fstyle;
  string leglabel;
  string drawopt;
  string legdrawopt;
  TAxis *xax,*yax,*zax; /* have to save separately since the internal graph
			   axes are not created until after being drawn. */
  TGraph *gr;
  TGraph2D *gr2d;
  TF1 *fitfn;
  TVectorD *contours;
};

static set<string> glset_graphFilesReadIn;  // keep track of graphs read in

static map<string, wGraph_t *>    glmap_id2graph;

//======================================================================

wGraph_t *findGraph(const string& gid, const string& errmsg="")
{
  map<string,wGraph_t *>::const_iterator it = glmap_id2graph.find(gid);
  if( it == glmap_id2graph.end() ) {
    // Try finding the first
    cerr << "Graph ID " << gid << " not found. " << errmsg << endl;
    if( gl_verbose) {
      cout << "Available graph IDs are: " << endl;
      for (it = glmap_id2graph.begin(); it != glmap_id2graph.end(); it++)
	cout << it->first << " ";
      cout << endl;
    }
    return NULL;
  }
  return it->second;
}                                                           // findGraph

//======================================================================

void dumpAxis(const TAxis *axis)
{
  cout << "Dumping axis:" << endl;
  cout << "AxisTitle   = " << axis->GetTitle() << endl;
  cout << "Ndivisions  = " << axis->GetNdivisions() << endl;
  cout << "AxisColor   = " << axis->GetAxisColor() << endl;
  cout << "LabelColor  = " << axis->GetLabelColor() << endl;
  cout << "LabelFont   = " << axis->GetLabelFont() << endl;
  cout << "LabelOffset = " << axis->GetLabelOffset() << endl;
  cout << "LabelSize   = " << axis->GetLabelSize() << endl;
  cout << "TickLength  = " << axis->GetTickLength() << endl;
  cout << "TitleOffset = " << axis->GetTitleOffset() << endl;
  cout << "TitleSize   = " << axis->GetTitleSize() << endl;
  cout << "TitleColor  = " << axis->GetTitleColor() << endl;
  cout << "TitleFont   = " << axis->GetTitleFont() << endl;
  cout << axis->TestBit(TAxis::kCenterTitle) << endl;
  cout << axis->TestBit(TAxis::kCenterLabels) << endl;
  cout << axis->TestBit(TAxis::kRotateTitle) << endl;
  cout << axis->TestBit(TAxis::kNoExponent) << endl;
  cout << axis->TestBit(TAxis::kTickPlus) << endl;
  cout << axis->TestBit(TAxis::kTickMinus) << endl;
  cout << axis->TestBit(TAxis::kMoreLogLabels) << endl;
}

//======================================================================

void loadVectorsFromFile(const char *filename, 
			 const char *scanfmt,
			 TVectorD&   vx,
			 TVectorD&   vy,
			 char        xheader[],
			 char        yheader[],
			 char        title[],
			 const char *titlescanfmt="")
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

  // Read in to std::vector, transfer to TVectorD after -
  // because TVectorT doesn't have a push_back equivalent!
  //
  bool firstline=true;
  while (!feof(fp) && fgets(linein,LINELEN,fp)) {
    double x, y;
    if( linein[0]=='#' ) {
      if (!v.size()) {           // first line, try to read headers
	TString hscanfmt(scanfmt);
	hscanfmt.ReplaceAll("lf","s");
	hscanfmt.ReplaceAll("f","s");
	int n=sscanf(&linein[1],hscanfmt.Data(),xheader,yheader);
	//int n=sscanf(&linein[1],"%s %*s %*s %*s %s",xh,yh);
	//cout << n << " " << hscanfmt << " " << TString(&linein[1]) << endl;
	if( n != 2 ) {
	  cerr << "failed to read in column headers" << endl;
	}
      }
      continue;                // comments are welcome
    }

    if (firstline && strlen(titlescanfmt)) {
      sscanf(linein, titlescanfmt, title);
      firstline=false;
    }
    
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

void loadVectorsFromFile(const char *filename, 
			 TVectorD&   vx,
			 TVectorD&   vy,
			 TVectorD&   vz)
{
  char linein[LINELEN];
  vector<double> v;

  FILE *fp = fopen(filename, "r");

  if (!fp) {
    cerr << "File failed to open, " << filename << endl;
    return;
  }

  if (gl_verbose) cout << "Loading vectors from file " << filename;

  // Read in to std::vector, transfer to TVectorD after -
  // because TVectorT doesn't have a push_back equivalent!
  //
  while (!feof(fp) && fgets(linein,LINELEN,fp)) {
    double x, y, z;
    if( linein[0]=='#' ) continue;                // comments are welcome
    if (sscanf(linein, "%lf %lf %lf", &x, &y, &z) != 3) {
      cerr << "scan failed, file " << filename << ", line = " << linein;
      cerr << ", skipping" << endl;
      //return;
      continue;
    }
    else {
      v.push_back(x); v.push_back(y); v.push_back(z);
    }
  }

  int vecsize = v.size()/3;
  vx.ResizeTo(vecsize);
  vy.ResizeTo(vecsize);
  vz.ResizeTo(vecsize);

  if (gl_verbose) cout << "; read " << vecsize << " lines" << endl;

  for (int i=0; i<vecsize; i++) {
    vx[i] = v[2*i];
    vy[i] = v[2*i+1];
    vz[i] = v[2*i+2];
  }
  fclose(fp);
}                                                 // loadVectorsFromFile

//======================================================================

void loadVectorsFromFile(const char *filename, 
			 const char *scanfmt,
			 TVectorD&   vx, TVectorD&   vy,
			 TVectorD&   ex, TVectorD&   ey
			 )
{
  char linein[LINELEN];
  vector<double> v;

  FILE *fp = fopen(filename, "r");

  if (!fp) {
    cerr << "File failed to open, " << filename << endl;
    return;
  }

  if (gl_verbose) cout << "Loading vectors from file " << filename;

  // Read in to std::vector, transfer to TVectorD after -
  // because TVectorT doesn't have a push_back equivalent!
  //
  while (!feof(fp) && fgets(linein,LINELEN,fp)) {
    double x, xerr, y, yerr;
    if( linein[0]=='#' ) continue;                // comments are welcome
    if (sscanf(linein, scanfmt, &x, &y, &xerr, &yerr) != 4) {
      cerr << "scan failed, file " << filename << ", line = " << linein << endl;
      return;
    }
    else {
      v.push_back(x);    v.push_back(y);
      v.push_back(xerr); v.push_back(yerr);
    }
  }

  int vecsize = v.size()/4;
  vx.ResizeTo(vecsize);
  vy.ResizeTo(vecsize);
  ex.ResizeTo(vecsize);
  ey.ResizeTo(vecsize);

  if (gl_verbose) cout << "; read " << vecsize << " lines" << endl;

  for (int i=0; i<vecsize; i++) {
    vx[i] = v[4*i];
    vy[i] = v[4*i+1];
    ex[i] = v[4*i+2];
    ey[i] = v[4*i+3];
  }
  fclose(fp);
}                                                 // loadVectorsFromFile

//======================================================================

void loadVectorsFromFile(const char *filename, 
			 const char *scanfmt,
			 TVectorD&   vx, TVectorD&   vy,
			 TVectorD&   exl,TVectorD&   exh,
			 TVectorD&   eyl,TVectorD&   eyh
			 )
{
  char linein[LINELEN];
  vector<double> v;

  FILE *fp = fopen(filename, "r");

  if (!fp) {
    cerr << "File failed to open, " << filename << endl;
    return;
  }

  if (gl_verbose) cout << "Loading vectors from file " << filename;

  // Read in to std::vector, transfer to TVectorD after -
  // because TVectorT doesn't have a push_back equivalent!
  //
  while (!feof(fp) && fgets(linein,LINELEN,fp)) {
    double x, xloerr, xhierr, y, yloerr,yhierr;
    if( linein[0]=='#' ) continue;                // comments are welcome
    if (sscanf(linein, scanfmt, &x, &xloerr, &xhierr, &y, &yloerr, &yhierr) != 6) {
      cerr << "scan failed, file " << filename << ", line = " << linein << endl;
      return;
    }
    else {
      v.push_back(x);      v.push_back(y);
      v.push_back(xloerr); v.push_back(xhierr);
      v.push_back(yloerr); v.push_back(yhierr);
    }
  }

  int vecsize = v.size()/6;
  vx.ResizeTo(vecsize);
  vy.ResizeTo(vecsize);
  exl.ResizeTo(vecsize);
  exh.ResizeTo(vecsize);
  eyl.ResizeTo(vecsize);
  eyh.ResizeTo(vecsize);

  if (gl_verbose) cout << "; read " << vecsize << " lines" << endl;

  for (int i=0; i<vecsize; i++) {
    vx[i]  = v[6*i];
    vy[i]  = v[6*i+1];
    exl[i] = v[6*i+2];
    exh[i] = v[6*i+3];
    eyl[i] = v[6*i+4];
    eyh[i] = v[6*i+5];
  }
  fclose(fp);
}                                                 // loadVectorsFromFile

//======================================================================

wGraph_t *loadVectorsByType(bool symerrors,
			    bool asymerrors,
			    const string& path,
			    const string& scanspec,
			    const string& titlescanspec=""
			    )
{
  wGraph_t *pwg = new wGraph_t();
  if (symerrors) {
    TVectorD vx,vy,ex,ey;
    loadVectorsFromFile(path.c_str(),scanspec.c_str(),vx,vy,ex,ey);
    pwg->gr = new TGraphErrors(vx,vy,ex,ey);
  } else if (asymerrors) {
    TVectorD vx,vy,exl,exh,eyl,eyh;
    loadVectorsFromFile(path.c_str(),scanspec.c_str(),vx,vy,exl,exh,eyl,eyh);
    pwg->gr = new TGraphAsymmErrors(vx,vy,exl,exh,eyl,eyh);
  } else {
    TVectorD vx,vy;
    char xheader[80],yheader[80],ctitle[80];
    xheader[0]=0;
    yheader[0]=0;
    ctitle[0]=0;
    loadVectorsFromFile(path.c_str(),scanspec.c_str(),vx,vy,
		      xheader,yheader,ctitle,titlescanspec.c_str());
    pwg->gr = new TGraph(vx,vy);
    if (strlen(xheader)) pwg->xax->SetTitle(xheader);
    if (strlen(yheader)) pwg->yax->SetTitle(yheader);
    if (strlen(ctitle))  pwg->gr->SetTitle("(HE "+TString(ctitle));
  }
  return pwg;
}                                                   // loadVectorsByType

//======================================================================

void
printVectorsToFile(wGraph_t *wg, const string& filename)
{
  ofstream of(filename.c_str());

  if (wg->gr->InheritsFrom("TGraphAsymmErrors")) {
    TGraphAsymmErrors *agr = (TGraphAsymmErrors *)wg->gr;
    of<<"#lox\tctrx\thix\tloy\tctry\thiy\texlo\texhi\teylo\teyhi"<<endl;
    for (int i=0; i<agr->GetN(); i++) {
      double x,y, exlo,exhi,eylo,eyhi, lox,hix,loy,hiy;
      agr->GetPoint(i,x,y);
      exlo = agr->GetErrorXlow(i);
      exhi = agr->GetErrorXhigh(i);
      eylo = agr->GetErrorYlow(i);
      eyhi = agr->GetErrorYhigh(i);
      lox = x-exlo; hix = x+exhi;
      loy = y-eylo; hiy = y+eyhi;
      of <<lox<<"\t"<<x<<"\t"<<hix<<"\t";
      of <<loy<<"\t"<<y<<"\t"<<hiy<<"\t";
      of <<exlo<<"\t"<<exhi<<"\t"<<eylo<<"\t"<<eyhi<<"\t"<<endl;
    }
  } else if (wg->gr->InheritsFrom("TGraphErrors")) {
    TGraphErrors *egr = (TGraphErrors *)wg->gr;
    of<<"#ctrx\tctry\tex\tey"<<endl;
    for (int i=0; i<egr->GetN(); i++) {
      double x,y, ex,ey;
      egr->GetPoint(i,x,y);
      ex = egr->GetErrorX(i);
      ey = egr->GetErrorY(i);
      of <<x<<"\t"<<ex<<"\t";
      of <<y<<"\t"<<ey<<"\t"<<endl;
    }
  } else {
    of<<"#x\ty"<<endl;
    for (int i=0; i<wg->gr->GetN(); i++) {
      double x,y;
      wg->gr->GetPoint(i,x,y);
      of<<x<<"\t"<<y<<endl;
    }
  }
  of.close();
}                                                  // printVectorsToFile

//======================================================================

TGraph *getGraphFromSpec(const string& gid,
			 const string& fullspec) // alias expansion assumed
{
  TGraph  *gr     = NULL;
  TFile *rootfile = NULL;
  vector<string> v_tokens;

  string gspec;
  string rootfn;

  if (gl_verbose) cout << "processing " << fullspec << endl;

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

  map<string,string>::const_iterator it = glmap_objpath2id.find(fullspec);
  if (it != glmap_objpath2id.end()) {
    // Allow the possibility to run the script a second time in root
    if (gl_verbose) cout << "Object " << fullspec << " already read in, here it is" << endl;
    map<string,wGraph_t *>::const_iterator git = glmap_id2graph.find(it->second);
    if (git == glmap_id2graph.end()) {
      if (gl_verbose) cout << "oops, sorry, I lied." << endl;
      return NULL;
    }
    gr = git->second->gr;
  } else {
    rootfile = openRootFile(rootfn);
    if (rootfile) {
      gr = (TGraph *)rootfile->Get(gspec.c_str());
      if (!gr) {
	cerr << "couldn't find " << gspec << " in " << rootfn << endl;
      } else {
	// success, record that you read it in.
	glmap_objpath2id.insert(pair<string,string>(fullspec,gid));
      }
    }
  }
  return gr;
}                                                    // getGraphFromSpec

//======================================================================
// takes the ID of a graph to fill into a pre-booked histo
//
void fill1DHistoFromGraph(std::string& gid,
			  wTH1 *&wth1)
{
  map<string, wGraph_t *>::const_iterator it=glmap_id2graph.find(gid);
  if (it==glmap_id2graph.end()) {
    cerr<<"Couldn't find graph with id "<<gid<<", define first"<<endl;
    exit(-1);
  }

  if (gl_verbose)
    cout << "Loading histo from graph " << gid << endl;

  TH1 *h = wth1->histo();

  for (int ibin=1; ibin <= h->GetNbinsX(); ibin++) {
    h->SetBinContent(ibin,it->second->gr->Eval(h->GetBinCenter(ibin)));
    if (gl_verbose)
      printf ("%d %f %g\n", ibin, 
	      h->GetBinCenter(ibin), 
	      it->second->gr->Eval(h->GetBinCenter(ibin)));
  }

}                                               //  fill1DHistoFromGraph

//======================================================================

wGraph_t * myBayesDivide(TH1 *numer,TH1 *denom)
{
  cout << numer->GetNbinsX() << " " << denom->GetNbinsX() << endl;

  if ( gl_verbose ) { 
    std::cout << "Dump of bin contents, errors" << std::endl ; 
    if ( numer->GetNbinsX() == denom->GetNbinsX() ) { 
      for (int ib=1; ib<=numer->GetNbinsX(); ib++) {
	std::cout << ib << ": " << numer->GetBinContent(ib) << "+/-" << numer->GetBinError(ib);
	std::cout << ", "       << denom->GetBinContent(ib) << "+/-" << denom->GetBinError(ib);
	std::cout << std::endl ; 
      }
    } else { 
      cerr << "Histograms being divided do not have same number of bins!!!" << endl ; 
      return NULL;
    }
  }

  wGraph_t * pwg = new wGraph_t();
      
  // equivalent to BayesDivide
  //
  if (gl_verbose) pwg->gr = new TGraphAsymmErrors(numer,denom,"debug");
  else            pwg->gr = new TGraphAsymmErrors(numer,denom,"");
  //if (gl_verbose) pwg->gr = new TGraphAsymmErrors(numer,denom,"cl=0.683 b(1,1) mode v");
  //else            pwg->gr = new TGraphAsymmErrors(numer,denom,"cl=0.683 b(1,1) mode");
  if (!pwg->gr) {
    cerr << "BayesDivide didn't work! wonder why..." << endl;
    return NULL;
  } else if (gl_verbose) {
    cout << pwg->gr->GetN() << " points in the graph" << endl;
  }

  // Fix in case something broke

  for (int i=0; i<pwg->gr->GetN(); i++) {
    if ( pwg->gr->GetErrorYhigh(i) == 0. || 
	 pwg->gr->GetErrorYlow(i)  == 0 )  {           // Something bad happened
      if ( gl_verbose )
	std::cout << "Problem with Bayes divide, checking..." << std::endl ;
      double pass  = numer->GetBinContent(i+1) ; 
      double total = denom->GetBinContent(i+1) ;
      if ( gl_verbose ) std::cout << pass << "/" << total << std::endl ;
      if ( pass == total ) {
	if ( gl_verbose ) std::cout << "Everything OK" << std::endl ;
      } else { 
	if ( gl_verbose ) std::cout << "Yep, something is broken" << std::endl ;
	double xval, yval ;
	pwg->gr->GetPoint(i,xval,yval) ;
	yval = pass / total ;
	// Use simplified efficiency assumption
	// double u1 = numer->GetBinError(i+1) / numer->GetBinContent(i+1) ; 
	// double u2 = denom->GetBinError(i+1) / denom->GetBinContent(i+1) ; 
	// double unc = yval * sqrt( u1*u1 + u2*u2 ) ; 
	double unc = sqrt( yval * (1.-yval)/denom->GetBinContent(i+1) ) ; 
	double uhi = ( (yval + unc > 1.)?(1.-yval):(unc) ) ; 
	double ulo = ( (yval - unc < 0.)?(yval):(unc) ) ;
	pwg->gr->SetPoint(i,xval,yval) ;
	((TGraphAsymmErrors*)pwg->gr)->SetPointError(i,pwg->gr->GetErrorXlow(i),
						     pwg->gr->GetErrorXhigh(i),ulo,uhi) ; 
	//                   pwg->gr->SetPointEYhigh(i,uhi) ; 
	//                   pwg->gr->SetPointEYlow(i,ulo) ; 
      }
    }   
    if (gl_verbose)
      std::cout<<i<<": "<<pwg->gr->GetErrorYhigh(i)<<"/"<<pwg->gr->GetErrorYlow(i)<<std::endl ; 
  }
  return pwg;
}                                                       // myBayesDivide

//======================================================================

wGraph_t *generateROCgraph(TH1 *sgnl,TH1 *bkgd,const char *direction=">")
{
  cout << sgnl->GetNbinsX() << " " << bkgd->GetNbinsX() << endl;

  if ( sgnl->GetNbinsX() != bkgd->GetNbinsX() ) { 
    cerr << "Histograms being divided do not have same number of bins!!!" << endl ; 
    return NULL;
  }

  int nbins = sgnl->GetNbinsX();

  wGraph_t * pwg = new wGraph_t();

  TH1 *sigeff = (TH1 *)sgnl->Clone("sigeff"); sigeff->Scale(1./sigeff->Integral(0,nbins+1));
  TH1 *bkgeff = (TH1 *)bkgd->Clone("sigeff"); bkgeff->Scale(1./bkgeff->Integral(0,nbins+1));

  if (direction[0]=='<') {
    sigeff = IntegrateLeft(sigeff);
    bkgeff = IntegrateLeft(bkgeff);
  } else if (direction[0]=='>') {
    sigeff = IntegrateRight(sigeff);
    bkgeff = IntegrateRight(bkgeff);
  } else {
    cerr << "Unknown direction " << direction << endl;
    return NULL;
  }

  cout << "sigeff underflow: " << sigeff->GetBinContent(0) << endl;
  cout << "bkgeff underflow: " << bkgeff->GetBinContent(0) << endl;

  pwg->gr = new TGraph(nbins);
  for (int ib=1; ib<=nbins; ib++) {
    pwg->gr->SetPoint(ib-1,sigeff->GetBinContent(ib),(1-bkgeff->GetBinContent(ib)));
    printf ("%d %f %f %f\n", ib-1, 
	    sigeff->GetXaxis()->GetBinCenter(ib), 
	    sigeff->GetBinContent(ib), 
	    1-bkgeff->GetBinContent(ib));
  }
  return pwg;
}                                                    // generateROCgraph

//======================================================================

wGraph_t *generateSeffOverBeffGraph(TH1 *sgnl,TH1 *bkgd,const char *direction=">")
{
  cout << sgnl->GetNbinsX() << " " << bkgd->GetNbinsX() << endl;

  if ( sgnl->GetNbinsX() != bkgd->GetNbinsX() ) { 
    cerr << "Histograms being divided do not have same number of bins!!!" << endl ; 
    return NULL;
  }

  int nbins = sgnl->GetNbinsX();

  wGraph_t * pwg = new wGraph_t();

  TH1 *sigeff = (TH1 *)sgnl->Clone("sigeff"); sigeff->Scale(1./sigeff->Integral(0,nbins+1));
  TH1 *bkgeff = (TH1 *)bkgd->Clone("sigeff"); bkgeff->Scale(1./bkgeff->Integral(0,nbins+1));

  if (direction[0]=='<') {
    sigeff = IntegrateLeft(sigeff);
    bkgeff = IntegrateLeft(bkgeff);
  } else if (direction[0]=='>') {
    sigeff = IntegrateRight(sigeff);
    bkgeff = IntegrateRight(bkgeff);
  } else {
    cerr << "Unknown direction " << direction << endl;
    return NULL;
  }

  cout << "sigeff underflow: " << sigeff->GetBinContent(0) << endl;
  cout << "bkgeff underflow: " << bkgeff->GetBinContent(0) << endl;

  pwg->gr = new TGraph(nbins);
  int npt=0;
  for (int ib=1; ib<=nbins; ib++,npt++) {
    if (bkgeff->GetBinContent(ib)) {
      pwg->gr->SetPoint(npt,sigeff->GetBinContent(ib),(sigeff->GetBinContent(ib)/
						       bkgeff->GetBinContent(ib)));
      printf ("%d %f %f %f\n", npt, 
	      sigeff->GetXaxis()->GetBinCenter(ib), 
	      sigeff->GetBinContent(ib), 
	      sigeff->GetBinContent(ib)/bkgeff->GetBinContent(ib)
	      );
    }
  }
  return pwg;
}                                           // generateSeffOverBeffGraph

//======================================================================

wGraph_t *generateSoverBgraph(TH1 *sgnl,TH1 *bkgd,const char *direction=">")
{
  cout << sgnl->GetNbinsX() << " " << bkgd->GetNbinsX() << endl;

  if ( sgnl->GetNbinsX() != bkgd->GetNbinsX() ) { 
    cerr << "Histograms being divided do not have same number of bins!!!" << endl ; 
    return NULL;
  }

  int nbins = sgnl->GetNbinsX();

  wGraph_t * pwg = new wGraph_t();

  TH1 *sigint = (TH1 *)sgnl->Clone("sigint");
  TH1 *bkgint = (TH1 *)bkgd->Clone("sigint");

  if (direction[0]=='<') {
    sigint = IntegrateLeft(sigint);
    bkgint = IntegrateLeft(bkgint);
  } else if (direction[0]=='>') {
    sigint = IntegrateRight(sigint);
    bkgint = IntegrateRight(bkgint);
  } else {
    cerr << "Unknown direction " << direction << endl;
    return NULL;
  }

  pwg->gr = new TGraph(nbins);
  for (int ib=1; ib<=nbins; ib++) {
    double bkg = bkgint->GetBinContent(ib);
    double sig = sigint->GetBinContent(ib);
    pwg->gr->SetPoint(ib-1,sigint->GetXaxis()->GetBinCenter(ib), (bkg>0.00) ? sig/bkg : 0.0 );

    printf ("%d %f %f %f %f\n", ib-1, 
	    sigint->GetXaxis()->GetBinCenter(ib), 
	    sig,bkg,(bkg>0.00)?sig/bkg:0.0 );
  }
  return pwg;
}                                                 // generateSoverBgraph

//======================================================================

wGraph_t *generateSoverErrBgraph(TH1 *sgnl,TH1 *bkgd,const char *direction=">")
{
  cout << sgnl->GetNbinsX() << " " << bkgd->GetNbinsX() << endl;

  if ( sgnl->GetNbinsX() != bkgd->GetNbinsX() ) { 
    cerr << "Histograms being divided do not have same number of bins!!!" << endl ; 
    return NULL;
  }

  int nbins = sgnl->GetNbinsX();

  wGraph_t * pwg = new wGraph_t();

  TH1 *sigint = (TH1 *)sgnl->Clone("sigint");
  TH1 *bkgint = (TH1 *)bkgd->Clone("sigint");

  if (direction[0]=='<') {
    sigint = IntegrateLeft(sigint);
    bkgint = IntegrateLeft(bkgint);
  } else if (direction[0]=='>') {
    sigint = IntegrateRight(sigint);
    bkgint = IntegrateRight(bkgint);
  } else {
    if (gl_verbose)
      cout << "No integration mode " << direction << endl;
  }

  pwg->gr = new TGraphErrors(nbins);
  printf ("%5s\t%10s\t%10s\t%10s\t%10s\t%10s\t%10s\t%10s\n",
	  "bin","Xval","Xerr","sigYval","bkgYval","bkgYerr","sig/bkgerr","sigerr/bkgerr");
  for (int ib=1; ib<=nbins; ib++) {
    double bkg    = bkgint->GetBinContent(ib);
    double bkgerr = bkgint->GetBinError(ib); bkgerr = sqrt(bkg + (bkgerr*bkgerr));
    double sig    = sigint->GetBinContent(ib);
    double sigerr = sigint->GetBinError(ib); sigerr = sqrt(sig + (sigerr*sigerr));
    pwg->gr->SetPoint(ib-1,sigint->GetXaxis()->GetBinCenter(ib), (bkgerr>0.00)?sig/bkgerr:0.0 );
    ((TGraphErrors *)(pwg->gr))->SetPointError(ib-1,
					       sigint->GetXaxis()->GetBinWidth(ib)/2,
					       sigerr/bkgerr); // treat bkgerr as errorless(!)

    if (gl_verbose)
      printf ("%5d\t%10f\t%10g\t%10g\t%10g\t%10g\t%10g\t%10g\n", ib-1, 
	      sigint->GetXaxis()->GetBinCenter(ib), 
	      sigint->GetXaxis()->GetBinWidth(ib)/2,
	      sig,bkg,bkgerr,
	      (bkgerr>0.0)?sig/bkgerr:0.0,
	      (bkgerr>0.0)?sigerr/bkgerr:0.0);

  }
  return pwg;
}                                              // generateSoverErrBgraph

//======================================================================

bool                              // returns true if success
processGraphSection(FILE *fp,
		    string& theline,
		    bool& new_section)
{
  vector<string> v_tokens;
  TString  title;
  TString  name;
  string   *gid  = NULL;
  float xmin=0.,xmax=0.,ymin=0.,ymax=0.,zmin=0.,zmax=0.;
  bool symerrors  = false;
  bool asymerrors = false;
  bool printvecs  = false;
  bool savegraph  = false;
  string save2file,printfile;
  wGraph_t wg;                                    // placeholder for read-in values
  vector<std::pair<string, wGraph_t *> > v_graphs;

  if (gl_verbose) cout << "Processing graph section" << endl;

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

      if (v_graphs.size()) {
	cerr << "graph(s) already defined" << endl; continue;
      }

      string path     = value;
      string scanspec = 
	asymerrors ? "%lf %lf %lf %lf %lf %lf" :
	(symerrors ? "%lf %lf %lf %lf" : "%lf %lf");
      string titlescanspec;

      Tokenize(value,v_tokens,",\"");
      if (v_tokens.size() > 1) {
	path     = v_tokens[0];
	scanspec = v_tokens[1];
	if (v_tokens.size() > 2)
	  titlescanspec = v_tokens[2];
      }

      // allow fileglobs
      if (path.find("*") == string::npos) {
	wGraph_t *pwg = loadVectorsByType(symerrors,asymerrors,path,scanspec,titlescanspec);
	v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));
      } else { // glob found
	glob_t globbuf;
	string fileglob = path;

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
	  string gidi = *gid;
	  if (globbuf.gl_pathc) {
	    path = globbuf.gl_pathv[i];
	    gidi = *gid + "_" + int2str(i);
	  }

	  if (inSet<string>(glset_graphFilesReadIn,path)) {
	    cerr << "vector file " << path << " already read in" << endl; continue;
	  }

	  wGraph_t *pwg = loadVectorsByType(symerrors,asymerrors,path,scanspec);
	  v_graphs.push_back(pair<string,wGraph_t *>(gidi,pwg));
	} // glob loop

	if (globbuf.gl_pathc) 
	  globfree(&globbuf);

	glmap_mobj2size.insert(pair<string,unsigned>(*gid,v_graphs.size()));
      } // if glob found

    //------------------------------
    } else if (key == "vectorfile2d") {
    //------------------------------
      if (!gid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (v_graphs.size()) {
	cerr << "graph(s) already defined" << endl; continue;
      }

      Tokenize(value,v_tokens,",");

      string path=v_tokens[0];
      if (inSet<string>(glset_graphFilesReadIn,path)) {
	cerr << "vector file " << path << " already read in" << endl; break;
      }

      wGraph_t *pwg = new wGraph_t();

      if (gl_verbose)
	cout << "Reading " << path << endl;

      switch(v_tokens.size()) {
      case 1:  pwg->gr2d = new TGraph2D(path.c_str()); break;
      case 2:  pwg->gr2d = new TGraph2D(path.c_str(),v_tokens[1].c_str()); break;
      case 3:  pwg->gr2d = new TGraph2D(path.c_str(),v_tokens[1].c_str(),v_tokens[2].c_str()); break;
      default:
	cerr << "malformed vectorfile2d spec path[,format[,option]] " << value << endl;
	break;
      }

      if (pwg->gr2d->IsZombie()) {
	cerr << "Unable to make Graph2D from file " << path << endl;
	exit(-1);
      }
      pwg->gr2d->SetName(gid->c_str());

      if (gl_verbose)
	pwg->gr2d->Print();

      v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));

    //------------------------------
    } else if (key == "path") {
    //------------------------------

      if (!gid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (v_graphs.size()) {
	cerr << "graph already defined" << endl; continue;
      }
      wGraph_t *pwg = new wGraph_t();
      pwg->gr  = getGraphFromSpec(*gid,value);
      if (!pwg->gr) continue;

      v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));

    //------------------------------
    } else if (key == "clone") {
    //------------------------------

      if (!gid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (v_graphs.size()) {
	cerr << "graph already defined" << endl; continue;
      }
      map<string,wGraph_t *>::const_iterator it = glmap_id2graph.find(value);
      if( it == glmap_id2graph.end() ) {
	cerr << "Graph ID " << value << " not found,";
	cerr << "clone must be defined after the clonee" << endl;
	break;
      }
      wGraph_t *pwg  = new wGraph_t(*(it->second),*gid);

      v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));

    //------------------------------
    } else if( key == "fromhisto" ) { // converts TH1 to TGraph
    //------------------------------
      if( !gid ) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (v_graphs.size()) {
	cerr << "graph(s) already defined" << endl; continue;
      }
      // look for multihist with this identifier
      std::map<string,unsigned>::const_iterator it=glmap_mobj2size.find(value);
      if (it!=glmap_mobj2size.end()) {
	for (size_t i=0; i<it->second; i++) {
	  string hidi=value+int2str(i);
	  TH1 *h = (TH1 *)findHisto(hidi,"");
	  assert(h);
	  wGraph_t *pwg = new wGraph_t();
	  pwg->gr = new TGraph(h);
	  v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));
	}
      } else {
	TH1 *h = (TH1 *)findHisto(value);
	assert(h);
	wGraph_t *pwg = new wGraph_t();
	pwg->gr = new TGraph(h);
	v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));
      }

    //------------------------------
    } else if( key == "fillfromtree" ) { // converts tree array variables into a group of graphs
    //------------------------------
      if( !gid ) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (v_graphs.size()) {
	cerr << "graph(s) already defined" << endl; continue;
      }

      // see if a trailing range spec has been given
      Tokenize(value,v_tokens,";");
      string treedrawspec=v_tokens[0];

      int ifirst=-1,ilast=-1;
      if (v_tokens.size() == 2) {
	string range=v_tokens[1];
	Tokenize(range,v_tokens,"-");
	if (v_tokens.size()==2) {
	  ifirst=str2int(v_tokens[0]);
	  ilast =str2int(v_tokens[1]);
	}
      }
      //cout << v_tokens.size() << " " << ifirst << " " << ilast << endl;
      if (ifirst>0 && 
	  ilast>=ifirst ) {
	for (int i=ifirst; i<=ilast; i++) {
	  wGraph_t *pwg = NULL;

	  // defined in spTree.C
	  void fillGraphFromTreeVar(std::string& drawspec,int index,wGraph_t *&pwg);
	  fillGraphFromTreeVar(treedrawspec,i,pwg);
	  assert(pwg);
	  string gidi= (*gid)+"_"+int2str(i-ifirst);
	  v_graphs.push_back(std::pair<string,wGraph_t *>(gidi,pwg));
	}

	glmap_mobj2size.insert(pair<string,unsigned>(*gid,v_graphs.size()));
      } else {
	wGraph_t *pwg = NULL;
	void fillGraphFromTreeVar(std::string& drawspec,int index,wGraph_t *&pwg);
	fillGraphFromTreeVar(treedrawspec,0,pwg);
	assert(pwg);
	v_graphs.push_back(std::pair<string,wGraph_t *>(*gid,pwg));
	glmap_mobj2size.insert(pair<string,unsigned>(*gid,v_graphs.size()));
      }

    //------------------------------
    } else if (key == "bayesdiv") {
    //------------------------------

      Tokenize(value,v_tokens,",/"); // either comma-separated or using '/'
      if (v_tokens.size() != 2) {
	cerr << "expect comma-separated list of exactly two histo specs to divide! ";
	cerr << theline << endl;
	continue;
      }

      TH1 *tmph1 = (TH1 *)findHisto(v_tokens[0]); if (!tmph1) exit(-1);
      TH1 *tmph2 = (TH1 *)findHisto(v_tokens[1]); if (!tmph2) exit(-1);

      wGraph_t *pwg = myBayesDivide(tmph1,tmph2);
      if (!pwg) exit(-1);

      v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));
      glmap_mobj2size.insert(pair<string,unsigned>(*gid,v_graphs.size()));

    //------------------------------
    } else if (key == "generateROC") {
    //------------------------------

      Tokenize(value,v_tokens,",;");
      if ((v_tokens.size() != 2) && 
	  (v_tokens.size() != 3) ) {
	cerr << "expect 'sighistoid1,bakhistoid2[;direction], where dir=> or dir=<";
	cerr << theline << endl;
	continue;
      }

      TH1 *tmph1 = (TH1 *)findHisto(v_tokens[0]); if (!tmph1) exit(-1);
      TH1 *tmph2 = (TH1 *)findHisto(v_tokens[1]); if (!tmph2) exit(-1);

      wGraph_t *pwg=NULL;
      if (v_tokens.size()==3)
	pwg = generateROCgraph(tmph1,tmph2,v_tokens[2].c_str());
      else
	pwg = generateROCgraph(tmph1,tmph2);
      if (!pwg) exit(-1);

      v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));
      glmap_mobj2size.insert(pair<string,unsigned>(*gid,v_graphs.size()));

    //------------------------------
    } else if (key == "generateSoverB") {
    //------------------------------

      Tokenize(value,v_tokens,",;");
      if ((v_tokens.size() != 2) && 
	  (v_tokens.size() != 3) ) {
	cerr << "expect 'sighistoid1,bakhistoid2[;direction], where dir=> or dir=<";
	cerr << theline << endl;
	continue;
      }

      TH1 *tmph1 = (TH1 *)findHisto(v_tokens[0]); if (!tmph1) exit(-1);
      TH1 *tmph2 = (TH1 *)findHisto(v_tokens[1]); if (!tmph2) exit(-1);

      wGraph_t *pwg=NULL;
      if (v_tokens.size()==3)
	pwg = generateSoverBgraph(tmph1,tmph2,v_tokens[2].c_str());
      else
	pwg = generateSoverBgraph(tmph1,tmph2);
      if (!pwg) exit(-1);

      v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));
      glmap_mobj2size.insert(pair<string,unsigned>(*gid,v_graphs.size()));

    //------------------------------
    } else if (key == "generateSoverErrB") {
    //------------------------------

      Tokenize(value,v_tokens,",;");
      if ((v_tokens.size() != 2) && 
	  (v_tokens.size() != 3) ) {
	cerr << "expect 'sighistoid1,bakhistoid2[;direction], where dir=> or dir=<";
	cerr << theline << endl;
	continue;
      }

      TH1 *tmph1 = (TH1 *)findHisto(v_tokens[0]); if (!tmph1) exit(-1);
      TH1 *tmph2 = (TH1 *)findHisto(v_tokens[1]); if (!tmph2) exit(-1);

      wGraph_t *pwg=NULL;
      if (v_tokens.size()==3)
	pwg = generateSoverErrBgraph(tmph1,tmph2,v_tokens[2].c_str());
      else
	pwg = generateSoverErrBgraph(tmph1,tmph2);
      if (!pwg) exit(-1);

      v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));
      glmap_mobj2size.insert(pair<string,unsigned>(*gid,v_graphs.size()));

    //------------------------------
    } else if (key == "generateSeffoverBeff") {
    //------------------------------

      Tokenize(value,v_tokens,",;");
      if ((v_tokens.size() != 2) && 
	  (v_tokens.size() != 3) ) {
	cerr << "expect 'sighistoid1,bakhistoid2[;direction], where dir=> or dir=<";
	cerr << theline << endl;
	continue;
      }

      TH1 *tmph1 = (TH1 *)findHisto(v_tokens[0]); if (!tmph1) exit(-1);
      TH1 *tmph2 = (TH1 *)findHisto(v_tokens[1]); if (!tmph2) exit(-1);

      wGraph_t *pwg=NULL;
      if (v_tokens.size()==3)
	pwg = generateSeffOverBeffGraph(tmph1,tmph2,v_tokens[2].c_str());
      else
	pwg = generateSeffOverBeffGraph(tmph1,tmph2);
      if (!pwg) exit(-1);

      v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));
      glmap_mobj2size.insert(pair<string,unsigned>(*gid,v_graphs.size()));

    //------------------------------
    } else if( key == "fittf1" ) {
    //------------------------------
      TF1 *tf1 = findTF1(value);
      if( !tf1 ) {
	cerr << "TF1 " << value << " must be defined first" << endl;
	continue;
      }
      string funcnewname = value+(*gid);
      wg.fitfn = new TF1(*tf1);
      wg.fitfn->SetName(funcnewname.c_str());

    //------------------------------
    } else if ( key == "contours" ) {
    //------------------------------
      Tokenize(value,v_tokens,",");
      wg.contours = new TVectorD(v_tokens.size());
      for (size_t i=0; i<v_tokens.size(); i++)
	wg.contours[i] = str2flt(v_tokens[i]);
    }
    //------------------------------
    else if( key == "avggraphs" ) {
    //------------------------------
      Tokenize(value,v_tokens,",");
      if (v_tokens.size() == 1) { // presumably a set of graphs
	std::map<string,unsigned>::const_iterator it=glmap_mobj2size.find(value);
	if (it!=glmap_mobj2size.end()) {
	  TVectorD vx,vy;
	  wGraph_t *pwg;
	  for (size_t i=0; i<it->second; i++) {
	    string gidi=value+"_"+int2str(i);
	    pwg = findGraph(gidi,"");
	    assert(pwg);
	    if (!i) {
	      vx.ResizeTo(pwg->gr->GetN());
	      vy.ResizeTo(pwg->gr->GetN());
	    }
	    for (int j=0; j<pwg->gr->GetN(); j++) {
	      double x,y;
	      pwg->gr->GetPoint(j,x,y);
	      if (!i)
		vx(j) = x;
	      else {
		if (vx(j)!=x) 
		  cerr << vx(j) << " != " << x << " for graph " << i << endl;
		assert(vx(j) == x); // exact mode
	      }
	      vy(j) += y;
	    }
	  }
	  vy     *= 1./(double)it->second;
	  pwg     = new wGraph_t();
	  pwg->gr = new TGraph(vx,vy);
	  v_graphs.push_back(pair<string,wGraph_t *>(*gid,pwg));
	} else {
	  cerr << "Expecting a comma-separated list of graphs or id pointing to a set of graphs" << endl;
	  exit(-1);
	}
      } // one token
    } // avggraphs

    else if( key == "name"  )        name      = TString(value);
    else if( key == "title"  )       title     = TString(value);
    else if( key == "xtitle" )       wg.xax->SetTitle      (TString(value));
    else if( key == "ytitle" )       wg.yax->SetTitle      (TString(value));
    else if( key == "ztitle" )       wg.zax->SetTitle      (TString(value));
    else if( key == "xtitleoffset" ) wg.xax->SetTitleOffset(str2flt(value));
    else if( key == "ytitleoffset" ) wg.yax->SetTitleOffset(str2flt(value));
    else if( key == "ztitleoffset" ) wg.zax->SetTitleOffset(str2flt(value));
    else if( key == "xtitlecenter" ) wg.xax->CenterTitle   (str2int(value));
    else if( key == "ytitlecenter" ) wg.yax->CenterTitle   (str2int(value));
    else if( key == "ztitlecenter" ) wg.zax->CenterTitle   (str2int(value));
    else if( key == "xndiv" )        wg.xax->SetNdivisions (str2int(value));
    else if( key == "yndiv" )        wg.yax->SetNdivisions (str2int(value));
    else if( key == "zndiv" )        wg.zax->SetNdivisions (str2int(value));
    else if( key == "xmin" )         xmin         = str2flt(value);
    else if( key == "xmax" )         xmax         = str2flt(value);
    else if( key == "ymin" )         ymin         = str2flt(value);
    else if( key == "ymax" )         ymax         = str2flt(value);
    else if( key == "zmin" )         zmin         = str2flt(value);
    else if( key == "zmax" )         zmax         = str2flt(value);
    else if( key == "linecolor" )    wg.lcolor    = str2int(value);
    else if( key == "linestyle" )    wg.lstyle    = str2int(value);
    else if( key == "linewidth" )    wg.lwidth    = str2int(value);
    else if( key == "markercolor" )  wg.mcolor    = str2int(value);
    else if( key == "markerstyle" )  wg.mstyle    = str2int(value);
    else if( key == "markersize"  )  wg.msize     = str2int(value);
    else if( key == "fillcolor" )    wg.fcolor    = str2int(value);
    else if( key == "fillstyle" )    wg.fstyle    = str2int(value);
    else if( key == "symerrors" )    symerrors    = (bool)str2int(value);
    else if( key == "asymerrors" )   asymerrors   = (bool)str2int(value);
    else if( key == "leglabel" )     wg.leglabel  = value;
    else if( key == "draw" )         wg.drawopt   = value;
    else if( key == "legdraw" )      wg.legdrawopt= value;
    else if( key == "setprecision" ) cout << setprecision(str2int(value));
    else if( key == "printvecs2file") { printvecs = true; printfile = value; }
    else if( key == "save2file" )    { savegraph  = true; save2file = value; }

    //------------------------------
    else if (!v_graphs.size()) {
    //------------------------------
      cerr<<"One of keys path,clone,vectorfile,vectorfile2d,fromhisto,fillfromtree,bayesdiv,generate*,avggraphs";
      cerr<<" must be defined before key..."<<key<<endl;

    //------------------------------
    } else { // math on graphs:
    //------------------------------
      if ( key == "xoffset" ) {
	float xoffset = str2flt(value);
	if (xoffset != 0.0) {
	  for (size_t i=0; i<v_graphs.size(); i++) {
	    wGraph_t *pwg = v_graphs[i].second;
	    int npts = pwg->gr->GetN();
	    if (npts) {
	      double *px = pwg->gr->GetX();
	      for (int j=0; j<npts; j++) px[j] += xoffset;
	    }
	  }
	}
      }
      else if( key == "yoffset" ) {
	float yoffset = str2flt(value);
	if (yoffset != 0.0) {
	  for (size_t i=0; i<v_graphs.size(); i++) {
	    wGraph_t *pwg = v_graphs[i].second;
	    int npts = pwg->gr->GetN();
	    if (npts) {
	      double *py = pwg->gr->GetY();
	      for (int j=0; j<npts; j++) py[j] += yoffset;
	    }
	  }
	}
      }
      else if( key == "xscale" ) {
	float xscale = str2flt(value);
	if (xscale != 1.0) {
	  for (size_t i=0; i<v_graphs.size(); i++) {
	    wGraph_t *pwg = v_graphs[i].second;
	    int npts = pwg->gr->GetN();
	    if (npts) {
	      double *px = pwg->gr->GetX();
	      for (int j=0; j<npts; j++) px[j] *= xscale;
	    }
	  }
	}
      }
      else if( key == "yscale" ) {
	float yscale = str2flt(value);
	if (yscale != 1.0) {
	  for (size_t i=0; i<v_graphs.size(); i++) {
	    wGraph_t *pwg = v_graphs[i].second;
	    int npts = pwg->gr->GetN();
	    if (npts) {
	      double *py = pwg->gr->GetY();
	      for (int j=0; j<npts; j++) py[j] *= yscale;
	    }
	  }
	}
      }
      else if( key == "norm2nth" ) {
	int norm2nth  = str2int(value);
	if (norm2nth >= 0) {
	  for (size_t i=0; i<v_graphs.size(); i++) {
	    wGraph_t *pwg = v_graphs[i].second; assert(pwg); assert(pwg->gr);
	    int npts = pwg->gr->GetN();
	    if (npts) {
	      double *py = pwg->gr->GetY();
	      if (norm2nth < npts && py[norm2nth] != 0.0) {
		double myyscale = 1./py[norm2nth];
		for (int j=0; j<npts; j++)
		  py[j] *= myyscale;
	      } else {
		cerr << "Cannot perform norm2nth!" << endl;
		exit(-1);
	      }
	    }
	  }
	}
      }
      else if( key == "dividebygraph" ) {
	map<string,wGraph_t *>::const_iterator it = glmap_id2graph.find(value);
	if( it == glmap_id2graph.end() ) {
	  cerr << "Graph ID " << value << " not found, define first" << endl;
	  exit(-1);
	}
	TGraph *divsrgr = it->second->gr;
	int nptsdivsr = divsrgr->GetN();
	for (size_t i=0; i<v_graphs.size(); i++) {
	  TGraph *gr = v_graphs[i].second->gr;
	  int npts = gr->GetN();
	  // divisor allowed to have more pts as long as all pts in num are represented
	  if (npts <= nptsdivsr) {
	    for (int j=0; j<npts; j++) {
	      int k=j;
	      double xtop,xbot,ytop,ybot;
	      gr->GetPoint(j,xtop,ytop) ;
	      divsrgr->GetPoint(k,xbot,ybot) ;
	      while ((xtop!=xbot) && (k<nptsdivsr)) {
		divsrgr->GetPoint(++k,xbot,ybot) ;
	      }
	      if (xtop!=xbot) {
		cerr << "x value " << xtop << " not represented in divisor " << value << endl;
		exit(-1);
	      }
	      gr->SetPoint(j,xtop,ytop/ybot);
	    }
	  } else {
	    cerr << "Cannot perform division, different # of points" << endl;
	    exit(-1);
	  }
	}
      }
      else {
	cerr << "unknown key " << key << endl;
      }
#if 0
      processCommonHistoParams(key,value,*wh);
#endif
    }
  } // getline loop

  //cout << title << endl;

  for (size_t i=0; i<v_graphs.size(); i++) {
    string gidi = v_graphs[i].first;
    wGraph_t *pwg = v_graphs[i].second;
    if (pwg->gr2d) {
      pwg->fitfn      =        wg.fitfn;
      pwg->contours   =        wg.contours;
      pwg->drawopt    =        wg.drawopt;
      pwg->leglabel   =        wg.leglabel;
      pwg->legdrawopt =        wg.legdrawopt;
      pwg->gr2d->SetName(name);
      pwg->gr2d->SetTitle(title);
      pwg->xax->SetTitle      (wg.xax->GetTitle());
      pwg->yax->SetTitle      (wg.yax->GetTitle());
      pwg->zax->SetTitle      (wg.zax->GetTitle());
      pwg->xax->SetTitleOffset(wg.xax->GetTitleOffset());
      pwg->yax->SetTitleOffset(wg.yax->GetTitleOffset());
      pwg->zax->SetTitleOffset(wg.zax->GetTitleOffset());
      pwg->xax->CenterTitle   (wg.xax->TestBit(TAxis::kCenterTitle));
      pwg->yax->CenterTitle   (wg.yax->TestBit(TAxis::kCenterTitle));
      pwg->zax->CenterTitle   (wg.zax->TestBit(TAxis::kCenterTitle));
      pwg->xax->SetNdivisions (wg.xax->GetNdivisions());
      pwg->yax->SetNdivisions (wg.yax->GetNdivisions());
      pwg->zax->SetNdivisions (wg.zax->GetNdivisions());
      pwg->lstyle     =        wg.lstyle;
      pwg->lcolor     =        wg.lcolor;
      pwg->lwidth     =        wg.lwidth;
      pwg->mcolor     =        wg.mcolor;
      pwg->mstyle     =        wg.mstyle;
      pwg->msize      =        wg.msize;
      pwg->fstyle     =        wg.fstyle;
      pwg->fcolor     =        wg.fcolor;

      if (xmax>xmin) pwg->xax->SetLimits(xmin,xmax);
      if (ymax>ymin) pwg->yax->SetLimits(ymin,ymax);
      if (zmax>zmin) pwg->zax->SetLimits(zmin,zmax);

      glmap_id2graph.insert(pair<string,wGraph_t *>(gidi,pwg));

    } else {
      pwg->gr->UseCurrentStyle();
      pwg->fitfn      =        wg.fitfn;
      pwg->contours   =        wg.contours;
      pwg->drawopt    =        wg.drawopt;
      pwg->leglabel   =        wg.leglabel;
      pwg->legdrawopt =        wg.legdrawopt;
      pwg->gr->SetName        (name);
      if (!strcmp(pwg->gr->GetTitle(),"Graph"))
	pwg->gr->SetTitle       (title);
      pwg->xax->SetTitle      (wg.xax->GetTitle());
      pwg->yax->SetTitle      (wg.yax->GetTitle());
      pwg->zax->SetTitle      (wg.zax->GetTitle());
      pwg->xax->SetTitleOffset(wg.xax->GetTitleOffset());
      pwg->yax->SetTitleOffset(wg.yax->GetTitleOffset());
      pwg->zax->SetTitleOffset(wg.zax->GetTitleOffset());
      pwg->xax->SetNdivisions (wg.xax->GetNdivisions());
      pwg->yax->SetNdivisions (wg.yax->GetNdivisions());
      pwg->zax->SetNdivisions (wg.zax->GetNdivisions());
      pwg->gr->SetLineStyle   (wg.lstyle);
      pwg->gr->SetLineColor   (wg.lcolor);
      pwg->gr->SetLineWidth   (wg.lwidth);
      pwg->gr->SetMarkerColor (wg.mcolor);
      pwg->gr->SetMarkerStyle (wg.mstyle);
      pwg->gr->SetMarkerSize  (wg.msize);
      pwg->gr->SetFillStyle   (wg.fstyle);
      pwg->gr->SetFillColor   (wg.fcolor);

      if (xmax>xmin) pwg->xax->SetLimits(xmin,xmax);
      if (ymax>ymin) pwg->yax->SetLimits(ymin,ymax);

      glmap_id2graph.insert(pair<string,wGraph_t *>(gidi,pwg));

      if (printvecs) printVectorsToFile(pwg,printfile);
      if (savegraph) save2File( pwg->gr,save2file ); // save with same name

    } // else not 2d
  } // loop over vector of graphs

  return (v_graphs.size());
}                                                 // processGraphSection

//======================================================================
