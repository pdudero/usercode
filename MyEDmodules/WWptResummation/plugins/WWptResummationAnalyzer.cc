// -*- C++ -*-
//
// Package:    
// Class:   WWptResummationAnalyzer.cc    
// 
/**\class WWptResummationAnalyzer WWptResummationAnalyzer.cc

 Description: <one line class summary>

 Implementation:


*/
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "FWCore/ServiceRegistry/interface/Service.h" 
#include "FWCore/Utilities/interface/InputTag.h"
#include "WWptResummation.h"

using namespace edm;
using namespace reco;
using namespace std;

//
// class decleration

 
class WWptResummationAnalyzer : public edm::EDAnalyzer {
public:
  explicit WWptResummationAnalyzer(const edm::ParameterSet&);
  ~WWptResummationAnalyzer();


private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
  // ----------member data ---------------------------

  double xsecpb_;

  WWptResummation WWptResummation_;
  InputTag sInputTruthLabel_;

};
/// PFJet Benchmark

//neuhaus - comment
//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
WWptResummationAnalyzer::WWptResummationAnalyzer(const edm::ParameterSet& iConfig)

{
  //now do what ever initialization is needed
  sInputTruthLabel_       =  iConfig.getParameter<InputTag>("InputTruthLabel");

  WWptResummation_.setup(iConfig.getParameter<edm::ParameterSet>("pars"));
}


WWptResummationAnalyzer::~WWptResummationAnalyzer()
{
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

// ------------ method called to for each event  ------------
void
WWptResummationAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
 // get gen particle collection
  Handle<GenParticleCollection> genparticles;
  bool isGen = iEvent.getByLabel(sInputTruthLabel_, genparticles);
  if (!isGen) { 
    std::cout << "Warning : no Gen Particles in input !" << std::endl;
    return;
  }

  edm::Handle< GenRunInfoProduct > genInfoProduct;
  iEvent.getRun().getByLabel("generator", genInfoProduct );
  xsecpb_ = (double)genInfoProduct->internalXSec().value();

  // Analyze (absolutely a "z" in "analyze" : we are in the U.S., dammit!) 
  WWptResummation_.process(*genparticles);
}


// ------------ method called once each job just before starting event loop  ------------
void 
WWptResummationAnalyzer::beginJob()
{

}

// ------------ method called once each job just after ending the event loop  ------------
void 
WWptResummationAnalyzer::endJob() {

  WWptResummation_.analyze(xsecpb_);
  WWptResummation_.write();
}

//define this as a plug-in
DEFINE_FWK_MODULE(WWptResummationAnalyzer);
