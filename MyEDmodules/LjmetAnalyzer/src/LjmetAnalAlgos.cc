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

  vector<double> v_eEtThrs   = iConfig.getParameter<vector<double> >("filter_elecETminGeV");

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

  LjmetDatasets_.resize(v_eEtThrs.size());
  for (uint32_t i=0; i<v_eEtThrs.size(); i++)
    LjmetDatasets_[i].filter_elecETminGeV = v_eEtThrs[i];

}                                      // LjmetAnalAlgos::LjmetAnalAlgos

//======================================================================

void
LjmetAnalAlgos::bookOneSet(string descr, vector<LjmetAnalDataset_t>::iterator it)
{
  it->rootdir->cd();
  TDirectory *dir = it->rootdir->mkdir(descr.c_str());
  LjmetCut *pLjmetCut = new LjmetCut(evtclass_,it->v_cuts.size(),descr, hpars_,dir);
  it->v_cuts.push_back(pLjmetCut);
}

//======================================================================

void
LjmetAnalAlgos::beginJob(void)
{
  vector<int> pdgidvec;

  outf = new TFile(outRootFileName_.c_str(),"RECREATE");

  std::vector<LjmetAnalDataset_t>::iterator it;
  for (it = LjmetDatasets_.begin(); it!=LjmetDatasets_.end(); it++) {
    char dirname[80];
    outf->cd();
    sprintf (dirname, "elecETminGeV=%d", (int)it->filter_elecETminGeV);
    it->rootdir = outf->mkdir(dirname);

    /*************************
     * Initialize cut vectors
     *************************/

    // book the histos in order of how the cuts are applied
    bookOneSet("noCuts",        it);
    bookOneSet("ctAtLeast1e1j", it);
    bookOneSet("ctJetETminGeV", it);
    bookOneSet("ctMinNumJets",  it);

#if 0
    // Fourth cut is deltaphi(e+bjet,recoTop)
    bookOneSet("dPhi(e+b,rT)");

    // Fifth cut is minGeV of HT+MET
    bookOneSet("HT+METminGeV");
#endif
  }
}                                           //  LjmetAnalAlgos::beginJob

//======================================================================

