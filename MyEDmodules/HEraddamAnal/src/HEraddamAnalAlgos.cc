
// -*- C++ -*-
//
// Package:    HEraddamAnalAlgos
// Class:      HEraddamAnalAlgos
// 
/**\class HEraddamAnalAlgos HEraddamAnalAlgos.cc MyEDmodules/HEraddamAnalAlgos/src/HEraddamAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HEraddamAnalAlgos.cc,v 1.2 2013/03/13 15:11:40 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>
#include <math.h> // floor

// user include files
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"

#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"
#include "EventFilter/HcalRawToDigi/interface/HcalDCCHeader.h"

#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"

#include "../interface/HEraddamAnalAlgos.hh"


#include "TH1D.h"
#include "TH2D.h"
#include "TF2.h"
#include "TProfile.h"
#include "TProfile2D.h"

using namespace std;

inline string int2str(int i) {
  ostringstream ss;
  ss << i;
  return ss.str();
}

//==================================================================

int date2day(const char *date)
{
  const int dayspermonthleapyear[12] = {
    31,29,31,30,31,30,31,31,30,31,30,31
  };

  const int dayspermonth[12] = {
    31,28,31,30,31,30,31,31,30,31,30,31
  };

  int dayofyear;

  TString datestr(date);

  TObjArray *tokens = datestr.Tokenize("-");

  assert(tokens->GetEntriesFast()==3);

  int year  = ((TObjString *)(*tokens)[0])->GetString().Atoi();
  int month = ((TObjString *)(*tokens)[1])->GetString().Atoi();
  int day   = ((TObjString *)(*tokens)[2])->GetString().Atoi();

  //assert((year==2011) || (year==2012));

  dayofyear=0;

  int imonth;
  const int *arr = (year==2012 ||
		    year==2016) ? dayspermonthleapyear : dayspermonth;

  for (imonth=0; imonth<12; imonth++) {
    if (month==imonth+1) break;
    dayofyear += arr[imonth];
  }
  assert(imonth<12);

  dayofyear += day;

  return dayofyear;
}                                                        // date2day

//==================================================================
const std::string HEraddamAnalAlgos::mysubdetstr_     = "HE";
const std::string HEraddamAnalAlgos::st_perEvtDigi_   = "perEvtDigi";
const std::string HEraddamAnalAlgos::st_doPerChannel_ = "doPerChannel";

//
// constructors and destructor
//
HEraddamAnalAlgos::HEraddamAnalAlgos(const edm::ParameterSet& iConfig)
{
  edm::Service<TFileService> fs;

  year_               = iConfig.getParameter<int>   ("year");
  ampCutsInfC_        = iConfig.getParameter<bool>  ("ampCutsInfC");
  minHitAmplitude_    = iConfig.getParameter<double>("minHit_GeVorfC");
  maxHitAmplitude_    = iConfig.getParameter<double>("maxHit_GeVorfC");
  doPerChannel_       = iConfig.getParameter<bool>  ("doPerChannel");
  doTree_             = iConfig.getUntrackedParameter<bool>("doTree",false);

  // cut string vector initialized in order
  v_cuts_.push_back("cutNone");
  v_cuts_.push_back("cutamp");
  st_lastCut_ = "cutamp";

  for (unsigned i=0; i<v_cuts_.size(); i++)
    m_cuts_[v_cuts_[i]] = new myAnalCut(i,v_cuts_[i],*fs);

  if (doPerChannel_) {
    getCut("cutNone")->setFlag(st_doPerChannel_);
    getCut("cutamp")->setFlag(st_doPerChannel_);
  }

  TDCalgo_   = new
    LaserTDCalgos(iConfig.getUntrackedParameter<edm::ParameterSet>("TDCpars"));

#if 0
  std::vector<int> v_raddamchannels =
    iConfig.getParameter<vector<int> >("heraddamchannels");

  if (!buildChannelSet(v_raddamchannels))
    throw cms::Exception("Invalid detID vector");
  else if (channeldetIds_.size()) {
    cout << "Raddam channel list:"<<endl;
    map<int,int>::const_iterator it;
    int i=0;
    for (it=channeldetIds_.begin(); it!=channeldetIds_.end();it++) {
      int denseid=it->first;
      cout << i<<": "<<denseid<<" "<<HcalDetId(denseid);
      if (!(++i%4)) cout << endl;
      else          cout << "\t";
    }
    cout << endl;
  }
#endif

  tdcwindowsfile_ =
    iConfig.getUntrackedParameter<string>("tdcwindowsfile");

  rundatesfile_ =
    iConfig.getUntrackedParameter<string>("rundatesfile");

  lumiprofilefile_ =
    iConfig.getUntrackedParameter<string>("lumiprofilefile");

  parseTDCwindows(tdcwindowsfile_);
  readRunDates(rundatesfile_);
  readLumiProfile(lumiprofilefile_);

  firstEvent_ = true;
}                                // HEraddamAnalAlgos::HEraddamAnalAlgos

//======================================================================

bool
HEraddamAnalAlgos::buildChannelSet(const std::vector<int>& v_idnumbers)
{
  // convert det ID numbers to valid detIds:
  if (v_idnumbers.size()%3) {
    return false;
  }

  // 99 = wildcard
  for (uint32_t i=0; i<v_idnumbers.size(); i+=3) {
    int ieta    = v_idnumbers[i];
    int iphi    = v_idnumbers[i+1];
    int depth   = v_idnumbers[i+2];
    int minieta = ieta;
    int maxieta = ieta;
    int miniphi = iphi;
    int maxiphi = iphi;
    int mindpth = depth;
    int maxdpth = depth;
    int phiskip = 1;

    if (depth==99)   { mindpth=1;   maxdpth=3;  }
    if (ieta==99)    { minieta=17;  maxieta=29; } else
      if (ieta==-99) { minieta=-29; maxieta=-17;}
    if (iphi==99)    { miniphi=1;   maxiphi=71; phiskip=1; }

    for (int depth=mindpth; depth<=maxdpth; depth++) {
      for (int ieta=minieta; ieta<=maxieta; ieta++) {
	for (int iphi=miniphi; iphi<=maxiphi; iphi+=phiskip) {
	  std::pair<int,int> thepair(HcalDetId(HcalEndcap,ieta,iphi,depth).rawId(),-1);
	  channeldetIds_.insert(thepair);
	}
      }
    }
  }
  // see if all the entries were invalid
  if (v_idnumbers.size() && !channeldetIds_.size()) return false;

  int i=0;
  for (std::map<int,int>::iterator it=channeldetIds_.begin();
       it != channeldetIds_.end(); 
       it++)
    it->second = i++;

  return true;
}                                  // HEraddamAnalAlgos::buildChannelSet

//======================================================================

bool
HEraddamAnalAlgos::buildChannelSet(const std::vector<DetId>& heCells)
{
  for (size_t i=0; i<heCells.size(); i++) {
    std::pair<int,int> thepair(heCells[i].rawId(),-1);
    channeldetIds_.insert(thepair);
  }

  int i=0;
  for (std::map<int,int>::iterator it=channeldetIds_.begin();
       it != channeldetIds_.end(); 
       it++)
    it->second = i++;

  cout << "Total " << channeldetIds_.size() << " detIds to process" << endl;

  return true;
}                                  // HEraddamAnalAlgos::buildChannelSet

//======================================================================

bool
HEraddamAnalAlgos::buildCalibChannelSet()
{
  for (int ieta=-1; ieta<=1; ieta+=2) {
    for (int iphi=3; iphi<=71; iphi+=4) {
      for (int ctype=0; ctype < 7; ctype++) {
	if (ctype==2) continue;
	std::pair<int,int> thepair(HcalCalibDetId(HcalEndcap,ieta,iphi,ctype).rawId(),-1);
	calibchanneldetIds_.insert(thepair);
      }
    }
  }

  cout << "Total " << calibchanneldetIds_.size() << " calibdetIds to process" << endl;

  return true;
}                               // HEraddamAnalAlgos::buildCalibChannelSet

//======================================================================

void
HEraddamAnalAlgos::parseTDCwindows(const std::string& tdcwindowsfile)
{
  FILE *fp = fopen(tdcwindowsfile.c_str(),"r");
  if (!fp) {
    cerr << "Couldn't find file " << tdcwindowsfile << endl;
    return;
  }
  cout << "Reading windows file " << tdcwindowsfile << endl;

  char instr[128];
  while (!feof(fp) && fgets(instr,128,fp)) {
    int denseid,tdcstart,tdcwidth;
    sscanf(instr,"%d %d %d",&denseid,&tdcstart,&tdcwidth);
    map<int,int>::const_iterator it=channeldetIds_.find(denseid);
    if (it==channeldetIds_.end()) {
      cerr << "Channel " << denseid << " not known" << endl;
      exit(-1);
    }
    m_tdcwins_[denseid] = std::pair<int,int>(tdcstart,tdcwidth);
  }
  fclose(fp);
}                                 //  HEraddamAnalAlgos::parseTDCwindows

//======================================================================

void
HEraddamAnalAlgos::readRunDates(const std::string& rundatesfile)
{
  FILE *fp = fopen(rundatesfile.c_str(),"r");
  if (!fp) {
    cerr << "Couldn't find file " << rundatesfile << endl;
    exit(-1);
  }
  cout << "Reading rundates file " << rundatesfile << endl;

  m_rundates_.clear();
  char instr[128];
  while (!feof(fp) && fgets(instr,128,fp)) {
    if (instr[0]=='#') continue; // comments welcome
    int runnum, daynum;
    if (sscanf(instr,"%d %d",&runnum,&daynum) != 2) {
      cerr << "bad format in run dates file " << rundatesfile << ":"<<instr<<endl;
      exit(-1);
    }
    m_rundates_[runnum] = daynum;
  }
  fclose(fp);
}                                    //  HEraddamAnalAlgos::readRunDates

//======================================================================

void
HEraddamAnalAlgos::readLumiProfile(const std::string& lumiprofilefile)
{
  FILE *fp = fopen(lumiprofilefile.c_str(),"r");
  if (!fp) {
    cerr << "Couldn't find file " << lumiprofilefile << endl;
    exit(-1);
  }
  cout << "Reading lumiprofile file " << lumiprofilefile << endl;

  m_lumiprofile_.clear();
  char instr[128];
  double intlumiofyear = 0.0;
  while (!feof(fp) && fgets(instr,128,fp)) {
    char date[12];
    char time[12];
    double intlumi_pb;
    int dayofyear;
    if (instr[0]=='#') continue; //comments are welcome...
    if (lumiprofilefile.find("2016") != string::npos) {
      if (sscanf(instr,"%s %s %lf",date,time,&intlumi_pb) != 3) {
	cerr << "bad format in lumi profile file " << lumiprofilefile << ":"<<instr<<endl;
	exit(-1);
      }
      intlumiofyear = intlumi_pb;
      dayofyear = date2day(date);
    } else {
      if (sscanf(instr,"%s %d %lf %lf",date,&dayofyear,&intlumi_pb,&intlumiofyear) != 4) {
	cerr << "bad format in lumi profile file " << lumiprofilefile << ":"<<instr<<endl;
	exit(-1);
      }
    }
    m_lumiprofile_[dayofyear] = std::pair<double,double>(intlumi_pb,intlumiofyear);
  }
  fclose(fp);
}                                 //  HEraddamAnalAlgos::readLumiProfile

//======================================================================

void
HEraddamAnalAlgos::lumiForDay(int     dayofyear,
			      float& intlumipbofday,
			      float& intlumipbofyear)
{
  std::map<int,std::pair<double,double> >::iterator itlo;

  itlo = m_lumiprofile_.lower_bound(dayofyear);

  intlumipbofday  = (float)itlo->second.first;
  intlumipbofyear = (float)itlo->second.second;
}

//======================================================================

bool
HEraddamAnalAlgos::isWithinWindow(void)
{
  return ((TDCphase_ >= TDCwinstart_[ididx_]) &&
	  (TDCphase_ <  TDCwinstart_[ididx_]+TDCwinwidth_[ididx_]));
}

//======================================================================

myAnalCut *
HEraddamAnalAlgos::getCut(const std::string& cutstr)
{
  std::map<std::string,myAnalCut *>::const_iterator it = m_cuts_.find(cutstr);
  if (it == m_cuts_.end()) {
    throw cms::Exception("Cut not found, you numnutz! You changed the name: ") << cutstr << endl;
  }

  return it->second;
}                                         // HEraddamAnalAlgos::getCut

//======================================================================

void
HEraddamAnalAlgos::bookHistos4allCuts(void)
{
  std::vector<myAnalHistos::HistoParams_t>   v_hpars1d; 
  std::vector<myAnalHistos::HistoParams_t>   v_hpars1dprof;

  /*****************************************
   * 1-D HISTOGRAMS FIRST:                 *
   *****************************************/

  st_TDCLaserFireTime_ = "TDClaserFireTime"; // Binned to .8 ns width
  add1dHisto(st_TDCLaserFireTime_,"TDC Laser Fire Time; Time (ns); Pulses",
	     376,1000,1300,
	     v_hpars1d);

  //==================== Collection sizes ====================

  st_digiColSize_ = "h1d_digiCollectionSize" + mysubdetstr_;
  add1dHisto( st_digiColSize_, "Digi Collection Size, "+mysubdetstr_+" "+rundescr_,
	      5201,-0.5, 5200.5, // 72chan/RBX*72RBX = 5184, more than HE or HO
	      v_hpars1d);

  st_fCamplitude_ = "h1d_fcAmplitude" + mysubdetstr_;
  add1dHisto( st_fCamplitude_, "fC total amplitude, "+mysubdetstr_+" "+rundescr_,
	      400,0, 20000,
	      v_hpars1d);

  st_fCamplitudeMinus_ = "h1d_fcAmplitude" + mysubdetstr_+"M";
  add1dHisto( st_fCamplitudeMinus_, "fC total amplitude, "+mysubdetstr_+"M "+rundescr_,
	      400,0, 20000,
	      v_hpars1d);

  st_fCamplitudePlus_ = "h1d_fcAmplitude" + mysubdetstr_+"P";
  add1dHisto( st_fCamplitudePlus_, "fC total amplitude, "+mysubdetstr_+"P "+rundescr_,
	      400,0, 20000,
	      v_hpars1d);

  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  string yaxist = normalizeDigis_ ? "Normalized fC" : "fC";

  st_avgPulse_   = "h1d_pulse" + mysubdetstr_;
  add1dHisto(st_avgPulse_,"Average Pulse Shape, "+mysubdetstr_+"; TS;"+yaxist,
	     10,-0.5,9.5,v_hpars1dprof);

  st_avgPulsePlus_   = "h1d_pulse" + mysubdetstr_ + "P";
  add1dHisto(st_avgPulsePlus_,"Average Pulse Shape, "+mysubdetstr_+"P; TS;"+yaxist,
	     10,-0.5,9.5,v_hpars1dprof);

  st_avgPulseMinus_   = "h1d_pulse" + mysubdetstr_ + "M";
  add1dHisto(st_avgPulseMinus_,"Average Pulse Shape, "+mysubdetstr_+"M; TS;"+yaxist,
	     10,-0.5,9.5,v_hpars1dprof);

  /*****************************************
   * BOOK 'EM, DANNO...                    *
   *****************************************/

  uint32_t total = v_hpars1d.size()+v_hpars1dprof.size();

  cout<<mysubdetstr_<<": Booking "<<total<<" histos for each of ";
  cout<<m_cuts_.size()<<" cuts..."<<std::endl;

  std::map<string, myAnalCut *>::const_iterator cutit;
  for (cutit = m_cuts_.begin(); cutit != m_cuts_.end(); cutit++) {
    cout << "Booking for cut " << cutit->first << endl;
    myAnalHistos *cutAH = cutit->second->cuthistos();
    cutAH->book1d<TH1F>      (v_hpars1d);
    cutAH->book1d<TProfile>  (v_hpars1dprof);

    if (cutit->second->doInverted()) {
      myAnalHistos *invAH = cutit->second->invhistos();
      invAH->book1d<TH1F>      (v_hpars1d);
      invAH->book1d<TProfile>  (v_hpars1dprof);
    }

    bookDetailHistos4cut(*(cutit->second));
      
  } // cut loop

  m_cuts_["cutNone"]->cuthistos()->mkSubdir<uint32_t>("_calibdigisfCperID");

  cout<<"Done."<<std::endl;
}                               // HEraddamAnalAlgos::bookHistos4allCuts

