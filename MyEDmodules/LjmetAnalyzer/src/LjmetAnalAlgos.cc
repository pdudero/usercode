// Current issues:
//
#include "MyEDmodules/LjmetAnalyzer/src/LjmetAnalAlgos.h"
#include "MyEDmodules/LjmetAnalyzer/src/pdgid.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"

#include <iostream>
#include <vector>

//======================================================================

using namespace std;
//using namespace HepMC;
using namespace reco;

//======================================================================

typedef math::XYZTLorentzVector LorentzVector;

template <class T>
class CompareET {
public:
  bool operator()(const T& h1,
		  const T& h2) const {
    return h1.et()>h2.et();
  }
};//needed for sorting by et

template <class T>
class CompareETp {
public:
  bool operator()(const T& h1,
		  const T& h2) const {
    return h1->et()>h2->et();
  }
};//needed for sorting by et

//======================================================================

LjmetAnalAlgos::LjmetAnalAlgos(bool verbosity,
			       const edm::ParameterSet& iConfig) :
  verbosity_(verbosity)
{
  outRootFileName_ = iConfig.getUntrackedParameter<string>("outRootFileName");
  evtclass_ = new GenEvtClass(verbosity);

  filter_eJetIsolationMindR_ = iConfig.getParameter<double>("filter_eJetIsolationMindR");
  filter_recjetETminGeV_     = iConfig.getParameter<double>("filter_recjetETminGeV");
  filter_elecETminGeV_       = iConfig.getParameter<double>("filter_elecETminGeV");

  cut_jetETminGeV_           = iConfig.getParameter<double>("cut_jetETminGeV");
  cut_minNumJets_            = iConfig.getParameter<int>("cut_minNumJets");

  hpars_.ethtmet.nbins = iConfig.getUntrackedParameter<int>("ethtmetNbins");
  hpars_.ethtmet.min   = iConfig.getUntrackedParameter<double>("ethtmetMinGeV");
  hpars_.ethtmet.max   = iConfig.getUntrackedParameter<double>("ethtmetMaxGeV");
  hpars_.numobj.nbins  = iConfig.getUntrackedParameter<int>("numobjNbins");
  hpars_.numobj.min    = iConfig.getUntrackedParameter<double>("numobjMinGeV");
  hpars_.numobj.max    = iConfig.getUntrackedParameter<double>("numobjMaxGeV");
  hpars_.dR.nbins      = iConfig.getUntrackedParameter<int>("dRnbins");
  hpars_.dR.min        = iConfig.getUntrackedParameter<double>("dRmin");
  hpars_.dR.max        = iConfig.getUntrackedParameter<double>("dRmax");
  hpars_.dphi.nbins    = iConfig.getUntrackedParameter<int>("dphiNbins");
  hpars_.dphi.min      = iConfig.getUntrackedParameter<double>("dphiMin");
  hpars_.dphi.max      = iConfig.getUntrackedParameter<double>("dphiMax");
  hpars_.signif.nbins  = iConfig.getUntrackedParameter<int>("signifNbins");
  hpars_.signif.min    = iConfig.getUntrackedParameter<double>("signifMinGeV");
  hpars_.signif.max    = iConfig.getUntrackedParameter<double>("signifMaxGeV");
}

//======================================================================

void
LjmetAnalAlgos::bookOneSet(string descr)
{
  outf->cd();
  TDirectory *dir = outf->mkdir(descr.c_str());
  LjmetCut *pLjmetCut = new LjmetCut(evtclass_,v_cuts.size(),descr, hpars_,dir);
  v_cuts.push_back(pLjmetCut);
}

//======================================================================

void
LjmetAnalAlgos::beginJob(void)
{
  vector<int> pdgidvec;

  outf = new TFile(outRootFileName_.c_str(),"RECREATE");

  /*************************
   * Initialize cut vectors
   *************************/

  // book the histos in order of how the cuts are applied
  bookOneSet("noCuts");
  bookOneSet("ctAtLeast1e1j");
  bookOneSet("ctJetETminGeV");
  bookOneSet("ctMinNumJets");

#if 0
  // Fourth cut is deltaphi(e+bjet,recoTop)
  bookOneSet("dPhi(e+b,rT)");

  // Fifth cut is minGeV of HT+MET
  bookOneSet("HT+METminGeV");
#endif
}

//======================================================================

