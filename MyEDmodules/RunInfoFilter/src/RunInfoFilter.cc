// -*- C++ -*-
//
// Package:    RunInfoFilter
// Class:      RunInfoFilter
// 
/**\class RunInfoFilter RunInfoFilter.cc MyEDmodules/RunInfoFilter/src/RunInfoFilter.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep 16 17:18:34 CEST 2008
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

//
// class declaration
//

class RunInfoFilter : public edm::EDFilter {
   public:
      explicit RunInfoFilter(const edm::ParameterSet&);
      ~RunInfoFilter();

   private:
      virtual void beginJob(const edm::EventSetup&) ;
      virtual bool filter(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      
      // ----------member data ---------------------------
  std::set<int> s_evNumbers_;
  std::set<int> s_bxNumbers_;
  std::set<int> s_lsNumbers_;
  std::set<int> s_rnNumbers_;

  int rnfilt_;
  int evfilt_;
  int lsfilt_;
  int bxfilt_;
  int evtpassed_;
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
RunInfoFilter::RunInfoFilter(const edm::ParameterSet& iConfig)
{
   //now do what ever initialization is needed

  std::vector<int> evNumberVec(iConfig.getParameter<std::vector<int> >("eventNumbers"));
  std::vector<int> bxNumberVec(iConfig.getParameter<std::vector<int> >("bunchNumbers"));
  std::vector<int> lsNumberVec(iConfig.getParameter<std::vector<int> >("lumiSectionNumbers"));
  std::vector<int> rnNumberVec(iConfig.getParameter<std::vector<int> >("runNumbers"));

  for (uint32_t i=0; i<evNumberVec.size(); i++) s_evNumbers_.insert(evNumberVec[i]);
  for (uint32_t i=0; i<bxNumberVec.size(); i++) s_bxNumbers_.insert(bxNumberVec[i]);
  for (uint32_t i=0; i<lsNumberVec.size(); i++) s_lsNumbers_.insert(lsNumberVec[i]);
  for (uint32_t i=0; i<rnNumberVec.size(); i++) s_rnNumbers_.insert(rnNumberVec[i]);

  rnfilt_ = 0;
  evfilt_ = 0;
  lsfilt_ = 0;
  bxfilt_ = 0;
  evtpassed_ = 0;
}


RunInfoFilter::~RunInfoFilter()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}

inline bool notInSet(std::set<int>& aset, int anum) {
  return (aset.find(anum) == aset.end());
}

//
// member functions
//

// ------------ method called on each new Event  ------------
bool
RunInfoFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  edm::EventID eventId = iEvent.id();
  uint32_t rnnum       = eventId.run();
  uint32_t evnum       = eventId.event();
  uint32_t lsnum       = iEvent.luminosityBlock();
  uint16_t bxnum       = iEvent.bunchCrossing();

  if (!s_rnNumbers_.empty() && notInSet(s_rnNumbers_,rnnum)) { rnfilt_++; return false; }
  if (!s_evNumbers_.empty() && notInSet(s_evNumbers_,evnum)) { evfilt_++; return false; }
  if (!s_lsNumbers_.empty() && notInSet(s_lsNumbers_,lsnum)) { lsfilt_++; return false; }
  if (!s_bxNumbers_.empty() && notInSet(s_bxNumbers_,bxnum)) { bxfilt_++; return false; }

  evtpassed_++;
  return true;
}

// ------------ method called once each job just before starting event loop  ------------
void 
RunInfoFilter::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
RunInfoFilter::endJob() {
  using namespace std;
  cout << "RunInfoFilter: #run   number filters = " << rnfilt_ << endl;
  cout << "RunInfoFilter: #event number filters = " << evfilt_ << endl;
  cout << "RunInfoFilter: #lumi section filters = " << lsfilt_ << endl;
  cout << "RunInfoFilter: #bunch number filters = " << bxfilt_ << endl;

  cout << "RunInfoFilter: #events passed        = " << evtpassed_ << endl;
}

//define this as a plug-in
DEFINE_FWK_MODULE(RunInfoFilter);
