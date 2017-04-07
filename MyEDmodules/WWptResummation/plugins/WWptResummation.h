#ifndef MyEDModules_WWptResummation_WWptResummation_h
#define MyEDModules_WWptResummation_WWptResummation_h


#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"

#include "FWCore/ServiceRegistry/interface/Service.h" 
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1D.h"
#include "TString.h"
#include "TFile.h"


class WWptResummation;

class TH1D;


class WWptResummation {
	
 public:
	
  WWptResummation();
  virtual ~WWptResummation();
	
  void setup(const edm::ParameterSet& pars);
  
  void process(const reco::GenParticleCollection& );
  void calculateQuantities(const reco::GenParticleCollection&);
  void loadResumHisto(const TString& filename, TH1* th1, double norm);

  double mpi_pi(double angle);
  void analyze( double xsec );
  void write();
	
 private:

  const double startval_ = 0.0;
  const double endval_   = 160.;
  const double binwidth_ = 0.25;

  int   finalstatus_; // final particle status (post ISR/FSR)
	
  // histograms
  TH1D *hWWpt;
  TH1D *hWmpt;
  TH1D *hWppt;
  TH1D *hWmpthi;
  TH1D *hWppthi;
  TH1D *hcentral;
  TH1D *hQup;
  TH1D *hQdn;
  TH1D *hRup;
  TH1D *hRdn;

  TH1D *hcentralRatio;
  TH1D *hQupRatio;
  TH1D *hQdnRatio;
  TH1D *hRupRatio;
  TH1D *hRdnRatio;

  double wwpt_,wmpt_,wppt_;
    
 protected:
		
};

#endif // MyEDModules_WWptResummation_WWptResummation_h