void
LjmetAnalAlgos::endJob()
{
  char s[80];

  outf->Write();

  /******************************
   * Output the cut statistics
   ******************************/

  for (uint32_t i=0; i<v_cuts.size(); i++) {
    sprintf (s, "Cut %d: %20s", i, v_cuts[i]->description().c_str());
    cout << s << endl;
    for (int j=0; j<evtclass_->numClasses(); j++) {
      for (int k=0; k<evtclass_->numSignatureTypes(); k++) {
	int nev  = v_cuts[i]->nEvents(j,k);
	int nev0 = v_cuts[0]->nEvents(j,k);
	if (nev0) { // don't bother printing out things you never had.
	  if (!i)
	    sprintf (s, "\tclass %d sig %d #events = %5d (100.0%%)",
		     j, k, nev);
	  else
	    sprintf (s, "\tclass %d sig %d #events = %5d (%5.1f%%)",
		     j, k, v_cuts[i]->nEvents(j,k),
		     100.*(float)nev/(float)v_cuts[i-1]->nEvents(j,k));
	  cout << s << endl;
	}
      }
    }
  }

  delete evtclass_;

  delete outf;
  for (uint32_t i=0; i<v_cuts.size(); i++) {
    delete v_cuts[i];
  }
}

//======================================================================

LjmetAnalAlgos::LjmetCut::LjmetCut(GenEvtClass *pEvtclass,
				   int cutnum,
				   std::string descr,
				   LjmetAnalHistos::AllHistoParams_t hpars,
				   TDirectory *rootDir) :
  active_(false),
  evtclass_(pEvtclass),
  cutnum_(cutnum),
  descr_(descr),
  hpars_(hpars),
  rootDir_(rootDir)
{
  char name[80];
  char title[80];

  rootDir_->cd();

  sprintf (name, "classhistcut%d", cutnum_); 
  sprintf (title, "classhist, %s", descr_.c_str()); 
  h2f_class      = new TH2F(name,title,5,-0.5,4.5,5,-0.5,4.5);

  for (int i=0; i<evtclass_->numClasses(); i++) {
    h2f_class->GetYaxis()->SetBinLabel(i+1,evtclass_->classDescr(i).c_str());
  }

  for (int i=0; i<evtclass_->numSignatureTypes(); i++)
    h2f_class->GetXaxis()->SetBinLabel
      (i+1,evtclass_->signDescr(GenEvtClass::EnumSignature_t(i)).c_str());
}

//======================================================================

void
LjmetAnalAlgos::LjmetCut::fill(LjmetAnalHistos::HistoVars_t& vars,double weight)
{
  int iec = (int)vars.eventclass;
  int isc = (int)vars.signatureclass;

  if (iec < evtclass_->numClasses()) {

    h2f_class->Fill((float)isc,(float)iec,weight);
    int key = iec*10 + isc;

    std::map<int,LjmetAnalHistos *>::const_iterator it = m_pHistos_.find(key);

    // book histos only if the sample shows evidence of those classified events
    if (it == m_pHistos_.end()) {
      m_pHistos_[key] = new LjmetAnalHistos();
      rootDir_->cd();
      ostringstream ssid;
      ssid << "cut" << cutnum_ << "cls" << iec << "sig" << isc;
      m_pHistos_[key]->bookHistos
	(hpars_,ssid.str(),
	 descr_+evtclass_->classDescr(iec)+evtclass_->signDescr(GenEvtClass::EnumSignature_t(isc)));
      m_pHistos_[key]->fill(vars,weight);
    }
    else
      it->second->fill(vars,weight);
  }
}

//======================================================================
#if 0
bool LjmetAnalAlgos::isElectron(CaloJet& cj)
{
  //  double theET = cj.et() * JEScor(cj.eta(), cj.et());
  double theET = cj.et();

  return ((cj.emEnergyFraction() >= emfraction4eid_) &&
	  (theET >= cut1_HLTelectronETminGeV_) &&
	  (fabs(cj.eta()) < 1.4 )); // barrel only (endcap n.a.)
}
#endif
//======================================================================

static double deltaPhi(double phi1, double phi2)
{
  double dPhi=fabs(phi1-phi2);
  if (dPhi>M_PI) dPhi=2*M_PI-dPhi;
  return dPhi;
}

//======================================================================

static double calcdR(double eta1, double phi1,
		     double eta2, double phi2)
{
  double delta1 = (eta1-eta2);
  double delta2 = deltaPhi(phi1,phi2);
  return sqrt(delta1*delta1 + delta2*delta2);
}

