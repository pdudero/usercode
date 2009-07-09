#include "MyAnalyzers/Stop2TopAnalyzer/src/cutPlots.h"
#include "MyAnalyzers/Stop2TopAnalyzer/src/pdgid.h"

#include <iostream>
using namespace std;

typedef unsigned int uint32_t;

cutPlots::cutPlots(double maxplottedETgev,
		   int    cutnumber,
		   const char *cutstr) {
  char name[80];
  char title[80];

  sprintf (name, "classhist%d", cutnumber); 
  sprintf (title, "classhist, %s", cutstr); 
  h1f_class      = new TH1F(name,title,21,-10.5,10.5);

  sprintf (name, "eEThist%d", cutnumber); 
  sprintf (title, "electronET, %s;GeV", cutstr); 
  h1d_electronET = new TH1D(name, title, 100,0.0,500.0);

  sprintf (name, "ljEThist%d", cutnumber); 
  sprintf (title, "leadingjet ET, %s;GeV", cutstr); 
  h1d_leadingjetET = new TH1D(name,title, 100, 0.0, maxplottedETgev);

  sprintf (name, "METh%d", cutnumber);
  sprintf (title, "MET, %s;GeV", cutstr); 
  h1d_met         = new TH1D(name, title, 100, 0.0, maxplottedETgev);

  sprintf (name, "HTh%d", cutnumber);
  sprintf (title, "HT, %s;GeV", cutstr); 
  h1d_ht          = new TH1D(name, title, 100, 0.0, 2*maxplottedETgev);

  sprintf (name, "HT+METh%d", cutnumber);
  sprintf (title, "HT+MET, %s;GeV", cutstr); 
  h1d_htplusmet   = new TH1D(name, title, 100, 0.0, 2*maxplottedETgev);

  sprintf (name, "rWmassh%d", cutnumber);
  sprintf (title, "recoWmass, %s;GeV", cutstr); 
  h1d_recoWmass   = new TH1D(name, title, 100, 0.0, 200.0);

  sprintf (name, "rTmassh%d", cutnumber);
  sprintf (title, "recoTopMass, %s;GeV", cutstr); 
  h1d_recoTopMass = new TH1D(name, title, 100, 0.0, 500.0);

  sprintf (name, "rTopEth%d", cutnumber);
  sprintf (title, "recoTopEt, %s;GeV", cutstr); 
  h1d_recoTopEt   = new TH1D(name, title, 100, 0.0, 1000.0);

  sprintf (name, "rTopPth%d", cutnumber);
  sprintf (title, "recoTopPt, %s;GeV", cutstr); 
  h1d_recoTopPt   = new TH1D(name, title, 100, 0.0, 1000.0);

  sprintf (name, "rmassChi2h%d", cutnumber);
  sprintf (title, "recoMassChi2, %s;GeV^2", cutstr); 
  h1d_recomassChi2= new TH1D(name, title, 100, 0.0, 500.0);

  sprintf (name, "rTboosth%d", cutnumber);
  sprintf (title, "recoTopBoost, %s;recoEt/recoM", cutstr); 
  h1d_recoTopBoost= new TH1D(name, title, 50, -0.5, 9.5);

  sprintf (name, "rTedRh%d", cutnumber); 
  sprintf (title, "recoTop/electron dR, %s;dR", cutstr); 
  h1d_recoTop_e_dr= new TH1D(name, title, 100, 0.0, 10.0);

  sprintf (name, "rTedPhih%d", cutnumber); 
  sprintf (title, "recoTop/electron dPhi, %s;dPhi", cutstr); 
  h1d_recoTop_e_dphi= new TH1D(name, title, 25, 0.0, 4.0);

  sprintf (name, "rTe+bjetdPhih%d", cutnumber);
  sprintf (title, "recoTop/e+bjet dPhi, %s;dPhi", cutstr); 
  h1d_recoTop_eplusbjet_dphi = new TH1D(name, title, 25, 0.0, 4.0);

  sprintf (name, "e+bjetmassh%d", cutnumber);
  sprintf (title, "e+bjet mass, %s;GeV", cutstr); 
  h1d_eplusbjetmass = new TH1D(name, title, 100, 0.0, 500.0);

  sprintf (name, "metsigh%d", cutnumber);
  sprintf (title, "METsignificance, %s;MET/sqrt(Ht) (sqrt(GeV))", cutstr); 
  h1d_metsignificance = new TH1D(name, title, 50, 0.0, 50.0);

  sprintf (name, "numeh%d", cutnumber);
  sprintf (title, "# ID'd electrons, %s;Gen;Reco", cutstr); 
  h2f_numElecs    = new TH2F(name, title,
			     16, -0.5, 15.5,
			     16, -0.5, 15.5);

  sprintf (name, "nume+th%d", cutnumber);
  sprintf (title, "# ID'd electrons, %s;Gen e's+tau's;Reco e's", cutstr); 
  h2f_numElecTaus = new TH2F(name, title,
			     16, -0.5, 15.5,
			     16, -0.5, 15.5);

  sprintf (name, "numjh%d", cutnumber);
  sprintf (title, "# jets, %s;# GenJets;# RecoJets", cutstr); 
  h2f_numJets     = new TH2F(name, title,
			     16, -0.5, 15.5,
			     16, -0.5, 15.5);

  sprintf (name, "numjoth%d", cutnumber);
  sprintf (title, "# jets > cutoff, %s;# GenJets;# RecoJets", cutstr); 
  h2f_numJetsOverThresh = new TH2F(name, title,
				   16, -0.5, 15.5,
				   16, -0.5, 15.5);
}

