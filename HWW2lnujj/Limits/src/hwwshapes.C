#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <map>
#include <set>
#include "TFile.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TString.h"
#include "TKey.h"
#include "TCollection.h"

static std::set<TString> s_chans;

struct HdataPerChan {
  HdataPerChan() :
    h(0),sumwinmin(0),sumwinmax(0) {}
  TH1  *h;
  double sumwinmin;
  double sumwinmax;
};

struct HdataPerMassPt {
  HdataPerMassPt() :
    massgev(0),cspb(0),unchi(0),unclo(0),br2lnujj(0),perchan() {}
  double massgev;
  double cspb;
  double unchi;
  double unclo;
  double br2lnujj;
  std::map<TString,HdataPerChan> perchan;
};

struct Bkgdata {
  Bkgdata() :
    masslo(0),masshi(0),channame(""),systname(""),gr(0) {}
  int masslo;
  int masshi;
  int nbins;
  TString name;
  TString channame;
  TString systname;
  TGraph *gr;
};

const double intlumipbinv = 2100.0;

#define NPTS 13
const int masspts[NPTS] = {
  160,170,180,190,200,250,300,350,400,450,500,550,600
};

std::ostream& operator <<(std::ostream& out,const Bkgdata& bd) {
  out << "BD:("<<bd.name << "," << bd.channame << ","<<bd.systname;
  out <<","<<bd.masslo<<","<<bd.masshi<<","<<bd.nbins<<")";
  return out;
}

//================================================================================

void readHxsTable(const std::string& fname,
		  std::map<int,HdataPerMassPt>& mhd)
{
  cout << "Reading cross sections table from " << fname << endl;

  FILE *mapf = fopen(fname.c_str(),"r");
  if (!mapf) {
    cerr << "Couldn't open " << fname << endl;
    exit(-1);
  }

  char str[128];
  while (!feof(mapf) && fgets(str,128,mapf)) {
    if (str[0] == '#') continue;
    int mHgev;
    int n = sscanf(str,"%d",&mHgev);
    HdataPerMassPt& hd = mhd[mHgev];
    n += sscanf(str,"%*d %lf %lf %lf",&hd.cspb,&hd.unchi,&hd.unclo);
    if (n != 4) {
      cerr << "Error reading from file " << fname << endl;
      exit(-1);
    }
    hd.massgev = (double)mHgev;
  }
  fclose(mapf);
}                                                                  // readHxsTable

//================================================================================

void readBRtable(const std::string& fname,
		  std::map<int,HdataPerMassPt>& mhd)
{
  cout << "Reading branching ratios table from " << fname << endl;

  FILE *brf = fopen(fname.c_str(),"r");
  if (!brf) {
    cerr << "Couldn't open " << fname << endl;
    exit(-1);
  }

  char str[128];
  while (!feof(brf) && fgets(str,128,brf)) {
    if (str[0] == '#') continue;
    int mHgev;
    double br;
    int n = sscanf(str,"%d %lf",&mHgev,&br);
    if (n != 2) {
      cerr << "Error reading from file " << fname << endl;
      exit(-1);
    }
    mhd[mHgev].br2lnujj = 2*br; // listed as for only one charge lepton
  }
  fclose(brf);
}                                                                  // readBRtable

//================================================================================
#if 0
void readEffAccTable(const std::string& fname,
		     std::map<int,HdataPerMassPt>& mhd)
{
  cout << "Reading eff times acc table from " << fname << endl;

  FILE *eaf = fopen(fname.c_str(),"r");
  if (!eaf) {
    cerr << "Couldn't open " << fname << endl;
    exit(-1);
  }

  char str[128];
  while (!feof(eaf) && fgets(str,128,eaf)) {
    if (str[0] == '#') continue;
    int mHgev;
    double eff_mu,acc_mu,eff_el,acc_el;
    int n = sscanf(str,"%d %lf %lf %lf %lf",&mHgev,&acc_el,&eff_el,&acc_mu,&eff_mu);
    if (n != 5) {
      cerr << "Error reading from file " << fname << endl;
      exit(-1);
    }

    mhd[mHgev].effXacc_mu = eff_mu*acc_mu;
    mhd[mHgev].effXacc_el = eff_el*acc_el;
  }
  fclose(eaf);
}                                                               // readEffAccTable
#endif
//================================================================================

