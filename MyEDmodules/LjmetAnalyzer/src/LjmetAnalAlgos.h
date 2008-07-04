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
#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include <string>
#include <vector>
#include <map>
#include "TH1F.h"
#include "TFile.h"

//======================================================================

typedef std::vector<reco::RecoCandidate *> RecoCandidateCollection;

//======================================================================
/** \class LjmetAnalAlgos specification
      
$Date: 2008/06/27 14:53:35 $
$Revision: 1.6 $
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
	       const RecoCandidateCollection& elecs,
	       double weight);

  void analyze(const reco::GenParticleCollection& genParticles,
	       const reco::CaloJetCollection& recJets,
	       const reco::CaloMETCollection& met,
	       const RecoCandidateCollection& elecs,
	       double weight);

private:

class LjmetCut {
public:
  LjmetCut(GenEvtClass *pEvtclass,
	   int cutnum,
	   std::string descr,
	   LjmetAnalHistos::AllHistoParams_t hpars,
	   TDirectory *rootDir);

  void fill(LjmetAnalHistos::HistoVars_t& vars, double weight);

  inline bool isActive() const          { return active_; }
  inline std::string& description()     { return descr_;  }
  inline void Activate(bool val)        { active_ = val;  }

  inline int  nEvents(int iec,int isc)  const {
    return (iec < evtclass_->numClasses()) ?
      ((int)h2f_class->GetBinContent(isc+1,iec+1)) : 0;
  }

private:
  bool                               active_;
  GenEvtClass                       *evtclass_;
  int                                cutnum_;
  std::string                        descr_;
  LjmetAnalHistos::AllHistoParams_t  hpars_;
  TDirectory                        *rootDir_;
  std::map<int, LjmetAnalHistos *>   m_pHistos_;
  TH2F                              *h2f_class;
};

 struct LjmetAnalDataset_t {
   double filter_elecETminGeV;
   std::vector<LjmetCut *> v_cuts;
   TDirectory *rootdir;
 };

// internal methods

 void   bookOneSet          (string descr,
			     vector<LjmetAnalDataset_t>::iterator it);

 void   sortElectrons       (const RecoCandidateCollection&    ElsIn,
			     RecoCandidateCollection&          sortedEls,
			     double                            filterETval);

 void   filterJets          (const std::vector<reco::CaloJet>& JetsIn,
			     const RecoCandidateCollection&    ElsIn,
			     std::vector<reco::CaloJet>&       filteredJetsOut);

 void   calcVars            (const std::vector<reco::CaloJet>& recjets,
			     const RecoCandidateCollection&    elecs,
			     const reco::CaloMETCollection&    metIn,
			     LjmetAnalHistos::HistoVars_t&     varsOut);

 void   applyCutsAndAccount (LjmetAnalHistos::HistoVars_t&     vars,
			     vector<LjmetCut *>&               v_cuts,
			     double                            weight);

 // user-configurable parameters
 LjmetAnalHistos::AllHistoParams_t hpars_;

 bool   verbosity_;
 string outRootFileName_;
 double emfraction4eid_;

 // filters:
 double filter_recjetETminGeV_;
 double filter_eJetIsolationMindR_;

 std::vector<LjmetAnalDataset_t> LjmetDatasets_;

 // cuts:
 double cut_jetETminGeV_;
 int    cut_minNumJets_;
 double cut_maxDphirTeb_;
 double cut_minHTplusMETgev_;

 GenEvtClass  *evtclass_;

 reco::CaloJetCollection::iterator emaxij; // pointer to max ET electron

 std::vector<LjmetCut *>           v_cutsLoElecEtThresh_;
 std::vector<LjmetCut *>           v_cutsHiElecEtThresh_;

 TFile                            *outf;
};

#endif // _LJMETANALALGOS_H
