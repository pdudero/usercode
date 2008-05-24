#include "MyEDmodules/LjmetAnalyzer/src/LjmetAnalHistos.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <iostream>

//======================================================================

using namespace std;

typedef unsigned int uint32_t;

//======================================================================

void LjmetAnalHistos::bookHistos(AllHistoParams_t& pars,
				 string setid,
				 string setdescr)
{
  char name[80];
  char title[80];

  // Leading electron/jet histos

  sprintf (name, "leEThist%s", setid.c_str()); 
  sprintf (title, "Leading electron ET, %s;GeV", setdescr.c_str()); 
  h1d_leET = new TH1D(name, title,
		      pars.ethtmet.nbins,
		      pars.ethtmet.min,
		      pars.ethtmet.max);

  sprintf (name, "leEtahist%s", setid.c_str()); 
  sprintf (title, "Leading electron Eta, %s", setdescr.c_str()); 
  h1d_leEta = new TH1D(name,title, 50, -5.0, 5.0);

  sprintf (name, "lePhihist%s", setid.c_str()); 
  sprintf (title, "Leading electron Phi, %s", setdescr.c_str()); 
  h1d_lePhi = new TH1D(name,title, 50, -5.0, 5.0);

  sprintf (name, "ljEThist%s", setid.c_str()); 
  sprintf (title, "Leading Jet ET, %s;GeV", setdescr.c_str()); 
  h1d_ljET = new TH1D(name,title,
			      pars.ethtmet.nbins,
			      pars.ethtmet.min,
			      pars.ethtmet.max);

  sprintf (name, "ljEtahist%s", setid.c_str()); 
  sprintf (title, "Leading Jet Eta, %s", setdescr.c_str()); 
  h1d_ljEta = new TH1D(name,title,
			       50, -5.0, 5.0);

  sprintf (name, "ljPhihist%s", setid.c_str()); 
  sprintf (title, "Leading Jet Phi, %s", setdescr.c_str()); 
  h1d_ljPhi = new TH1D(name,title,
			       50, -5.0, 5.0);

  // All electron/jet histos

  sprintf (name, "alleEThist%s", setid.c_str()); 
  sprintf (title, "All reco electrons ET, %s;GeV", setdescr.c_str()); 
  h1d_alleET = new TH1D(name, title,
			pars.ethtmet.nbins,
			pars.ethtmet.min,
			pars.ethtmet.max);

  // MET, etc.

  sprintf (name, "METh%s", setid.c_str());
  sprintf (title, "MET, %s;GeV", setdescr.c_str()); 
  h1d_met         = new TH1D(name, title,
			      pars.ethtmet.nbins,
			      pars.ethtmet.min,
			      3*pars.ethtmet.max);
#if 0
  sprintf (name, "HTh%s", setid.c_str());
  sprintf (title, "HT, %s;GeV", setdescr.c_str()); 
  h1d_ht          = new TH1D(name, title,
			     pars.ethtmet.nbins,
			     pars.ethtmet.min,
			     3*pars.ethtmet.max);

  sprintf (name, "HT+METh%s", setid.c_str());
  sprintf (title, "HT+MET, %s;GeV", setdescr.c_str()); 
  h1d_htplusmet   = new TH1D(name, title,
			     pars.ethtmet.nbins,
			     pars.ethtmet.min,
			     3*pars.ethtmet.max);

  sprintf (name, "metsigh%s", setid.c_str());
  sprintf (title, "METsignificance, %s;MET/sqrt(Ht) (sqrt(GeV))", setdescr.c_str()); 
  h1d_metsignificance = new TH1D(name, title,
				 pars.signif.nbins,
				 pars.signif.min,
				 pars.signif.max);
#endif
  sprintf (name, "numeh%s", setid.c_str());
  sprintf (title, "# ID'd electrons, %s;Gen;Reco", setdescr.c_str()); 
  h2f_numElecs    = new TH2F(name, title,
			     pars.numobj.nbins,
			     pars.numobj.min,
			     pars.numobj.max,
			     pars.numobj.nbins,
			     pars.numobj.min,
			     pars.numobj.max);

  sprintf (name, "numjh%s", setid.c_str());
  sprintf (title, "# jets, %s;# GenJets;# RecoJets", setdescr.c_str()); 
  h2f_numJets     = new TH2F(name, title,
			     pars.numobj.nbins,
			     pars.numobj.min,
			     pars.numobj.max,
			     pars.numobj.nbins,
			     pars.numobj.min,
			     pars.numobj.max);

  sprintf (name, "numevsjh%s", setid.c_str());
  sprintf (title, "# reco els vs reco jets, %s;# RecoJets;# RecoEls", setdescr.c_str()); 
  h2f_numElecsVsNumJets = new TH2F(name, title,
				   pars.numobj.nbins,
				   pars.numobj.min,
				   pars.numobj.max,
				   pars.numobj.nbins,
				   pars.numobj.min,
				   pars.numobj.max);

  sprintf (name, "jetetbynumber%s", setid.c_str());
  sprintf (title, "Sorted Jet Spectrum, %s;Jet #; ET(GeV)", setdescr.c_str()); 
  h2f_jetetbynumber = new TH2F(name, title,
			       15, 0.5, 15.5,
			       pars.ethtmet.nbins,
			       pars.ethtmet.min,
			       pars.ethtmet.max);

  sprintf (name, "numjCoHh%s", setid.c_str());
  sprintf (title, "# jets CoHemi, %s;# RecoJets", setdescr.c_str()); 
  h1f_numJetsCoHemi = new TH1F(name, title,
			       pars.numobj.nbins,
			       pars.numobj.min,
			       pars.numobj.max);

  sprintf (name, "numjAntiHh%s", setid.c_str());
  sprintf (title, "# jets AntiHemi, %s;# RecoJets", setdescr.c_str()); 
  h1f_numJetsAntiHemi = new TH1F(name, title,
				 pars.numobj.nbins,
				 pars.numobj.min,
				 pars.numobj.max);

  sprintf (name, "njCoVsAntiHh%s", setid.c_str());
  sprintf (title, "# jets CoHemi vs AntiHemi, %s;# AntiHemi Jets;# CoHemi Jets", setdescr.c_str()); 
  h2f_numJetsCoHemiVsAntiHemi = new TH2F(name, title,
					 pars.numobj.nbins,
					 pars.numobj.min,
					 pars.numobj.max,
					 pars.numobj.nbins,
					 pars.numobj.min,
					 pars.numobj.max);

  sprintf (name, "leljdRh%s", setid.c_str());
  sprintf (title, "leading electron*leading jet dR, %s; ", setdescr.c_str()); 
  h1f_leljdR = new TH1F(name, title, pars.dR.nbins, pars.dR.min, pars.dR.max);

  sprintf (name, "lealljdRh%s", setid.c_str());
  sprintf (title, "leading electron*all jet dR, %s; ", setdescr.c_str()); 
  h1f_lealljdR = new TH1F(name, title, pars.dR.nbins, pars.dR.min, pars.dR.max);

  sprintf (name, "lecjdRh%s", setid.c_str());
  sprintf (title, "leading electron*closest jet dR, %s; ", setdescr.c_str()); 
  h1f_lecjdR = new TH1F(name, title, pars.dR.nbins, pars.dR.min, pars.dR.max);

  sprintf (name, "leljdphih%s", setid.c_str());
  sprintf (title, "leading electron*leading jet dphi, %s; ", setdescr.c_str()); 
  h1f_leljdphi = new TH1F(name, title, pars.dphi.nbins, pars.dphi.min, pars.dphi.max);

  sprintf (name, "lealljdphih%s", setid.c_str());
  sprintf (title, "leading electron*all jet dphi, %s; ", setdescr.c_str()); 
  h1f_lealljdphi = new TH1F(name, title, pars.dphi.nbins, pars.dphi.min, pars.dphi.max);

#if 0
  sprintf (name, "rWmassh%s", setid.c_str());
  sprintf (title, "recoWmass, %s;GeV", setdescr.c_str()); 
  h1d_recoWmass   = new TH1D(name, title, 100, 0.0, 200.0);

  sprintf (name, "rTmassh%s", setid.c_str());
  sprintf (title, "recoTopMass, %s;GeV", setdescr.c_str()); 
  h1d_recoTopMass = new TH1D(name, title, 100, 0.0, 500.0);

  sprintf (name, "rTopEth%s", setid.c_str());
  sprintf (title, "recoTopEt, %s;GeV", setdescr.c_str()); 
  h1d_recoTopEt   = new TH1D(name, title, 100, 0.0, 1000.0);

  sprintf (name, "rTopPth%s", setid.c_str());
  sprintf (title, "recoTopPt, %s;GeV", setdescr.c_str()); 
  h1d_recoTopPt   = new TH1D(name, title, 100, 0.0, 1000.0);

  sprintf (name, "rmassChi2h%s", setid.c_str());
  sprintf (title, "recoMassChi2, %s;GeV^2", setdescr.c_str()); 
  h1d_recomassChi2= new TH1D(name, title, 100, 0.0, 500.0);

  sprintf (name, "rTboosth%s", setid.c_str());
  sprintf (title, "recoTopBoost, %s;recoEt/recoM", setdescr.c_str()); 
  h1d_recoTopBoost= new TH1D(name, title, 50, -0.5, 9.5);

  sprintf (name, "rTedRh%s", setid.c_str()); 
  sprintf (title, "recoTop/electron dR, %s;dR", setdescr.c_str()); 
  h1d_recoTop_e_dr= new TH1D(name, title, 100, 0.0, 10.0);

  sprintf (name, "rTedPhih%s", setid.c_str()); 
  sprintf (title, "recoTop/electron dPhi, %s;dPhi", setdescr.c_str()); 
  h1d_recoTop_e_dphi= new TH1D(name, title, 25, 0.0, 4.0);

  sprintf (name, "rTe+bjetdPhih%s", setid.c_str());
  sprintf (title, "recoTop/e+bjet dPhi, %s;dPhi", setdescr.c_str()); 
  h1d_recoTop_eplusbjet_dphi = new TH1D(name, title, 25, 0.0, 4.0);

  sprintf (name, "e+bjetmassh%s", setid.c_str());
  sprintf (title, "e+bjet mass, %s;GeV", setdescr.c_str()); 
  h1d_eplusbjetmass = new TH1D(name, title, 100, 0.0, 500.0);
#endif
}                                         // LjmetAnalHistos::bookHistos

