// -*- C++ -*-
//
// Package:    phase2upVBFjetAnal
// Class:      phase2upVBFjetAnal
// 
/**\class phase2upVBFjetAnal phase2upVBFjetAnal.cc MyEDmodules/phase2upVBFjetAnal/plugins/phase2upVBFjetAnal.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Phillip Dudero
//         Created:  Fri, 07 Nov 2014 20:40:05 GMT
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
//
// class declaration
//
#include "phase2upVBFjetAnalAlgos.h"

class phase2upVBFjetAnal : public edm::EDAnalyzer {
   public:
      explicit phase2upVBFjetAnal(const edm::ParameterSet&);
      ~phase2upVBFjetAnal();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------
  phase2upVBFjetAnalAlgos algos_;
  edm::InputTag sGenParticleTag_;
  edm::InputTag sGenJetAlgo_;
  edm::InputTag sRecJetAlgo_;
  edm::InputTag sRecJetCoreAlgo_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
phase2upVBFjetAnal::phase2upVBFjetAnal(const edm::ParameterSet& iConfig)

{
  //now do what ever initialization is needed
  sGenParticleTag_ = 
    iConfig.getParameter<edm::InputTag>("GenParticleLabel");
  sGenJetAlgo_ = 
    iConfig.getParameter<edm::InputTag>("GenJetTruthLabel");
  sRecJetAlgo_ = 
    iConfig.getParameter<edm::InputTag>("RecoJetLabel");
  sRecJetCoreAlgo_ = 
    iConfig.getParameter<edm::InputTag>("RecoJetCoreLabel");

  algos_.setup(iConfig.getParameter<edm::ParameterSet>("benchmarkParams"));

}


phase2upVBFjetAnal::~phase2upVBFjetAnal()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
phase2upVBFjetAnal::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

 // get gen particle collection
  Handle<reco::GenParticleCollection> genparticles;
  bool isGen = iEvent.getByLabel(sGenParticleTag_, genparticles);
  if (!isGen) { 
    std::cout << "Warning : no Gen Particles in input !" << std::endl;
    return;
  }

 // get gen jet collection
  Handle<reco::GenJetCollection> genjets;
  isGen = iEvent.getByLabel(sGenJetAlgo_, genjets);
  if (!isGen) { 
    std::cout << "Warning : no Gen jets in input !" << std::endl;
    return;
  }

  // get reco PFJet collection
  Handle<reco::PFJetCollection> pfjets;
  bool isReco = iEvent.getByLabel(sRecJetAlgo_, pfjets);   
  if (!isReco) { 
    std::cout << "Warning : no PF jets in input !" << std::endl;
    return;
  }

  // get reco PFJet "core" collection
  Handle<reco::PFJetCollection> pfjetcores;
  bool hasCores = iEvent.getByLabel(sRecJetCoreAlgo_, pfjetcores);
  if (!hasCores) { 
    std::cout << "Warning : no PF jet cores in input !" << std::endl;
    return;
  }

  algos_.process(*genparticles, *pfjets, *pfjetcores, *genjets);

}


// ------------ method called once each job just before starting event loop  ------------
void 
phase2upVBFjetAnal::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
phase2upVBFjetAnal::endJob() 
{
  algos_.write();
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
phase2upVBFjetAnal::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(phase2upVBFjetAnal);
