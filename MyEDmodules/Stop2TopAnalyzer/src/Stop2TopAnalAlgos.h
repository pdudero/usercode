#ifndef _STOP2TOPANALALGOS_H
#define _STOP2TOPANALALGOS_H

#include "SimDataFormats/HepMCProduct/interface/HepMCProduct.h"
#include "CLHEP/HepMC/GenVertex.h"
#include "MyAnalyzers/Stop2TopAnalyzer/src/cutPlots.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/JetReco/interface/GenJet.h"

#include <vector>
#include <map>
#include "TH1F.h"
#include "TFile.h"

//======================================================================

using namespace std;
using namespace HepMC;

//======================================================================

//======================================================================
/** \class Stop2TopAnalAlgos specification
      
$Date: 2005/10/06 22:21:33 $
$Revision: 1.2 $
\author P. Dudero - Minnesota
*/
class Stop2TopAnalAlgos {
public:
  Stop2TopAnalAlgos(bool           verbosity,
		    string         outRootFileName,
		    double         emfraction4eid,
		    double         recjetETgevthresh,
		    double         maxplottedETgev,
		    double         hltcut1_electronETminGeV,
		    double         cut2_electronETminGev,
		    int            cut3_minNumJets,
		    double         cut4_maxDphirTeb,
		    double         cut5_minHTplusMETgev);

  void beginJob(void);
  void endJob();
  void processEvent(edm::HepMCProduct& HepMCEvt,
		    CaloJetCollection& recjets2,
		    CaloJetCollection& recjets5,
		    CaloJetCollection& recjets7);
private:
  // internal methods
  double JEScor(double reta, double rEt);
  bool   isElectron(CaloJet& cj);
  void   classifyEvent(const HepMC::GenEvent* MCEvt);
  void   fillJetVecs(CaloJetCollection& ejetcandidates,
		     CaloJetCollection& hjetcandidates);
  void   calcVars();
  void   recoWandTop();
  void   applyCutsAndAccount();

  // user-configurable parameters
  bool   verbosity_;
  string outRootFileName_;
  double emfraction4eid_;
  double recjetETgevthresh_;
  double maxplottedETgev_;
  double hltcut1_electronETminGeV_;
  double cut2_electronETminGeV_;
  int    cut3_minNumJets_;
  double cut4_maxDphirTeb_;
  double cut5_minHTplusMETgev_;
  double dRisolationMin_;

  CaloJetCollection ejets; // "jets" identified as electrons
  CaloJetCollection hjets; //  reconstructed hadronic jets

  CaloJetCollection::iterator emaxij; // pointer to max ET electron

  cutPlots::plotVars_t  vars;
  TFile                *outf;
  vector<int>           v_nevents_sig;
  vector<int>           v_nevents_oth;
  vector<bool>          v_cuts;
  vector<const char *>  v_cutstrings;
  vector<cutPlots *>    v_plots_sig;
  vector<cutPlots *>    v_plots_oth;
};

#endif // _STOP2TOPANALALGOS_H
