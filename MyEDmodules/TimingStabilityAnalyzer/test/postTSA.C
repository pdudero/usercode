#include <iostream>
#include <sstream>
#include <map>
#include "TH2F.h"
#include "TH1D.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TObject.h"
#include "TKey.h"

using namespace std;

inline string int2str(int i) {
  std::ostringstream ss;
  ss << i;
  return ss.str();
}

inline unsigned int str2int(const string& str) {
  return (unsigned int)strtoul(str.c_str(),NULL,0);
}

struct Histos_t {
  TH2F *hbm, *hbp, *hem, *hep;
};

void postTSA(const char *filename)
{
  TFile *inputf  = new TFile(filename);
  TFile *outputf = new TFile("postTSA.root","RECREATE");

  if (inputf->IsZombie()) {
    cout << "Couldn't open " << filename << endl;
    return;
  }
  if (!inputf->cd("tsa")) {
    cout << "tsa dir not found" << endl;
    return;
  }

  TDirectory *indir = gDirectory;
  uint32_t numruns = gDirectory->GetNkeys()/4;

  cout << "Total " << numruns << " runs processed" << endl;

  outputf->cd();

  TH2F *allHBHE = new TH2F("allHBHE","HBHE Timing vs Run",
			   numruns+2,0,numruns+2,401,-100.25,100.25); assert(allHBHE);
  TH2F *allHB   = new TH2F("allHB","HB Timing vs Run",
			   numruns+2,0,numruns+2,401,-100.25,100.25); assert(allHB);
  TH2F *allHE   = new TH2F("allHE","HE Timing vs Run",
			   numruns+2,0,numruns+2,401,-100.25,100.25); assert(allHE);

  TDirectory *rbxdir = outputf->mkdir("TimingPerRBX");
  rbxdir->cd();

  TH2F *allRBXes[4][18];
  for (int i=1; i<=18; i++) {
    char s[20];
    sprintf (s,"HBM%02d",i); string rbx(s);
    allRBXes[0][i-1] = new TH2F(rbx.c_str(),rbx.c_str(),
				numruns+2,0,numruns+2,401,-100.25,100.25);
    sprintf (s,"HBP%02d",i); rbx = string(s);
    allRBXes[1][i-1] = new TH2F(rbx.c_str(),rbx.c_str(),
				numruns+2,0,numruns+2,401,-100.25,100.25);
    sprintf (s,"HEM%02d",i); rbx = string(s);
    allRBXes[2][i-1] = new TH2F(rbx.c_str(),rbx.c_str(),
				numruns+2,0,numruns+2,401,-100.25,100.25);
    sprintf (s,"HEP%02d",i); rbx = string(s);
    allRBXes[3][i-1] = new TH2F(rbx.c_str(),rbx.c_str(),
				numruns+2,0,numruns+2,401,-100.25,100.25);
  }

  indir->cd();
  TIter nextkey( gDirectory->GetListOfKeys() );
  TKey    *key;
  TObject *obj;
  for ( int irun=0; (key=(TKey*)nextkey()); irun++) {
    obj=key->ReadObj(); if (!obj->InheritsFrom("TH2")) continue;
    string   name = string(((TNamed *)obj)->GetName());
    if (!name.substr(0,3).compare("run")) break;
  }

  // keys do not come in sorted order, so have to do this in stages
  Histos_t histos;
  std::map<uint32_t, Histos_t> m_histosPerRun;
  for ( int irun=0; key; irun++) {
    histos.hbm=(TH2F *)obj;
    key=(TKey*)nextkey(); obj=key->ReadObj(); if (!obj->InheritsFrom("TH2")) return; histos.hbp=(TH2F *)obj;
    key=(TKey*)nextkey(); obj=key->ReadObj(); if (!obj->InheritsFrom("TH2")) return; histos.hem=(TH2F *)obj;
    key=(TKey*)nextkey(); obj=key->ReadObj(); if (!obj->InheritsFrom("TH2")) return; histos.hep=(TH2F *)obj;
    key=(TKey*)nextkey();
    if (!key) break;      obj=key->ReadObj(); if (!obj->InheritsFrom("TH2")) return;

    string   name = string(histos.hbm->GetName()); cout << name << ", ";
    string runstr = name.substr(3,6);
    uint32_t runnum = str2int(runstr);
    string subdet = name.substr(9,3);
    assert(!name.substr(0,3).compare("run"));
    assert(runnum);
    assert(name.compare("HBM"));

    name = string(histos.hbp->GetName()); cout << name << ", ";
    assert(runnum == str2int(name.substr(3,6)));
    subdet = name.substr(9,3);
    assert(!name.substr(0,3).compare("run"));
    assert(name.compare("HBP"));

    name = string(histos.hep->GetName()); cout << name << ", ";
    assert(runnum == str2int(name.substr(3,6)));
    subdet = name.substr(9,3);
    assert(!name.substr(0,3).compare("run"));
    assert(name.compare("HEP"));

    name = string(histos.hem->GetName()); cout << name << endl;
    assert(runnum == str2int(name.substr(3,6)));
    subdet = name.substr(9,3);
    assert(!name.substr(0,3).compare("run"));
    assert(name.compare("HEM"));

    m_histosPerRun.insert(std::pair<uint32_t,Histos_t>(runnum,histos));
  }

  // Now we can compile the plots
  std::map<uint32_t,Histos_t>::const_iterator it;
  int irun=0;
  for (it=m_histosPerRun.begin();it!=m_histosPerRun.end(); it++) {
    TH2F *hbm = it->second.hbm, *hbp = it->second.hbp;
    TH2F *hem = it->second.hem, *hep = it->second.hep;

    string runstr = int2str(it->first);

    int nbinsy = allHB->GetYaxis()->GetNbins();

    TH1D *sumhb   = hbm->ProjectionY(); sumhb->Add(hbp->ProjectionY());
    TH1D *sumhe   = hem->ProjectionY(); sumhe->Add(hep->ProjectionY());

    allHB  ->GetXaxis()->SetBinLabel(irun+2,runstr.c_str());
    allHE  ->GetXaxis()->SetBinLabel(irun+2,runstr.c_str());
    allHBHE->GetXaxis()->SetBinLabel(irun+2,runstr.c_str());

    TH1D *sumhbhe = (TH1D *)sumhb->Clone("HBHE"); sumhbhe->Add(sumhe);

    for (int j=1; j<=nbinsy; j++) {
      allHB  ->SetBinContent(irun+2,j,sumhb  ->GetBinContent(j));
      allHE  ->SetBinContent(irun+2,j,sumhe  ->GetBinContent(j));
      allHBHE->SetBinContent(irun+2,j,sumhbhe->GetBinContent(j));
      allHB  ->SetBinError  (irun+2,j,sumhb  ->GetBinError(j));
      allHE  ->SetBinError  (irun+2,j,sumhe  ->GetBinError(j));
      allHBHE->SetBinError  (irun+2,j,sumhbhe->GetBinError(j));
    }

    // Now per-RBX:
    for (int k=1; k<=18; k++) {
      allRBXes[0][k-1]->GetXaxis()->SetBinLabel(irun+2,runstr.c_str());
      allRBXes[1][k-1]->GetXaxis()->SetBinLabel(irun+2,runstr.c_str());
      allRBXes[2][k-1]->GetXaxis()->SetBinLabel(irun+2,runstr.c_str());
      allRBXes[3][k-1]->GetXaxis()->SetBinLabel(irun+2,runstr.c_str());
      for (int j=1; j<=nbinsy; j++) {
	allRBXes[0][k-1]->SetBinContent(irun+2,j,hbm->GetBinContent(k,j));
	allRBXes[1][k-1]->SetBinContent(irun+2,j,hbp->GetBinContent(k,j));
	allRBXes[2][k-1]->SetBinContent(irun+2,j,hem->GetBinContent(k,j));
	allRBXes[3][k-1]->SetBinContent(irun+2,j,hep->GetBinContent(k,j));
	allRBXes[0][k-1]->SetBinError  (irun+2,j,hbm->GetBinError(k,j));
	allRBXes[1][k-1]->SetBinError  (irun+2,j,hbp->GetBinError(k,j));
	allRBXes[2][k-1]->SetBinError  (irun+2,j,hem->GetBinError(k,j));
	allRBXes[3][k-1]->SetBinError  (irun+2,j,hep->GetBinError(k,j));
      }
    }
    irun++;
  }
  outputf->cd();
  outputf->Write();
}
