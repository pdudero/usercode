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
// $Id: METcleaningComparator.cc,v 1.4 2010/07/15 15:22:09 dudero Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Common/interface/TriggerNames.h"

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

  bool reflaggerTouchedThisEvent   (const  edm::Handle<HBHERecHitCollection>& hbhehandle,
				    const  edm::Handle<HFRecHitCollection>&   hfhandle,
				    const CaloMETCollection& dirtyMET,
				    const CaloMETCollection& cleanMET,
				    std::vector<double>& v_hbheHitEnergiesRemoved,
				    std::vector<double>& v_hfHitEnergiesRemoved,
				    double& deltasumet,
				    double& deltamet,
				    double& signifOfdeltamet,
				    double& deltasignif);
  bool jetidCleanerTouchedThisEvent(const CaloMETCollection& dirtyMET,
				    const METCollection&     cleanMET,
				    double& deltasumet,
				    double& deltamet,
				    double& signifOfdeltamet,
				    double& deltasignif);
  
  bool HCALfilterTouchedThisEvent  (const edm::TriggerResults& trgresults,
				    const edm::TriggerNames&   trgnames);
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
  double minSumET4plotGeV_;
  double maxSumET4plotGeV_;

  double mindMET4plotGeV_;
  double maxdMET4plotGeV_;
  double mindSumET4plotGeV_;
  double maxdSumET4plotGeV_;

  TH3F *h3f_correlations_;

  TH1F *h1f_dirtySumET_;
  TH1F *h1f_clnSumETevfilt_;
  TH1F *h1f_clnSumETreflag_;
  TH1F *h1f_clnSumETjetID_;

  TH1F *h1f_dirtyMET_;
  TH1F *h1f_clnMETevfilt_;
  TH1F *h1f_clnMETreflag_;
  TH1F *h1f_clnMETjetID_;

  TH1F *h1f_nPVall_;
  TH1F *h1f_nPVevfilt_;
  TH1F *h1f_nPVreflag_;
  TH1F *h1f_nPVjetID_;

  TH2F *h2f_deltaMETjetidVSreflagger_;

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

  minSumET4plotGeV_  = iConfig.getUntrackedParameter<double>("minSumET4plotGeV"); 
  maxSumET4plotGeV_  = iConfig.getUntrackedParameter<double>("maxSumET4plotGeV"); 
  mindSumET4plotGeV_ = iConfig.getUntrackedParameter<double>("mindSumET4plotGeV"); 
  maxdSumET4plotGeV_ = iConfig.getUntrackedParameter<double>("maxdSumET4plotGeV"); 

  minMET4plotGeV_    = iConfig.getUntrackedParameter<double>("minMET4plotGeV"); 
  maxMET4plotGeV_    = iConfig.getUntrackedParameter<double>("maxMET4plotGeV"); 
  mindMET4plotGeV_   = iConfig.getUntrackedParameter<double>("mindMET4plotGeV"); 
  maxdMET4plotGeV_   = iConfig.getUntrackedParameter<double>("maxdMET4plotGeV"); 

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
METcleaningComparator::reflaggerTouchedThisEvent(const edm::Handle<HBHERecHitCollection>& hbhehandle,
						 const edm::Handle<HFRecHitCollection>&   hfhandle,
						 const CaloMETCollection& dirtyMET,
						 const CaloMETCollection& cleanMET,
						 std::vector<double>& v_hbheHitEnergiesRemoved,
						 std::vector<double>& v_hfHitEnergiesRemoved,
						 double& deltasumet,
						 double& deltamet,
						 double& signifOfdeltamet,
						 double& deltasignif)
{
  if (hbhehandle.isValid()) {
    const HBHERecHitCollection& hbherechits = *hbhehandle;
    for (size_t i=0; i<hbherechits.size(); i++)
      if (hbherechits[i].flagField(hbheFlagBit_))
	v_hbheHitEnergiesRemoved.push_back(hbherechits[i].energy());
  }

  if (hfhandle.isValid()) {
    const HFRecHitCollection& hfrechits = *hfhandle;
    for (size_t i=0; i<hfrechits.size(); i++)
      if (hfrechits[i].flagField(hfFlagBit_))
	v_hfHitEnergiesRemoved.push_back(hfrechits[i].energy());
  }

  deltamet   = cleanMET.front().pt()    - dirtyMET.front().pt();    // signed
  deltasumet = dirtyMET.front().sumEt() - cleanMET.front().sumEt(); // nonnegative

  signifOfdeltamet = deltamet/dirtyMET.front().sumEt();
  deltasignif  =
    cleanMET.front().pt()/cleanMET.front().sumEt() - 
    dirtyMET.front().pt()/dirtyMET.front().sumEt();

  return (deltasumet != 0.0);

}                    // METcleaningComparator::reflaggerTouchedThisEvent

