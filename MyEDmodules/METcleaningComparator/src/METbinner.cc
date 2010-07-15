// -*- C++ -*-
//
// Package:    METbinner
// Class:      METbinner
// 
/**\class METbinner METbinner.cc MyEDmodules/METbinner/src/METbinner.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Phil Dudero
//         Created:  Tue May 11 16:17:42 CDT 2010
// $Id: METbinner.cc,v 1.3 2010/05/31 13:35:25 dudero Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "TH3F.h"

//
// class declaration
//

class METbinner : public edm::EDAnalyzer {
public:
  explicit METbinner(const edm::ParameterSet&);
  ~METbinner();


private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  // ----------member data ---------------------------
  double minMET4plotGeV_;
  double maxMET4plotGeV_;
  double minSumET4plotGeV_;
  double maxSumET4plotGeV_;

  TH1F *h1f_metSumET_;
  TH1F *h1f_metMET_;

  myEventData *metInput_;
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
METbinner::METbinner(const edm::ParameterSet& iConfig)
{
  edm::ParameterSet metDataPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("metInput");
  metInput_ = new myEventData(metDataPset);

  minSumET4plotGeV_  = iConfig.getUntrackedParameter<double>("minSumET4plotGeV"); 
  maxSumET4plotGeV_  = iConfig.getUntrackedParameter<double>("maxSumET4plotGeV"); 

  minMET4plotGeV_    = iConfig.getUntrackedParameter<double>("minMET4plotGeV"); 
  maxMET4plotGeV_    = iConfig.getUntrackedParameter<double>("maxMET4plotGeV"); 
}

METbinner::~METbinner()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}

// ------------ method called to for each event  -----------------------
void
METbinner::analyze(const edm::Event& iEvent,
			       const edm::EventSetup& iSetup)
{
   using namespace edm;

   metInput_->get(iEvent,iSetup);

   h1f_metSumET_->Fill(metInput_->calomet()->front().sumEt());
   h1f_metMET_  ->Fill(metInput_->calomet()->front().pt());
}

// ------------ method called once each job just before starting event loop  ------------
void 
METbinner::beginJob()
{
  int nbinsSumET = (int)(maxSumET4plotGeV_-minSumET4plotGeV_);
  int nbinsMET   = (int)(maxMET4plotGeV_  -minMET4plotGeV_);

  edm::Service<TFileService> fs;

  h1f_metSumET_ = fs->make<TH1F>("h1f_metSumET",
				   "Uncleaned SumET; #Sigma E_{T} (GeV); dN/GeV",
				   nbinsSumET,minSumET4plotGeV_,maxSumET4plotGeV_);
  h1f_metMET_   = fs->make<TH1F>("h1f_metMET",
				   "Uncleaned MET; MET (GeV); dN/GeV",
				   nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);
}

// ------------ method called once each job just after ending the event loop  ------------
void 
METbinner::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(METbinner);
