
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/Registry.h"
#include "DataFormats/Provenance/interface/ParameterSetID.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"

// Class:      myEventData
// 
/**\class myEventData myEventData.cc MyEDmodules/myEventData/src/myEventData.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: myEventData.cc,v 1.12 2011/08/16 11:55:47 dudero Exp $
//
//

//
// constructors and destructor
//
myEventData::myEventData(const edm::ParameterSet& edPset, edm::ConsumesCollector && iC) :
  fedRawDataTag_(edPset.getUntrackedParameter<edm::InputTag>("fedRawDataLabel",edm::InputTag(""))),
  tbTrigDataTag_(edPset.getUntrackedParameter<edm::InputTag>("tbTrigDataLabel",edm::InputTag(""))),
  laserDigiTag_(edPset.getUntrackedParameter<edm::InputTag>("laserDigiLabel",edm::InputTag(""))),
  hbheRechitTag_(edPset.getUntrackedParameter<edm::InputTag>("hbheRechitLabel",edm::InputTag(""))),
  hbheDigiTag_(edPset.getUntrackedParameter<edm::InputTag>("hbheDigiLabel",edm::InputTag(""))),
  hfRechitTag_(edPset.getUntrackedParameter<edm::InputTag>("hfRechitLabel",edm::InputTag(""))),
  hfDigiTag_(edPset.getUntrackedParameter<edm::InputTag>("hfDigiLabel",edm::InputTag(""))),
  hoRechitTag_(edPset.getUntrackedParameter<edm::InputTag>("hoRechitLabel",edm::InputTag(""))),
  hoDigiTag_(edPset.getUntrackedParameter<edm::InputTag>("hoDigiLabel",edm::InputTag(""))),
  zdcRechitTag_(edPset.getUntrackedParameter<edm::InputTag>("zdcRechitLabel",edm::InputTag(""))),
  zdcDigiTag_(edPset.getUntrackedParameter<edm::InputTag>("zdcDigiLabel",edm::InputTag(""))),
  hcalibDigiTag_(edPset.getUntrackedParameter<edm::InputTag>("hcalibDigiLabel",edm::InputTag(""))),
  simHitTag_(edPset.getUntrackedParameter<edm::InputTag>("simHitLabel",edm::InputTag(""))),
  caloMETtag_(edPset.getUntrackedParameter<edm::InputTag>("caloMETlabel",edm::InputTag(""))),
  recoMETtag_(edPset.getUntrackedParameter<edm::InputTag>("recoMETlabel",edm::InputTag(""))),
  twrTag_(edPset.getUntrackedParameter<edm::InputTag>("twrLabel",edm::InputTag(""))),
  vertexTag_(edPset.getUntrackedParameter<edm::InputTag>("vertexLabel",edm::InputTag(""))),
  trgResultsTag_(edPset.getUntrackedParameter<edm::InputTag>("trgResultsLabel",edm::InputTag(""))),
  hbheNoiseResultTag_(edPset.getUntrackedParameter<edm::InputTag>("hbheNoiseResultLabel",edm::InputTag(""))),
  verbose_(edPset.getUntrackedParameter<bool>("verbose",false))
{
  firstEvent_ = true;

  using namespace std;
  if (fedRawDataTag_.label().size()) {
    fedRawDataTok_ = iC.consumes<FEDRawDataCollection>(fedRawDataTag_);
    cout << "fedRawDataTag_ = " << fedRawDataTag_ << endl;
  }
  if (tbTrigDataTag_.label().size()) {
    tbTrigDataTok_ = iC.consumes<HcalTBTriggerData>(tbTrigDataTag_);
    cout << "tbTrigDataTag_ = " << tbTrigDataTag_ << endl;
  }
  if (laserDigiTag_.label().size()) {
    laserDigiTok_ = iC.consumes<HcalLaserDigi>(laserDigiTag_);
    cout << "laserDigiTag_  = " << laserDigiTag_  << endl;
  }
  if (hbheRechitTag_.label().size()) {
    hbheRechitTok_ = iC.consumes<HBHERecHitCollection>(hbheRechitTag_);
    cout << "hbheRechitTag_ = " << hbheRechitTag_ << endl;
  }
  if (hbheDigiTag_.label().size()) {
    hbheDigiTok_ = iC.consumes<HBHEDigiCollection>(hbheDigiTag_);
    cout << "hbheDigiTag_   = " << hbheDigiTag_   << endl;
  }
  if (hfRechitTag_.label().size()) {
    hfRechitTok_ = iC.consumes<HFRecHitCollection>(hfRechitTag_);
    cout << "hfRechitTag_   = " << hfRechitTag_   << endl;
  }
  if (hfDigiTag_.label().size()) {
    hfDigiTok_ = iC.consumes<HFDigiCollection>(hfDigiTag_);
    cout << "hfDigiTag_     = " << hfDigiTag_     << endl;
  }
  if (hoRechitTag_.label().size()) {
    hoRechitTok_ = iC.consumes<HORecHitCollection>(hoRechitTag_);
    cout << "hoRechitTag_   = " << hoRechitTag_   << endl;
  }
  if (hoDigiTag_.label().size()) {
    hoDigiTok_ = iC.consumes<HODigiCollection>(hoDigiTag_);
    cout << "hoDigiTag_     = " << hoDigiTag_     << endl;
  }
  if (zdcRechitTag_.label().size()) {
    zdcRechitTok_ = iC.consumes<ZDCRecHitCollection>(zdcRechitTag_);
    cout << "zdcRechitTag_  = " << zdcRechitTag_  << endl;
  }
  if (zdcDigiTag_.label().size()) {
    zdcDigiTok_ = iC.consumes<ZDCDigiCollection>(zdcDigiTag_);
    cout << "zdcDigiTag_    = " << zdcDigiTag_    << endl;
  }
  if (hcalibDigiTag_.label().size()) {
    hcalibDigiTok_ = iC.consumes<HcalCalibDigiCollection>(hcalibDigiTag_);
    cout << "hcalibDigiTag_ = " << hcalibDigiTag_ << endl;
  }
  if (simHitTag_.label().size()) {
    simHitTok_ = iC.consumes<edm::PCaloHitContainer>(simHitTag_);
    cout << "simHitTag_     = " << simHitTag_     << endl;
  }
  if (caloMETtag_   .label().size()) {
    caloMETtok_ = iC.consumes<reco::CaloMETCollection>(caloMETtag_);
    cout << "caloMETtag_    = " << caloMETtag_    << endl;
  }
  if (recoMETtag_   .label().size()) {
    recoMETtok_ = iC.consumes<reco::METCollection>(recoMETtag_);
    cout << "recoMETtag_    = " << recoMETtag_    << endl;
  }
  if (twrTag_.label().size()) {
    twrTok_ = iC.consumes<CaloTowerCollection>(twrTag_);
    cout << "twrTag_        = " << twrTag_        << endl;
  }
  if (vertexTag_    .label().size()) {
    vertexTok_ = iC.consumes<reco::VertexCollection>(vertexTag_);
    cout << "vertexTag_     = " << vertexTag_     << endl;
  }
  if (trgResultsTag_.label().size()) {
    trgResultsTok_ = iC.consumes<edm::TriggerResults>(trgResultsTag_);
    cout << "trgResultsTag_ = " << trgResultsTag_ << endl;
  }
  if (hbheNoiseResultTag_.label().size()) {
    hbheNoiseResultTok_ = iC.consumes<bool>(hbheNoiseResultTag_);
    cout << "hbheNoiseResultTag_ = " << hbheNoiseResultTag_ << endl;
  }
}


//======================================================================

#if 0
const std::vector<edm::EventRange>
myEventData::getEvents2Process(void)
{
  using namespace edm;
  static const std::vector<EventRange> empty;
  pset::Registry* reg = pset::Registry::instance();
  ParameterSetID toplevel = pset::getProcessParameterSetID(reg);
  ParameterSet pSet;
  reg->getMapped(toplevel, pSet);
  ParameterSet sourcePSet;
  sourcePSet  = pSet.getParameter<ParameterSet>("@main_input");
  return
    sourcePSet.getUntrackedParameter<std::vector<EventRange> >("eventsToProcess",empty);
}

//======================================================================

const std::vector<edm::LuminosityBlockRange>
myEventData::getLumis2Process(void)
{
  using namespace edm;
  static const std::vector<LuminosityBlockRange> empty;
  pset::Registry* reg = pset::Registry::instance();
  ParameterSetID toplevel = pset::getProcessParameterSetID(reg);
  ParameterSet pSet;
  reg->getMapped(toplevel, pSet);
  ParameterSet sourcePSet;
  sourcePSet  = pSet.getParameter<ParameterSet>("@main_input");
  return
    sourcePSet.getUntrackedParameter<std::vector<LuminosityBlockRange> >("lumisToProcess",empty);
}
#endif
//======================================================================

//
// member functions
//

// ------------ method called to for each event  ------------
void
myEventData::get(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  using namespace std;

  if (firstEvent_) {
    iSetup.get<HcalDbRecord>().get( hcalconditions_ );
    iSetup.get<HcalRecNumberingRecord>().get(hcaltopo_);
    iSetup.get<CaloGeometryRecord>().get(geometry_);
    //iSetup.get<HcalElectronicsMapRcd>().get(electronicsMapLabel_, hcalemap_);
    firstEvent_ = false;
  }

  eventId_ = iEvent.id();
  runn_    = eventId_.run();
  evtn_    = eventId_.event();
  lsn_     = iEvent.luminosityBlock();
  bxn_     = iEvent.bunchCrossing();

  iSetup.get<RunInfoRcd>().get( runInfo_ );

  unixTime_   = iEvent.time().unixTime();
  mikeOffset_ = iEvent.time().microsecondOffset();

  if (tbTrigDataTag_.label().size()) {
    if(!iEvent.getByToken(tbTrigDataTok_,hcaltbtrigdata_)) {
      if (verbose_)
	cerr << "myEventData::get: " <<
	  "HCAL TB trigger data not found, "<< hcaltbtrigdata_ << std::endl;
    } else if (verbose_) 
	cout << "myEventData::get: " << "Got HCAL TB trigger data " << std::endl;
  }

  if (fedRawDataTag_.label().size()) {
    if(!iEvent.getByToken(fedRawDataTok_,fedrawdata_)) {
      cerr << "myEventData::get: " <<
	"FED raw data not found, "<< fedRawDataTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got FED raw data "<< fedRawDataTag_ << std::endl;
  }

  if (laserDigiTag_.label().size()) {
    if(!iEvent.getByToken(laserDigiTok_,laserdigi_)) {
      cerr << "myEventData::get: " <<
	"Laser Digi not found, "<< laserDigiTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got Laser digi "<< laserDigiTag_ << std::endl;
  }

  if (hbheDigiTag_.label().size()) {
    if(!iEvent.getByToken(hbheDigiTok_,hbhedigis_)) {
      cerr << "myEventData::get: " <<
	"Digis not found, "<< hbheDigiTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got HBHE digis "<< hbheDigiTag_ << std::endl;
  }

  if (simHitTag_.label().size()){
    if (!iEvent.getByToken(simHitTok_, hsimhits_)) {
      cerr << "myEventData::get: " <<
	"Simhits not found, " << simHitTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got sim hits " << simHitTag_ << std::endl;
  }

  if (hbheRechitTag_.label().size()){
    if (!iEvent.getByToken(hbheRechitTok_,hbherechits_)) {
      cerr << "myEventData::get: " <<
	"HBHE Rechits not found, " << hbheRechitTag_  << std::endl;
      return;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got HBHE Rechits " << hbheRechitTag_ << std::endl;
  }

  if (hfRechitTag_.label().size()){
    if (!iEvent.getByToken(hfRechitTok_,hfrechits_)) {
      cerr << "myEventData::get: " <<
	"HF Rechits not found, " << hfRechitTag_ << std::endl;
      return;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got HF Rechits " << hfRechitTag_ << std::endl;
  }

  if (hfDigiTag_.label().size()){
    if(!iEvent.getByToken(hfDigiTok_,hfdigis_)) {
      cerr << "myEventData::get: " <<
	"Digis not found, "<< hfDigiTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got HF digis "<< hfDigiTag_ << std::endl;
  }
 
  if (hoRechitTag_.label().size()) {
    if (!iEvent.getByToken(hoRechitTok_,horechits_)) {
      cerr << "myEventData::get: " <<
	"HO Rechits not found, " << hoRechitTag_ << std::endl;
      return;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got HO Rechits " << hoRechitTag_ << std::endl;
  }

  if (hoDigiTag_.label().size()){
    if(!iEvent.getByToken(hoDigiTok_,hodigis_)) {
      cerr << "myEventData::get: " <<
	"Digis not found, "<< hoDigiTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got HO digis "<< hoDigiTag_ << std::endl;
  }
  
  if (zdcRechitTag_.label().size()){
    if (!iEvent.getByToken(zdcRechitTok_,zdcrechits_)) {
      cerr << "myEventData::get: " <<
	"ZDC Rechits not found, " << zdcRechitTag_ << std::endl;
      return;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got ZDC Rechits " << zdcRechitTag_ << std::endl;
  }

  if (zdcDigiTag_.label().size()){
    if(!iEvent.getByToken(zdcDigiTok_,zdcdigis_)) {
      cerr << "myEventData::get: " <<
	"Digis not found, "<< zdcDigiTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got ZDC digis "<< zdcDigiTag_ << std::endl;
  }

  if (hcalibDigiTag_.label().size()){
    if(!iEvent.getByToken(hcalibDigiTok_,hcalibdigis_)) {
      cerr << "myEventData::get: " <<
	"Digis not found, "<< hcalibDigiTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got HCAL Calib digis "<< hcalibDigiTag_ << std::endl;
  }

  // CaloTowers
  if (twrTag_.label().size()){
    if (!iEvent.getByToken(twrTok_,towers_)) {
      cerr << "myEventData::get: " <<
	"Calo Towers not found, " << twrTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got Calo Towers " << twrTag_ << std::endl;
  }
  
  // MET
  if (caloMETtag_.label().size()){
    if (!iEvent.getByToken(caloMETtok_, calomet_)) {
      cerr << "myEventData::get: " <<
	"Calo MET not found, " << caloMETtag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got Calo MET " << caloMETtag_ << std::endl;
  }

  if (recoMETtag_.label().size()){
    if (!iEvent.getByToken(recoMETtok_, recomet_)) {
      cerr << "myEventData::get: " <<
	"Reco MET not found, " << recoMETtag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got Reco MET " << recoMETtag_ << std::endl;
  }

#if 1
  // Reco Vertices
  if (vertexTag_.label().size()){
    if (!iEvent.getByToken(vertexTok_, vertices_)) {
      cerr << "myEventData::get: " <<
	"Reco::Vertex collection not found, " << vertexTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got Vertices " << vertexTag_ << std::endl;
#endif
  // Trigger Results
  if (trgResultsTag_.label().size())
    if (!iEvent.getByToken(trgResultsTok_, trgResults_)) {
      cerr << "myEventData::get: " <<
	"Trigger Results not found, " << trgResultsTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got Trigger Results " << trgResultsTag_ << std::endl;
  }

  // HBHE Noise Results
  if (hbheNoiseResultTag_.label().size()){
    if (!iEvent.getByToken(hbheNoiseResultTok_, hbheNoiseResult_)) {
      cerr << "myEventData::get: " <<
	"HBHE Noise Results not found, " << hbheNoiseResultTag_ << std::endl;
    } else if (verbose_) 
      cout << "myEventData::get: " << "Got HBHE Noise Results " << hbheNoiseResultTag_ << std::endl;
  }
}
