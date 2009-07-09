// Current issues:
//
#include "DataFormats/JetReco/interface/CommonJetData.h"
#include "MyAnalyzers/Stop2TopAnalyzer/src/Stop2TopAnalAlgos.h"
#include "MyAnalyzers/Stop2TopAnalyzer/src/pdgid.h"

#include <iostream>

//======================================================================

using namespace std;
using namespace HepMC;

//======================================================================

//======================================================================

Stop2TopAnalAlgos::Stop2TopAnalAlgos(bool           verbosity,
				     string         outRootFileName,
				     double         emfraction4eid,
				     double         recjetETgevthresh,
				     double         maxplottedETgev,
				     double         hltcut1_electronETminGeV,
				     double         cut2_electronETminGeV,
				     int            cut3_minNumJets,
				     double         cut4_maxDphirTeb,
				     double         cut5_minHTplusMETgev) :
  verbosity_(verbosity),
  outRootFileName_(outRootFileName),
  emfraction4eid_(emfraction4eid),
  recjetETgevthresh_(recjetETgevthresh),
  maxplottedETgev_(maxplottedETgev),
  hltcut1_electronETminGeV_(hltcut1_electronETminGeV),
  cut2_electronETminGeV_(cut2_electronETminGeV),
  cut3_minNumJets_(cut3_minNumJets),
  cut4_maxDphirTeb_(cut4_maxDphirTeb),
  cut5_minHTplusMETgev_(cut5_minHTplusMETgev)
{
  if (hltcut1_electronETminGeV < 5.0) { // required by JEScor
    throw cms::Exception("Error on input", "hltcut1_electronETminGeV too low");
  }
  if (recjetETgevthresh < 5.0) { // required by JEScor
    throw cms::Exception("Error on input", "recjetETgevthresh too low");
  }
}

//======================================================================

void Stop2TopAnalAlgos::beginJob(void)
{
  vector<int> pdgidvec;
  int hn=0;

  /****************************************
   * Initialize Generated Particle Record
   ****************************************/

  pdgidvec.push_back(downq);
  pdgidvec.push_back(upq);
  pdgidvec.push_back(strangeq);
  pdgidvec.push_back(charmq);
  pdgidvec.push_back(bottomq);
  pdgidvec.push_back(topq);

  pdgidvec.push_back(electron);
  pdgidvec.push_back(nu_e);
  pdgidvec.push_back(muminus);
  pdgidvec.push_back(nu_mu);
  pdgidvec.push_back(tauminus);
  pdgidvec.push_back(nu_tau);

  vars.myprec = new myParticleRecord(pdgidvec);

  outf = new TFile(outRootFileName_.c_str(),"RECREATE");

  /*************************
   * Initialize cut vectors
   *************************/

  // "Zeroth" cut is no cut, never active;
  v_cuts.push_back(false);
  v_cutstrings.push_back("no cuts");
  v_plots_sig.push_back(new cutPlots(maxplottedETgev_,hn++,"no cuts, sig"));
  v_plots_oth.push_back(new cutPlots(maxplottedETgev_,hn++,"no cuts, oth"));
  v_nevents_sig.push_back(0);
  v_nevents_oth.push_back(0);

  // First cut is the HLT selection - electron ET min
  v_cuts.push_back(false);
  v_cutstrings.push_back("hltminElectronET");
  v_plots_sig.push_back(new cutPlots(maxplottedETgev_,hn++,"HLT eET cut, sig"));
  v_plots_oth.push_back(new cutPlots(maxplottedETgev_,hn++,"HLT eET cut, oth"));
  v_nevents_sig.push_back(0);
  v_nevents_oth.push_back(0);

  // Second cut is another electron minGeV above HLT threshold
  v_cuts.push_back(false);
  v_cutstrings.push_back("electronETminGeV");
  v_plots_sig.push_back(new cutPlots(maxplottedETgev_,hn++,"electronET cut, sig"));
  v_plots_oth.push_back(new cutPlots(maxplottedETgev_,hn++,"electronET cut, oth"));
  v_nevents_sig.push_back(0);
  v_nevents_oth.push_back(0);

  // Third cut is # of additional jets
  v_cuts.push_back(false);
  v_cutstrings.push_back("minNumJets");
  v_plots_sig.push_back(new cutPlots(maxplottedETgev_,hn++,"#jets cut, sig"));
  v_plots_oth.push_back(new cutPlots(maxplottedETgev_,hn++,"#jets cut, oth"));
  v_nevents_sig.push_back(0);
  v_nevents_oth.push_back(0);

  // Fourth cut is deltaphi(e+bjet,recoTop)
  v_cuts.push_back(false);
  v_cutstrings.push_back("dPhi(e+b,rT)");
  v_plots_sig.push_back(new cutPlots(maxplottedETgev_,hn++,"e+b,rT dphi cut, sig"));
  v_plots_oth.push_back(new cutPlots(maxplottedETgev_,hn++,"e+b,rT dphi cut, oth"));
  v_nevents_sig.push_back(0);
  v_nevents_oth.push_back(0);

  // Fifth cut is minGeV of HT+MET
  v_cuts.push_back(false);
  v_cutstrings.push_back("HT+METminGeV");
  v_plots_sig.push_back(new cutPlots(maxplottedETgev_,hn++,"minHT+MET cut, sig"));
  v_plots_oth.push_back(new cutPlots(maxplottedETgev_,hn++,"minHT+MET cut, oth"));
  v_nevents_sig.push_back(0);
  v_nevents_oth.push_back(0);
}