//======================================================================

void
HEraddamAnalAlgos::bookPerChanDetail(myAnalHistos *myAH)
{
  assert(myAH);

  // make per channel subfolders
  myAH->mkSubdir<uint32_t>("_digisfCperID");
  myAH->mkSubdir<uint32_t>("_fCampperID");

  // don't need to do any more here, individual channel histos will be
  // booked as they are encountered the first time.

}                                // HEraddamAnalAlgos::bookPerChanDetail

//======================================================================

void
HEraddamAnalAlgos::bookDetailHistos4cut(myAnalCut& cut)
{
  cout<<"Detail histos for cut "<<cut.description()<<std::endl;

  bool doinv = cut.doInverted();

  myAnalHistos *cutAH = cut.cuthistos();
  myAnalHistos *invAH = cut.invhistos();

  if (cut.flagSet(st_doPerChannel_)) {
    bookPerChanDetail(cutAH);
    if (doinv)
      bookPerChanDetail(invAH);
  }

  cout<<"Done."<<std::endl;
}                           // HEraddamAnalAlgos::bookDetailHistos4cut

//==================================================================

void
HEraddamAnalAlgos::fillHistos4cut(myAnalCut& thisCut)
{
  //edm::LogInfo("Filling histograms for subdet ") << mysubdetstr_ << std::endl;

  myAnalHistos *myAH;
  if (thisCut.isActive()) {
    if (thisCut.doInverted()) { 
      myAH = thisCut.invhistos();
    } else
      return;
  } else {
    myAH = thisCut.cuthistos();
  }

  int    zside = detID_.zside();

  /* Takes care of most histos! */
  myAH->setCurDetId(detID_);

  // make sure all autofill variables are set by this point!
  myAH->autofill<float>();

  myAH->fill1d<TH1D>(st_TDCLaserFireTime_,TDCphase_);

  CaloSamples filldigifC;

  myAH->fill1d<TH1D>(st_fCamplitude_,fCamplitude_[ididx_]);
  myAH->fill1d<TH1D>((zside>0) ?
		     st_fCamplitudePlus_.c_str():
		     st_fCamplitudeMinus_.c_str(),
		     fCamplitude_[ididx_]);

  filldigifC = digifC_;
  int digisize = std::min(10,digifC_.size());
    
  TProfile *avgPulse = myAH->get<TProfile>(st_avgPulse_.c_str());
  for (int its=0; its<digisize; ++its)
    avgPulse->Fill(its,filldigifC[its]);
  
  avgPulse =
    myAH->get<TProfile>(((zside>0)?
			 st_avgPulsePlus_.c_str():
			 st_avgPulseMinus_.c_str()));
  
  for (int its=0; its<digisize; ++its)
    avgPulse->Fill(its,filldigifC[its]);

  //cout << detID_ << "\t" << feID_.rbx() << "\t" << feID_.rbx().substr(3,2) << "\t";
  //cout << feID_.rm() << " maps to RBX/RM# " << iRBX << "/" << fRM_ << endl;

  stringstream chname;
  uint32_t     dix;

  dix = detID_.rawId(); chname << detID_;

  if (   // (myAH != thisCut.invhistos()) &&
      thisCut.flagSet(st_doPerChannel_)) {

    //========== Pulse shape per channel ==========

    if (1){  // perchOverThresh) {

      perChanHistos *perchFolder = myAH->getAttachedHisto<uint32_t>("_digisfCperID");
      if (!perchFolder)
	throw cms::Exception("Folder _digisfCperID not found for histos "+myAH->name());
      fillDigiPulseHistos(perchFolder,dix,chname.str(),filldigifC);
      
      perchFolder = myAH->getAttachedHisto<uint32_t>("_fCampperID");
      if (!perchFolder)
	throw cms::Exception("Folder _fCampperID not found for histos "+myAH->name());
      fillfCamp4chan(perchFolder,dix,chname.str());
    }

  } // if doPerChannel

}                                   // HEraddamAnalAlgos::fillHistos4cut

