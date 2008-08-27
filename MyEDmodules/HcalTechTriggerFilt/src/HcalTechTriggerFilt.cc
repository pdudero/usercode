// -*- C++ -*-
//
// Package:    HcalTechTriggerFilt
// Class:      HcalTechTriggerFilt
// 
/**\class HcalTechTriggerFilt HcalTechTriggerFilt.cc MyEDmodules/HcalTechTriggerFilt/src/HcalTechTriggerFilt.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Wed Aug 27 02:54:39 CEST 2008
// $Id$
//
//


// system include files
#include <memory>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"

//
// class declaration
//

class HcalTechTriggerFilt : public edm::EDFilter {
public:
  explicit HcalTechTriggerFilt(const edm::ParameterSet&);
  ~HcalTechTriggerFilt();

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual bool filter(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
      
  // ----------member data ---------------------------

  edm::InputTag    gtDigiLabel_;
  std::vector<int> techTrigBitVec_;
  int              npassed_;
  int              nfilt_;
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
HcalTechTriggerFilt::HcalTechTriggerFilt(const edm::ParameterSet& iConfig) :
  gtDigiLabel_(iConfig.getUntrackedParameter<edm::InputTag>("gtDigiLabel")),
  techTrigBitVec_(iConfig.getParameter<std::vector<int> >("techTriggerBits"))
{
   //now do what ever initialization is needed

  if (!techTrigBitVec_.size())
    throw cms::Exception("HcalTechTriggerFilt: Empty trigger bit vector in setup");
}


HcalTechTriggerFilt::~HcalTechTriggerFilt()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

//======================================================================
#if 0
std::vector<bool> HFtrigAnal::determineTriggers(const edm::Event& iEvent)
{
  std::vector<bool> l1Triggers; //DT,CSC,RPC,HCAL,ECAL
                                //0 , 1 , 2 , 3  , 4
  for(int i=0;i<5;i++)
    l1Triggers.push_back(false);

  // get hold of L1GlobalReadoutRecord
  Handle<L1GlobalTriggerReadoutRecord> L1GTRR;
  iEvent.getByLabel(l1GTReadoutRecTag_,L1GTRR);
  const unsigned int sizeOfDecisionWord(L1GTRR->decisionWord().size());
  if (!(L1GTRR.isValid())) {
    LogWarning("EcalCosmicsHists") << l1GTReadoutRecTag_ << " not available";
    return l1Triggers;
  }
  else {
    l1Names_.resize(sizeOfDecisionWord);
    l1Accepts_.resize(sizeOfDecisionWord);
    for (unsigned int i=0; i!=sizeOfDecisionWord; ++i) {
      l1Accepts_[i]=0;
      l1Names_[i]="NameNotAvailable";
    }
    for (unsigned int i=0; i!=sizeOfDecisionWord; ++i) {
      if (L1GTRR->decisionWord()[i]) {
	l1Accepts_[i]++;
      }
    }
  }
  
  L1GlobalTriggerReadoutRecord const* gtrr = L1GTRR.product();
  
  for(int ibx=-1; ibx<=1; ibx++) {
    bool hcal_top = false;
    bool hcal_bot = false;
    const L1GtPsbWord psb = gtrr->gtPsbWord(0xbb0d,ibx);
    std::vector<int> valid_phi;
    if((psb.aData(4)&0x3f) >= 1) {valid_phi.push_back( (psb.aData(4)>>10)&0x1f ); }
    if((psb.bData(4)&0x3f) >= 1) {valid_phi.push_back( (psb.bData(4)>>10)&0x1f ); }
    if((psb.aData(5)&0x3f) >= 1) {valid_phi.push_back( (psb.aData(5)>>10)&0x1f ); }
    if((psb.bData(5)&0x3f) >= 1) {valid_phi.push_back( (psb.bData(5)>>10)&0x1f ); }
    std::vector<int>::const_iterator iphi;
    for(iphi=valid_phi.begin(); iphi!=valid_phi.end(); iphi++) {
      //std::cout << "Found HCAL mip with phi=" << *iphi << " in bx wrt. L1A = " << ibx << std::endl;
      if(*iphi<9) hcal_top=true;
      if(*iphi>8) hcal_bot=true;
    }
    if(ibx==0 && hcal_top && hcal_bot) l1Triggers[3]=true;
  }
  
  return l1Triggers;
}
#endif
//======================================================================

// ------------ method called on each new Event  ------------
bool
HcalTechTriggerFilt::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle< L1GlobalTriggerReadoutRecord > gtRecord;

  try {
    iEvent.getByLabel(gtDigiLabel_, gtRecord);
  }
  catch(const cms::Exception&) {
    edm::LogWarning("HFtrigAnal::analyze") <<
      "Global trigger bits not found, running local mode"<< std::endl;
  }
      
  TechnicalTriggerWord ttw = gtRecord->technicalTriggerWord();

  bool techtriggered=false;
  for (uint32_t i=0; i<techTrigBitVec_.size(); i++) {
    uint32_t bit = techTrigBitVec_[i];
    if (bit < ttw.size())
      techtriggered = techtriggered || ttw[bit];
  }

  if (techtriggered) npassed_++;
  else               nfilt_++;

  return techtriggered;
}

// ------------ method called once each job just before starting event loop  ------------
void 
HcalTechTriggerFilt::beginJob(const edm::EventSetup&)
{
  npassed_ = 0;
  nfilt_   = 0;
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalTechTriggerFilt::endJob() {
  using namespace std;
  cout << "HcalTechTriggerFilt: #events passed   = " << npassed_ << endl;
  cout << "HcalTechTriggerFilt: #events filtered = " << nfilt_   << endl;
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalTechTriggerFilt);