//======================================================================

void Stop2TopAnalAlgos::endJob()
{
  char s[80];

  outf->Write();

  /******************************
   * Output the cut statistics
   ******************************/

  for (uint32_t i=0; i<v_nevents_sig.size(); i++) {
    if (!i)
      sprintf (s, "Cut %d: %20s, #sig events = %5d (100.0%%), #other events = %5d (100.0%%)",
	       i, v_cutstrings[i], v_nevents_sig[i], v_nevents_oth[i]);
    else
      sprintf (s, "Cut %d: %20s, #sig events = %5d (%5.1f%%), #other events = %5d (%5.1f%%)",
	       i, v_cutstrings[i],
	       v_nevents_sig[i], 100.*(float)v_nevents_sig[i]/(float)v_nevents_sig[i-1],
	       v_nevents_oth[i], 100.*(float)v_nevents_oth[i]/(float)v_nevents_oth[i-1]);

    cout << s << endl;
  }

  delete vars.myprec;

  delete outf;
  for (uint32_t i=0; i<v_plots_sig.size(); i++) {
    delete v_plots_sig[i];
    delete v_plots_oth[i];
  }
}

//======================================================================

void Stop2TopAnalAlgos::classifyEvent(const HepMC::GenEvent* MCEvt)
{
  bool found_Wplus      = false;
  bool found_Wminus     = false;
  bool stop2top_event   = false;
  bool background_event = false;

  int nParticles = MCEvt->particles_size();
  int nVertices  = MCEvt->vertices_size();
    
  if (verbosity_) {
    cout << "          Number of Raw Particles collected:         ";
    cout << nParticles << endl;
    cout << "          Number of Vertices collected:         ";
    cout << nVertices << endl;
  }

  vars.myprec->clearcounts();
  vars.eventclass = 0;

  // Classify the event by the decay of the W's from
  // the stop->top->W + b chain
  //
  GenEvent::vertex_const_iterator v;
  for (v  = MCEvt->vertices_begin();
       v != MCEvt->vertices_end();
       ++v ) {

    std::vector< GenParticle * >  theChildren((*v)->listChildren());
    std::vector< GenParticle * >  theParents((*v)->listParents());

    int p_pdgid = (*theParents.begin())->pdg_id();

    // Verify this is a stop to top event
    if (!stop2top_event && p_pdgid==stop1) {
      bool found_top = false;
      bool found_x01 = false;
      std::vector< GenParticle * >::const_iterator c;
      for (c=theChildren.begin(); c!=theChildren.end(); c++) {
	int c_pdgid = (*c)->pdg_id();
	if (c_pdgid == topq) found_top = true;
	if (c_pdgid == neutralino01) found_x01 = true;
      }
      if (found_top && found_x01)
	stop2top_event = true;
    }

    if ((p_pdgid==topq) ||
        (p_pdgid==antitopq)) {
      if (!stop2top_event)
	background_event = true; // should not find top before finding stop

      if (verbosity_)
	cout << "Parent is: " << p_pdgid << ", children are: ";

      std::vector< GenParticle * >::const_iterator c;
      for (c=theChildren.begin(); c!=theChildren.end(); c++) {
	int c_pdgid = (*c)->pdg_id();
	if (verbosity_) cout << c_pdgid << " ";
	if (c_pdgid != Wplus) vars.myprec->add(c_pdgid);
      }
      if (verbosity_) cout << endl;
    }

    if (!found_Wplus && p_pdgid == Wplus) {
      found_Wplus = true;

      if (verbosity_)
	cout << "Parent is: " << p_pdgid << ", children are: ";

      std::vector< GenParticle * >::const_iterator c;
      for (c=theChildren.begin(); c!=theChildren.end(); c++) {
	int c_pdgid = (*c)->pdg_id();
	if (verbosity_) cout << c_pdgid << " ";
	if (c_pdgid != Wplus) vars.myprec->add(c_pdgid);
      }
      if (verbosity_) cout << endl;
    }

    if (!found_Wminus && p_pdgid == Wminus) {
      found_Wminus = true;

      if (verbosity_)
	cout << "Parent is: " << p_pdgid << ", children are: ";

      std::vector< GenParticle * >::const_iterator c;
      for (c=theChildren.begin(); c!=theChildren.end(); c++) {
	int c_pdgid = (*c)->pdg_id();
	if (verbosity_) cout << c_pdgid << " ";
	if (c_pdgid != Wminus) vars.myprec->add(c_pdgid);
      }
      if (verbosity_) cout << endl;
    }
  } // vertex loop

  int quarkcount = vars.myprec->quarkcount();

  if ((!stop2top_event && !background_event) ||
      !found_Wplus || !found_Wminus)
    vars.eventclass = 0;
  else {
    if ((vars.myprec->count(electron) == 2) &&
	(vars.myprec->count(nu_e) == 2) &&
	quarkcount >= 2)
      vars.eventclass = 9;
    else if ((vars.myprec->count(muminus)  == 2) &&
	     (vars.myprec->count(nu_mu)    == 2) &&
	     quarkcount >= 2)
      vars.eventclass = 10;
    else if ((vars.myprec->count(tauminus) == 2) &&
	     (vars.myprec->count(nu_tau)   == 2) &&
	     quarkcount >= 2)
      vars.eventclass = 8;
    else if ((vars.myprec->count(electron) == 1) &&
	     (vars.myprec->count(muminus)  == 1) &&
	     (vars.myprec->count(nu_e)     == 1) &&
	     (vars.myprec->count(nu_mu)    == 1) &&
	     quarkcount >= 2)
      vars.eventclass = 7;
    else if ((vars.myprec->count(electron) == 1) &&
	     (vars.myprec->count(tauminus) == 1) &&
	     (vars.myprec->count(nu_e)     == 1) &&
	     (vars.myprec->count(nu_tau)   == 1) &&
	     quarkcount >= 2)
      vars.eventclass = 5;
    else if ((vars.myprec->count(muminus)  == 1) &&
	     (vars.myprec->count(tauminus) == 1) &&
	     (vars.myprec->count(nu_mu)    == 1) &&
	     (vars.myprec->count(nu_tau)   == 1) &&
	     quarkcount >= 2)
      vars.eventclass = 6;
    else if ((vars.myprec->count(electron) == 1) &&
	     (quarkcount >= 4))
      vars.eventclass = 3;
    else if ((vars.myprec->count(muminus) == 1) &&
	     (quarkcount >= 4))
      vars.eventclass = 4;
    else if ((vars.myprec->count(tauminus) == 1) &&
	     (quarkcount >= 4))
      vars.eventclass = 2;
    else if (quarkcount >= 6)
      vars.eventclass = 1;
    else
      vars.eventclass = 0;
  }

  if (background_event)
    vars.eventclass *= -1;

  if (verbosity_)
    cout << "event class = " << vars.eventclass << endl;

  if (verbosity_ || !vars.eventclass) {
    cout << "#e = " << vars.myprec->count(electron);
    cout << ", #nu_e = " << vars.myprec->count(nu_e);
    cout << ", #mu- = " << vars.myprec->count(muminus);
    cout << ", #nu_mu = " << vars.myprec->count(nu_mu);
    cout << ", #tau- = " << vars.myprec->count(tauminus);
    cout << ", #nu_tau = " << vars.myprec->count(nu_tau);
    cout << ", #quark = " << quarkcount << endl;
  }
}                                    // Stop2TopAnalAlgos::classifyEvent