TGraph *getGraph(TFile *fp,
		 const std::string& name)
{
  cout << "Getting graph " << name << endl;

  TGraph *gr = (TGraph *)fp->Get(name.c_str());
  if (!gr) {
    cerr << "Couldn't get " << name << endl;
    exit(-1);
  }
  return gr;
}                                                                      // getGraph

//================================================================================

TH1D *getHist(TFile *fp,
	      const TString& name)
{
  //cout << "Getting histogram " << name << endl;

  TH1D *h = (TH1D *)fp->Get(name.Data());
  if (!h) {
    cerr << "Couldn't get " << name << endl;
    exit(-1);
  }
  return h;
}                                                                       // getHist

//================================================================================

void getDataBackgrounds(TFile *fp,
			std::map<TString,Bkgdata>& m_bkgdata)
{
  TIter nextkey( fp->GetListOfKeys() );
  TKey *key;
  int nbins;
  while ( (key = (TKey*)nextkey())) {
    TObject *obj    = key->ReadObj();
    TString objname = obj->GetName();

    TObjArray *tokens = objname.Tokenize("_");
    int nent=tokens->GetEntriesFast();
    if (nent < 5) { // signal histo, or unknown
      continue;
    }

    TString one   = ((TObjString *)(*tokens)[0])->GetString();
    TString two   = ((TObjString *)(*tokens)[1])->GetString();
    TString three = ((TObjString *)(*tokens)[2])->GetString();
    TString four  = ((TObjString *)(*tokens)[3])->GetString();
    TString five  = ((TObjString *)(*tokens)[4])->GetString();

    if( !two.CompareTo("MassRange") &&
	(five.Contains("otal") ||
	 five.Contains("data")) ) {
      TObjArray *tok2 = three.Tokenize("-");
      if (tok2->GetEntriesFast() != 2)
	cerr << "Unrecognized mass range " << three << endl;
      else {
	Bkgdata bd;
	s_chans.insert(one);
	bd.channame = one;
	bd.name     = five;
	bd.masslo   = ((TObjString *)(*tok2)[0])->GetString().Atoi();
	bd.masshi   = ((TObjString *)(*tok2)[1])->GetString().Atoi();
	bd.gr       = (TGraph *)obj;
	if (!four.CompareTo("hist")) nbins = bd.gr->GetN(); // depend on data "hist" to come first
	bd.nbins    = nbins;

	// for histograms with systematics:
	if (nent > 5)
	  bd.systname = ((TObjString *)(*tokens)[5])->GetString();
	  
	m_bkgdata[objname] = bd;
	cout << "Got " << setw(15) << bd.name;
	cout << " for channel " << bd.channame << " and massrange ";
	cout << bd.masslo << "-" << bd.masshi;
	cout << ", nbins = "<<bd.nbins<<", binwidth = "<<(bd.masshi-bd.masslo)/bd.nbins;
	if (bd.systname.Length()) cout << " and systematic " << bd.systname;
	cout << endl;
      }
    } else {
      cerr << "Unknown object " << objname << endl;
    }
  } // key loop
}                                                            // getDataBackgrounds