// ---------------------------------------------------------------------

bool
METcleaningComparator::jetidCleanerTouchedThisEvent(const CaloMETCollection& dirtyMET,
						    const METCollection&     cleanMET,
						    double& deltasumet,
						    double& deltamet,
						    double& signifOfdeltamet,
						    double& deltasignif)
{
  deltamet   = cleanMET.front().pt()    - dirtyMET.front().pt();    // signed
  deltasumet = dirtyMET.front().sumEt() - cleanMET.front().sumEt(); // nonnegative

  signifOfdeltamet = deltamet/dirtyMET.front().sumEt();
  deltasignif  =
    cleanMET.front().pt()/cleanMET.front().sumEt() - 
    dirtyMET.front().pt()/dirtyMET.front().sumEt();

  return (deltasumet != 0.0);

}                 // METcleaningComparator::jetidCleanerTouchedThisEvent

// ---------------------------------------------------------------------

bool
METcleaningComparator::HCALfilterTouchedThisEvent(const edm::TriggerResults& trgresults,
						  const edm::TriggerNames&   trgnames)
{
  if (firstEvent_) { // latch the evfiltpath index

    unsigned i;
    for (i=0; i!=trgnames.size(); i++) {
      if (trgnames.triggerName(i) == evfiltpath_) {
	evfiltpathindex_ = i; break;
      }
    }
    if (i==trgnames.size())
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

   // get trigger names
   //const edm::TriggerNames & triggerNames = iEvent.triggerNames(trgresults);

   dirtyInput_->get(iEvent,iSetup);
   jetidOutput_->get(iEvent,iSetup);
   reflagOutput_->get(iEvent,iSetup);
   evfiltOutput_->get(iEvent,iSetup);

   std::vector<double> v_hbheHitEnergies,v_hfHitEnergies;
   double deltaMETreflag,deltaSumETreflag,deltasignifReflag,signifOfdeltametReflag;
   double deltaMETjetid, deltaSumETjetid,deltasignifJetid,signifOfdeltametJetid;

   //bool evfilt = HCALfilterTouchedThisEvent(*(evfiltOutput_->trgResults()));
   bool evfilt = HCALfilterTouchedThisEvent(*(evfiltOutput_->hbheNoiseResult()));
   bool reflag = reflaggerTouchedThisEvent (reflagOutput_->hbherechits(),
					    reflagOutput_->hfrechits(),
					    *(dirtyInput_->calomet()),
					    *(reflagOutput_->calomet()),
					    v_hbheHitEnergies,
					    v_hfHitEnergies,
					    deltaSumETreflag,
					    deltaMETreflag,
					    signifOfdeltametReflag,
					    deltasignifReflag);
   bool jetid  = jetidCleanerTouchedThisEvent(*(dirtyInput_->calomet()),
					      *(jetidOutput_->recomet()),
					      deltaSumETjetid,
					      deltaMETjetid,
					      signifOfdeltametJetid,
					      deltasignifJetid);

   h1f_dirtySumET_->Fill(dirtyInput_->calomet()->front().sumEt());
   h1f_dirtyMET_  ->Fill(dirtyInput_->calomet()->front().pt());

   if (!evfilt) {
     h1f_clnSumETevfilt_->Fill(dirtyInput_->calomet()->front().sumEt());
     h1f_clnMETevfilt_  ->Fill(dirtyInput_->calomet()->front().pt());
   }

   h1f_clnSumETreflag_->Fill(reflagOutput_->calomet()->front().sumEt());
   h1f_clnMETreflag_  ->Fill(reflagOutput_->calomet()->front().pt());

   h1f_clnSumETjetID_->Fill(jetidOutput_->recomet()->front().sumEt());
   h1f_clnMETjetID_  ->Fill(jetidOutput_->recomet()->front().pt());

   h3f_correlations_->Fill(evfilt,reflag,jetid);

   if (jetid || reflag) {
     myAnalHistos *myAH = histosPerCategory[evfilt][reflag][jetid];
     myAH->fill1d<TH1F>("h1f_deltaSumETreflagger", deltaSumETreflag);
     myAH->fill1d<TH1F>("h1f_deltaSumETjetid",     deltaSumETjetid);

     myAH->fill1d<TH1F>("h1f_deltaMETreflagger",        deltaMETreflag);
     myAH->fill1d<TH1F>("h1f_deltaMETjetid",            deltaMETjetid);

     myAH->fill2d<TH2F>("h2f_SignifVSdeltaMETreflagger", deltaMETreflag,signifOfdeltametReflag);
     myAH->fill2d<TH2F>("h2f_SignifVSdeltaMETjetid",     deltaMETjetid,signifOfdeltametJetid);

     myAH->fill1d<TH1F>("h1f_deltasignifReflagger",      deltasignifReflag);
     myAH->fill1d<TH1F>("h1f_deltasignifJetid",          deltasignifJetid);

     if (v_hbheHitEnergies.size())
       myAH->fill1d<TH1F>("h1f_hbheHitEnergiesRemoved", v_hbheHitEnergies[i]);
     if (v_hfHitEnergies.size())
       myAH->fill1d<TH1F>("h1f_hfHitEnergiesRemoved",   v_hfHitEnergies[i]);

     h2f_deltaMETjetidVSreflagger_->Fill(deltaMETjetid,deltaMETreflag);
   }

   if (dirtyInput_->vertices().isValid()) {
     int npv = dirtyInput_->vertices()->size();
     h1f_nPVall_->Fill(npv);
     if (evfilt) h1f_nPVevfilt_->Fill(npv);
     if (reflag) h1f_nPVreflag_->Fill(npv);
     if (jetid)  h1f_nPVjetID_ ->Fill(npv);
   }
}

// ------------ method called once each job just before starting event loop  ------------
void 
METcleaningComparator::beginJob()
{
  int nbinsSumET = (int)(maxSumET4plotGeV_-minSumET4plotGeV_);
  int nbinsMET   = (int)(maxMET4plotGeV_  -minMET4plotGeV_);

  edm::Service<TFileService> fs;

  h1f_dirtySumET_     = fs->make<TH1F>("h1f_dirtySumET",
				       "Uncleaned SumET; #Sigma E_{T} (GeV); dN/GeV",
				       nbinsSumET,minSumET4plotGeV_,maxSumET4plotGeV_);
  h1f_clnSumETevfilt_ = fs->make<TH1F>("h1f_clnSumETevfilt",
				       "Cleaned SumET (Event Filter); #Sigma E_{T} (GeV); dN/GeV",
				       nbinsSumET,minSumET4plotGeV_,maxSumET4plotGeV_);
  h1f_clnSumETreflag_ = fs->make<TH1F>("h1f_clnSumETreflag",
				       "Cleaned SumET (Reflagger algo); #Sigma E_{T} (GeV); dN/GeV",
				       nbinsSumET,minSumET4plotGeV_,maxSumET4plotGeV_);
  h1f_clnSumETjetID_  = fs->make<TH1F>("h1f_clnSumETjetID",
				       "Cleaned SumET (Jet ID); #Sigma E_{T} (GeV); dN/GeV",
				       nbinsSumET,minSumET4plotGeV_,maxSumET4plotGeV_);

  h1f_dirtyMET_       = fs->make<TH1F>("h1f_dirtyMET",
				       "Uncleaned MET; MET (GeV); dN/GeV",
				       nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);

  h1f_clnMETevfilt_   = fs->make<TH1F>("h1f_clnMETevfilt",
				       "Cleaned MET (Event Filter); MET (GeV); dN/GeV",
				       nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);

  h1f_clnMETreflag_   = fs->make<TH1F>("h1f_clnMETreflag",
				       "Cleaned MET (Reflagger algo); MET (GeV); dN/GeV",
				       nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);

  h1f_clnMETjetID_    = fs->make<TH1F>("h1f_clnMETjetID",
				       "Cleaned MET (Jet ID); MET (GeV); dN/GeV",
				       nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);


  nbinsSumET = (int)(maxdSumET4plotGeV_-mindSumET4plotGeV_);
  nbinsMET   = (int)(maxdMET4plotGeV_  -mindMET4plotGeV_);

  //  if (nbinsMET>1000) nbinsMET /= 10;
  h2f_deltaMETjetidVSreflagger_ = fs->make<TH2F>("h2f_deltaMETjetidVSreflagger",
 "#Delta MET (Reflagger) vs. #Delta MET (Jet ID); #Delta MET JetID (GeV); #Delta MET Reflagger (GeV)",
						 nbinsMET,mindMET4plotGeV_,maxdMET4plotGeV_,
						 nbinsMET,mindMET4plotGeV_,maxdMET4plotGeV_);

  myAnalHistos::HistoParams_t hpars1("","",nbinsMET,mindMET4plotGeV_,maxdMET4plotGeV_);
  myAnalHistos::HistoParams_t hpars2("","",nbinsSumET,mindSumET4plotGeV_,maxdSumET4plotGeV_);
  myAnalHistos::HistoParams_t hpars3("","",2000,0,2000.);

  myAnalHistos::HistoParams_t hpars4("","",nbinsMET,mindMET4plotGeV_,maxdMET4plotGeV_,100,-1.,1.);
  myAnalHistos::HistoParams_t hpars5("","",100,-1.,1.);

  // noise rejection rates as a function of pileup events (#PVs)
  h1f_nPVall_       = fs->make<TH1F>("h1f_nPVall", "Number of primary vertices",
				     11, -0.5, 10.5);

  h1f_nPVevfilt_    = fs->make<TH1F>("h1f_nPVevfilt",
     "Noise Rejection Rate vs. Pileup Events (Event Filter); # PVs; # Events touched/Total ",
				     11, -0.5, 10.5);
  h1f_nPVreflag_    = fs->make<TH1F>("h1f_nPVreflag",
     "Noise Rejection Rate vs. Pileup Events (Reflagger); # PVs; # Events touched/Total ",
				     11, -0.5, 10.5);
  h1f_nPVjetID_     = fs->make<TH1F>("h1f_nPVjetID",
     "Noise Rejection Rate vs. Pileup Events (Jet ID); # PVs; # Events touched/Total ",
				     11, -0.5, 10.5);

  for (int i=0; i<2; i++)
    for (int j=0; j<2; j++)
      for (int k=0; k<2; k++) {
	char s[80];
	sprintf (s,"evflt%dreflg%djetid%d",i,j,k);
	if (!(j+k)) continue; // only need histos for reflagger and/or jetid active
	myAnalHistos *myAH = new myAnalHistos(s);
	hpars1.name=hpars1.title="h1f_deltaMETreflagger";         myAH->book1d<TH1F>(hpars1);
	hpars1.name=hpars1.title="h1f_deltaMETjetid";             myAH->book1d<TH1F>(hpars1);

	hpars2.name=hpars2.title="h1f_deltaSumETreflagger";       myAH->book1d<TH1F>(hpars2);
	hpars2.name=hpars2.title="h1f_deltaSumETjetid";           myAH->book1d<TH1F>(hpars2);

	hpars3.name=hpars3.title="h1f_hbheHitEnergiesRemoved";    myAH->book1d<TH1F>(hpars3);
	hpars3.name=hpars3.title="h1f_hfHitEnergiesRemoved";      myAH->book1d<TH1F>(hpars3);

	hpars4.name=hpars4.title="h2f_SignifVSdeltaMETreflagger"; myAH->book2d<TH2F>(hpars4);
	hpars4.name=hpars4.title="h2f_SignifVSdeltaMETjetid";     myAH->book2d<TH2F>(hpars4);

	hpars5.name=hpars5.title="h1f_deltasignifReflagger";      myAH->book1d<TH1F>(hpars5);
	hpars5.name=hpars5.title="h1f_deltasignifJetid";          myAH->book1d<TH1F>(hpars5);

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

  //h1f_nPVevfilt_->Divide(h1f_nPVall_);
  //h1f_nPVreflag_->Divide(h1f_nPVall_);
  //h1f_nPVjetID_ ->Divide(h1f_nPVall_);
}

//define this as a plug-in
DEFINE_FWK_MODULE(METcleaningComparator);
