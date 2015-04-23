// -*- C++ -*-
//
// Package:    GenParticleFilter
// Class:      GenParticleFilter
// 
/**\class GenParticleFilter GenParticleFilter.cc MyEDmodules/GenParticleFilter/plugins/GenParticleFilter.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Phillip Russell Dudero
//         Created:  Sat, 21 Feb 2015 01:18:27 GMT
// $Id$
//
//


// system include files
#include <memory>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"

//
// class declaration
//

class GenParticleFilter : public edm::EDFilter {
   public:
      explicit GenParticleFilter(const edm::ParameterSet&);
      ~GenParticleFilter();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginJob() override;
      virtual bool filter(edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;
      
      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------
  bool verbose_;
  edm::InputTag sGenParticleTag_;
  std::vector<int> excludelist_;
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
GenParticleFilter::GenParticleFilter(const edm::ParameterSet& iConfig)
{
   //now do what ever initialization is needed
  verbose_ = iConfig.getUntrackedParameter<bool>("verbose",false);
  
  sGenParticleTag_ = 
    iConfig.getParameter<edm::InputTag>("GenParticleLabel");

  excludelist_ = 
    iConfig.getParameter<std::vector<int> >("excludeIDlist");

}


GenParticleFilter::~GenParticleFilter()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called on each new Event  ------------
bool
GenParticleFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

 // get gen particle collection
  Handle<reco::GenParticleCollection> genparticles;
  bool isGen = iEvent.getByLabel(sGenParticleTag_, genparticles);
  if (!isGen) { 
    std::cout << "Warning : no Gen Particles in input !" << std::endl;
    return false;
  }

  for( unsigned i = 0; i < genparticles->size(); i++ ) {
    const reco::Candidate *p = &(*genparticles)[i];

    if (verbose_)
      std::cout<<"id="<<p->pdgId()<<" st="<<p->status()<<" pt="<<p->pt()<<" nmth="<<p->numberOfMothers()<<" ndau="<<p->numberOfDaughters()<<std::endl;

    if ( p->status() == 3 ) {
      for (unsigned j=0; j < excludelist_.size(); j++) {
	if (p->pdgId() == excludelist_[j]) {
	  if (verbose_) std::cout << "rejecting " << std::endl;
	  return false;
	}
      }
    }
  }

   return true;
}

// ------------ method called once each job just before starting event loop  ------------
void 
GenParticleFilter::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
GenParticleFilter::endJob() {
}

// ------------ method called when starting to processes a run  ------------
/*
void
GenParticleFilter::beginRun(edm::Run const&, edm::EventSetup const&)
{ 
}
*/
 
// ------------ method called when ending the processing of a run  ------------
/*
void
GenParticleFilter::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
GenParticleFilter::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
GenParticleFilter::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
GenParticleFilter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}
//define this as a plug-in
DEFINE_FWK_MODULE(GenParticleFilter);