//================================================================================
#if 0
void dumpPerBin(  TGraph *data,
		  TGraph *wjbk,
		  TGraph *wwbk,
		  std::map<int,HdataPerMassPt>& mhd)
{
  int     datan  = data->GetN();
  double *datax  = data->GetX();
  double *datay  = data->GetY();
  int     wjbkn  = wjbk->GetN();
  double *wjbkx  = wjbk->GetX();
  double *wjbky  = wjbk->GetY();
  int     wwbkn  = wwbk->GetN();
  double *wwbkx  = wwbk->GetX();
  double *wwbky  = wwbk->GetY();

  cout << datan << " " << wjbkn << " " << wwbkn << endl;

  printf("#M4(GeV)\tnObs\tnW+Jets\tnWW+WZ");

  for (int i=0; i<NPTS; i++)  printf("\t%5d",masspts[i]);  printf("\n#");
  for (int i=0; i<125; i++)   printf("-");                 printf("\n");

  int datasum=0;
  double wjbksum=0.,wwbksum=0.;
  for (int i=1,j=1,k=1; i<datan; i++) {
    double x = datax[i];
    while(wjbkx[j] < x) { j++; if (j >= wjbkn) exit(0); } // RooCurves are strange beasts...
    while(wwbkx[k] < x) { k++; if (k >= wwbkn) exit(0); }

    datasum += datay[i];
    wjbksum += wjbky[j];
    wwbksum += wwbky[k];

#ifdef DEBUG
    printf ("%2d %2d %3d %2d %5.1f\t\t%3d\t%5.1f\t%5.1f",
	    i,j,k,m,x,(int)datay[i],wjbky[j],wwbky[k]);
#else
    printf ("%5.1f\t\t%3d\t%5.1f\t%5.1f",
	    x,(int)datay[i],wjbky[j],wwbky[k]);
#endif
    for (int n=0; n<NPTS; n++) {
      HdataPerMassPt& hd = mhd[masspts[n]];
      TH1D *h = hd.h;

      int m = h->FindBin(x);
      double nev = h->GetBinContent(m);
      double ovflwcor = 1.0/h->Integral(0,h->GetNbinsX()+1);

      //cout<<intlumipbinv<<" "<<hd.cspb<<" "<<hd.br2lnujj<<" "<<hd.effXacc_el<<" "<<hd.effXacc_mu<<endl;

      nev *= ovflwcor * intlumipbinv * hd.cspb * hd.br2lnujj * (hd.effXacc_el + hd.effXacc_mu)/2.;

      printf("\t%5.1f",nev);
    }

    printf("\n");
  }
  printf ("\t\t%3d\t%5.1f\t%5.1f", datasum,wjbksum,wwbksum);

  for (int n=0; n<NPTS; n++) {
    HdataPerMassPt& hd = mhd[masspts[n]];

    //cout<<intlumipbinv<<" "<<hd.cspb<<" "<<hd.br2lnujj<<" "<<hd.effXacc_el<<" "<<hd.effXacc_mu<<endl;
    double nev = intlumipbinv * hd.cspb * hd.br2lnujj * (hd.effXacc_el + hd.effXacc_mu)/2.;
    printf("\t%5.1f",nev);
  }
  printf("\n");
}                                                                    // dumpPerBin
#endif
//================================================================================

void getSignalDataForMasspoint(TFile *fp,
			       HdataPerMassPt& hd)
{
  int imass = (int)hd.massgev;

  std::set<TString>::const_iterator chit;
  for (chit=s_chans.begin(); chit!=s_chans.end(); chit++) {
    TString hname = Form("%s_HiggsTemplate_Mass_%d",chit->Data(),imass);
    HdataPerChan perchan;
    TH1 *h = getHist(fp,hname);
    assert(h);

    // Find limit window from gaussian fit to signal peak.
    //
    double wid  = h->GetRMS();
    double mean = h->GetMean();

    //TCanvas *c1 = new TCanvas(s,s,300,300);
    TFitResultPtr r = h->Fit("gaus","QNS","",hd.massgev-2.5*wid,hd.massgev+2.5*wid);

    TAxis *xax = h->GetXaxis();

    int lobin=xax->FindFixBin(r->Parameter(1)-2*r->Parameter(2)); perchan.sumwinmin=std::max(150.,xax->GetBinLowEdge(lobin));
    int hibin=xax->FindFixBin(r->Parameter(1)+2*r->Parameter(2)); perchan.sumwinmax=std::min(800.,xax->GetBinUpEdge(hibin));

    cout<<imass<<" mean= "<<mean<<", RMS= "<<wid;
    cout << ", Fit mean = "<<r->Parameter(1)<<", Fit sigma= "<<r->Parameter(2);
    cout << ", binrange = "<<perchan.sumwinmin<<"-"<<perchan.sumwinmax<<endl;

    //cout << r->Parameter(0) << " " << r->Parameter(1) << " " << r->Parameter(2) << endl;

    perchan.h = h;
    hd.perchan[(*chit)] = perchan;
  }
}                                                     // getSignalDataForMasspoint

//================================================================================