//======================================================================

void
HEraddamAnalAlgos::fillDigiPulseHistos(perChanHistos     *digiFolder,
				       uint32_t           chkey,
				       const std::string& chname,
				       const CaloSamples& filldigi)
{
  perChanHistos::HistoParams_t hpars(int2str(chkey),chname,10,-0.5,9.5);

  int digisize = std::min(10,filldigi.size());
  
  // pulses in fC
  if (!digiFolder) throw cms::Exception("digi folder not found");

  TProfile *hpulse = digiFolder->exists(chkey)   ?
    digiFolder->get<TProfile>(chkey) :
    digiFolder->book1d<TProfile>(chkey,hpars,false);

  if (hpulse) {
    for (int its=0; its<digisize; ++its) {
      hpulse->Fill(its,filldigi[its]);
    }
  }
}                            // HEraddamAnalAlgos::fillDigiPulseHistos

//======================================================================

void
HEraddamAnalAlgos::fillfCamp4chan(perChanHistos     *perchFolder,
				  uint32_t           chkey,
				  const std::string& chname)
{
  perChanHistos::HistoParams_t hpars(int2str(chkey),chname,20000,0,20000);
  
  if (!perchFolder) throw cms::Exception("perch folder not found");

  TH1D *h1d = perchFolder->exists(chkey)   ?
    perchFolder->get<TH1D>(chkey) :
    perchFolder->book1d<TH1D>(chkey,hpars,false);

  if (h1d)
    h1d->Fill(fCamplitude_[ididx_]);
}                                   // HEraddamAnalAlgos::fillfCamp4chan

