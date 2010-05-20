// -*- C++ -*-
//
// Package:    METcleaningComparator
// Class:      METcleaningComparator
// 
/**\class METcleaningComparator METcleaningComparator.cc MyEDmodules/METcleaningComparator/src/METcleaningComparator.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Phil Dudero
//         Created:  Tue May 11 16:17:42 CDT 2010
// $Id: METcleaningComparator.cc,v 1.1 2010/05/20 10:56:47 dudero Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/TriggerNames.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"
#include "RecoLocalCalo/HcalRecAlgos/interface/HcalCaloFlagLabels.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "TH3F.h"

//
// class declaration
//

class METcleaningComparator : public edm::EDAnalyzer {
public:
  explicit METcleaningComparator(const edm::ParameterSet&);
  ~METcleaningComparator();


private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  bool reflaggerTouchedThisEvent   (const HBHERecHitCollection& rechits);
  bool tcmetcleanerTouchedThisEvent(const CaloMETCollection& dirtyMET,
				    const METCollection&     cleanMET);
  bool HCALfilterTouchedThisEvent  (const edm::TriggerResults& trgresults);

  void compare(const myEventData& d1,
	       const myEventData& d2);

  // ----------member data ---------------------------
  bool firstEvent_;
  std::string evfiltpath_;
  int  evfiltpathindex_;
  int  hbheFlagBit_;

  TH3F *h3f_correlations_;
  TH1F *h1f_tcmetdiffs_;

  typedef HBHERecHitCollection::const_iterator HBHERecHitIt;

  myEventData *dirtyInput_;
  myEventData *tcmetOutput_;
  myEventData *reflagOutput_;
  myEventData *evfiltOutput_;
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
METcleaningComparator::METcleaningComparator(const edm::ParameterSet& iConfig) :
  firstEvent_(true)
{

   //now do what ever initialization is needed

  edm::ParameterSet dirtyDataPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("dirtyInput");
  dirtyInput_ = new myEventData(dirtyDataPset);

  edm::ParameterSet tcmetDataPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("tcmetCleanOutput");
  tcmetOutput_ = new myEventData(tcmetDataPset);

  edm::ParameterSet reflagDataPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("reflagCleanOutput");
  reflagOutput_ = new myEventData(reflagDataPset);

  edm::ParameterSet evfiltDataPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("evfiltCleanOutput");
  evfiltOutput_ = new myEventData(evfiltDataPset);

  evfiltpath_ =
    iConfig.getUntrackedParameter<std::string>("evfiltPathName");

  hbheFlagBit_ = iConfig.getUntrackedParameter<int>
    ("hbheFlagBit", HcalCaloFlagLabels::HBHEHpdHitMultiplicity); 

  edm::Service<TFileService> fs;
  h3f_correlations_ = fs->make<TH3F>("h3f_correlations",
    "3 Methods of MET Cleaning; Event Filter; Reflagger; JetID vars",
				    2,0.,2.,2,0.,2.,2,0.,2.);
  h1f_tcmetdiffs_ = fs->make<TH1F>("h1f_tcmetdiffs",
    "Dirty MET - Clean MET (TC algo); #Delta MET",
				   1000,-500.,500.);
}

METcleaningComparator::~METcleaningComparator()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//
void
METcleaningComparator::compare(const myEventData& d1,
			       const myEventData& d2)
{
  assert (d1.hbherechits().isValid() &&
	  d2.hbherechits().isValid());

  const HBHERecHitCollection& rechits1 = *(d1.hbherechits());
  const HBHERecHitCollection& rechits2 = *(d2.hbherechits());

  HBHERecHitIt hitit1,hitit2;
  for (hitit1  = rechits1.begin(), hitit2  = rechits2.begin();
       (hitit1 != rechits1.end())  || (hitit1 != rechits1.end()); ) {

    // handle gaps in either dataset and notate
    //
    while((hitit1 != rechits1.end()) &&           // gap in rechits2
	  ((hitit2 == rechits2.end()) ||
	   (hitit1->id() < hitit2->id())  )  ) {
      // process...
      hitit1++;
    }
    while( (hitit2  != rechits2.end()) &&         // gap in rechits1
	   ((hitit1 == rechits1.end()) ||
	    (hitit2->id() < hitit1->id()) )   ) {
      // process...
      hitit2++;
    }

    //if (inSet<int>(detIds2mask_,hitit1->id().hashed_index())) continue;

  } // loop over rechits
  
}

// ----------------------------------------------------------

bool
METcleaningComparator::reflaggerTouchedThisEvent(const HBHERecHitCollection& rechits)
{
  for (size_t i=0; i<rechits.size(); i++)
    if (rechits[i].flagField(hbheFlagBit_))
      return true;

  return false;
}                    // METcleaningComparator::reflaggerTouchedThisEvent

// ---------------------------------------------------------------------

bool
METcleaningComparator::tcmetcleanerTouchedThisEvent(const CaloMETCollection& dirtyMET,
						    const METCollection&     cleanMET)
{
  double tcmetdiff =  dirtyMET.front().sumEt() - cleanMET.front().sumEt();
  h1f_tcmetdiffs_->Fill(tcmetdiff);
  return (tcmetdiff != 0.0);

}                 // METcleaningComparator::tcmetcleanerTouchedThisEvent

// ---------------------------------------------------------------------

bool
METcleaningComparator::HCALfilterTouchedThisEvent(const edm::TriggerResults& trgresults)
{
  if (firstEvent_) { // latch the evfiltpath index
    edm::TriggerNames triggerNames;

    // get trigger names
    triggerNames.init(trgresults);

    unsigned i;
    for (i=0; i!=trgresults.size(); i++) {
      if (triggerNames.triggerName(i) == evfiltpath_) {
	evfiltpathindex_ = i; break;
      }
    }
    if (i==trgresults.size())
      throw cms::Exception("Path not found: ") << evfiltpath_;
    firstEvent_ = false;
  }

  return (!trgresults.accept(evfiltpathindex_));

}                   // METcleaningComparator::HCALfilterTouchedThisEvent

// ------------ method called to for each event  -----------------------
void
METcleaningComparator::analyze(const edm::Event& iEvent,
			       const edm::EventSetup& iSetup)
{
   using namespace edm;

   dirtyInput_->get(iEvent,iSetup);
   tcmetOutput_->get(iEvent,iSetup);
   reflagOutput_->get(iEvent,iSetup);
   evfiltOutput_->get(iEvent,iSetup);

#if 0
   compare(*dirtyData_, *tcmetData_);
   compare(*dirtyData_, *reflagData_);
   compare(*tcmetData_, *reflagData_);
#endif

   h3f_correlations_->Fill(HCALfilterTouchedThisEvent(*(evfiltOutput_->trgResults())),
			   reflaggerTouchedThisEvent(*(reflagOutput_->hbherechits())),
			   tcmetcleanerTouchedThisEvent(*(dirtyInput_->calomet()),
							*(tcmetOutput_->recomet()))    );
}


// ------------ method called once each job just before starting event loop  ------------
void 
METcleaningComparator::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
METcleaningComparator::endJob() {
  cout << "evfilt\treflag\tJetID\t#events\t%" << endl;
  int nevents = h3f_correlations_->GetEntries();
  for (int i=1; i<=2; i++)
    for (int j=1; j<=2; j++)
      for (int k=1; k<=2; k++) {
	int thisbin=h3f_correlations_->GetBinContent(i,j,k);
	cout << i-1 << "\t" << j-1 << "\t" << k-1 << "\t";
	cout << thisbin << "\t" << 100.*(double)thisbin/nevents;
	cout << endl;
      }
}

//define this as a plug-in
DEFINE_FWK_MODULE(METcleaningComparator);
