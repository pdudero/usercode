#include <iostream>
#include "MyEDmodules/MyZeeAnalyzer/src/MyZeeAnalyzer.h"
#include "CLHEP/Vector/LorentzVector.h"

//
// constants, enums and typedefs
//
static const double Zmass = 91.1876; // PDG Z mass

//
// static data member definitions
//

//
// constructors and destructor
//
MyZeeAnalyzer::MyZeeAnalyzer(const edm::ParameterSet& iConfig):
  histocontainer_(),
  eleLabel_(iConfig.getUntrackedParameter<edm::InputTag>("electronTag")),
  minInvMassZeeWindow_(iConfig.getParameter<double>("minInvMassZeeWindow")),
  maxInvMassZeeWindow_(iConfig.getParameter<double>("maxInvMassZeeWindow")),
  hMaxPt_(iConfig.getParameter<double>("MaxPtOnHistograms"))
{
   //now do what ever initialization is needed

}


MyZeeAnalyzer::~MyZeeAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//
//======================================================================

void
MyZeeAnalyzer::filterEls
(const edm::View<pat::Electron>& elsIn,
 std::vector<reco::RecoCandidate *>& elsOut)
{
     edm::View<pat::Electron>::const_iterator it;
  for(it = elsIn.begin(); it!= elsIn.end(); ++it)
    elsOut.push_back((reco::RecoCandidate *)(&(*it)));
}


//======================================================================

void
MyZeeAnalyzer::lookForZees
(const std::vector<reco::RecoCandidate*>& genericEls)
{
  uint32_t i, j;
  double mindiff = 9e99;
  double mindiffpt, mindiffmass;
  for(i=0; i<genericEls.size(); i++) {
    for(j=i; j<genericEls.size(); j++) {
      const reco::RecoCandidate* el1=genericEls[i];
      const reco::RecoCandidate* el2=genericEls[j];

      CLHEP::HepLorentzVector el1p4(el1->px(),el1->py(),el1->pz(),el1->energy());
      CLHEP::HepLorentzVector el2p4(el2->px(),el2->py(),el2->pz(),el2->energy());
      CLHEP::HepLorentzVector sump4 = el1p4+el2p4;
          
      double invMass  = sump4.m();
      double massdiff = fabs(invMass-Zmass);
      double diElPt   = sump4.perp();

      if ((invMass >= minInvMassZeeWindow_) &&
          (invMass <= maxInvMassZeeWindow_)   ) {
	histocontainer_["diElPtAll"]->Fill(diElPt);
	histocontainer_["diElMassAll"]->Fill(invMass);
	
	if (massdiff < mindiff) {
	  mindiff     = massdiff;
	  mindiffpt   = diElPt;
	  mindiffmass = invMass;
	}
      }
    }
  }

  if (mindiff < 9e99) {
    histocontainer_["diElPtClosest2Z"]->Fill(mindiffpt);
    histocontainer_["diElMassClosest2Z"]->Fill(mindiffmass);
  }
}

// ------------ method called to for each event  ------------
void
MyZeeAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   //
   // first: get all objects from the event.
   //

   edm::Handle<edm::View<pat::Electron> > electronHandle;
   iEvent.getByLabel(eleLabel_,electronHandle);
   edm::View<pat::Electron> electrons = *electronHandle;

   std::vector<reco::RecoCandidate*> genericEls;
   
   // pre-filter Els on the basis of electron Identification
   filterEls(electrons, genericEls);

   //
   // for the other objects just quickly book the multiplicity.
   // Again, just use the same infrastructure as for jets if you
   // want to loop over them.
   //
   for (uint32_t i=0; i<genericEls.size(); i++) {
     histocontainer_["numelecsperPt"]->Fill(genericEls[i]->pt());
   }

   if (genericEls.size() > 1) lookForZees(genericEls);

}
// ------------ method called once each job just before starting event loop  ------------
void 
MyZeeAnalyzer::beginJob(const edm::EventSetup&)
{
  //
  // define some histograms using the framework tfileservice.
  // Define the output file name in your .cfg.
  //
  edm::Service<TFileService> fs;
  
  //
  // histocontainer_ is of type std::map<std::string, TH1D*>.
  // This means you can use it with this syntax:
  // histocontainer_["histname"]->Fill(x); 
  // histocontainer_["histname"]->Draw(); 
  // etc, etc. Essentially you use the histname string to look up a
  // pointer to a TH1D* 
  // which you can do everything to you would normally do in ROOT.
  //
  
  //
  // here we book new histograms:
  //

  histocontainer_["numelecsperPt"] = fs->make<TH1D>("numelecsperPt",
						    "# Electrons vs Pt",
						    50, 0.0, hMaxPt_);
  histocontainer_["diElPtClosest2Z"] = fs->make<TH1D>("diElPtClosest2Z",
						      "diElectron Pt (Closest to Z)",
						      50, 0.0, hMaxPt_);
  histocontainer_["diElPtAll"]       = fs->make<TH1D>("diElPtAll",
						      "diElectron Pt (All combos)",
						      50, 0.0, hMaxPt_);
  histocontainer_["diElMassAll"]     = fs->make<TH1D>("diElMassAll",
						      "diElectron Mass (All combos)",
						      50, 0.0, hMaxPt_);
  histocontainer_["diElMassClosest2Z"] = fs->make<TH1D>("diElMassClosest2Z",
						      "diElectron Mass (Closest to Z)",
						      50, 0.0, hMaxPt_);
}

// ------------ method called once each job just after ending the event loop  ------------
void 
MyZeeAnalyzer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(MyZeeAnalyzer);