//======================================================================

double Stop2TopAnalAlgos::JEScor(double reta, double rEt)
{
  double correction = 0.0;

  if (fabs(reta) <= 1.4) {           // BARREL
    if      ((rEt >=   5.0) &&
	     (rEt <   10.0))      correction = 1.0/0.389;
    else if ((rEt >=  10.0) &&
	     (rEt <   20.0))      correction = 1.0/0.566;
    else if ((rEt >=  20.0) &&
	     (rEt <   50.0))      correction = 1.0/0.666;
    else if ((rEt >=  50.0) &&
	     (rEt <  200.0))      correction = 1.0/0.745;
    else if  (rEt >= 200.0)       correction = 1.0/0.788;
  }
  else if ((fabs(reta) >  1.4) &&
	   (fabs(reta) <= 3.0)) {   // ENDCAP
    if      ((rEt >=   5.0) &&
	     (rEt <   10.0))      correction = 1.0/0.186;
    else if ((rEt >=  10.0) &&
	     (rEt <   20.0))      correction = 1.0/0.221;
    else if ((rEt >=  20.0) &&
	     (rEt <   50.0))      correction = 1.0/0.268;
    else if ((rEt >=  50.0) &&
	     (rEt <  200.0))      correction = 1.0/0.325;
    else if  (rEt >= 200.0)       correction = 1.0/0.325; // set to previous
  }
  else if ((fabs(reta) >  3.0) &&
	   (fabs(reta) <= 5.0)) {   // HF
    if      ((rEt >=   5.0) &&
	     (rEt <   10.0))      correction = 1.0/0.443;
    else if ((rEt >=  10.0) &&
	     (rEt <   20.0))      correction = 1.0/0.61;
    else if ((rEt >=  20.0) &&
	     (rEt <   50.0))      correction = 1.0/0.681;
    else if ((rEt >=  50.0) &&
	     (rEt <  200.0))      correction = 1.0/0.726;
    else if  (rEt >= 200.0)       correction = 1.0/0.726; // set to previous
  }

  return correction;
}                                           // Stop2TopAnalAlgos::JEScor