//==================================================================
//
void
HEraddamAnalAlgos::processDigi(const HcalCalibDataFrame& df)
{
  double totamp  = 0;

  if ( (df.id().det() == DetId::Hcal) &&
       (df.id().hcalSubdet()==HcalEndcap) &&
       (df.id().calibFlavor()==HcalCalibDetId::CalibrationBox) ) {
    calibID_ = df.id();

    //cout << calibID_ << endl;

    map<int,int>::const_iterator it=calibchanneldetIds_.find(df.id().rawId());
    assert (it!=calibchanneldetIds_.end());

    cbididx_=it->second;

    cbieta_[cbididx_] = calibID_.ieta();
    cbiphi_[cbididx_] = calibID_.iphi();
    cbtype_[cbididx_] = calibID_.cboxChannel();

    calibdigifC_ = CaloSamples(DetId(df.id().rawId()),df.size());

    double nominal_ped = (df[0].nominal_fC() + df[1].nominal_fC())/2.0;

    int zside = calibID_.ieta()/abs(calibID_.ieta()); // calibID_.zside() always returns zero!!

    //cout << "iphi="<<calibID_.iphi()<<", iquad="<<iquad<<", zside="<<zside<<endl;

    for (int i=0; i<df.size(); i++) {
      calibdigifC_[i] = df[i].nominal_fC()-nominal_ped;
      totamp += calibdigifC_[i];
    }
    switch(calibID_.cboxChannel()) {
    case HcalCalibDetId::cbox_MixerHigh: {
      if (zside>0) { HEPmixhi_[cbididx_] = totamp; }
      else         { HEMmixhi_[cbididx_] = totamp; }
      break;
    }
    case HcalCalibDetId::cbox_MixerLow: {
      if (zside>0) { HEPmixlo_[cbididx_] = totamp; }
      else         { HEMmixlo_[cbididx_] = totamp; }
      break;
    }
    case HcalCalibDetId::cbox_RadDam_Layer0_RM4: {
      if (zside>0) { HEPlay0rm4_[cbididx_] = totamp; }
      else         { HEMlay0rm4_[cbididx_] = totamp; }
      break;
    }
    case HcalCalibDetId::cbox_RadDam_Layer7_RM4: {
      if (zside>0) { HEPlay7rm4_[cbididx_] = totamp; }
      else         { HEMlay7rm4_[cbididx_] = totamp; }
      break;
    }
    case HcalCalibDetId::cbox_RadDam_Layer0_RM1: {
      if (zside>0) { HEPlay0rm1_[cbididx_] = totamp; }
      else         { HEMlay0rm1_[cbididx_] = totamp; }
      break;
    }
    case HcalCalibDetId::cbox_RadDam_Layer7_RM1: {
      if (zside>0) { HEPlay7rm1_[cbididx_] = totamp; }
      else         { HEMlay7rm1_[cbididx_] = totamp; }
      break;
    }
    default:break;
    }
  }
}                                   // HEraddamAnalAlgos::processDigi