//======================================================================

void
LjmetAnalAlgos::sortElectrons(const RecoCandidateCollection& ElsIn,
				RecoCandidateCollection& sortedEls)
{
  RecoCandidateCollection::const_iterator it;
  //  reco::ElectronCollection::const_iterator it;

  for (it=ElsIn.begin(); it!=ElsIn.end(); it++){
    if ((*it)->et() > filter_elecETminGeV_)
      sortedEls.push_back(*it);
  }

  // sort by decreasing ET
  //
  std::sort(sortedEls.begin(), sortedEls.end(), CompareETp<reco::RecoCandidate *>());
}

//======================================================================

void
LjmetAnalAlgos::filterJets(const std::vector<reco::CaloJet>& JetsIn,
			   const RecoCandidateCollection& ElsIn,
			   std::vector<reco::CaloJet>& filteredJetsOut)
{
  std::vector<reco::CaloJet> sortedJets;
  CaloJetCollection::const_iterator it;

  // sort first
  for (it=JetsIn.begin(); it!=JetsIn.end(); it++){
    sortedJets.push_back(*it);
  }

  // sort by decreasing ET
  //
  std::sort(sortedJets.begin(), sortedJets.end(), CompareET<reco::CaloJet>());

  filteredJetsOut.clear();

  std::vector<reco::CaloJet>::iterator ij;
  for (ij=sortedJets.begin(); ij != sortedJets.end(); ij++) {

    // filter out low ET jets
    if (ij->et() < filter_recjetETminGeV_)
      break; // already sorted by ET, so all the rest are junk

    // Also check if this jet has any cone overlap with the previously
    // identified electrons, and erase from further consideration if so.
    //
    RecoCandidateCollection::const_iterator el;
    for (el  = ElsIn.begin(); el != ElsIn.end(); el++) {
      double dR = calcdR((*el)->eta(), (*el)->phi(),
			 ij->eta(), ij->phi());
      if (dR < filter_eJetIsolationMindR_) break;
    }

    if (el != ElsIn.end()) continue;

    filteredJetsOut.push_back(*ij);
  }
}                                        // LjmetAnalAlgos::filterJets

//======================================================================

void
LjmetAnalAlgos::calcVars(const std::vector<reco::CaloJet>& recjets,
			 const RecoCandidateCollection& elecs,
			 const reco::CaloMETCollection& metIn)
{
  vars_.absmet         = 0.;
  vars_.lemetdphi      = 0.;
  vars_.ht             = 0.;
  vars_.htplusmet      = 0.;
  vars_.leadingjetET   = 0.;
  vars_.leadingjetEta  = 0.;
  vars_.leadingjetPhi  = 0.;
  vars_.maxElectronET  = 0.;
  vars_.maxElectronEta = 0.;
  vars_.maxElectronPhi = 0.;
  vars_.numElecs       = 0;
  vars_.numJets        = 0;

  vars_.numJetsCoHemi   = 0;
  vars_.numJetsAntiHemi = 0;

  vars_.v_electronETs.clear();
  vars_.v_jetETs.clear();
#if 0
  vars_.v_maxjet_e_dr.clear();
#endif

  vars_.v_ejetdR.clear();
  vars_.v_ejetdphi.clear();

  /****************************************
   * Collect electron statistics
   ****************************************/

  vars_.genElectronCount = evtclass_->getElectronCount();

  /* Create own collection of electrons (so that they can be sorted)
  ** and count and categorize
  */

  vars_.numElecs = elecs.size();
  RecoCandidateCollection::const_iterator el = elecs.begin();

  if (vars_.numElecs>0) {
    vars_.maxElectronET  = (*el)->et();
    vars_.maxElectronEta = (*el)->eta();
    vars_.maxElectronPhi = (*el)->phi();

    RecoCandidateCollection::const_iterator e;
    for (e=el; e!=elecs.end(); e++)
      vars_.v_electronETs.push_back((*e)->et());
  }

  /****************************************
   * Collect hadron jet statistics
   ****************************************/

  vars_.genQuarkCount = evtclass_->getQuarkCount();

  vars_.numJets = recjets.size();
  std::vector<CaloJet>::const_iterator lj = recjets.begin();

  if (vars_.numJets>0) {
    vars_.leadingjetET  = lj->et();
    vars_.leadingjetEta = lj->eta();
    vars_.leadingjetPhi = lj->phi();

    std::vector<CaloJet>::const_iterator j;
    for (j=lj; j!=recjets.end(); j++) {
      vars_.v_jetETs.push_back(j->et());
      if (vars_.numElecs>0) {
	CLHEP::Hep3Vector barrelElec3V((*el)->px(), (*el)->py(), (*el)->pz());

	vars_.v_ejetdR.push_back(calcdR(j->eta(),j->phi(),
					(*el)->eta(),(*el)->phi()));

	vars_.v_ejetdphi.push_back(deltaPhi(j->phi(),(*el)->phi()));

	CLHEP::Hep3Vector jet3V(j->px(),j->py(),j->pz());

	double dot = barrelElec3V.dot(jet3V);
	if (dot > 0)
	  vars_.numJetsCoHemi++;
	else if (dot < 0)
	  vars_.numJetsAntiHemi++;
      }
    }
  }

  if (metIn.size() > 0) {
    vars_.absmet = metIn.begin()->sumEt();
    if (vars_.numElecs>0)
      vars_.lemetdphi = deltaPhi(metIn.begin()->phi(),
				 vars_.maxElectronPhi);
  }

#if 0
  vars_.htplusmet = vars_.ht + vars_.absmet;
  vars_.metsignificance = vars_.absmet/sqrt(vars_.ht);

  if (maxET > 0.0) {
    vars_.leadingjetET = maxET;

    /**********************************************
     * ...max ET jet/electron angle distribution
     **********************************************/

    CaloJetCollection::iterator ej;
    for (ej  = ejets.begin(); ej != ejets.end(); ej++) {
      vars_.v_maxjet_e_dr.push_back(calcdR(ej->eta(), maxij->eta(),
					  ej->phi(), maxij->phi()));
    }
  }
#endif
}                                         // LjmetAnalAlgos::calcVars