//======================================================================

bool Stop2TopAnalAlgos::isElectron(CaloJet& cj)
{
  //  double theET = cj.et() * JEScor(cj.eta(), cj.et());
  double theET = cj.et();

  return ((cj.energyFractionInECAL() >= emfraction4eid_) &&
	  (theET >= hltcut1_electronETminGeV_) &&
	  (fabs(cj.eta()) < 1.4 )); // barrel only (endcap n.a.)
}

//======================================================================

static double deltaPhi(double phi1, double phi2) {
  double dPhi=fabs(phi1-phi2);
  if (dPhi>M_PI) dPhi=2*M_PI-dPhi;
  return dPhi;
}

//======================================================================

static double calcdR(double eta1, double eta2,
		     double phi1, double phi2) {
  double delta1 = (eta1-eta2);
  double delta2 = deltaPhi(phi1,phi2);
  return sqrt(delta1*delta1 + delta2*delta2);
}

//======================================================================

void Stop2TopAnalAlgos::fillJetVecs(CaloJetCollection& ejetcandidates,
				    CaloJetCollection& hjetcandidates)
{
  ejets.clear();
  hjets.clear();

  vars.numjets = 0;
  vars.numjetsoverthresh = 0;

  CaloJetCollection::iterator ij;
  for (ij=ejetcandidates.begin(); ij != ejetcandidates.end(); ij++) {
    if (isElectron(*ij))
      ejets.push_back(*ij);
  }

  for (ij=hjetcandidates.begin(); ij != hjetcandidates.end(); ij++) {
    bool   jetis_e = false;
    double jescor  = JEScor(ij->eta(), ij->et());

    // Also check if this jet has any cone overlap with the previously
    // identified electrons, and erase from further consideration if so.
    //
    CaloJetCollection::iterator ej;
    for (ej  = ejets.begin(); ej != ejets.end(); ej++) {
      if (calcdR(ej->eta(), ij->eta(),
		 ej->phi(), ij->phi()) < dRisolationMin_) {
	jetis_e = true;
	break;
      }
    }

    if (!jetis_e) {
      vars.numjets++;

      CommonJetData common (ij->px()*jescor, ij->py()*jescor, ij->pz()*jescor,
			    ij->energy()*jescor, ij->nConstituents());

      CaloJet theJet (common, ij->getSpecific (), ij->getTowerIndices());

      if (theJet.et() >= recjetETgevthresh_) {
	hjets.push_back(theJet);
	vars.numjetsoverthresh++;
      }
    }
  }
}                                      // Stop2TopAnalAlgos::fillJetVecs