//==================================================================
// Following routine receives digi in ADC (df)
// and sets member variables digifC_ and digiGeV_ as output.
//
void
HEraddamAnalAlgos::processDigi(const myDataFrame& df)
{
  CaloSamples dfC; // dfC is the linearized (fC) digi

  const HcalCalibrations& calibs = conditions_->getHcalCalibrations(df.id());
  const HcalQIECoder *qieCoder   = conditions_->getHcalCoder( df.id() );
  const HcalQIEShape *qieShape   = conditions_->getHcalShape();
  HcalCoderDb coder( *qieCoder, *qieShape );
  coder.adc2fC( df, dfC );

  digiGeV_ = dfC;
  fCamplitude_[ididx_] = 0;
  pkSample_ = -1;
  float peakfC = -1;
  for (int i=0; i<dfC.size(); i++) {
    int capid=df[i].capid();
    digiGeV_[i] = (dfC[i]-calibs.pedestal(capid)); // pedestal subtraction
    if (digiGeV_[i] > peakfC) {
      peakfC = digiGeV_[i];
      pkSample_ = i;
    }
    fCamplitude_[ididx_] += digiGeV_[i];
    digiGeV_[i]*= calibs.respcorrgain(capid) ;    // fC --> GeV
  }
  digifC_ = dfC;

  // pick the peak and a neighboring sample such that the two samples
  // have the largest sum, and take the ratio
  //
  if      (pkSample_ == dfC.size()-1) {
    s0adc_[ididx_] = df[pkSample_-2].adc();
    s1adc_[ididx_] = df[pkSample_-1].adc();
    s2adc_[ididx_] = df[pkSample_].adc();
  }
  else if (pkSample_ == 0) {
    s0adc_[ididx_] = 0;
    s1adc_[ididx_] = df[pkSample_].adc();
    s2adc_[ididx_] = df[pkSample_+1].adc();
  }
  else if (dfC[pkSample_+1]>dfC[pkSample_-1]) {
    s0adc_[ididx_] = 0;
    s1adc_[ididx_] = df[pkSample_].adc();
    s2adc_[ididx_] = df[pkSample_+1].adc();
  }
  else  {
    if (pkSample_ <= 1)
      s0adc_[ididx_] = 0;
    else
      s0adc_[ididx_] = df[pkSample_-2].adc();
    s1adc_[ididx_] = df[pkSample_-1].adc();
    s2adc_[ididx_] = df[pkSample_].adc();
  }
}                                   // HEraddamAnalAlgos::processDigi

