// -*- C++ -*-
//
// Package:    METcomparator
// Class:      METcomparator
// 
/**\class METcomparator METcomparator.cc MyEDmodules/METcomparator/src/METcomparator.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Phil Dudero
//         Created:  Tue May 11 16:17:42 CDT 2010
// $Id: METcomparator.cc,v 1.4 2010/07/15 15:22:09 dudero Exp $
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

#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"
#include "RecoLocalCalo/HcalRecAlgos/interface/HcalCaloFlagLabels.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

//
// class declaration
//

class METcomparator : public edm::EDAnalyzer {
public:
  explicit METcomparator(const edm::ParameterSet&);
  ~METcomparator();


private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  std::string addHitCategory(const std::string& descr,
			     bool doInverted=false);

  myAnalCut *getCut(const std::string& cutstr);

  void compare(const myEventData& d1,
	       const myEventData& d2);

  void bookHistos(void);

  void fillHistos4cut(myAnalCut& thisCut);

  bool reflaggerTouchedThisEvent(const edm::Handle<HBHERecHitCollection>& hbhehandle,
				 const edm::Handle<HFRecHitCollection>&   hfhandle,
				 const CaloMETCollection& dirtyMET,
				 const CaloMETCollection& cleanMET);

  // ----------member data ---------------------------

  std::map<std::string, myAnalCut *> m_cuts_;
  std::vector<std::string> v_hitCategories_;   // vector of parallel cut strings
  std::string st_cutNone_,st_cutReflagger_;

  int  ntrigreject_;
  bool firstEvent_;
  int  hbheFlagBit_;
  int  hfFlagBit_;
  double minMET4plotGeV_;
  double maxMET4plotGeV_;
  double minSumET4plotGeV_;
  double maxSumET4plotGeV_;

  double mindMET4plotGeV_;
  double maxdMET4plotGeV_;
  double mindSumET4plotGeV_;
  double maxdSumET4plotGeV_;

  typedef HBHERecHitCollection::const_iterator HBHERecHitIt;

  double deltaMET_,deltaSumET_,deltasignif_,signifOfdeltaMET_;

  std::vector<double> v_hbheHitEnergies_,v_hfHitEnergies_;

  myEventData *dirtyInput_;
  myEventData *reflagOutput_;
};

//
// constants, enums and typedefs
//

const std::string st_dirtySumET   = "h1f_dirtySumET";
const std::string st_cleanSumET   = "h1f_cleanSumET";
const std::string st_dirtyMET     = "h1f_dirtyMET";
const std::string st_cleanMET     = "h1f_cleanMET";
const std::string st_nPV          = "h1f_nPV";

inline string int2str(int i) {
  std::ostringstream ss;
  ss << i;
  return ss.str();
}

//
// static data member definitions
//

//
// constructors and destructor
//
METcomparator::METcomparator(const edm::ParameterSet& iConfig) :
  ntrigreject_(0), firstEvent_(true)
{
  cout << "'Dirty' MET Input:"<<endl;
  edm::ParameterSet dirtyDataPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("dirtyInput");
  dirtyInput_ = new myEventData(dirtyDataPset);

  cout << "Reflagger-filtered MET Output:"<<endl;
  edm::ParameterSet reflagDataPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("reflagFltrdOutput");
  reflagOutput_ = new myEventData(reflagDataPset);

  hbheFlagBit_ = iConfig.getUntrackedParameter<int>
    ("hbheFlagBit", HcalCaloFlagLabels::HBHETimingShapedCutsBits); 

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


  // all cuts applied on top of the previous one
  //
  bool doInverted  = true;
  st_cutNone_      = addHitCategory("none");
  st_cutReflagger_ = addHitCategory("reflagger",doInverted);

}

METcomparator::~METcomparator()
{
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

std::string
METcomparator::addHitCategory(const std::string& descr,
			      bool doInverted)
{
  edm::Service<TFileService> fs;

  size_t N = v_hitCategories_.size();
  string cutstr = "cut" + int2str(N) + descr;
  v_hitCategories_.push_back(cutstr);
  m_cuts_[cutstr] = new myAnalCut(N,cutstr,*fs,doInverted);
  return cutstr;
}

//======================================================================

myAnalCut *
METcomparator::getCut(const std::string& cutstr)
{
  std::map<std::string,myAnalCut *>::const_iterator it = m_cuts_.find(cutstr);
  if (it == m_cuts_.end()) {
    throw cms::Exception("Cut not found: ") << cutstr << endl;
  }

  return it->second;
}                                               // METcomparator::getCut

//======================================================================

void
METcomparator::compare(const myEventData& d1,
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
}                                              // METcomparator::compare

//======================================================================

void
METcomparator::bookHistos(void)
{
  int nbinsSumET = (int)(maxSumET4plotGeV_-minSumET4plotGeV_);
  int nbinsMET   = (int)(maxMET4plotGeV_  -minMET4plotGeV_);
  
  std::vector<myAnalHistos::HistoParams_t>   v_hpars1d;
  std::vector<myAnalHistos::HistoParams_t>   v_hpars2d;

  add1dHisto(st_dirtySumET, "Uncleaned SumET; #Sigma E_{T} (GeV); dN/GeV",
	     nbinsSumET,minSumET4plotGeV_,maxSumET4plotGeV_, v_hpars1d);
  add1dHisto(st_cleanSumET,"Cleaned SumET (Reflagger algo); #Sigma E_{T} (GeV); dN/GeV",
	     nbinsSumET,minSumET4plotGeV_,maxSumET4plotGeV_, v_hpars1d);
  add1dHisto(st_dirtyMET,"Uncleaned MET; MET (GeV); dN/GeV",
	     nbinsMET,minMET4plotGeV_,maxMET4plotGeV_,v_hpars1d);
  add1dHisto(st_cleanMET,"Cleaned MET (Reflagger algo); MET (GeV); dN/GeV",
	     nbinsMET,minMET4plotGeV_,maxMET4plotGeV_, v_hpars1d);

  nbinsSumET = (int)(maxdSumET4plotGeV_-mindSumET4plotGeV_);
  nbinsMET   = (int)(maxdMET4plotGeV_  -mindMET4plotGeV_);

  //  if (nbinsMET>1000) nbinsMET /= 10;

  myAnalHistos::HistoParams_t hpars1("","",nbinsMET,mindMET4plotGeV_,maxdMET4plotGeV_);
  myAnalHistos::HistoParams_t hpars2("","",nbinsSumET,mindSumET4plotGeV_,maxdSumET4plotGeV_);
  myAnalHistos::HistoParams_t hpars3("","",2000,0,2000.);

  myAnalHistos::HistoParams_t hpars4("","",nbinsMET,mindMET4plotGeV_,maxdMET4plotGeV_,100,-1.,1.);
  myAnalHistos::HistoParams_t hpars5("","",100,-1.,1.);
  myAnalHistos::HistoParams_t hpars6("","",nbinsMET,minMET4plotGeV_,maxMET4plotGeV_,
				           nbinsMET,minMET4plotGeV_,maxMET4plotGeV_);

  // noise rejection rates as a function of pileup events (#PVs)
  //"Noise Rejection Rate vs. Pileup Events (Reflagger); # PVs; # Events touched/Total ",
  //
  add1dHisto(st_nPV,"Number of primary vertices",11, -0.5, 10.5, v_hpars1d);

  hpars1.name=hpars1.title="h1f_deltaMETreflagger";         v_hpars1d.push_back(hpars1);
  hpars2.name=hpars2.title="h1f_deltaSumETreflagger";       v_hpars1d.push_back(hpars2);
  hpars3.name=hpars3.title="h1f_hbheHitEnergiesRemoved";    v_hpars1d.push_back(hpars3);
  hpars3.name=hpars3.title="h1f_hfHitEnergiesRemoved";      v_hpars1d.push_back(hpars3);

  hpars4.name=hpars4.title="h2f_SignifVSdeltaMETreflagger";
  hpars4.title += ";(#Delta MET = MET_{Dirty} - MET{Clean}};#Delta MET / #Sigma E_{T, dirty}";
  v_hpars2d.push_back(hpars4);

  hpars5.name=hpars5.title="h1f_deltasignifReflagger";
  hpars5.title += ";(#frac{MET_{Clean},#Sigma E_{T,Clean}} - #frac{MET_{Dirty},#Sigma E_{T,Dirty}}";
  v_hpars1d.push_back(hpars5);

  hpars6.name=hpars6.title="h2f_cleanVSdirty";
  hpars6.title += "; MET_{Dirty} (GeV); MET_{Clean} (GeV)";
  v_hpars2d.push_back(hpars6);

  std::map<std::string,myAnalCut *>::const_iterator cutit;
  for (cutit = m_cuts_.begin(); cutit != m_cuts_.end(); cutit++) {
    myAnalHistos *myAH = cutit->second->cuthistos();
    myAH->book1d<TH1F>(v_hpars1d);
    myAH->book2d<TH2F>(v_hpars2d);
    if (cutit->second->doInverted()) {
      myAH = cutit->second->invhistos();
      myAH->book1d<TH1F>(v_hpars1d);
      myAH->book2d<TH2F>(v_hpars2d);
    }
  }

}                                        // METcomparator::bookHistos

//==================================================================

void
METcomparator::fillHistos4cut(myAnalCut& thisCut)
{
  myAnalHistos *myAH;
  if (thisCut.isActive()) {
    if (thisCut.doInverted()) {
      myAH = thisCut.invhistos();
    } else
      return;
  } else {
    myAH = thisCut.cuthistos();
  }

  myAH->fill1d<TH1F>(st_dirtySumET,dirtyInput_->calomet()->front().sumEt());
  myAH->fill1d<TH1F>(st_dirtyMET,  dirtyInput_->calomet()->front().pt());

  myAH->fill1d<TH1F>(st_cleanSumET,reflagOutput_->calomet()->front().sumEt());
  myAH->fill1d<TH1F>(st_cleanMET,  reflagOutput_->calomet()->front().pt());


  myAH->fill1d<TH1F>("h1f_deltaSumETreflagger",       deltaSumET_);
  myAH->fill1d<TH1F>("h1f_deltaMETreflagger",         deltaMET_);
  myAH->fill2d<TH2F>("h2f_SignifVSdeltaMETreflagger", deltaMET_,signifOfdeltaMET_);
  myAH->fill2d<TH2F>("h2f_cleanVSdirty",              dirtyInput_->calomet()->front().pt(),
                                                      reflagOutput_->calomet()->front().pt());
  myAH->fill1d<TH1F>("h1f_deltasignifReflagger",      deltasignif_);

  if (v_hbheHitEnergies_.size())
    for (size_t i=0; i<v_hbheHitEnergies_.size(); i++)
      myAH->fill1d<TH1F>("h1f_hbheHitEnergiesRemoved", v_hbheHitEnergies_[i]);
  if (v_hfHitEnergies_.size())
    for (size_t i=0; i<v_hfHitEnergies_.size(); i++)
      myAH->fill1d<TH1F>("h1f_hfHitEnergiesRemoved",   v_hfHitEnergies_[i]);

  if (dirtyInput_->vertices().isValid()) {
    int npv = dirtyInput_->vertices()->size();
    myAH->fill1d<TH1F>(st_nPV,npv);
  }
}                                        // METcomparator::fillHistos4cut

// ----------------------------------------------------------

bool
METcomparator::reflaggerTouchedThisEvent(const edm::Handle<HBHERecHitCollection>& hbhehandle,
					 const edm::Handle<HFRecHitCollection>&   hfhandle,
					 const CaloMETCollection& dirtyMET,
					 const CaloMETCollection& cleanMET)
{
  bool retval = false;
  if (hbhehandle.isValid()) {
    const HBHERecHitCollection& hbherechits = *hbhehandle;
    for (size_t i=0; i<hbherechits.size(); i++) {
      if (hbherechits[i].flagField(hbheFlagBit_)) {
	retval = true;
	v_hbheHitEnergies_.push_back(hbherechits[i].energy());
      }
    }
  }

  if (hfhandle.isValid()) {
    const HFRecHitCollection& hfrechits = *hfhandle;
    for (size_t i=0; i<hfrechits.size(); i++)
      if (hfrechits[i].flagField(hfFlagBit_))
	v_hfHitEnergies_.push_back(hfrechits[i].energy());
  }

  deltaMET_   = cleanMET.front().pt()    - dirtyMET.front().pt();    // signed
  deltaSumET_ = dirtyMET.front().sumEt() - cleanMET.front().sumEt(); // nonnegative

  signifOfdeltaMET_ = deltaMET_/dirtyMET.front().sumEt();
  deltasignif_  =
    cleanMET.front().pt()/cleanMET.front().sumEt() - 
    dirtyMET.front().pt()/dirtyMET.front().sumEt();

  return retval;
}                    // METcomparator::reflaggerTouchedThisEvent

// ------------ method called to for each event  -----------------------
void
METcomparator::analyze(const edm::Event& iEvent,
		       const edm::EventSetup& iSetup)
{
  using namespace edm;

  dirtyInput_->get(iEvent,iSetup);
  reflagOutput_->get(iEvent,iSetup);

  v_hbheHitEnergies_.clear();
  v_hfHitEnergies_.clear();

  bool reflag = reflaggerTouchedThisEvent (reflagOutput_->hbherechits(),
					   reflagOutput_->hfrechits(),
					   *(dirtyInput_->calomet()),
					   *(reflagOutput_->calomet()));

  getCut(st_cutNone_)     ->Activate (false);
  getCut(st_cutReflagger_)->Activate(!reflag);

  for (size_t i=0; i<v_hitCategories_.size(); i++)
    fillHistos4cut(*getCut(v_hitCategories_[i]));
}

// ------------ method called once each job just before starting event loop  ------------
void 
METcomparator::beginJob()
{
  bookHistos();
}

// ------------ method called once each job just after ending the event loop  ------------
void 
METcomparator::endJob() {
  cout << "ntrigreject = " << ntrigreject_ << endl;
}

//define this as a plug-in
DEFINE_FWK_MODULE(METcomparator);