//======================================================================

void Stop2TopAnalAlgos::calcVars()
{
  double etxtot=0.0, etytot=0.0;

  vars.absmet        = 0.;
  vars.ht            = 0.;
  vars.htplusmet     = 0.;
  vars.leadingjetET  = 0.;
  vars.maxElectronET = 0.;
  vars.v_electronETs.clear();
#if 0
  vars.v_maxjet_e_dr.clear();
#endif

  /****************************************
   * Collect electron "jet" statistics
   ****************************************/

  CaloJetCollection::iterator ij;
  emaxij=ejets.end();
  for (ij=ejets.begin(); ij != ejets.end(); ij++) {
    double theET = ij->et();

    vars.v_electronETs.push_back(theET);
    if (theET > vars.maxElectronET) {
	vars.maxElectronET = theET;
	emaxij = ij;
    }

    etxtot  += theET*cos(ij->phi());
    etytot  += theET*sin(ij->phi());
    vars.ht += theET;
  }

  /****************************************
   * Collect hadron jet statistics
   ****************************************/

  double maxET = 0.0;
  CaloJetCollection::iterator hmaxij = hjets.end();
  for (ij=hjets.begin(); ij != hjets.end(); ij++) {
    double theET = ij->et();
      
    etxtot  += theET*cos(ij->phi());
    etytot  += theET*sin(ij->phi());
    vars.ht += theET;

    if (theET > maxET) {
      maxET = theET;
      hmaxij = ij;
    }
  }

  vars.absmet    = sqrt(etxtot*etxtot + etytot*etytot);
  vars.htplusmet = vars.ht + vars.absmet;
  vars.metsignificance = vars.absmet/sqrt(vars.ht);

  if (maxET > 0.0) {
    vars.leadingjetET = maxET;

    if (maxET > maxplottedETgev_)
      cout << "Max plotted ET exceeded: " << maxET << endl;

    /**********************************************
     * ...max ET jet/electron angle distribution
     **********************************************/
#if 0
    CaloJetCollection::iterator ej;
    for (ej  = ejets.begin(); ej != ejets.end(); ej++) {
      vars.v_maxjet_e_dr.push_back(calcdR(ej->eta(), maxij->eta(),
					  ej->phi(), maxij->phi()));
    }
#endif
  }
}                                         // Stop2TopAnalAlgos::calcVars

//======================================================================

static const double Wmass   = 80.425;
static const double topMass = 172.3;

