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
// $Id: METcleaningComparator.cc,v 1.2 2010/05/20 15:23:10 dudero Exp $
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
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
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

  bool reflaggerTouchedThisEvent   (const HBHERecHitCollection& rechits,
				    const HFRecHitCollection& hfrechits,
				    double& deltamethbhe,
				    double& deltamethf);
  bool jetidCleanerTouchedThisEvent(const CaloMETCollection& dirtyMET,
				    const METCollection&     cleanMET,
				    double& deltamet);
  
  bool HCALfilterTouchedThisEvent  (const edm::TriggerResults& trgresults);
  bool HCALfilterTouchedThisEvent  (const bool hbheNoiseResult);

  void compare(const myEventData& d1,
	       const myEventData& d2);

  // ----------member data ---------------------------
  int  ntrigreject_;
  bool firstEvent_;
  int  hbheFlagBit_;
  int  hfFlagBit_;
  std::string evfiltpath_;
  int  evfiltpathindex_;
  double minMET4plotGeV_;
  double maxMET4plotGeV_;

  TH3F *h3f_correlations_;

  TH1F *h1f_dirtyMET_;
  TH1F *h1f_clnMETevfilt_;
  TH1F *h1f_clnMETreflag_;
  TH1F *h1f_clnMETjetID_;

  myAnalHistos *histosPerCategory[2][2][2];

  typedef HBHERecHitCollection::const_iterator HBHERecHitIt;

  myEventData *trgEvData_;
  myEventData *dirtyInput_;
  myEventData *jetidOutput_;
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
  ntrigreject_(0), firstEvent_(true)
{
  edm::ParameterSet trigResultsPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("triggerResults");
  trgEvData_ = new myEventData(trigResultsPset);

  edm::ParameterSet dirtyDataPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("dirtyInput");
  dirtyInput_ = new myEventData(dirtyDataPset);

  edm::ParameterSet jetidDataPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("jetidCleanOutput");
  jetidOutput_ = new myEventData(jetidDataPset);

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

  hfFlagBit_ = iConfig.getUntrackedParameter<int>
    ("hfFlagBit", HcalCaloFlagLabels::HFLongShort); 

  minMET4plotGeV_ = iConfig.getUntrackedParameter<double>("minMET4plotGeV"); 
  maxMET4plotGeV_ = iConfig.getUntrackedParameter<double>("maxMET4plotGeV"); 

  edm::Service<TFileService> fs;
  h3f_correlations_ = fs->make<TH3F>("h3f_correlations",
    "3 Methods of MET Cleaning; Event Filter; Reflagger; JetID vars",
				     2,0.,2.,2,0.,2.,2,0.,2.);
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
METcleaningComparator::reflaggerTouchedThisEvent(const HBHERecHitCollection& hbherechits,
						 const HFRecHitCollection& hfrechits,
						 double& deltamethbhe,
						 double& deltamethf)
{
  deltamethbhe=deltamethf=0;
  for (size_t i=0; i<hbherechits.size(); i++)
    if (hbherechits[i].flagField(hbheFlagBit_))
      deltamethbhe += hbherechits[i].energy();

  for (size_t i=0; i<hfrechits.size(); i++)
    if (hfrechits[i].flagField(hfFlagBit_))
      deltamethf += hfrechits[i].energy();

  return ((deltamethbhe+deltamethf) != 0.0);

}                    // METcleaningComparator::reflaggerTouchedThisEvent

// ---------------------------------------------------------------------

bool
METcleaningComparator::jetidCleanerTouchedThisEvent(const CaloMETCollection& dirtyMET,
						    const METCollection&     cleanMET,
						    double& deltamet)
{
  deltamet =  dirtyMET.front().sumEt() - cleanMET.front().sumEt();
  return (deltamet != 0.0);

}                 // METcleaningComparator::jetidCleanerTouchedThisEvent

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

// ---------------------------------------------------------------------

bool
METcleaningComparator::HCALfilterTouchedThisEvent(const bool hbheNoiseResult)
{
  return (!hbheNoiseResult);
}                   // METcleaningComparator::HCALfilterTouchedThisEvent

// ------------ method called to for each event  -----------------------
void
METcleaningComparator::analyze(const edm::Event& iEvent,
			       const edm::EventSetup& iSetup)
{
   using namespace edm;

   // contains the trigger record
   //
   trgEvData_->get(iEvent,iSetup);

   const edm::TriggerResults& trgresults = *(trgEvData_->trgResults());
   unsigned i;
   for (i=0; i!=trgresults.size(); i++) {
     if (!trgresults.accept(i)) {
       ntrigreject_++;
       return;
     }
   }

   dirtyInput_->get(iEvent,iSetup);
   jetidOutput_->get(iEvent,iSetup);
   reflagOutput_->get(iEvent,iSetup);
   evfiltOutput_->get(iEvent,iSetup);

   double deltaMETHBHEreflagger,deltaMETHFreflagger,deltaMETjetid;
   //bool evfilt = HCALfilterTouchedThisEvent(*(evfiltOutput_->trgResults()));
   bool evfilt = HCALfilterTouchedThisEvent(*(evfiltOutput_->hbheNoiseResult()));
   bool reflag = reflaggerTouchedThisEvent (*(reflagOutput_->hbherechits()),
					    *(reflagOutput_->hfrechits()),
					    deltaMETHBHEreflagger,
					    deltaMETHFreflagger);
   bool jetid  = jetidCleanerTouchedThisEvent(*(dirtyInput_->calomet()),
					      *(jetidOutput_->recomet()),
					      deltaMETjetid);

   h1f_dirtyMET_->Fill(dirtyInput_->calomet()->front().sumEt());

   if (!evfilt)
     h1f_clnMETevfilt_->Fill(dirtyInput_->calomet()->front().sumEt());

   h1f_clnMETreflag_->Fill(reflagOutput_->calomet()->front().sumEt());
   h1f_clnMETjetID_->Fill(jetidOutput_->recomet()->front().sumEt());

   h3f_correlations_->Fill(evfilt,reflag,jetid);

   if (jetid || reflag) {
     myAnalHistos *myAH = histosPerCategory[evfilt][reflag][jetid];
     myAH->fill1d<TH1F>("h1f_deltaMETHBHEreflagger",  deltaMETHBHEreflagger);
     myAH->fill1d<TH1F>("h1f_deltaMETHFreflagger",    deltaMETHFreflagger);
     myAH->fill1d<TH1F>("h1f_deltaMETjetid",          deltaMETjetid);
     myAH->fill1d<TH1F>("h1f_deltaMETjetid-reflagger",
			deltaMETjetid-(deltaMETHBHEreflagger+deltaMETHFreflagger));
   }
}

// ------------ method called once each job just before starting event loop  ------------
void 
METcleaningComparator::beginJob()
{
  int nbinsMET = (int)(maxMET4plotGeV_-minMET4plotGeV_);

  myAnalHistos::HistoParams_t hpars("","",nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);

  edm::Service<TFileService> fs;
  h1f_dirtyMET_     = fs->make<TH1F>("h1f_dirtyMET",
				     "Uncleaned MET; MET (GeV); dN/GeV",
				     nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);
  h1f_clnMETevfilt_ = fs->make<TH1F>("h1f_clnMETevfilt",
				     "Cleaned MET (Event Filter); MET (GeV); dN/GeV",
				     nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);
  h1f_clnMETreflag_ = fs->make<TH1F>("h1f_clnMETreflag",
				     "Cleaned MET (Reflagger algo); MET (GeV); dN/GeV",
				     nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);
  h1f_clnMETjetID_  = fs->make<TH1F>("h1f_clnMETjetID",
				     "Cleaned MET (Jet ID); MET (GeV); dN/GeV",
				     nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);

  for (int i=0; i<2; i++)
    for (int j=0; j<2; j++)
      for (int k=0; k<2; k++) {
	char s[80];
	sprintf (s,"evflt%dreflg%djetid%d",i,j,k);
	if (!(j+k)) continue;
	myAnalHistos *myAH = new myAnalHistos(s);
	hpars.name=hpars.title="h1f_deltaMETHFreflagger";     myAH->book1d<TH1F>(hpars);
	hpars.name=hpars.title="h1f_deltaMETHBHEreflagger";   myAH->book1d<TH1F>(hpars);
	hpars.name=hpars.title="h1f_deltaMETjetid";           myAH->book1d<TH1F>(hpars);
	hpars.name=hpars.title="h1f_deltaMETjetid-reflagger"; myAH->book1d<TH1F>(hpars);
	histosPerCategory[i][j][k] = myAH;
      }
}

// ------------ method called once each job just after ending the event loop  ------------
void 
METcleaningComparator::endJob() {
  cout << "ntrigreject = " << ntrigreject_ << endl;
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
