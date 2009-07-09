//

#include "MyAnalyzers/Stop2TopAnalyzer/src/Stop2TopAnalJEScor.h"

#include <iostream>

//======================================================================

using namespace std;

//======================================================================

//======================================================================

Stop2TopAnalJEScor::Stop2TopAnalJEScor(bool           verbosity,
				       string         outRootFileName,
				       double         maxdR,
				       vector<double> v_genETbins,
				       vector<double> v_recETbins,
				       double         emfractioncut) :
  verbosity_(verbosity),
  outRootFileName_(outRootFileName),
  maxdR_(maxdR),
  v_genETbins_(v_genETbins),
  v_recETbins_(v_recETbins),
  emfractioncut_(emfractioncut)
{
  if ((!v_genETbins_.size()) ||
      (!v_genETbins_.size())) {
    throw cms::Exception("Error on input", "must have at least one ET bin");
  }
  genjetETgevcut_ = v_genETbins_[0];
  recjetETgevcut_ = v_recETbins_[0];
}

//======================================================================

void Stop2TopAnalJEScor::beginJob(void)
{
  char name[50];
  char title[50];

  outf = new TFile(outRootFileName_.c_str(),"RECREATE");

  jetETscale_vs_phi_cone5 = new TH2D("h2d1",
				     "Cone=0.5;Phi;rEt/gEt",
				     35, -3.5, 3.5,
				     150, 0.0, 1.5);
  jetETscale_vs_phi_cone7 = new TH2D("h2d2",
				     "Cone=0.7;Phi;rEt/gEt",
				     35, -3.5, 3.5,
				     150, 0.0, 1.5);

  for (uint32_t i=0; i<v_genETbins_.size()-1; i++) {
    sprintf (name, "h2d3_%d", i);
    sprintf (title, "Cone=0.5,gEt>%5.1f;gEta;rEt/gEt", v_genETbins_[i]);
    m_jetETscale_vs_geta_et_hists_cone5[v_genETbins_[i]] = new TH2D(name, title,
								    50, -5.0, 5.0,
								    150, 0.0, 1.5);
    sprintf (name, "h2d4_%d", i);
    sprintf (title, "Cone=0.7,gEt>%5.1f;gEta;rEt/gEt", v_genETbins_[i]);
    m_jetETscale_vs_geta_et_hists_cone7[v_genETbins_[i]] = new TH2D(name, title,
								    50, -5.0, 5.0,
								    150, 0.0, 1.5);
  }

  for (uint32_t i=0; i<v_recETbins_.size()-1; i++) {
    sprintf (name, "h2d5_%d", i);
    sprintf (title, "Cone=0.5,rEt>%5.1f;rEta;rEt/gEt", v_recETbins_[i]);
    m_jetETscale_vs_reta_et_hists_cone5[v_recETbins_[i]] = new TH2D(name, title,
								    50, -5.0, 5.0,
								    150, 0.0, 1.5);
    sprintf (name, "h2d6_%d", i);
    sprintf (title, "Cone=0.7,rEt>%5.1f;rEta;rEt/gEt", v_recETbins_[i]);
    m_jetETscale_vs_reta_et_hists_cone7[v_recETbins_[i]] = new TH2D(name, title,
								    50, -5.0, 5.0,
								    150, 0.0, 1.5);
  }

  emRecoJet_vs_geta_cone5 = new TH1F("h7", "Cone=0.5, EMf>0.9;Eta;#Reco Jets",
				     50, -5.0, 5.0);
  emRecoJet_vs_geta_cone7 = new TH1F("h8", "Cone=0.7, EMf>0.9;Eta;#Reco Jets",
				     50, -5.0, 5.0);
}

//======================================================================

void Stop2TopAnalJEScor::endJob()
{
  outf->Write();
}

//======================================================================

void Stop2TopAnalJEScor::dumpJets(CaloJetCollection& jetc,
				const char *intro)
{
  cout << intro << endl;
  char s[80];
  CaloJetCollection::const_iterator cj;
  for (cj = jetc.begin(); cj != jetc.end(); cj++) {
    sprintf (s, "Eta=%6.3f, Phi=%6.3f, ET=%5.1f",
	     cj->eta(),cj->phi(),cj->et());
    cout << s << endl;
  }
}

//======================================================================

void Stop2TopAnalJEScor::dumpJets(GenJetCollection& jetc,
				const char *intro)
{
  cout << intro << endl;
  char s[80];
  GenJetCollection::const_iterator gj;
  for (gj = jetc.begin(); gj != jetc.end(); gj++) {
    sprintf (s, "Eta=%6.3f, Phi=%6.3f, ET=%5.1f",
	     gj->eta(),gj->phi(),gj->et());
    cout << s << endl;
  }
}

//======================================================================

static double deltaPhi(double phi1, double phi2) {
  double dPhi=fabs(phi1-phi2);
  if (dPhi>M_PI) dPhi=2*M_PI-dPhi;
  return dPhi;
}

//======================================================================