void Stop2TopAnalAlgos::recoWandTop()
{
  vars.recoWmass      = 0.0;
  vars.recoTopMass    = 0.0;
  vars.recoTopEt      = 0.0;
  vars.recoTopBoost   = 0.0;
  vars.recoTop_e_dr   = 0.0;
  vars.recoTop_e_dphi = 0.0;
  vars.recomassChi2   = 0.0;
  vars.eplusbjetmass  = 0.0;
  vars.recoTop_eplusbjet_dphi = 0.0;

  if (hjets.size() < 4) return;

  double minChi2 = 1e99;

  // Loop over possible combinations of 2 jets reconstructing a W,
  // and that combination plus a 3rd jet (b jet) reconstructing the top.
  // Minimize the chi2 w.r.t. the known masses of the particles.
  //
  CaloJetCollection::iterator bestij1,bestij2,bestij3;
  CommonJetData::LorentzVector recoTopLV;

  CaloJetCollection::iterator ij1;
  for (ij1=hjets.begin(); ij1 != hjets.end(); ij1++) {
    CaloJetCollection::iterator ij2;
    for (ij2=ij1+1; ij2 != hjets.end(); ij2++) {

      CommonJetData::LorentzVector lvj1  = ij1->getCommonData().mP4;
      CommonJetData::LorentzVector lvj2  = ij2->getCommonData().mP4;
      CommonJetData::LorentzVector sum2j = lvj1 + lvj2;

      double tstmassW = sum2j.M();
      double Wchi2    = (tstmassW - Wmass)*(tstmassW - Wmass);

      CaloJetCollection::iterator ij3;
      for (ij3=hjets.begin(); ij3 != hjets.end(); ij3++) {
	if ((ij3 == ij1) || (ij3 == ij2)) continue;

	CommonJetData::LorentzVector lvj3  = ij3->getCommonData().mP4;
	CommonJetData::LorentzVector sum3j = sum2j + lvj3;

	double tstmassTop = sum3j.M();
	double topChi2 = (2./3.)*(tstmassTop - topMass)*(tstmassTop - topMass);

	double sumChi2 = Wchi2+topChi2;
	if (sumChi2 < minChi2) {
	  minChi2           = sumChi2;
	  bestij1           = ij1;
	  bestij2           = ij2;
	  bestij3           = ij3;
	  recoTopLV         = sum3j;
	  vars.recoWmass    = tstmassW;
	  vars.recoTopMass  = tstmassTop;
	  vars.recoTopEt    = recoTopLV.E()*sin(recoTopLV.Theta());
	  vars.recoTopPt    = recoTopLV.Pt();
	  vars.recoTopBoost = vars.recoTopEt/vars.recoTopMass;
	  vars.recomassChi2 = minChi2;

	  if (emaxij != ejets.end()) {
	    vars.recoTop_e_dr = calcdR(emaxij->eta(), recoTopLV.Eta(),
				       emaxij->phi(), recoTopLV.Phi());
	    vars.recoTop_e_dphi = deltaPhi(emaxij->phi(), recoTopLV.Phi());
	  }
	}
      } // inner loop
    } // middle loop
  } // outer loop

  // Found the best combination, now find the highest ET jet remaining
  // and call that the b-jet from the other top decay.
  //
  double maxET = 0.0;
  CaloJetCollection::iterator hmaxij = hjets.end();

  for (ij1=hjets.begin(); ij1 != hjets.end(); ij1++) {
    if ((ij1 == bestij1) ||
	(ij1 == bestij2) ||
	(ij1 == bestij3)) continue;
      
    double theET = ij1->et();
    if (theET > maxET) {
      maxET = theET;
      hmaxij = ij1;
    }
  }

  // ...and recombine with the electron for further study.
  if ((hmaxij != hjets.end()) &&
      (emaxij != ejets.end())) {
    CommonJetData::LorentzVector lvj4  = hmaxij->getCommonData().mP4;
    CommonJetData::LorentzVector elv   = emaxij->getCommonData().mP4;
    CommonJetData::LorentzVector sumj4e = lvj4 + elv;

    vars.eplusbjetmass = sumj4e.M();
    vars.recoTop_eplusbjet_dphi = deltaPhi(sumj4e.Phi(),recoTopLV.Phi());
  }
}                                      // Stop2TopAnalAlgos::recoWandTop

//======================================================================

void Stop2TopAnalAlgos::applyCutsAndAccount()
{
  bool totalcut = false;
  int  hn=1;

  /***********************************
   * Determine what cuts are active
   ***********************************/
  v_cuts[hn++] = (vars.maxElectronET          < hltcut1_electronETminGeV_);
  v_cuts[hn++] = (vars.maxElectronET          < cut2_electronETminGeV_);
  v_cuts[hn++] = (vars.numjetsoverthresh      < cut3_minNumJets_);
  v_cuts[hn++] = (vars.recoTop_eplusbjet_dphi > cut4_maxDphirTeb_);
  v_cuts[hn]   = (vars.htplusmet              < cut5_minHTplusMETgev_);

  /*****************
   * Do accounting
   *****************/

  for (uint32_t i=0; i<v_cuts.size(); i++) {
    totalcut = totalcut || v_cuts[i];
    if (!totalcut) {
      if (abs(vars.eventclass) == 3) {
	v_nevents_sig[i]++;
	v_plots_sig[i]->fill(vars);
      }
      else {
	v_nevents_oth[i]++;
	v_plots_oth[i]->fill(vars);
      }
    }
    else
      break;
  }
}                              // Stop2TopAnalAlgos::applyCutsAndAccount

//======================================================================

void Stop2TopAnalAlgos::processEvent(edm::HepMCProduct& HepMCEvt,
				     CaloJetCollection& recjets2,
				     CaloJetCollection& recjets5,
				     CaloJetCollection& recjets7)
{
  const HepMC::GenEvent* MCEvt = HepMCEvt.GetEvent();

  classifyEvent(MCEvt);

  dRisolationMin_ = 0.5+0.2;
  fillJetVecs(recjets2, recjets5);

  calcVars();

  recoWandTop();

  applyCutsAndAccount();
}

//======================================================================