void
LjmetAnalAlgos::endJob()
{
  char s[80];

  outf->Write();

  /******************************
   * Output the cut statistics
   ******************************/
  
  std::vector<LjmetAnalDataset_t>::iterator it;
  for (it=LjmetDatasets_.begin(); it!=LjmetDatasets_.end(); it++) {

    cout << "=====================" << it->rootdir->GetName();
    cout << "=====================" << endl;

    for (uint32_t i=0; i<it->v_cuts.size(); i++) {
      sprintf (s, "Cut %d: %20s", i, it->v_cuts[i]->description().c_str());
      cout << s << endl;
      for (int j=0; j<evtclass_->numClasses(); j++) {
	for (int k=0; k<evtclass_->numSignatureTypes(); k++) {
	  int nev  = it->v_cuts[i]->nEvents(j,k);
	  int nev0 = it->v_cuts[0]->nEvents(j,k);
	  if (nev0) { // don't bother printing out things you never had.
	    if (!i)
	      sprintf (s, "\tclass %d sig %d #events = %5d (100.0%%)",
		       j, k, nev);
	    else
	      sprintf (s, "\tclass %d sig %d #events = %5d (%5.1f%%)",
		       j, k, it->v_cuts[i]->nEvents(j,k),
		       100.*(float)nev/(float)it->v_cuts[i-1]->nEvents(j,k));
	    cout << s << endl;
	  }
	}
      }
    }
    for (uint32_t i=0; i<it->v_cuts.size(); i++)
      delete it->v_cuts[i];
  }

  delete evtclass_;
  delete outf;
}                                              // LjmetAnalAlgos::endJob

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

  int   inumclasses = evtclass_->numClasses();
  float fnumclasses = (float)inumclasses;
  int   inumsigs    = evtclass_->numSignatureTypes();
  float fnumsigs    = (float)inumsigs;

  sprintf (name, "classhistcut%d", cutnum_); 
  sprintf (title, "classhist, %s", descr_.c_str()); 
  h2f_class      = new TH2F(name,title,
			    inumclasses,-0.5,fnumclasses-0.5,
			    inumsigs,-0.5,fnumsigs-0.5);

  for (int i=0; inumclasses; i++) {
    h2f_class->GetYaxis()->SetBinLabel(i+1,evtclass_->classDescr(i).c_str());
  }

  for (int i=0; i<inumsigs; i++)
    h2f_class->GetXaxis()->SetBinLabel
      (i+1,evtclass_->signDescr(GenEvtClass::EnumSignature_t(i)).c_str());

}                                  // LjmetAnalAlgos::LjmetCut::LjmetCut

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
}                                      // LjmetAnalAlgos::LjmetCut::fill

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
			      RecoCandidateCollection& sortedEls,
			      double filterETval)
{
  RecoCandidateCollection::const_iterator it;
  //  reco::ElectronCollection::const_iterator it;

  for (it=ElsIn.begin(); it!=ElsIn.end(); it++){
    if ((*it)->et() > filterETval)
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
}                                          // LjmetAnalAlgos::filterJets

//======================================================================

void
LjmetAnalAlgos::calcVars(const std::vector<reco::CaloJet>& recjets,
			 const RecoCandidateCollection&    elecs,
			 const reco::CaloMETCollection&    metIn,
			 LjmetAnalHistos::HistoVars_t&     varsOut)
{
  varsOut.absmet         = 0.;
  varsOut.lemetdphi      = 0.;
  varsOut.ht             = 0.;
  varsOut.htplusmet      = 0.;
  varsOut.leadingjetET   = 0.;
  varsOut.leadingjetEta  = 0.;
  varsOut.leadingjetPhi  = 0.;
  varsOut.maxElectronET  = 0.;
  varsOut.maxElectronEta = 0.;
  varsOut.maxElectronPhi = 0.;
  varsOut.numElecs       = 0;
  varsOut.numJets        = 0;

  varsOut.numJetsCoHemi   = 0;
  varsOut.numJetsAntiHemi = 0;

  varsOut.v_electronETs.clear();
  varsOut.v_jetETs.clear();
#if 0
  varsOut.v_maxjet_e_dr.clear();
#endif

  varsOut.v_ejetdR.clear();
  varsOut.v_ejetdphi.clear();

  /****************************************
   * Collect electron statistics
   ****************************************/

  varsOut.genElectronCount = evtclass_->getElectronCount();

  /* Create own collection of electrons (so that they can be sorted)
  ** and count and categorize
  */

  varsOut.numElecs = elecs.size();
  RecoCandidateCollection::const_iterator el = elecs.begin();

  if (varsOut.numElecs>0) {
    varsOut.maxElectronET  = (*el)->et();
    varsOut.maxElectronEta = (*el)->eta();
    varsOut.maxElectronPhi = (*el)->phi();

    RecoCandidateCollection::const_iterator e;
    for (e=el; e!=elecs.end(); e++)
      varsOut.v_electronETs.push_back((*e)->et());
  }

  /****************************************
   * Collect hadron jet statistics
   ****************************************/

  varsOut.genQuarkCount = evtclass_->getQuarkCount();

  varsOut.numJets = recjets.size();
  std::vector<CaloJet>::const_iterator lj = recjets.begin();

  if (varsOut.numJets>0) {
    varsOut.leadingjetET  = lj->et();
    varsOut.leadingjetEta = lj->eta();
    varsOut.leadingjetPhi = lj->phi();

    std::vector<CaloJet>::const_iterator j;
    for (j=lj; j!=recjets.end(); j++) {
      varsOut.v_jetETs.push_back(j->et());
      if (varsOut.numElecs>0) {
	CLHEP::Hep3Vector barrelElec3V((*el)->px(), (*el)->py(), (*el)->pz());

	varsOut.v_ejetdR.push_back(calcdR(j->eta(),j->phi(),
					(*el)->eta(),(*el)->phi()));

	varsOut.v_ejetdphi.push_back(deltaPhi(j->phi(),(*el)->phi()));

	CLHEP::Hep3Vector jet3V(j->px(),j->py(),j->pz());

	double dot = barrelElec3V.dot(jet3V);
	if (dot > 0)
	  varsOut.numJetsCoHemi++;
	else if (dot < 0)
	  varsOut.numJetsAntiHemi++;
      }
    }
  }

  if (metIn.size() > 0) {
    varsOut.absmet = metIn.begin()->sumEt();
    if (varsOut.numElecs>0)
      varsOut.lemetdphi = deltaPhi(metIn.begin()->phi(),
				   varsOut.maxElectronPhi);
  }

#if 0
  varsOut.htplusmet = varsOut.ht + varsOut.absmet;
  varsOut.metsignificance = varsOut.absmet/sqrt(varsOut.ht);

  if (maxET > 0.0) {
    varsOut.leadingjetET = maxET;

    /**********************************************
     * ...max ET jet/electron angle distribution
     **********************************************/

    CaloJetCollection::iterator ej;
    for (ej  = ejets.begin(); ej != ejets.end(); ej++) {
      varsOut.v_maxjet_e_dr.push_back(calcdR(ej->eta(), maxij->eta(),
					  ej->phi(), maxij->phi()));
    }
  }
#endif
}                                            // LjmetAnalAlgos::calcVars

//======================================================================

void
LjmetAnalAlgos::applyCutsAndAccount(LjmetAnalHistos::HistoVars_t& vars,
				    vector<LjmetCut *>& v_cuts,
				    double weight)
{
  bool totalcut = false;
  int  hn=1;

  v_cuts[0]->Activate(false);

  /***********************************
   * Determine what cuts are active
   ***********************************/
  v_cuts[hn++]->Activate(!vars.numElecs || !vars.numJets);
  v_cuts[hn++]->Activate(vars.leadingjetET < cut_jetETminGeV_);
  v_cuts[hn++]->Activate(vars.numJets < cut_minNumJets_);
#if 0
  v_cuts[hn++]->Activate(vars.recoTop_eplusbjet_dphi  > cut_maxDphirTeb_);
  v_cuts[hn]->Activate  (vars.htplusmet              < cut_minHTplusMETgev_);
#endif

  /*****************
   * Do accounting
   *****************/

  for (uint32_t i=0; i<v_cuts.size(); i++) {
    totalcut = totalcut || v_cuts[i]->isActive();
    if (!totalcut)
      v_cuts[i]->fill(vars, weight);
    else
      break;
  }
}                                 // LjmetAnalAlgos::applyCutsAndAccount

//======================================================================

void
LjmetAnalAlgos::analyze(const HepMC::GenEvent& genEvt,
			const reco::CaloJetCollection& recJets,
			const reco::CaloMETCollection& met,
			const RecoCandidateCollection& elecs,
			double weight)
//			const reco::ElectronCollection& elecs)
{
  GenEvtClass::EnumSample_t     eventclass;
  GenEvtClass::EnumSignature_t  signatureclass;

  evtclass_->classifyEvent(genEvt,eventclass,signatureclass);

  std::vector<LjmetAnalDataset_t>::iterator it;
  for (it = LjmetDatasets_.begin(); it!=LjmetDatasets_.end(); it++) {
    RecoCandidateCollection       sortedEls;
    std::vector<reco::CaloJet>    filteredRJs;
    LjmetAnalHistos::HistoVars_t  vars;

    vars.eventclass     = eventclass;
    vars.signatureclass = signatureclass;

    sortElectrons(elecs,sortedEls,it->filter_elecETminGeV);
    filterJets(recJets,sortedEls,filteredRJs);
    calcVars(filteredRJs, sortedEls, met, vars);
    applyCutsAndAccount(vars,it->v_cuts,weight);
  }
}                                    // LjmetAnalAlgos::analyze(HepMC...

//======================================================================

void
LjmetAnalAlgos::analyze(const reco::GenParticleCollection& genParticles,
			const reco::CaloJetCollection& recJets,
			const reco::CaloMETCollection& met,
			const RecoCandidateCollection& elecs,
			double weight)
//			const reco::ElectronCollection& elecs)
{
  GenEvtClass::EnumSample_t     eventclass;
  GenEvtClass::EnumSignature_t  signatureclass;

  evtclass_->classifyEvent(genParticles,eventclass,signatureclass);

  std::vector<LjmetAnalDataset_t>::iterator it;
  for (it = LjmetDatasets_.begin(); it!=LjmetDatasets_.end(); it++) {
    RecoCandidateCollection       sortedEls;
    std::vector<reco::CaloJet>    filteredRJs;
    LjmetAnalHistos::HistoVars_t  vars;

    vars.eventclass     = eventclass;
    vars.signatureclass = signatureclass;

    sortElectrons(elecs,sortedEls,it->filter_elecETminGeV);
    filterJets(recJets,sortedEls,filteredRJs);
    calcVars(filteredRJs, sortedEls, met, vars);
    applyCutsAndAccount(vars,it->v_cuts,weight);
  }
}                               // LjmetAnalAlgos::analyze(genParticles...

//======================================================================
