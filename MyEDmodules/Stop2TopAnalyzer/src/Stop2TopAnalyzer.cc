//

#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Handle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Common/interface/Provenance.h"
#include "SimDataFormats/HepMCProduct/interface/HepMCProduct.h"
#include "CLHEP/HepMC/GenVertex.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "MyAnalyzers/Stop2TopAnalyzer/src/Stop2TopAnalAlgos.h"
#include "MyAnalyzers/Stop2TopAnalyzer/src/Stop2TopAnalJEScor.h"

#include <iostream>
#include <vector>

//======================================================================

using namespace std;
using namespace HepMC;

//======================================================================

//======================================================================
/** \class Stop2TopAnalyzer specification
      
$Date: 2005/10/06 22:21:33 $
$Revision: 1.2 $
\author P. Dudero - Minnesota
*/
class Stop2TopAnalyzer : public edm::EDAnalyzer {
public:
  explicit Stop2TopAnalyzer(edm::ParameterSet const& conf);
  virtual ~Stop2TopAnalyzer() { }
  virtual void beginJob(const edm::EventSetup& c);
  virtual void endJob();
  virtual void analyze(edm::Event const& e, edm::EventSetup const& c);
private:
  bool               verbosity_;
  bool               doJEScor_;
  Stop2TopAnalAlgos  *anal_;
  Stop2TopAnalJEScor *jescor_;
  int                evt;
};

//======================================================================

Stop2TopAnalyzer::Stop2TopAnalyzer(edm::ParameterSet const& conf) :
  verbosity_(conf.getUntrackedParameter<bool>("verbosity")),
  doJEScor_(conf.getParameter<bool>("doJEScorrection"))
{
  if (doJEScor_) {
    jescor_ =
      new Stop2TopAnalJEScor(verbosity_,
			     conf.getUntrackedParameter<string>("outRootFileName"),
			     conf.getParameter<double>("max_dR"),
			     conf.getParameter<vector<double> >("genETbins"),
			     conf.getParameter<vector<double> >("recETbins"),
			     conf.getParameter<double>("emfraction4eid"));
  }
  else {
    anal_
      = new Stop2TopAnalAlgos(verbosity_,
			      conf.getUntrackedParameter<string>("outRootFileName"),
			      conf.getParameter<double>("emfraction4eid"),
			      conf.getParameter<double>("recjetETgevthresh"),
			      conf.getParameter<double>("maxplottedETgev"),
			      conf.getParameter<double>("hltcut1_electronETminGeV"),
			      conf.getParameter<double>("cut2_electronETminGeV"),
			      conf.getParameter<int>   ("cut3_minNumJets"),
			      conf.getParameter<double>("cut4_maxDphirTeb"),
			      conf.getParameter<double>("cut5_minHTplusMETgev"));
  }
  
  evt = 0;
}

//======================================================================

void Stop2TopAnalyzer::beginJob(const edm::EventSetup& c)
{
  if (doJEScor_)
    jescor_->beginJob();
  else
    anal_->beginJob();
}

//======================================================================

void Stop2TopAnalyzer::endJob()
{
  if (doJEScor_)
    jescor_->endJob();
  else
    anal_->endJob();
}

//======================================================================

void Stop2TopAnalyzer::analyze(edm::Event const& iEvent,
			       edm::EventSetup const& c)
{
  // get event id information
  int nrun = iEvent.id().run();
  int nevt = iEvent.id().event();

  if (verbosity_) {
    cout  << "Processing run" << nrun << "," << "event " << nevt << endl;
  }

  //////////////////////
  // get MC information
  /////////////////////

  edm::HepMCProduct HepMCEvt;
  edm::Handle<CaloJetCollection> cj2h, cj5h, cj7h;
  edm::Handle<GenJetCollection>  gj2h, gj5h, gj7h;
  std::vector<edm::Handle<edm::HepMCProduct> > AllHepMCEvt;
  iEvent.getManyByType(AllHepMCEvt);

  if (!AllHepMCEvt.size()) {
    cout  << "Unable to find HepMCProduct in event!";
    return;
  }

  HepMCEvt = *AllHepMCEvt[0];

  iEvent.getByLabel("mcone2gen",gj2h);
  iEvent.getByLabel("mcone5gen",gj5h);
  iEvent.getByLabel("mcone7gen",gj7h);

  GenJetCollection  genjets2 = *gj2h;
  GenJetCollection  genjets5 = *gj5h;
  GenJetCollection  genjets7 = *gj7h;

  iEvent.getByLabel("mcone2",cj2h);
  iEvent.getByLabel("mcone5",cj5h);
  iEvent.getByLabel("mcone7",cj7h);

  CaloJetCollection recjets2 = *cj2h;
  CaloJetCollection recjets5 = *cj5h;
  CaloJetCollection recjets7 = *cj7h;

  if (doJEScor_)
    jescor_->processEvent(genjets2, genjets5, genjets7,
			  recjets2, recjets5, recjets7);
  else
    anal_->processEvent(HepMCEvt,
			recjets2, recjets5, recjets7);
}

//======================================================================


#include "PluginManager/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_SEAL_MODULE();
DEFINE_ANOTHER_FWK_MODULE(Stop2TopAnalyzer)