void writeSignalDataForChannel(TFile *allHistFile,
			       HdataPerMassPt& hd,
			       const TString& channame,
			       double xmin,
			       double xmax,
			       int    binwidth,
			       bool doSigXsecUpDown=true)
{
  int imass             = (int)hd.massgev;
  HdataPerChan& perchan = hd.perchan[channame];

  int rebin = binwidth/5; // assumes 5GeV binning on higgs histos
  int nbins = (int)(xmax-xmin)/binwidth;

  assert(nbins);

  // Copy contents to window-restricted nominal, up and down varied signal histograms
  // and write to output file.
  //
  TString nameNm = Form("Signal_%s%dto%d_Mass_%d",            channame.Data(),(int)xmin,(int)xmax,imass);
  TString nameUp = Form("Signal_%s%dto%d_Mass_%d_SigxsecUp",  channame.Data(),(int)xmin,(int)xmax,imass);
  TString nameDn = Form("Signal_%s%dto%d_Mass_%d_SigxsecDown",channame.Data(),(int)xmin,(int)xmax,imass);

  printf("Booking TH1D(%40s,%40s,%d,%4.0lf,%4.0lf)\n",nameNm.Data(),nameNm.Data(),nbins,xmin,xmax);
  printf("Booking TH1D(%40s,%40s,%d,%4.0lf,%4.0lf)\n",nameUp.Data(),nameUp.Data(),nbins,xmin,xmax);
  printf("Booking TH1D(%40s,%40s,%d,%4.0lf,%4.0lf)\n",nameDn.Data(),nameDn.Data(),nbins,xmin,xmax);

  TH1D *hnm = new TH1D(nameNm.Data(),nameNm.Data(),nbins,xmin,xmax);
  TH1D *hup = new TH1D(nameUp.Data(),nameUp.Data(),nbins,xmin,xmax);
  TH1D *hdn = new TH1D(nameDn.Data(),nameDn.Data(),nbins,xmin,xmax);
  
  // make copies of the histograms that are restricted to the bin range lobin-hibin
  int lobin = perchan.h->FindFixBin(xmin);
  int hibin = perchan.h->FindFixBin(xmax); // one higher than we want
  for (int ibin=lobin; ibin<hibin; ) {
    double sum=0;
    for (int j=0; j<rebin; j++)
      sum += perchan.h->GetBinContent(ibin++);
    int newbin = 1+((ibin-lobin)/rebin);
    hnm->SetBinContent(newbin,sum);
    hup->SetBinContent(newbin,(1.0+(hd.unchi/100.0))*sum);
    hdn->SetBinContent(newbin,(1.0+(hd.unclo/100.0))*sum);
  }

  double ovflwcor = 1.0/perchan.h->Integral(0,perchan.h->GetNbinsX()+1);
  double norm     = 
    hd.cspb*
    ovflwcor*
    intlumipbinv*
    hd.br2lnujj;
    
  hnm->Scale(norm);
  hup->Scale(norm);
  hdn->Scale(norm);
  
  allHistFile->WriteTObject(hnm);
  if (doSigXsecUpDown) {
    allHistFile->WriteTObject(hup);
    allHistFile->WriteTObject(hdn);
  }
}                                                     // writeSignalDataForChannel

//================================================================================

