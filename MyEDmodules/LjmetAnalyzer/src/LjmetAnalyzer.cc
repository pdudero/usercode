// -*- C++ -*-
//
// Package:    LjmetAnalyzer
// Class:      LjmetAnalyzer
// 
/**\class LjmetAnalyzer LjmetAnalyzer.cc MyEDmodules/LjmetAnalyzer/src/LjmetAnalyzer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  pts/0
//         Created:  Wed Oct 17 14:05:17 CEST 2007
// $Id: LjmetAnalyzer.cc,v 1.3 2008/04/29 16:02:53 dudero Exp $
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
#include "FWCore/ParameterSet/interface/InputTag.h"

#include "MyEDmodules/LjmetAnalyzer/src/LjmetAnalAlgos.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/EgammaCandidates/interface/Electron.h"
#include "DataFormats/EgammaCandidates/interface/PixelMatchGsfElectronFwd.h"
#include "AnalysisDataFormats/Egamma/interface/ElectronID.h"
#include "AnalysisDataFormats/Egamma/interface/ElectronIDAssociation.h"

#include "SimDataFormats/HepMCProduct/interface/GenInfoProduct.h"

//======================================================================

using namespace std;
using namespace reco;

//======================================================================

//
// class declaration
//

class LjmetAnalyzer : public edm::EDAnalyzer {
public:
  explicit LjmetAnalyzer(const edm::ParameterSet&);
  ~LjmetAnalyzer();


private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  void filterEls(edm::Handle<reco::ElectronIDAssociationCollection>& elIDassocHandle,
		 edm::Handle<reco::PixelMatchGsfElectronCollection>& elsInHandle,
		 RecoCandidateCollection& elsOut);

  // ----------member data ---------------------------

  bool             verbose_;
  edm::InputTag    caloJetLabel_;
  edm::InputTag    electronLabel_;
  edm::InputTag    metLabel_;
  edm::InputTag    elIDAssocProducer_;
  LjmetAnalAlgos  *algos_;
  int              ngsfcol_;
  int              nfiltel_;
  int              nevt_;
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
LjmetAnalyzer::LjmetAnalyzer(const edm::ParameterSet& iConfig) :
  verbose_(iConfig.getUntrackedParameter<bool>("verbosity")),
  caloJetLabel_(iConfig.getUntrackedParameter<edm::InputTag>("caloJetLabel")),
  electronLabel_(iConfig.getUntrackedParameter<edm::InputTag>("electronLabel")),
  metLabel_(iConfig.getUntrackedParameter<edm::InputTag>("metLabel")),
  elIDAssocProducer_(iConfig.getUntrackedParameter<edm::InputTag>("elIDAssociationProducer"))
{
  //now do what ever initialization is needed
  algos_ = new LjmetAnalAlgos(verbose_, iConfig);

  ngsfcol_ = 0;
  nfiltel_ = 0;
  nevt_    = 0;
}


LjmetAnalyzer::~LjmetAnalyzer()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

void
LjmetAnalyzer::filterEls(edm::Handle<reco::ElectronIDAssociationCollection>& elIDassocHandle,
			 edm::Handle<reco::PixelMatchGsfElectronCollection>& elsInHandle,
			 RecoCandidateCollection& elsOut)
{
  for(unsigned int i = 0; i < elsInHandle->size(); ++i) {
    edm::Ref<reco::PixelMatchGsfElectronCollection> e(elsInHandle, i);
    bool cutBasedID = elIDassocHandle->find(e)->val->cutBasedDecision();

    if (cutBasedID)
      elsOut.push_back((reco::RecoCandidate *)(&(*e)));
    else
      nfiltel_++;
  }
}

// ------------ method called to for each event  ------------
void
LjmetAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

  // get event id information
  int runn = iEvent.id().run();
  int evtn = iEvent.id().event();

  nevt_++;

  if (verbose_) {
    cout  << "Processing run" << runn << "," << "event " << evtn << endl;
  }

  /*********************************************
   ***         GET JET INFORMATION           ***
   *********************************************/

  Handle<CaloJetCollection> caloJets;
  iEvent.getByLabel( caloJetLabel_, caloJets );

  // reco::Electron and PixelMatchGsfElectron both separately inherit from
  // RecoCandidate...
  RecoCandidateCollection genericEls;

  /*********************************************
   ***      GET ELECTRON INFORMATION         ***
   *********************************************/

  /************************************
   *** TRY ElectronCollection FIRST ***
   ************************************/

  bool getGsfElectrons = true; // false;

  try{
    Handle<ElectronCollection> electrons;
    iEvent.getByLabel( electronLabel_, electrons );

    const Provenance& prov = iEvent.getProvenance(electrons.id());
    const string& branchName = prov.branchName();

    edm::LogInfo("LjmetAnalyzer::analyze") <<
      "Retrieving reco::electron collection " << branchName;

    ElectronCollection::const_iterator it;
    for (it = electrons->begin(); it != electrons->end(); it++) {
      genericEls.push_back((reco::RecoCandidate *)&(*it)); 
    }
  }catch(const Exception&) {

  /*********************************************
   *** TRY PixelMatchGsfElectronCollection   ***
   *********************************************/
    getGsfElectrons = true;
  }

  if (getGsfElectrons) {
    try{
      Handle<reco::PixelMatchGsfElectronCollection> electrons;
      iEvent.getByLabel( electronLabel_, electrons );

      const Provenance& prov = iEvent.getProvenance(electrons.id());
      const string& branchName = prov.branchName();

      edm::LogInfo("LjmetAnalyzer::analyze") <<
	"Retrieving PixelMatchGsfElectronCollection " << branchName;

      Handle<reco::ElectronIDAssociationCollection> elIDAssocHandle;
      iEvent.getByLabel(elIDAssocProducer_, elIDAssocHandle);

      // pre-filter Els on the basis of electron Identification
      filterEls(elIDAssocHandle, electrons, genericEls);

      ngsfcol_++;

    }catch(const Exception&) {
      throw cms::Exception("LjmetAnalyzer") <<
  	"No electrons of type reco::Electron or PixelMatchGsfElectron found";
    }
  }

  /*********************************************
   ***         GET MET INFORMATION           ***
   *********************************************/

  Handle<CaloMETCollection> met;
  iEvent.getByLabel( metLabel_, met );

  /*********************************************
   ***         GET SRC INFORMATION           ***
   *********************************************/

  try {
    edm::Handle<edm::GenInfoProduct> gi;
    iEvent.getRun().getByLabel( "source", gi);

    double xsecpb = gi->external_cross_section(); // precalculated xsec in pb
    cout << "Got cross-section: " << xsecpb << "pb" << endl;
  }
  catch(const Exception&) {
    if(verbose_){
      std::cout << "GenInfoProduct not found, couldn't obtain cross-section"<< std::endl;
    }
  }

  /*********************************************
   ***      TRY HepMCProduct FIRST           ***
   *********************************************/

  edm::Handle<edm::HepMCProduct> hepMCEvt;
  bool getGenParticles = false;

  try{
    iEvent.getByLabel("source",hepMCEvt);
    if(verbose_){
      std::cout << "source HepMCProduct found"<< std::endl;
    }
    const HepMC::GenEvent* genEvt=hepMCEvt->GetEvent();

    algos_->analyze(*genEvt, *caloJets, *met, genericEls);

  }catch(const Exception&) {
    if(verbose_){
      std::cout << "no HepMCProduct found"<< std::endl;
    }

#if 0
    getGenParticles = true;
  }

  if (getGenParticles) {

    /*********************************************
     ***      TRY GenParticleCandidates        ***
     *********************************************/

    Handle<CandidateCollection> genParticles;

    try{
      iEvent.getByLabel( "genParticleCandidates", genParticles );
      if(verbose_){
	std::cout << "genParticleCandidates found"<< std::endl;
      }

      algos_->analyze(*genParticles, *caloJets, *met, genericEls);

    }catch(const Exception&) {
      if(verbose_){
	std::cout << "no genParticleCandidates found"<< std::endl;
      }
#endif
      throw cms::Exception("LjmetAnalyzer") << "Source data not found";
    }
#if 0
  }
#endif
}

// ------------ method called once each job just before starting event loop  ------------
void 
LjmetAnalyzer::beginJob(const edm::EventSetup&)
{
  algos_->beginJob();
}

// ------------ method called once each job just after ending the event loop  -----------
void 
LjmetAnalyzer::endJob() {
  algos_->endJob();

  cout << "Filtered " << nfiltel_ << " electrons out of ";
  cout << ngsfcol_ << " GSF electron collections, " << nevt_ << " events" << endl;
}

//define this as a plug-in
DEFINE_FWK_MODULE(LjmetAnalyzer);