//======================================================================

void
LjmetAnalAlgos::applyCutsAndAccount(double weight)
{
  bool totalcut = false;
  int  hn=1;

  v_cuts[0]->Activate(false);

  /***********************************
   * Determine what cuts are active
   ***********************************/
  v_cuts[hn++]->Activate(!vars_.numElecs || !vars_.numJets);
  v_cuts[hn++]->Activate(vars_.leadingjetET < cut_jetETminGeV_);
  v_cuts[hn++]->Activate(vars_.numJets < cut_minNumJets_);
#if 0
  v_cuts[hn++]->Activate(vars_.recoTop_eplusbjet_dphi  > cut_maxDphirTeb_);
  v_cuts[hn]->Activate  (vars_.htplusmet              < cut_minHTplusMETgev_);
#endif

  /*****************
   * Do accounting
   *****************/

  for (uint32_t i=0; i<v_cuts.size(); i++) {
    totalcut = totalcut || v_cuts[i]->isActive();
    if (!totalcut)
      v_cuts[i]->fill(vars_, weight);
    else
      break;
  }
}                              // LjmetAnalAlgos::applyCutsAndAccount

//======================================================================

void
LjmetAnalAlgos::analyze(const HepMC::GenEvent& genEvt,
			const reco::CaloJetCollection& recJets,
			const reco::CaloMETCollection& met,
			const RecoCandidateCollection& elecs,
			double weight)
//			const reco::ElectronCollection& elecs)
{
  evtclass_->classifyEvent(genEvt,vars_.eventclass,vars_.signatureclass);

  RecoCandidateCollection  sortedEls;
  sortElectrons(elecs,sortedEls);

  std::vector<reco::CaloJet>  filteredRJs;
  filterJets(recJets,sortedEls,filteredRJs);

  calcVars(filteredRJs, sortedEls, met);

  applyCutsAndAccount(weight);
}

//======================================================================

void
LjmetAnalAlgos::analyze(const reco::GenParticleCollection& genParticles,
			const reco::CaloJetCollection& recJets,
			const reco::CaloMETCollection& met,
			const RecoCandidateCollection& elecs,
			double weight)
//			const reco::ElectronCollection& elecs)
{
  evtclass_->classifyEvent(genParticles,vars_.eventclass,vars_.signatureclass);

  RecoCandidateCollection  sortedEls;
  sortElectrons(elecs,sortedEls);

  std::vector<reco::CaloJet>  filteredRJs;
  filterJets(recJets,sortedEls,filteredRJs);

  calcVars(filteredRJs, sortedEls, met);

  applyCutsAndAccount(weight);
}

//======================================================================