void cutPlots::fill(plotVars_t& vars)
{
#if 0
  for (uint32_t i=0; i<vars.v_electronETs.size(); i++)
    h1d_electronET->Fill(vars.v_electronETs[i]);
#endif

  //  if (vars.maxElectronET > 0.0)
  h1d_electronET->Fill(vars.maxElectronET);

#if 0
  for (uint32_t i=0; i<vars.v_maxjet_e_dr.size(); i++)
    h1d_maxjet_e_dr->Fill(vars.v_maxjet_e_dr[i]);
#endif

  h1d_leadingjetET->Fill(vars.leadingjetET);

  h1d_met->Fill(vars.absmet);

  h1d_ht->Fill(vars.ht);

  h1d_htplusmet->Fill(vars.htplusmet);

  //  if (vars.recoWmass > 0.0)
  h1d_recoWmass->Fill(vars.recoWmass);

  //    if (vars.recoTopMass > 0.0)
  h1d_recoTopMass->Fill(vars.recoTopMass);

  //  if (vars.recoTopEt > 0.0)
  h1d_recoTopEt->Fill(vars.recoTopEt);

  //  if (vars.recoTopPt > 0.0)
  h1d_recoTopPt->Fill(vars.recoTopPt);

  //  if (vars.recomassChi2 > 0.0)
  h1d_recomassChi2->Fill(vars.recomassChi2);

  //  if (vars.recoTopBoost > 0.0)
  h1d_recoTopBoost->Fill(vars.recoTopBoost);

  //  if (vars.recoTop_e_dr > 0.0)
  h1d_recoTop_e_dr->Fill(vars.recoTop_e_dr);

  //  if (vars.recoTop_e_dphi > 0.0)
  h1d_recoTop_e_dphi->Fill(vars.recoTop_e_dphi);

  //  if (vars.recoTop_eplusbjet_dphi > 0.0)
  h1d_recoTop_eplusbjet_dphi->Fill(vars.recoTop_eplusbjet_dphi);

  //  if (vars.eplusbjetmass > 0.0)
  h1d_eplusbjetmass->Fill(vars.eplusbjetmass);

  //  if (vars.metsignificance > 0.0)
  h1d_metsignificance->Fill(vars.metsignificance);

  h1f_class->Fill((float)vars.eventclass);

  h2f_numElecs->Fill(vars.myprec->count(electron),
		     vars.v_electronETs.size());
  h2f_numElecTaus->Fill(vars.myprec->count(electron)+
			vars.myprec->count(tauminus),
			vars.v_electronETs.size());
  h2f_numJets->Fill((float)vars.myprec->quarkcount(),
		    (float)vars.numjets);
  h2f_numJetsOverThresh->Fill((float)vars.myprec->quarkcount(),
			      (float)vars.numjetsoverthresh);
}