//======================================================================

void HEraddamAnalAlgos::processDigis
  (const edm::Handle<HcalCalibDigiCollection>& digihandle)
{
  assert(digihandle.isValid());

  myAnalHistos *myAH = m_cuts_["cutNone"]->cuthistos();

  myAH->fill1d<TH1D>(st_digiColSize_,digihandle->size());

  for (unsigned idf = 0; idf < digihandle->size();++idf) {

    const HcalCalibDataFrame&  df = (*digihandle)[idf];

    processDigi(df);

    uint32_t dix = calibID_.rawId(); 
    stringstream chname; chname << calibID_;

    perChanHistos *perchFolder = myAH->getAttachedHisto<uint32_t>("_calibdigisfCperID");
    if (!perchFolder)
      throw cms::Exception("Folder _calibdigisfCperID not found for histos "+myAH->name());

    fillDigiPulseHistos(perchFolder,dix,chname.str(), calibdigifC_);

  } // loop over digis

}                                     // HEraddamAnalAlgos::processDigis

//======================================================================

void HEraddamAnalAlgos::processDigis
  (const edm::Handle<myDigiCollection>& digihandle)
{
  assert(digihandle.isValid());

  myAnalHistos *myAH = m_cuts_["cutNone"]->cuthistos();

  myAH->fill1d<TH1D>(st_digiColSize_,digihandle->size());

  for (unsigned idf = 0; idf < digihandle->size();++idf) {

    const myDataFrame&  df = (*digihandle)[idf];

    map<int,int>::const_iterator it=channeldetIds_.find(df.id().rawId());
    if (it==channeldetIds_.end()) continue;

    ididx_=it->second;

    //cout << it->first << " " << it->second << endl;

    if (df.id().det() == DetId::Hcal) {
      detID_  = HcalDetId(df.id());
      feID_   = lmap_->getHcalFrontEndId(detID_);

      //int  absieta = detID_.ietaAbs();
      int    zside = detID_.zside();
      int iRMinRBX = feID_.rm();
      //int   ipix = feID_.pixel();
      
      denseID_[ididx_] = it->first;
      ieta_[ididx_]    = detID_.ieta();
      iphi_[ididx_]    = detID_.iphi();
      depth_[ididx_]   = detID_.depth();
      iRBX_[ididx_]    = atoi(((feID_.rbx()).substr(3,2)).c_str());
      iRM_[ididx_]     = zside * ((iRBX_[ididx_]-1)*4 + iRMinRBX);
    }

    processDigi(df);

    std::map<int,std::pair<int,int> >::const_iterator it2 = 
      m_tdcwins_.find(detID_.rawId());

    if (it2 == m_tdcwins_.end()) {
      TDCwinstart_[ididx_] = TDCwinwidth_[ididx_] = 0;
    } else {
      //cout << it2->second.first << " " << it2->second.second << endl;
      TDCwinstart_[ididx_] = it2->second.first;
      TDCwinwidth_[ididx_] = it2->second.second;
    }

    fillHistos4cut(*(m_cuts_["cutNone"]));

    if ((fCamplitude_[ididx_] >= minHitAmplitude_) &&
	(fCamplitude_[ididx_] <= maxHitAmplitude_) )
      fillHistos4cut(*(m_cuts_["cutamp"]));

  } // loop over digis

}                                     // HEraddamAnalAlgos::processDigis

