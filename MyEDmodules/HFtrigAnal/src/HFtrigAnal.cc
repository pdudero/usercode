// Analysis of HF trigger
//
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/InputTag.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"
#include "MyEDmodules/HFtrigAnal/src/HFtrigAnalAlgos.hh"

class TFile;
class TH1F;
class TH2F;

class HFtrigAnal : public edm::EDAnalyzer {
 public:
  HFtrigAnal (const edm::ParameterSet& fConfiguration);
  virtual ~HFtrigAnal ();

  // analysis itself
  virtual void analyze(const edm::Event& fEvent, const edm::EventSetup& fSetup);

  // begin of the job
  virtual void beginJob(const edm::EventSetup& fSetup);

  // very end of the job
  virtual void endJob(void);

 private:

  // ----------member data ---------------------------

  bool             verbose_;
  edm::InputTag    hfDigiLabel_;
  edm::InputTag    rechitLabel_;
  HFtrigAnalAlgos *algo_;
  int              nevt_;
};

using namespace std;

static int runN=0, eventN=0;

//
// constants, enums and typedefs
//

//======================================================================


HFtrigAnal::HFtrigAnal (const edm::ParameterSet& iConfig) :
  verbose_(iConfig.getUntrackedParameter<bool>("verbosity")),
  hfDigiLabel_(iConfig.getUntrackedParameter<edm::InputTag>("hfDigiLabel")),
  rechitLabel_(iConfig.getUntrackedParameter<edm::InputTag>("hfrechitLabel"))
{
   //now do what ever initialization is needed
  algo_ = new HFtrigAnalAlgos(verbose_, iConfig);
}

//======================================================================

HFtrigAnal::~HFtrigAnal () {
  //delete mFile;
}

//======================================================================


void HFtrigAnal::beginJob(const edm::EventSetup& fSetup) {
  algo_->beginJob();
}

//======================================================================

void HFtrigAnal::endJob(void)
{
  cout << "#RunN " << runN << " Events processed " << eventN << endl;
  algo_->endJob();
}

//======================================================================

void HFtrigAnal::analyze(const edm::Event& fEvent, const edm::EventSetup& fSetup) {

  // event ID
  edm::EventID eventId = fEvent.id();
  uint32_t runNer = eventId.run ();
  uint32_t eventNumber = eventId.event ();

  uint32_t lumiBlock = fEvent.luminosityBlock();

  int bxCross = fEvent.bunchCrossing();
  boost::uint16_t bxCrossHw = 0;
  if ((bxCross & 0xFFF) == bxCross) {
    bxCrossHw = static_cast<boost::uint16_t> (bxCross);
  } else {
#if 0
    bxCrossHw = 0; // Bx number too large, set to 0!
    LogDebug("HFtrigAnal::analyze")
      << "\nBunch cross number [hex] = "
      << std::hex << bxCross
      << "\n  larger than 12 bits. Set to 0! \n"
      << std::dec << std::endl;
#endif

    // Get the bunch number out of the trigger record:
    edm::Handle< L1GlobalTriggerReadoutRecord > gtRecord;
    if (fEvent.getByLabel("gtDigis", gtRecord))
      bxCrossHw = gtRecord->gtFdlWord().bxNr();
  }

  if (runN==0) runN=runNer;
  eventN++;

  edm::Handle<HFDigiCollection> hfdigis;
  try {
    fEvent.getByLabel(hfDigiLabel_,hfdigis);
    if (verbose_) std::cout<<"Analysis-> total HF digis= "<<hfdigis->size()<<std::endl;
  }
  catch(const cms::Exception&) {
    edm::LogWarning("HFtrigAnal::analyze") <<
      "Digis not found"<< std::endl;
    return;
  }

  edm::Handle<HFRecHitCollection> hfrechits;
  try {
    fEvent.getByLabel(rechitLabel_,hfrechits);
    if (verbose_) std::cout<<"Analysis-> total HF rechits= "<<hfrechits->size()<<std::endl;
  }
  catch(const cms::Exception&) {
    edm::LogWarning("HFtrigAnal::analyze") <<
      "Rechits not found"<< std::endl;
    return;
  }

  algo_->analyze(*hfdigis,*hfrechits, bxCrossHw, eventNumber, runNer, lumiBlock);

}

#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_SEAL_MODULE();
DEFINE_ANOTHER_FWK_MODULE(HFtrigAnal);
