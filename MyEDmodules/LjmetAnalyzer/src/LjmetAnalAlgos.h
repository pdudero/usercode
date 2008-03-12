#ifndef _LJMETANALALGOS_H
#define _LJMETANALALGOS_H

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/JetReco/interface/CaloJet.h"
#include "DataFormats/JetReco/interface/CaloJetfwd.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"

#include "MyEDmodules/LjmetAnalyzer/src/LjmetAnalHistos.h"
#include "MyEDmodules/LjmetAnalyzer/src/GenEvtClass.h"

#include "CLHEP/HepMC/GenEvent.h"
#include "CLHEP/HepMC/GenVertex.h"
#include "SimDataFormats/HepMCProduct/interface/HepMCProduct.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"

#include <string>
#include <vector>
#include <map>
#include "TH1F.h"
#include "TFile.h"

//======================================================================

typedef std::vector<reco::RecoCandidate *> RecoCandidateCollection;

//======================================================================
/** \class LjmetAnalAlgos specification
      
$Date: 2008/03/12 14:21:28 $
$Revision: 1.1.1.1 $
\author P. Dudero - Minnesota
*/
class LjmetAnalAlgos {
public:

  LjmetAnalAlgos(bool           verbosity,
		 const edm::ParameterSet& iConfig);

  void beginJob(void);
  void endJob();

  void analyze(const HepMC::GenEvent& genEvt,
	       const reco::CaloJetCollection& recJets,
	       const reco::CaloMETCollection& met,
	       const RecoCandidateCollection& elecs);
  //	       const reco::ElectronCollection& elecs);

  void analyze(const reco::CandidateCollection& genParticles,
	       const reco::CaloJetCollection& recJets,
	       const reco::CaloMETCollection& met,
	       const RecoCandidateCollection& elecs);
  //	       const reco::ElectronCollection& elecs);

private:

class LjmetCut {
public:
  LjmetCut(GenEvtClass *pEvtclass,
	   int cutnum,
	   std::string descr,
	   LjmetAnalHistos::AllHistoParams_t hpars,
	   TDirectory *rootDir);

  void fill(LjmetAnalHistos::HistoVars_t& vars);

  inline bool isActive() const          { return active_; }
  inline std::string& description()     { return descr_;  }
  inline void setActive(bool val)       { active_ = val;  }

  inline int  nEvents(int iec)  const {
    return (iec < evtclass_->numClasses()) ? v_nev_[iec] : 0;
  }

private:
  bool                               active_;
  GenEvtClass                       *evtclass_;
  int                                cutnum_;
  std::string                        descr_;
  LjmetAnalHistos::AllHistoParams_t  hpars_;
  TDirectory                        *rootDir_;
  std::vector<int>                   v_nev_;
  std::map<int, LjmetAnalHistos *>   m_pHistos_;
  TH1F                              *h1f_class;
};

// internal methods

 void   bookOneSet(string descr);
 // bool   isElectron(reco::CaloJet& cj);
 void   sortElectrons(const RecoCandidateCollection& ElsIn,
		      RecoCandidateCollection& sortedEls);
 void   filterJets(const std::vector<reco::CaloJet>& JetsIn,
		   const RecoCandidateCollection& ElsIn,
		   std::vector<reco::CaloJet>& filteredJetsOut);
 void   calcVars(const std::vector<reco::CaloJet>& recjets,
		 const RecoCandidateCollection& elecs,
		 const reco::CaloMETCollection& metIn);
 void   recoWandTop();
 void   applyCutsAndAccount();

 // user-configurable parameters
 LjmetAnalHistos::AllHistoParams_t hpars_;

 bool   verbosity_;
 string outRootFileName_;
 double emfraction4eid_;

 // filters:
 double filter_recjetETminGeV_;
 double filter_elecETminGeV_;
 double filter_eJetIsolationMindR_;

 // cuts:
 double cut1_HLTelectronETminGeV_;
 double cut2_electronETminGeV_;
 int    cut3_minNumJets_;
 double cut4_maxDphirTeb_;
 double cut5_minHTplusMETgev_;

 GenEvtClass  *evtclass_;

 reco::CaloJetCollection ejets; // "jets" identified as electrons
 reco::CaloJetCollection hjets; //  reconstructed hadronic jets

 reco::CaloJetCollection::iterator emaxij; // pointer to max ET electron

 LjmetAnalHistos::HistoVars_t      vars_;
 TFile                            *outf;
 std::vector<LjmetCut *>           v_cuts;
};

#endif // _LJMETANALALGOS_H