//======================================================================

// ------------ method called to for each event  ------------
void
HEraddamAnalAlgos::process(const myEventData& ed)
{
  bool isLocalHCALrun = ed.hcaltbtrigdata().isValid();
  bool isLaserEvent   = false;
  if (isLocalHCALrun) {
    TDCalgo_->process(ed);
    if (ed.hcaltbtrigdata()->wasLaserTrigger()) isLaserEvent = true;
  } else
    cerr << "Not a local HCAL run!" << endl;

#if 0
  // Abort Gap laser 
  if ((!isLocalHCALrun || !isLaserEvent) &&
      ed.fedrawdata().isValid())
  {
    //checking FEDs for calibration information
    for (int i=FEDNumbering::MINHCALFEDID;i<=FEDNumbering::MAXHCALFEDID; i++) {
      const FEDRawData& fedData = ed.fedrawdata()->FEDData(i) ;
      if ( fedData.size() < 24 ) continue ;
      int value = ((const HcalDCCHeader*)(fedData.data()))->getCalibType() ;
      if(value==hc_HBHEHPD || value==hc_HOHPD || value==hc_HEPMT){ isLaserEvent=true; break;} 
    }
  }   
#endif

  if(!isLaserEvent) { cerr << "Not a laser event!" << endl; // return;
  } else {
    nlaserEv_++;
    TDCphase_ = (float)TDCalgo_->phase();
  }

  runnum_    = ed.runNumber();

  if (m_rundates_.find(runnum_) == m_rundates_.end()) {
    cerr << "no run date for run " << runnum_ << endl;
    exit(-1);
  }

  dayofyear_ = m_rundates_[runnum_];

  lumiForDay(dayofyear_,intlumipbofday_,intlumipbofyear_);

  if (firstEvent_) {
    firstEvent_ = false;
    if (isLocalHCALrun) 
      cout << "Local TB trigger data detected\n" << endl;

    conditions_ = ed.hcalConditions();
    geometry_   = ed.hcalGeometry();

    buildChannelSet(geometry_->getValidDetIds(DetId::Hcal, HcalEndcap));
    buildCalibChannelSet();
 
    HcalLogicalMapGenerator gen;
    lmap_ = new HcalLogicalMap(gen.createMap());

    if (!rundescr_.size())
      rundescr_ = "Run "+int2str(runnum_);

    bookHistos4allCuts();
  }

  lsnum_  = ed.lumiSection();
  bxnum_  = ed.bxNumber();
  evtnum_ = ed.evtNumber();

  //ed.runInfo().product()->printAllValues();
#if 0
  int ut = ed.unixTime();
  cout << "ut = " << ut << endl;

  if (ut) {
    dayofyear_ = (localtime((time_t *)&ut))->tm_yday;
    cout << "dayofyear = " << dayofyear_ << endl;
  }
#endif
  processDigis(ed.hcalcalibdigis());

  processDigis(ed.hbhedigis());

  if (doTree_) tree_->Fill();

  neventsProcessed_++;
}                                       // HEraddamAnalAlgos::process

