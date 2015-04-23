#ifndef phase2upVBFjetAnalAlgos_h
#define phase2upVBFjetAnalAlgos_h

#include "RecoParticleFlow/Benchmark/interface/PFBenchmarkAlgo.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/JetReco/interface/BasicJetCollection.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"

#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"

#include "TH1F.h"
#include "TH2F.h"
#include <string>
#include <vector>
#include <map>


class phase2upVBFjetAnalAlgos;

class TH1F;
class TH2F;


class phase2upVBFjetAnalAlgos {
	
 public:
	
  phase2upVBFjetAnalAlgos();
  virtual ~phase2upVBFjetAnalAlgos();
	
  void setup(const edm::ParameterSet& benchPars);

  myAnalCut *getCut(const std::string& cutstr);
  void calcQGdiscriminators(const reco::PFJetCollection& pfJets,
			    const reco::PFJetCollection& pfJetCores);
  bool calcVBFCut(const reco::PFJetCollection& pfJets);
  bool calcMatchQuarkCut(const reco::GenParticleCollection& genParts,
			 const reco::PFJetCollection& pfJets);
  bool calcVBFgapjetveto(const reco::PFJetCollection& pfJets, 
			 bool dynamicveto=false);
  void fillHistos4cut(const reco::PFJetCollection& pfJets,
		      const reco::GenJetCollection& genJets,
		      myAnalCut& thisCut);

  void process(const reco::GenParticleCollection&,
	       const reco::PFJetCollection&,
	       const reco::PFJetCollection&,
	       const reco::GenJetCollection& );
  void gettrue (const reco::GenJet* truth, double& true_ChargedHadEnergy, 
		double& true_NeutralHadEnergy, double& true_ChargedEmEnergy, 
		double& true_NeutralEmEnergy);
  void printPFJet (const reco::PFJet*);
  void printGenJet (const reco::GenJet*);
  double resPtMax() const {return resPtMax_;};
  double resChargedHadEnergyMax() const {return resChargedHadEnergyMax_;};
  double resNeutralHadEnergyMax() const {return resNeutralHadEnergyMax_;};
  double resNeutralEmEnergyMax() const {return resNeutralEmEnergyMax_;};
//  void save();	
  void write();
	
 private:

  void BOOK1D(const std::string& name,
	      const std::string& title,
	      int    nbinsx,
	      double minx,
	      double maxx);
  void BOOK2D(const std::string& name,
	      const std::string& title,
	      int    nbinsx,
	      double minx,
	      double maxx,
	      int    nbinsy,
	      double miny,
	      double maxy);

  void DBOOK1D(const std::string& name,
	      const std::string& title,
	      int    nbinsx,
	      double minx,
	      double maxx);
  void DBOOK2D(const std::string& name,
	      const std::string& title,
	      int    nbinsx,
	      double minx,
	      double maxx,
	      int    nbinsy,
	      double miny,
	      double maxy);
  void EBOOK1D(const std::string& name,
	       const std::string& title,
	       int    nbinsx,
	       double minx,
	       double maxx);

  std::vector<myAnalHistos::HistoParams_t> v_hpars1d_;
  std::vector<myAnalHistos::HistoParams_t> v_hpars2d_;
  std::map<std::string, myAnalCut *> m_cuts_;

  // histograms
  // Jets inclusive  distributions  (Pt > 20 GeV)
	
 protected:
  const int njptbins_ = 100;

  double dRjet1core_;
  double dRjet2core_;
  double RPTjet1core_;
  double RPTjet2core_;
		
  PFBenchmarkAlgo *jetalgo_;
  reco::Candidate *vbfq_[2];
  bool debug_;
  bool isSignal_;
  bool isVBF_;
  bool matchQs_;
  bool plotAgainstReco_;
  bool nogapjets_;
  bool nogapjetsdynamic_;
  bool doHemisphere_cut_;
  double deltaRMax_;
  double maxplotE_;
  double recPt_cut_;
  double maxEta_cut_;
  double minDeltaEtajj_cut_;
  double minMjj_cut_;

  int    nQmatchJets_;
  double resPtMax_;
  double resChargedHadEnergyMax_;
  double resNeutralHadEnergyMax_;
  double resNeutralEmEnergyMax_; 
  double q1pt_;
  double q2pt_;
  double j1q1deltaptoverpt_;
  double j2q2deltaptoverpt_;
  unsigned int entry_;
};

#endif // phase2upVBFjetAnalAlgos_h