void Stop2TopAnalJEScor::correlateJets(GenJetCollection&  genjetc,
				       CaloJetCollection& calojetc,
				       int conesize,
				       const char *intro,
				       vector<pair<GenJet,CaloJet> >& v_jetCorr,
				       vector<GenJet>& v_genjetUncorr)
{
  double dR,mindR,corlimit;
  char s[80];

  if (verbosity_)
    cout << intro << endl;

  corlimit = (double)conesize/10.0;

  GenJetCollection::iterator gj;
  CaloJetCollection::iterator cj,mcj;

  for (gj = genjetc.begin(); gj != genjetc.end(); gj++) {

    if (gj->et() < genjetETgevcut_) continue;

    mindR=corlimit;
    mcj = calojetc.end();
    if (!calojetc.size()) break;

    for (cj = calojetc.begin(); cj != calojetc.end(); cj++) {

      if (cj->et() < recjetETgevcut_) continue;

      dR = sqrt(pow((gj->eta() - cj->eta()),2) +
		pow(deltaPhi(gj->phi(),cj->phi()),2));

      if (dR < mindR) {
	mindR = dR;
	mcj = cj;
      }
    }

    if (mcj != calojetc.end()) {
      if (verbosity_) {
	sprintf(s, "GenET=%5.1f, RecET=%5.1f, Ratio=%6.3f, dR=%6.2f",
		gj->et(), mcj->et(), mcj->et()/gj->et(), mindR);
	cout << s  << endl;
      }
      
      pair<GenJet, CaloJet> mypair;
      mypair.first  = *gj;
      mypair.second = *mcj;
      v_jetCorr.push_back(mypair);

      calojetc.erase(mcj);
    }
    else {
      if (verbosity_) {
	sprintf(s, "GenET=%5.1f, UNCORRELATED", gj->et());
	cout << s << endl;
      }

      v_genjetUncorr.push_back(*gj);
    }
  }
}                                                       // correlateJets

//======================================================================

void Stop2TopAnalJEScor::collectJetStats(vector<pair<GenJet,CaloJet> >& v_jetCorr,
					 vector<GenJet>& v_genjetUncorr,
					 int conesize)
{
  char s[80];
  TH2D *ph_jesvsphi;
  TH1F *ph_emrjvseta;
  map<double,TH2D *> *pm_jesvsgetaet;
  map<double,TH2D *> *pm_jesvsretaet;

  if (conesize == 5) {
    ph_jesvsphi    = jetETscale_vs_phi_cone5;
    ph_emrjvseta   = emRecoJet_vs_geta_cone5;
    pm_jesvsgetaet = &m_jetETscale_vs_geta_et_hists_cone5;
    pm_jesvsretaet = &m_jetETscale_vs_reta_et_hists_cone5;
  }
  else if (conesize == 7) {
    ph_jesvsphi    = jetETscale_vs_phi_cone7;
    ph_emrjvseta   = emRecoJet_vs_geta_cone7;
    pm_jesvsgetaet = &m_jetETscale_vs_geta_et_hists_cone7;
    pm_jesvsretaet = &m_jetETscale_vs_reta_et_hists_cone7;
  }
  else
    throw cms::Exception("Error on input","invalid cone size");

  vector<pair<GenJet,CaloJet> >::iterator ivgcj;
  for (ivgcj = v_jetCorr.begin(); ivgcj != v_jetCorr.end(); ivgcj++) {
    double geta   = ivgcj->first.eta();
    double gphi   = ivgcj->first.phi();
    double gEt    = ivgcj->first.et();
    double reta   = ivgcj->second.eta();
    double rEt    = ivgcj->second.et();
    double ratio  = rEt/gEt;

    double emfraction = ivgcj->second.energyFractionInECAL();

    sprintf (s, "rEta=%6.3f rET=%7.2f", reta, rEt);
    cout << s << endl;

    ph_jesvsphi->Fill(gphi, ratio);

    uint32_t i;
    for (i=0; i < v_genETbins_.size()-1; i++)
      if (gEt < v_genETbins_[i+1]) break;

    if (i != v_genETbins_.size()-1) {
      (*pm_jesvsgetaet)[v_genETbins_[i]]->Fill(geta, ratio);
    }

    for (i=0; i<v_recETbins_.size()-1; i++)
      if (rEt < v_recETbins_[i+1]) break;

    if (i != v_recETbins_.size()-1) {
      (*pm_jesvsretaet)[v_recETbins_[i]]->Fill(reta, ratio);
    }

    if (emfraction >= emfractioncut_)
      ph_emrjvseta->Fill(geta);
  }
}

//======================================================================

void Stop2TopAnalJEScor::processEvent(GenJetCollection&  genjets2,
				      GenJetCollection&  genjets5,
				      GenJetCollection&  genjets7,
				      CaloJetCollection& recjets2,
				      CaloJetCollection& recjets5,
				      CaloJetCollection& recjets7)
{
  if (verbosity_) {
    //    dumpJets(genjets2, "Gen Jets, cone size=0.2: ");
    dumpJets(genjets5, "\nGen Jets, cone size=0.5: ");
    dumpJets(genjets7, "\nGen Jets, cone size=0.7: ");

    //    dumpJets(recjets2, "Reco Jets, cone size=0.2: ");
    dumpJets(recjets5, "\nReco Jets, cone size=0.5: ");
    dumpJets(recjets7, "\nReco Jets, cone size=0.7: ");
  }

  vector<pair<GenJet,CaloJet> > v_jetCorr5, v_jetCorr7;
  vector<GenJet> v_genjetUncorr5, v_genjetUncorr7;

  correlateJets(genjets5, recjets5, 5,
		"\nCorrelations, cone size=0.5: ",
		v_jetCorr5, v_genjetUncorr5);

  correlateJets(genjets7, recjets7, 7,
		"\nCorrelations, cone size=0.7: ",
		v_jetCorr7, v_genjetUncorr7);

  collectJetStats(v_jetCorr5, v_genjetUncorr5, 5);
  collectJetStats(v_jetCorr7, v_genjetUncorr7, 7);
}

//======================================================================
