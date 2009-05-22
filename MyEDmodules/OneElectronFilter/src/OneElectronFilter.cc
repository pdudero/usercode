// -*- C++ -*-
//
// Package:    OneElectronFilter
// Class:      OneElectronFilter
// 
/**\class OneElectronFilter OneElectronFilter.cc MyEDmodules/OneElectronFilter/src/OneElectronFilter.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phil Dudero
//         Created:  Fri Jan  9 14:07:29 CST 2009
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/RecoCandidate/interface/RecoEcalCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoEcalCandidateFwd.h"

//
// class declaration
//

class OneElectronFilter : public edm::EDFilter {
   public:
      explicit OneElectronFilter(const edm::ParameterSet&);
      ~OneElectronFilter();

   private:
      virtual void beginJob(const edm::EventSetup&) ;
      virtual bool filter(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

  void filterEls
  (const edm::RefVector<reco::PixelMatchGsfElectronCollection>& elsIn,
   std::vector<reco::RecoEcalCandidate *>& elsOut);
      
      // ----------member data ---------------------------
  edm::InputTag sourceElectrons_;
  double elMinPtGeV_;
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
OneElectronFilter::OneElectronFilter(const edm::ParameterSet& iConfig) :
  sourceElectrons_(iConfig.getUntrackedParameter<edm::InputTag>("sourceElectrons")),
  elMinPtGeV_(iConfig.getParameter<double>("elMinPtGeV"))
{
   //now do what ever initialization is needed

}


OneElectronFilter::~OneElectronFilter()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

//======================================================================

void
OneElectronFilter::filterEls
(const edm::RefVector<reco::PixelMatchGsfElectronCollection>& elsIn,
 std::vector<reco::RecoEcalCandidate *>& elsOut)
{
  edm::RefVector<reco::PixelMatchGsfElectronCollection>::const_iterator it;
  for(it = elsIn.begin(); it!= elsIn.end(); ++it)
    if ((*it)->pt() >= elMinPtGeV_)
      elsOut.push_back((reco::RecoEcalCandidate *)(&(**it)));
}

//======================================================================

// ------------ method called on each new Event  ------------
bool
OneElectronFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  // These declarations create handles to the types of records that you want
  // to retrieve from event "e".

  edm::Handle<edm::RefVector<reco::PixelMatchGsfElectronCollection> > robustEls;

  std::vector<reco::RecoEcalCandidate*> genericEls;

  iEvent.getByLabel(sourceElectrons_,robustEls);

  // pre-filter Els on the basis of electron Identification
  filterEls((*robustEls), genericEls);

  return genericEls.size();
}

// ------------ method called once each job just before starting event loop  ------------
void 
OneElectronFilter::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
OneElectronFilter::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(OneElectronFilter);
