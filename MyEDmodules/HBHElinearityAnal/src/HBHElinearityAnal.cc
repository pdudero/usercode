// -*- C++ -*-
//
// Package:    HBHElinearityAnal
// Class:      HBHElinearityAnal
// 
/**\class HBHElinearityAnal HBHElinearityAnal.cc MyEDmodules/HBHElinearityAnal/src/HBHElinearityAnal.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Phil Dudero
//         Created:  Thu Jul 29 04:08:26 CDT 2010
// $Id$
//
//


// system include files
#include <memory>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TProfile2D.h"

//
// class declaration
//

class HBHElinearityAnal : public edm::EDAnalyzer {
   public:
      explicit HBHElinearityAnal(const edm::ParameterSet&);
      ~HBHElinearityAnal();


   private:
      virtual void beginJob() ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

      // ----------member data ---------------------------

  edm::InputTag srcTag_;
  float printAboveThreshold_;

  TH2F *h2d_5sampVs4sampHBHE_;
  TH2F *h2d_5sampVs4sampHB_;
  TH2F *h2d_5sampVs4sampHE_;
  TH2F *h2d_fracDeltaVs4E_, *h2d_fracDeltaVs4EHB_;
  TH2F *h2d_rhTvsE_;
  TH3F *h3d_OccMapPerFracDeltaEgt050_;
  TH3F *h3d_OccMapPerFracDeltaEgt100_;
  TH3F *h3d_OccMapPerFracDeltaEgt200_;
  TH3F *h3d_OccMapPerFracDeltaEgt400_;

  TProfile2D *p2d_TimeVsFracDelta4E_, *p2d_TimeVsFracDelta4EHB_;

  TProfile2D *p2d_FracDeltaProfMapEgt050_;
  TProfile2D *p2d_FracDeltaProfMapEgt100_;
  TProfile2D *p2d_FracDeltaProfMapEgt200_;
  TProfile2D *p2d_FracDeltaProfMapEgt400_;

  TProfile2D *p2d_TimeProfMapEgt050_;
  TProfile2D *p2d_TimeProfMapEgt100_;
  TProfile2D *p2d_TimeProfMapEgt200_;
  TProfile2D *p2d_TimeProfMapEgt400_;
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
HBHElinearityAnal::HBHElinearityAnal(const edm::ParameterSet& iConfig)
{
  srcTag_              = iConfig.getUntrackedParameter<edm::InputTag>("src",edm::InputTag("hbhereco"));
  printAboveThreshold_ = iConfig.getUntrackedParameter<double>("printHitAboveGeV_",500.0);

  edm::Service<TFileService> fs;
  h2d_5sampVs4sampHBHE_ = fs->make <TH2F> ("h2d_5sampVs4sampHBHE",
					   "HBHE 4-sample linearity; E_{4ts} (GeV); E_{5ts} (GeV)",
				       201,-2.5,1002.5,201,-2.5,1002.5);
  h2d_5sampVs4sampHB_ = fs->make <TH2F> ("h2d_5sampVs4sampHB",
					 "HB 4-sample linearity; E_{4ts} (GeV); E_{5ts} (GeV)",
				       201,-2.5,1002.5,201,-2.5,1002.5);
  h2d_5sampVs4sampHE_ = fs->make <TH2F> ("h2d_5sampVs4sampHE",
					 "HE 4-sample linearity; E_{4ts} (GeV); E_{5ts} (GeV)",
					 201,-2.5,1002.5,201,-2.5,1002.5);
  h2d_fracDeltaVs4E_ = fs->make <TH2F> ("h2d_fracDeltaVs4E",
"Fractional Delta vs E_{4ts}, HBHE; E_{4ts} (GeV); (E_{5ts}-E_{4ts}) / E_{4ts}",
					201,-2.5,1002.5,201,-1.005,1.005);

  h2d_fracDeltaVs4EHB_ = fs->make <TH2F> ("h2d_fracDeltaVs4EHB",
"Fractional Delta vs 4-sample Energy, HB only; E_{4ts} (GeV); (E_{5ts}-E_{4ts}) / E_{4ts}",
					201,-2.5,1002.5,201,-1.005,1.005);

  h3d_OccMapPerFracDeltaEgt050_ = fs->make<TH3F>("h3d_OccMapperFracDeltaEgt050",
"Occupancy Map for E_{4ts}>50GeV, HBHE; i#eta; i#phi; (E_{5ts}-E_{4ts}) / E_{4ts}",
					       83,-41.5,41.5,72,0.5,72.5,40,-1.0,1.0);
  h3d_OccMapPerFracDeltaEgt100_ = fs->make<TH3F>("h3d_OccMapperFracDeltaEgt100",
"Occupancy Map for E_{4ts}>100GeV, HBHE; i#eta; i#phi; (E_{5ts}-E_{4ts}) / E_{4ts}",
					       83,-41.5,41.5,72,0.5,72.5,40,-1.0,1.0);
  h3d_OccMapPerFracDeltaEgt200_ = fs->make<TH3F>("h3d_OccMapperFracDeltaEgt200",
"Occupancy Map for E_{4ts}>200GeV, HBHE; i#eta; i#phi; (E_{5ts}-E_{4ts}) / E_{4ts}",
					       83,-41.5,41.5,72,0.5,72.5,40,-1.0,1.0);
  h3d_OccMapPerFracDeltaEgt400_ = fs->make<TH3F>("h3d_OccMapperFracDeltaEgt400",
"Occupancy Map for E_{4ts}>400GeV, HBHE; i#eta; i#phi; (E_{5ts}-E_{4ts}) / E_{4ts}",
					       83,-41.5,41.5,72,0.5,72.5,40,-1.0,1.0);

  p2d_FracDeltaProfMapEgt050_ = fs->make<TProfile2D>("p2d_FracDeltaProfMapEgt050",
		       "#Delta E/E Profile Map for E_{4ts}>50GeV, HBHE; i#eta; i#phi",
						     83,-41.5,41.5,72,0.5,72.5);
  p2d_FracDeltaProfMapEgt100_ = fs->make<TProfile2D>("p2d_FracDeltaProfMapEgt100",
		       "#Delta E/E Profile Map for E_{4ts}>100GeV, HBHE; i#eta; i#phi",
						     83,-41.5,41.5,72,0.5,72.5);
  p2d_FracDeltaProfMapEgt200_ = fs->make<TProfile2D>("p2d_FracDeltaProfMapEgt200",
		       "#Delta E/E Profile Map for E_{4ts}>200GeV, HBHE; i#eta; i#phi",
						     83,-41.5,41.5,72,0.5,72.5);
  p2d_FracDeltaProfMapEgt400_ = fs->make<TProfile2D>("p2d_FracDeltaProfMapEgt400",
		       "#Delta E/E Profile Map for E_{4ts}>400GeV, HBHE; i#eta; i#phi",
						     83,-41.5,41.5,72,0.5,72.5);

  p2d_TimeProfMapEgt050_ = fs->make<TProfile2D>("p2d_TimeProfMapEgt050",
		       "Hit Time Profile Map for E_{4ts}>50GeV, HBHE; i#eta; i#phi",
						83,-41.5,41.5,72,0.5,72.5);
  p2d_TimeProfMapEgt100_ = fs->make<TProfile2D>("p2d_TimeProfMapEgt100",
		       "Hit Time Profile Map for E_{4ts}>100GeV, HBHE; i#eta; i#phi",
						83,-41.5,41.5,72,0.5,72.5);
  p2d_TimeProfMapEgt200_ = fs->make<TProfile2D>("p2d_TimeProfMapEgt200",
		       "Hit Time Profile Map for E_{4ts}>200GeV, HBHE; i#eta; i#phi",
						83,-41.5,41.5,72,0.5,72.5);
  p2d_TimeProfMapEgt400_ = fs->make<TProfile2D>("p2d_TimeProfMapEgt400",
		       "Hit Time Profile Map for E_{4ts}>200GeV, HBHE; i#eta; i#phi",
						83,-41.5,41.5,72,0.5,72.5);

  p2d_TimeVsFracDelta4E_ = fs->make<TProfile2D>("p2d_TimeVsFracDelta4E",
"Hit Time Profile vs. #Delta E/E, E_{4ts}, HBHE; E_{4ts} (GeV); #Delta E / E_{4ts}",
						201,-2.5,1002.5,201,-1.005,1.005);
  p2d_TimeVsFracDelta4EHB_ = fs->make<TProfile2D>("p2d_TimeVsFracDelta4EHB",
"Hit Time Profile vs. #Delta E/E, E_{4ts}, HB only; E_{4ts} (GeV); #Delta E / E_{4ts}",
						  201,-2.5,1002.5,201,-1.005,1.005);
  h2d_rhTvsE_ = fs->make<TH2F>("h2d_rhTvsE", 
			       "Hit Timing vs Energy; E_{4ts} (GeV); T_{hit} (ns)",
			       1101,-0.5,1100.5,201,-100.5,100.5);
}


HBHElinearityAnal::~HBHElinearityAnal()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to for each event  ------------
void
HBHElinearityAnal::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   using namespace std;

   Handle<HBHERecHitCollection> reco4samp;
   iEvent.getByLabel(srcTag_,reco4samp);

   Handle<HBHERecHitCollection> reco5samp;
   iEvent.getByLabel("myaltreco",reco5samp);

   if (!reco4samp.isValid()) {
     cerr << "Couldn't find 4 sample reconstructed hits" << endl;
     return;
   }
   if (!reco5samp.isValid()) {
     cerr << "Couldn't find 5 sample reconstructed hits" << endl;
     return;
   }
   if (reco4samp->size() != reco5samp->size()) {
     cerr << "# 4 sample hits = " << reco4samp->size() << ", ";
     cerr << "# 5 sample hits = " << reco5samp->size() << endl;
     return;
   }
   const HBHERecHitCollection& rechits4 = *reco4samp;
   const HBHERecHitCollection& rechits5 = *reco5samp;

   HBHERecHitCollection::const_iterator p4 = rechits4.begin();
   HBHERecHitCollection::const_iterator p5 = rechits5.begin();
   
   for ( ; p4 != rechits4.end(); p4++,p5++) {
     if (p4->id() != p5->id()) {
       cerr << "iterators aren't tracking." << endl;
       continue;
     }
     float p4E = p4->energy();
     float p5E = p5->energy();
     float p4T = p4->time();

     bool isHB = (p4->id().subdet() == HcalBarrel);

     h2d_5sampVs4sampHBHE_->Fill(p4E,p5E);

     if (isHB)
       h2d_5sampVs4sampHB_->Fill(p4E,p5E);
     else
       h2d_5sampVs4sampHE_->Fill(p4E,p5E);

     float fracdelta = 1.0;
     if (p4E != 0.0)
       fracdelta = (p5E-p4E)/p4E;

     h2d_fracDeltaVs4E_->Fill(p4E,fracdelta);
     p2d_TimeVsFracDelta4E_->Fill(p4E,fracdelta, p4T);
     if (isHB) {
       h2d_fracDeltaVs4EHB_->Fill(p4E,fracdelta);
       p2d_TimeVsFracDelta4EHB_->Fill(p4E,fracdelta, p4T);
     }

     h2d_rhTvsE_->Fill(p4E, p4T);

     int ieta = p4->id().ieta();
     int iphi = p4->id().iphi();
     if (p4E>50.f) {
       p2d_TimeProfMapEgt050_->Fill(ieta,iphi,p4T);
       p2d_FracDeltaProfMapEgt050_->Fill(ieta,iphi,fracdelta);
       h3d_OccMapPerFracDeltaEgt050_->Fill(ieta,iphi,fracdelta);
       if (p4E>100.f) {
	 p2d_TimeProfMapEgt100_->Fill(ieta,iphi,p4T);
	 p2d_FracDeltaProfMapEgt100_->Fill(ieta,iphi,fracdelta);
	 h3d_OccMapPerFracDeltaEgt100_->Fill(ieta,iphi,fracdelta);
	 if (p4E>200.f) {
	   p2d_TimeProfMapEgt200_->Fill(ieta,iphi,p4T);
	   p2d_FracDeltaProfMapEgt200_->Fill(ieta,iphi,fracdelta);
	   h3d_OccMapPerFracDeltaEgt200_->Fill(ieta,iphi,fracdelta);
	   if (p4E>400.f) {
	     p2d_TimeProfMapEgt400_->Fill(ieta,iphi,p4T);
	     p2d_FracDeltaProfMapEgt400_->Fill(ieta,iphi,fracdelta);
	     h3d_OccMapPerFracDeltaEgt400_->Fill(ieta,iphi,fracdelta);
	   }
	 }
       }
     }

     if (p4E > printAboveThreshold_) {
       edm::EventID eid = iEvent.id();
       char s[80];
       sprintf (s, "Run/LS/Evt/BX = %d/%d/%d/%d",
		eid.run(),   iEvent.luminosityBlock(),
		eid.event(), iEvent.bunchCrossing());
       cout << s << ", 4-sample Hit = " << *p4;
       cout      << ", 5-sample Hit = " << *p5 << endl;
     }
   }
}


// ------------ method called once each job just before starting event loop  ------------
void 
HBHElinearityAnal::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HBHElinearityAnal::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(HBHElinearityAnal);