//======================================================================

void  LjmetAnalHistos::fill(HistoVars_t& vars)
{
  for (uint32_t i=0; i<vars.v_electronETs.size(); i++) {
    h1d_alleET->Fill(vars.v_electronETs[i]);
  }

  if (vars.numElecs > 0) {
    h1d_leET->Fill(vars.maxElectronET);
    h1d_leEta->Fill(vars.maxElectronEta);
    h1d_lePhi->Fill(vars.maxElectronPhi);
  }

#if 0
  for (uint32_t i=0; i<vars.v_maxjet_e_dr.size(); i++)
    h1d_maxjet_e_dr->Fill(vars.v_maxjet_e_dr[i]);
#endif

  if (vars.numJets > 0) {
    h1d_ljET->Fill(vars.leadingjetET);
    h1d_ljEta->Fill(vars.leadingjetEta);
    h1d_ljPhi->Fill(vars.leadingjetPhi);
  }

  h1d_met->Fill(vars.absmet);
#if 0
  h1d_ht->Fill(vars.ht);

  h1d_htplusmet->Fill(vars.htplusmet);

  //  if (vars.metsignificance > 0.0)
  h1d_metsignificance->Fill(vars.metsignificance);
#endif
  h2f_numElecs->Fill(vars.genElectronCount,
		     vars.numElecs);
  h2f_numJets->Fill((float)vars.genQuarkCount,
		    (float)vars.numJets);

  h2f_numElecsVsNumJets->Fill(vars.numJets,vars.numElecs);

  h1f_numJetsCoHemi->Fill((float)vars.numJetsCoHemi);
  h1f_numJetsAntiHemi->Fill((float)vars.numJetsAntiHemi);

  h2f_numJetsCoHemiVsAntiHemi->Fill((float)vars.numJetsAntiHemi,
				    (float)vars.numJetsCoHemi);

  for (unsigned int i=0; i<vars.v_jetETs.size(); i++) {
    h2f_jetetbynumber->Fill((i+1)*1.0,vars.v_jetETs[i]);
  }

  double mindR = 1e99;
  for (unsigned int i=0; i<vars.v_ejetdR.size(); i++) {
    double dR = vars.v_ejetdR[i];
    if (!i) h1f_leljdR->Fill(dR);
    if (dR < mindR) mindR = dR;
    h1f_lealljdR->Fill(vars.v_ejetdR[i]);
  }
  if (vars.v_ejetdR.size())
    h1f_lecjdR->Fill(mindR);

  for (unsigned int i=0; i<vars.v_ejetdphi.size(); i++) {
    if (!i) h1f_leljdphi->Fill(vars.v_ejetdphi[0]);
    h1f_lealljdphi->Fill(vars.v_ejetdphi[i]);
  }

#if 0
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
#endif
}                                          // LjmetAnalHistos::fill
