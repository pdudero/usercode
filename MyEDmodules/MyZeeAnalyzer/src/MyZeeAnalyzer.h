#ifndef MyEDmodules_MyZeeAnalyzer_MyZeeAnalyzer_h
#define MyEDmodules_MyZeeAnalyzer_MyZeeAnalyzer_h

// -*- C++ -*-
//
// Package:    PatAlgos
// Class:      MyZeeAnalyzer
// 
/**\class MyZeeAnalyzer MyZeeAnalyzer.cc  MyEDmodules/MyZeeAnalyzer/src/MyZeeAnalyzer.cc

 Description: <A very (very) simple CMSSW analyzer for PAT objects>

 Implementation:
 
 this analyzer shows how to loop over PAT output. 
*/
//
// Original Author:  Freya Blekman
//         Created:  Mon Apr 21 10:03:50 CEST 2008
// $Id: MyZeeAnalyzer.h,v 1.1 2008/06/04 15:04:33 srappocc Exp $
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
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/ParameterSet/interface/InputTag.h"


#include "DataFormats/PatCandidates/interface/Electron.h"

#include "TH1D.h"
#include <map>

#include "DataFormats/Common/interface/View.h"
#include <string>
//
// class decleration
//

class MyZeeAnalyzer : public edm::EDAnalyzer {
   public:
      explicit MyZeeAnalyzer(const edm::ParameterSet&);
      ~MyZeeAnalyzer();


   private:
      virtual void beginJob(const edm::EventSetup&) ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

      void filterEls   (const edm::View<pat::Electron>& elsIn,
			std::vector<reco::RecoCandidate *>& elsOut);
      void lookForZees (const std::vector<reco::RecoCandidate*>& genericEls);


      // ----------member data ---------------------------

  
  std::map<std::string,TH1D*> histocontainer_; // simple map to contain all histograms. Histograms are booked in the beginJob() method
  
  edm::InputTag eleLabel_;
  double minInvMassZeeWindow_;
  double maxInvMassZeeWindow_;
  double hMaxPt_;
};

#endif