//======================================================================

void
HEraddamAnalAlgos::beginJob()
{
  neventsProcessed_=0;

  edm::Service<TFileService> fs;

  if (doTree_) {
    tree_ = fs->make<TTree>("mytree","Hcal Results Tree");
    tree_->Branch("lsnum",           &lsnum_,          "lsnum/I");
    tree_->Branch("bxnum",           &bxnum_,          "bxnum/I");
    tree_->Branch("evtnum",          &evtnum_,         "evtnum/I");
    tree_->Branch("runnum",          &runnum_,         "runnum/I");
    tree_->Branch("year",            &year_,           "year/I");
    tree_->Branch("dayofyear",       &dayofyear_,      "dayofyear/I");
    tree_->Branch("intlumipbofday",  &intlumipbofday_, "intlumipbofday/F");
    tree_->Branch("intlumipbofyear", &intlumipbofyear_,"intlumipbofyear/F");
    tree_->Branch("TDCphase",        &TDCphase_,       "TDCphase/F");
    tree_->Branch("TDCwinstart",     TDCwinstart_,     "TDCwinstart[2592]/I");
    tree_->Branch("TDCwinwidth",     TDCwinwidth_,     "TDCwinwidth[2592]/I");
    tree_->Branch("denseID",         denseID_,         "denseID[2592]/I");
    tree_->Branch("iRBX",            iRBX_ ,           "iRBX[2592]/I");
    tree_->Branch("iRM",             iRM_ ,            "iRM[2592]/I");
    tree_->Branch("ieta",            ieta_ ,           "ieta[2592]/I");
    tree_->Branch("iphi",            iphi_ ,           "iphi[2592]/I");
    tree_->Branch("depth",           depth_ ,          "depth[2592]/I");
    tree_->Branch("s0adc",           s0adc_,           "s0adc[2592]/I");
    tree_->Branch("s1adc",           s1adc_,           "s1adc[2592]/I");
    tree_->Branch("s2adc",           s2adc_,           "s2adc[2592]/I");
    tree_->Branch("fCamplitude",     fCamplitude_,     "fCamplitude[2592]/F");
    tree_->Branch("HEPmixhi",        HEPmixhi_,        "HEPmixhi[216]/F");
    tree_->Branch("HEPmixlo",        HEPmixlo_,        "HEPmixlo[216]/F");
    tree_->Branch("HEMmixhi",        HEMmixhi_,        "HEMmixhi[216]/F");
    tree_->Branch("HEMmixlo",        HEMmixlo_,        "HEMmixlo[216]/F");
    tree_->Branch("HEPlay0rm4",      HEPlay0rm4_,      "HEPlay0rm4[216]/F");
    tree_->Branch("HEMlay0rm4",      HEMlay0rm4_,      "HEMlay0rm4[216]/F");
    tree_->Branch("HEPlay7rm4",      HEPlay7rm4_,      "HEPlay7rm4[216]/F");
    tree_->Branch("HEMlay7rm4",      HEMlay7rm4_,      "HEMlay7rm4[216]/F");
    tree_->Branch("HEPlay0rm1",      HEPlay0rm1_,      "HEPlay0rm1[216]/F");
    tree_->Branch("HEMlay0rm1",      HEMlay0rm1_,      "HEMlay0rm1[216]/F");
    tree_->Branch("HEPlay7rm1",      HEPlay7rm1_,      "HEPlay7rm1[216]/F");
    tree_->Branch("HEMlay7rm1",      HEMlay7rm1_,      "HEMlay7rm1[216]/F");
  }

  std::cout << "----------------------------------------"  << "\n"
	    << "Parameters being used: "                << "\n" <<
    "subdet_             = "      << mysubdetstr_       << "\n" << 
    "ampCutsInfC_        = "      << ampCutsInfC_       << "\n" << 
    "minHitAmplitude_    = "<<dec << minHitAmplitude_   << "\n" << 
    "maxHitAmplitude_    = "      << maxHitAmplitude_   << "\n" << 
    "normalizeDigis_     = "      << normalizeDigis_    << "\n" <<
    "doPerChannel_       = "      << doPerChannel_      << "\n" <<
    "doTree_             = "      << doTree_            << "\n" <<
    "tdcwindowsfile_     = "      << tdcwindowsfile_    << "\n" <<
    "rundatesfile_       = "      << rundatesfile_      << "\n" <<
    "lumiprofilefile_    = "      << lumiprofilefile_   << "\n";

}

//======================================================================

void
HEraddamAnalAlgos::endAnal()
{
  std::cout << neventsProcessed_ << " laser events processed" << std::endl;

  //positivize2dHistos();
}

//======================================================================