void writeHistosForMasspoint(HdataPerMassPt& hd,
			     const std::map<TString,Bkgdata>& m_bkgdata,
			     TFile  *allHistFile)
{
  std::map<TString,HdataPerChan>::const_iterator chit;
  for (chit=hd.perchan.begin(); chit!=hd.perchan.end(); chit++) {

    cout << "Writing histos for masspoint " << hd.massgev << ", channel " << chit->first <<endl;

    const TString&     channame = chit->first;
    HdataPerChan        perchan = chit->second;
    double                 xmin = perchan.sumwinmin;

    // search/collect backgrounds for mass/channel specs that match the input signal,
    // apply the appropriate window to each.
    // backgrounds/data/signal split into mass subranges, which causes each channel
    // to be further subdivided.
    //
    TH1D *h;
    for (double xmax=xmin; xmax < std::min(800.,perchan.sumwinmax); ) {
      int binwidth;
      for (std::map<TString,Bkgdata>::const_iterator it = m_bkgdata.begin();
	   it != m_bkgdata.end();
	   it++) {
	const Bkgdata& bd = it->second;

	// @#%! TEDIOUS! Have to maintain separate values of xmin,xmax for the
	// fine-binned source higgs histo and 
	//
	if( (xmin >= (double)bd.masslo) &&
	    (xmin <  (double)bd.masshi) &&
	    !bd.channame.CompareTo(channame) ) {

	  cout << bd << endl;

	  //cout << "1. " << xmin<<"-"<<xmax<<", "<<binwidth<<endl;

	  // have to adjust the sum window min and max to the available bin boundaries
	  //
	  binwidth  = (bd.masshi-bd.masslo)/bd.nbins;

	  double x;
	  for (x = bd.masslo; x <= xmin; x += binwidth)
	    ;
	  xmin = x - binwidth;

	  //cout << "2. " << xmin<<"-"<<xmax<<", "<<binwidth<<endl;

	  xmax = std::min(perchan.sumwinmax,(double)bd.masshi);
	  
	  //cout << "3. " << xmin<<"-"<<xmax<<", "<<binwidth<<endl;

	  if (xmax == perchan.sumwinmax) {
	    for (x = bd.masslo; x < xmax; x += binwidth)
	      ;
	    xmax = std::min(x,(double)bd.masshi);
	  }

	  cout << "4. " << xmin<<"-"<<xmax<<", "<<binwidth<<endl;

	  //
	  // find next alpha mass-range, if there is one,
	  // and truncate this channel at the beginning of it.
	  //
	  std::map<TString,Bkgdata>::const_iterator it2 = it;
	  for (++it2;
	       it2 != m_bkgdata.end();
	       it2++) {
	    const Bkgdata& bd2 = it2->second;
	    if( !bd2.name.CompareTo(bd.name) &&
		!bd2.channame.CompareTo(bd.channame) ) {
	      cout << "bd: "<<bd.masslo<<"-"<<bd.masshi<<", bd2: "<<bd2.masslo<<"-"<<bd2.masshi<<", xmax="<<xmax<<endl;
	      if ( (bd2.masslo > bd.masslo) &&
		   (bd2.masslo < bd.masshi) &&
		   (xmax       > bd2.masslo)  ) {
		xmax = (double)bd2.masslo;
		break;
	      }
	    }
	  }

	  cout << "5. " << xmin<<"-"<<xmax<<", "<<binwidth<<endl;

	  if (xmax <= xmin) continue;
	  
	  TString name;
	  if (!bd.name.CompareTo("data"))
	    name = Form("data_obs_%s%dto%d_Mass_%d",
			channame.Data(),
			(int)xmin,(int)xmax,
			(int)hd.massgev);
	  else if (bd.systname.Length()) // put systematics at the end
	    name = Form("%s_%s%dto%d_Mass_%d_%s",
			bd.name.Data(),
			channame.Data(),
			(int)xmin,(int)xmax,
			(int)hd.massgev,
			bd.systname.Data());
	  else
	    name = Form("%s_%s%dto%d_Mass_%d",
			bd.name.Data(),
			channame.Data(),
			(int)xmin,(int)xmax,
			(int)hd.massgev);

	  int nbins = (int)((xmax-xmin)/binwidth);

	  printf("Booking TH1D(%40s,%40s,%d,%4.0lf,%4.0lf)\n",
		 name.Data(),name.Data(),nbins,xmin,xmax);
	  h = new TH1D(name.Data(),name.Data(), nbins,xmin,xmax);

	  for (int ibin=1; ibin <= nbins; ibin++)
	    h->SetBinContent(ibin,bd.gr->Eval(h->GetBinCenter(ibin)));

	  allHistFile->WriteTObject(h);

	} // if bkgdata specs match signal
      } // bkgdata loop

      writeSignalDataForChannel(allHistFile,hd,channame,xmin,xmax,binwidth);

      xmin = xmax;
      cout << "---" << endl;

    } // alpha mass range loop

    cout << "------" << endl;

  } // channel loop
}                                                      //  writeHistosForMasspoint

//================================================================================

void hwwshapes(const char *infname  = "Histograms_data_and_template.root",
	       const char *outfname = "kal-histo-shapes-TH1.root")
{
  std::map<int,HdataPerMassPt> m_signals; // signal (Higgs) data per mass point
  std::map<TString,Bkgdata> m_bkgdata;    // backgrounds, data per channel

  TFile *fp = new TFile(infname);

  if (fp->IsZombie()) {
    cerr << "Couldn't open file " << string(infname) << endl;
    exit(-1);
  }

  getDataBackgrounds(fp,m_bkgdata);

  readHxsTable   ("ggHtable.txt",    m_signals);
  readBRtable    ("twikiBRtable.txt",m_signals);
  //readEffAccTable("finaleffacc.txt", m_signals);

  TFile *allHistFile = new TFile(outfname, "RECREATE");

  if (allHistFile->IsZombie())  {
    cerr << "Couldn't open output file " << outfname << endl;
    exit(-1);
  }

  fp->cd();

  for (int i=0; i<NPTS; i++) {

    HdataPerMassPt& hd = m_signals[masspts[i]];

    getSignalDataForMasspoint(fp,hd);

    writeHistosForMasspoint(hd,m_bkgdata,allHistFile);
  }

  //dumpPerBin(grdata,grwjbk,grwwbk,m_signals);

  fp->Close();
}
