
// -*- C++ -*-
//
// Package:    HFraddamAnalAlgos
// Class:      HFraddamAnalAlgos
// 
/**\class HFraddamAnalAlgos HFraddamAnalAlgos.cc MyEDmodules/HFraddamAnalAlgos/src/HFraddamAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HFraddamAnalAlgos.cc,v 1.2 2013/03/13 15:11:40 dudero Exp $
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

#include "../interface/HFraddamAnalAlgos.hh"


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

static const double hftwrRadii[] = { // in meters
  1.570-1.423,  // 41
  1.570-1.385,  // 40
  1.570-1.350,  // 39
  1.570-1.308,  // 38
  1.570-1.258,  // 37
  1.570-1.198,  // 36
  1.570-1.127,  // 35
  1.570-1.042,  // 34
  1.570-0.941,  // 33
  1.570-0.821,  // 32
  1.570-0.677,  // 31
  1.570-0.505,  // 30
  1.570-0.344   // 29
};

//==================================================================

int date2day(const char *date)
{
  const int dayspermonthin2012[12] = {
    31,29,31,30,31,30,31,31,30,31,30,31
  };

  const int dayspermonthin2011[12] = {
    31,28,31,30,31,30,31,31,30,31,30,31
  };

  int dayofyear;

  TString datestr(date);

  TObjArray *tokens = datestr.Tokenize("-");

  assert(tokens->GetEntriesFast()==3);

  int year  = ((TObjString *)(*tokens)[0])->GetString().Atoi();
  int month = ((TObjString *)(*tokens)[1])->GetString().Atoi();
  int day   = ((TObjString *)(*tokens)[2])->GetString().Atoi();

  assert((year==2011) || (year==2012));

  dayofyear=0;

  int imonth;
  const int *arr = (year==2011) ? dayspermonthin2011 : dayspermonthin2012;

  for (imonth=0; imonth<12; imonth++) {
    if (month==imonth+1) break;
    dayofyear += arr[imonth];
  }
  assert(imonth<12);

  dayofyear += day;

  return dayofyear;
}                                                        // date2day

//==================================================================
const std::string HFraddamAnalAlgos::mysubdetstr_     = "HF";
const std::string HFraddamAnalAlgos::st_perEvtDigi_   = "perEvtDigi";
const std::string HFraddamAnalAlgos::st_doPerChannel_ = "doPerChannel";

//
// constructors and destructor
//
HFraddamAnalAlgos::HFraddamAnalAlgos(const edm::ParameterSet& iConfig)
{
  edm::Service<TFileService> fs;

  ampCutsInfC_        = iConfig.getParameter<bool>  ("ampCutsInfC");
  minHitAmplitude_    = iConfig.getParameter<double>("minHit_GeVorfC");
  maxHitAmplitude_    = iConfig.getParameter<double>("maxHit_GeVorfC");
  doPerChannel_       = iConfig.getParameter<bool>  ("doPerChannel");
  doTree_             = iConfig.getUntrackedParameter<bool>("doTree",false);

  // cut string vector initialized in order
  v_cuts_.push_back("cutNone");
  v_cuts_.push_back("cutamp");
  v_cuts_.push_back("cutTDCwindow");
  v_cuts_.push_back("cutAll");
  st_lastCut_ = "cutAll";

  for (unsigned i=0; i<v_cuts_.size(); i++)
    m_cuts_[v_cuts_[i]] = new myAnalCut(i,v_cuts_[i],*fs);

  if (doPerChannel_) {
    getCut("cutNone")->setFlag(st_doPerChannel_);
    getCut("cutamp")->setFlag(st_doPerChannel_);
    getCut("cutTDCwindow")->setFlag(st_doPerChannel_);
    getCut("cutAll")->setFlag(st_doPerChannel_);
  }

  TDCalgo_   = new
    LaserTDCalgos(iConfig.getUntrackedParameter<edm::ParameterSet>("TDCpars"));

  std::vector<int> v_raddamchannels =
    iConfig.getParameter<vector<int> >("hfraddamchannels");
  
  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());

  if (!buildChannelSet(v_raddamchannels))
    throw cms::Exception("Invalid detID vector");
  else if (channeldetIds_.size()) {
    cout << "Raddam channel list:"<<endl;
    map<int,int>::const_iterator it;
    int i=0;
    for (it=channeldetIds_.begin(); it!=channeldetIds_.end();it++) {
      int denseid=it->first;
      cout << i<<": "<<denseid<<" "<<HcalDetId::detIdFromDenseIndex(denseid);
      if (!(++i%4)) cout << endl;
      else          cout << "\t";
    }
    cout << endl;
  }

  tdcwindowsfile_ =
    iConfig.getUntrackedParameter<string>("tdcwindowsfile");

  rundatesfile_ =
    iConfig.getUntrackedParameter<string>("rundatesfile");

  s2overs1meansfile_ =
    iConfig.getUntrackedParameter<string>("s2overs1meansfile");

  lumiprofilefile_ =
    iConfig.getUntrackedParameter<string>("lumiprofilefile");

  parseTDCwindows(tdcwindowsfile_);
  readRunDates(rundatesfile_);
  readS2overS1means(s2overs1meansfile_);
  readLumiProfile(lumiprofilefile_);

  firstEvent_ = true;
}                                // HFraddamAnalAlgos::HFraddamAnalAlgos

//======================================================================

bool
HFraddamAnalAlgos::buildChannelSet(const std::vector<int>& v_idnumbers)
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

    if (depth==99)   { mindpth=1;   maxdpth=2;  }
    if (ieta==99)    { minieta=29;  maxieta=41; } else
      if (ieta==-99) { minieta=-41; maxieta=-29;}
    if (iphi==99)    { miniphi=1;   maxiphi=71; phiskip=2; }

    for (int depth=mindpth; depth<=maxdpth; depth++) {
      for (int ieta=minieta; ieta<=maxieta; ieta++) {
	for (int iphi=miniphi; iphi<=maxiphi; iphi+=phiskip) {
	  if (!HcalDetId::validDetId(HcalForward,ieta,iphi,depth)) continue;
	  std::pair<int,int> thepair(HcalDetId(HcalForward,ieta,iphi,depth).hashed_index(),-1);
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
}                                  // HFraddamAnalAlgos::buildChannelSet

//======================================================================

void
HFraddamAnalAlgos::parseTDCwindows(const std::string& tdcwindowsfile)
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
}                                 //  HFraddamAnalAlgos::parseTDCwindows

//======================================================================

void
HFraddamAnalAlgos::readRunDates(const std::string& rundatesfile)
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
    int runnum, daynum;
    if (sscanf(instr,"%d %d",&runnum,&daynum) != 2) {
      cerr << "bad format in run dates file " << rundatesfile << ":"<<instr<<endl;
      exit(-1);
    }
    m_rundates_[runnum] = daynum;
  }
  fclose(fp);
}                                    //  HFraddamAnalAlgos::readRunDates

//======================================================================

void
HFraddamAnalAlgos::readS2overS1means(const std::string& meansfile)
{
  FILE *fp = fopen(meansfile.c_str(),"r");
  if (!fp) {
    cerr << "Couldn't find file " << meansfile << endl;
    exit(-1);
  }
  cout << "Reading means file " << meansfile << endl;

  memset(s2overs1means_,0,56*sizeof(float));

  char instr[128];
  while (!feof(fp) && fgets(instr,128,fp)) {
    unsigned index;
    float mean;
    if (instr[0]=='#') continue; //comments are welcome...
    if (sscanf(instr,"%u %f",&index,&mean) != 2) {
      cerr << "bad format in means file " << meansfile << ":"<<instr<<endl;
      exit(-1);
    }
    assert(index < 56);
    s2overs1means_[index] = mean;
  }
  fclose(fp);
}                               //  HFraddamAnalAlgos::readS2overS1Means

//======================================================================

void
HFraddamAnalAlgos::readLumiProfile(const std::string& lumiprofilefile)
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
    double intlumi_ub;
    if (instr[0]=='#') continue; //comments are welcome...
    if (sscanf(instr,"%s , %lf",date,&intlumi_ub) != 2) {
      cerr << "bad format in lumi profile file " << lumiprofilefile << ":"<<instr<<endl;
      exit(-1);
    }
    int dayofyear = date2day(date);
    intlumi_ub /= 1e6; // switch to 1/pb
    intlumiofyear += intlumi_ub;
    m_lumiprofile_[dayofyear] = std::pair<double,double>(intlumi_ub,intlumiofyear);
  }
  fclose(fp);
}                                 //  HFraddamAnalAlgos::readLumiProfile

//======================================================================

void
HFraddamAnalAlgos::lumiForDay(int     dayofyear,
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
HFraddamAnalAlgos::isWithinWindow(void)
{
  return ((TDCphase_ >= TDCwinstart_[ididx_]) &&
	  (TDCphase_ <  TDCwinstart_[ididx_]+TDCwinwidth_[ididx_]));
}

//======================================================================

myAnalCut *
HFraddamAnalAlgos::getCut(const std::string& cutstr)
{
  std::map<std::string,myAnalCut *>::const_iterator it = m_cuts_.find(cutstr);
  if (it == m_cuts_.end()) {
    throw cms::Exception("Cut not found, you numnutz! You changed the name: ") << cutstr << endl;
  }

  return it->second;
}                                         // HFraddamAnalAlgos::getCut

//======================================================================

void
HFraddamAnalAlgos::bookHistos4allCuts(void)
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
	      5201,-0.5, 5200.5, // 72chan/RBX*72RBX = 5184, more than HF or HO
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

  yaxist = "S3/S2";

  st_avgS3overS2vsPhase_ = "h1d_avgS3overS2vsPhase"+mysubdetstr_;
  add1dHisto(st_avgS3overS2vsPhase_,"Average S3/S2 vs Phase, "+mysubdetstr_+"; Phase;"+yaxist,
	     200,1040,1240,v_hpars1dprof);

  st_avgS3overS2vsPhasePlus_   = "h1d_avgS3overS2vsPhase" + mysubdetstr_ + "P";
  add1dHisto(st_avgS3overS2vsPhasePlus_,"Average S3/S2 vs Phase, "+mysubdetstr_+"P; Phase;"+yaxist,
	     200,1040,1240,v_hpars1dprof);

  st_avgS3overS2vsPhaseMinus_   = "h1d_avgS3overS2vsPhase" + mysubdetstr_ + "M";
  add1dHisto(st_avgS3overS2vsPhaseMinus_,"Average S3/S2 vs Phase, "+mysubdetstr_+"M; Phase;"+yaxist,
	     200,1040,1240,v_hpars1dprof);

  yaxist = "S4/S3";

  st_avgS4overS3vsPhase_ = "h1d_avgS4overS3vsPhase"+mysubdetstr_;
  add1dHisto(st_avgS4overS3vsPhase_,"Average S4/S3 vs Phase, "+mysubdetstr_+"; Phase;"+yaxist,
	     200,1040,1240,v_hpars1dprof);

  st_avgS4overS3vsPhasePlus_   = "h1d_avgS4overS3vsPhase" + mysubdetstr_ + "P";
  add1dHisto(st_avgS4overS3vsPhasePlus_,"Average S4/S3 vs Phase, "+mysubdetstr_+"P; Phase;"+yaxist,
	     200,1040,1240,v_hpars1dprof);

  st_avgS4overS3vsPhaseMinus_   = "h1d_avgS4overS3vsPhase" + mysubdetstr_ + "M";
  add1dHisto(st_avgS4overS3vsPhaseMinus_,"Average S4/S3 vs Phase, "+mysubdetstr_+"M; Phase;"+yaxist,
	     200,1040,1240,v_hpars1dprof);

  yaxist = "S2/S1";

  st_avgS2overS1vsPhase_ = "h1d_avgS2overS1vsPhase"+mysubdetstr_;
  add1dHisto(st_avgS2overS1vsPhase_,"Average S2/S1 vs Phase, "+mysubdetstr_+"; Phase;"+yaxist,
	     200,1040,1240,v_hpars1dprof);

  st_avgS2overS1vsPhasePlus_   = "h1d_avgS2overS1vsPhase" + mysubdetstr_ + "P";
  add1dHisto(st_avgS2overS1vsPhasePlus_,"Average S2/S1 vs Phase, "+mysubdetstr_+"P; Phase;"+yaxist,
	     200,1040,1240,v_hpars1dprof);

  st_avgS2overS1vsPhaseMinus_   = "h1d_avgS2overS1vsPhase" + mysubdetstr_ + "M";
  add1dHisto(st_avgS2overS1vsPhaseMinus_,"Average S2/S1 vs Phase, "+mysubdetstr_+"M; Phase;"+yaxist,
	     200,1040,1240,v_hpars1dprof);

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
}                               // HFraddamAnalAlgos::bookHistos4allCuts

//======================================================================

void
HFraddamAnalAlgos::bookPerChanDetail(myAnalHistos *myAH)
{
  assert(myAH);

  // make per channel subfolders
  myAH->mkSubdir<uint32_t>("_digisfCperID");
  myAH->mkSubdir<uint32_t>("_S3overS2perID");
  myAH->mkSubdir<uint32_t>("_S4overS3perID");
  myAH->mkSubdir<uint32_t>("_S2overS1perID");
  myAH->mkSubdir<uint32_t>("_fCampperID");

  // don't need to do any more here, individual channel histos will be
  // booked as they are encountered the first time.

}                                // HFraddamAnalAlgos::bookPerChanDetail

//======================================================================

void
HFraddamAnalAlgos::bookDetailHistos4cut(myAnalCut& cut)
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
}                           // HFraddamAnalAlgos::bookDetailHistos4cut

//==================================================================

void
HFraddamAnalAlgos::fillHistos4cut(myAnalCut& thisCut)
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

  TProfile *profavgS3overS2vsPhase = myAH->get<TProfile>(st_avgS3overS2vsPhase_.c_str());
  profavgS3overS2vsPhase->Fill(TDCphase_,filldigifC[3]/filldigifC[2]);
  
  profavgS3overS2vsPhase = myAH->get<TProfile>((zside>0) ?
					       st_avgS3overS2vsPhasePlus_.c_str():
					       st_avgS3overS2vsPhaseMinus_.c_str());
  profavgS3overS2vsPhase->Fill(TDCphase_,filldigifC[3]/filldigifC[2]);

  TProfile *profavgS4overS3vsPhase = myAH->get<TProfile>(st_avgS4overS3vsPhase_.c_str());
  profavgS4overS3vsPhase->Fill(TDCphase_,filldigifC[4]/filldigifC[3]);

  profavgS4overS3vsPhase = myAH->get<TProfile>((zside>0) ?
					       st_avgS4overS3vsPhasePlus_.c_str():
					       st_avgS4overS3vsPhaseMinus_.c_str());
  profavgS4overS3vsPhase->Fill(TDCphase_,filldigifC[4]/filldigifC[3]);
 

  float s2overs1 = s2overs1_[ididx_];

  TProfile *profavgS2overS1vsPhase = myAH->get<TProfile>(st_avgS2overS1vsPhase_.c_str());
  profavgS2overS1vsPhase->Fill(TDCphase_,s2overs1);
  
  profavgS2overS1vsPhase = myAH->get<TProfile>((zside>0) ?
						    st_avgS2overS1vsPhasePlus_.c_str():
						    st_avgS2overS1vsPhaseMinus_.c_str());
  profavgS2overS1vsPhase->Fill(TDCphase_,s2overs1);


  //cout << detID_ << "\t" << feID_.rbx() << "\t" << feID_.rbx().substr(3,2) << "\t";
  //cout << feID_.rm() << " maps to RBX/RM# " << iRBX << "/" << fRM_ << endl;

  stringstream chname;
  uint32_t     dix;

  dix = detID_.denseIndex(); chname << detID_;

  if (   // (myAH != thisCut.invhistos()) &&
      thisCut.flagSet(st_doPerChannel_)) {

    //========== Pulse shape per channel ==========

    if (1){  // perchOverThresh) {

      perChanHistos *perchFolder = myAH->getAttachedHisto<uint32_t>("_digisfCperID");
      if (!perchFolder)
	throw cms::Exception("Folder _digisfCperID not found for histos "+myAH->name());
      fillDigiPulseHistos(perchFolder,dix,chname.str(),filldigifC);
      
      perchFolder = myAH->getAttachedHisto<uint32_t>("_S3overS2perID");
      if (!perchFolder)
	throw cms::Exception("Folder _S3overS2perID not found for histos "+myAH->name());
      fillS3overS2forchan(perchFolder,dix,chname.str(),filldigifC);
      
      perchFolder = myAH->getAttachedHisto<uint32_t>("_S4overS3perID");
      if (!perchFolder)
	throw cms::Exception("Folder _S4overS3perID not found for histos "+myAH->name());
      fillS4overS3forchan(perchFolder,dix,chname.str(),filldigifC);

      perchFolder = myAH->getAttachedHisto<uint32_t>("_S2overS1perID");
      if (!perchFolder)
	throw cms::Exception("Folder _S2overS1perID not found for histos "+myAH->name());
      fillS2overS1forchan(perchFolder,dix,chname.str(),s2overs1);

      perchFolder = myAH->getAttachedHisto<uint32_t>("_fCampperID");
      if (!perchFolder)
	throw cms::Exception("Folder _fCampperID not found for histos "+myAH->name());
      fillfCamp4chan(perchFolder,dix,chname.str());
    }

  } // if doPerChannel

}                                   // HFraddamAnalAlgos::fillHistos4cut

//======================================================================

void
HFraddamAnalAlgos::fillDigiPulseHistos(perChanHistos     *digiFolder,
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
}                            // HFraddamAnalAlgos::fillDigiPulseHistos

//======================================================================

void
HFraddamAnalAlgos::fillfCamp4chan(perChanHistos     *perchFolder,
				  uint32_t           chkey,
				  const std::string& chname)
{
  perChanHistos::HistoParams_t hpars(int2str(chkey),chname,400,0,20000);
  
  if (!perchFolder) throw cms::Exception("perch folder not found");

  TH1D *h1d = perchFolder->exists(chkey)   ?
    perchFolder->get<TH1D>(chkey) :
    perchFolder->book1d<TH1D>(chkey,hpars,false);

  if (h1d)
    h1d->Fill(fCamplitude_[ididx_]);
}                                   // HFraddamAnalAlgos::fillfCamp4chan

//======================================================================

void
HFraddamAnalAlgos::fillS3overS2forchan(perChanHistos     *perchFolder,
				       uint32_t           chkey,
				       const std::string& chname,
				       const CaloSamples& filldigi)
{
  perChanHistos::HistoParams_t hpars(int2str(chkey),chname,200,1040,1240);
  
  // pulses in fC
  if (!perchFolder) throw cms::Exception("perch folder not found");

  TProfile *pratio = perchFolder->exists(chkey)   ?
    perchFolder->get<TProfile>(chkey) :
    perchFolder->book1d<TProfile>(chkey,hpars,false);

  if (pratio && (filldigi[2]>0)) {
    pratio->Fill(TDCphase_,filldigi[3]/filldigi[2]);
  }
}                          // HFraddamAnalAlgos::fillS3overS2forchan

//======================================================================

void
HFraddamAnalAlgos::fillS4overS3forchan(perChanHistos     *perchFolder,
				       uint32_t           chkey,
				       const std::string& chname,
				       const CaloSamples& filldigi)
{
  perChanHistos::HistoParams_t hpars(int2str(chkey),chname,200,1040,1240);
  
  // pulses in fC
  if (!perchFolder) throw cms::Exception("perch folder not found");

  TProfile *pratio = perchFolder->exists(chkey)   ?
    perchFolder->get<TProfile>(chkey) :
    perchFolder->book1d<TProfile>(chkey,hpars,false);

  if (pratio && (filldigi[3]>0)) {
      pratio->Fill(TDCphase_,filldigi[4]/filldigi[3]);
  }
}                          // HFraddamAnalAlgos::fillS4overS3forchan

//======================================================================

void
HFraddamAnalAlgos::fillS2overS1forchan(perChanHistos     *perchFolder,
				       uint32_t           chkey,
				       const std::string& chname,
				       const float        s2overs1)
{
  perChanHistos::HistoParams_t hpars(int2str(chkey),chname,200,1040,1240);
  
  // pulses in fC
  if (!perchFolder) throw cms::Exception("perch folder not found");

  TProfile *pratio = perchFolder->exists(chkey)   ?
    perchFolder->get<TProfile>(chkey) :
    perchFolder->book1d<TProfile>(chkey,hpars,false);

  if (pratio) {
      pratio->Fill(TDCphase_,s2overs1);
  }
}                          // HFraddamAnalAlgos::fillS2overS1forchan


//==================================================================
// Following routine receives digi in ADC (df)
// and sets member variables digifC_ and digiGeV_ as output.
//
void
HFraddamAnalAlgos::processDigi(const HcalCalibDataFrame& df)
{
  double totamp  = 0;

  if ( (df.id().det() == DetId::Hcal) &&
       (df.id().hcalSubdet()==HcalForward) &&
       (df.id().calibFlavor()==HcalCalibDetId::CalibrationBox) ) {
    //cout << df.id() << endl;
    calibID_ = df.id();
    //feID_  = lmap_->getHcalFrontEndId(calibID_);

    calibdigifC_ = CaloSamples(DetId(df.id().rawId()),df.size());

    double nominal_ped = (df[0].nominal_fC() + df[1].nominal_fC())/2.0;

    //cout << calibID_ << endl;

    int iquad = (calibID_.iphi()-1)/18;
    int zside = calibID_.ieta()/abs(calibID_.ieta()); // calibID_.zside() always returns zero!!

    //cout << "iphi="<<calibID_.iphi()<<", iquad="<<iquad<<", zside="<<zside<<endl;

    for (int i=0; i<df.size(); i++) {
      calibdigifC_[i] = df[i].nominal_fC()-nominal_ped;
      totamp += calibdigifC_[i];
    }
    switch(calibID_.cboxChannel()) {
    case HcalCalibDetId::cbox_MixerHigh: {
      if (zside>0) { HFPmixhi_[iquad] = totamp; }
      else         { HFMmixhi_[iquad] = totamp; }
      break;
    }
    case HcalCalibDetId::cbox_MixerLow: {
      if (zside>0) { HFPmixlo_[iquad] = totamp; }
      else         { HFMmixlo_[iquad] = totamp; }
      break;
    }
    case HcalCalibDetId::cbox_HF_ScintillatorPIN: {
      if (zside>0) { HFPscpin_[iquad] = totamp; }
      else         { HFMscpin_[iquad] = totamp; }
      break;
    }
    default:break;
    }
  }
}                                   // HFraddamAnalAlgos::processDigi

//==================================================================
// Following routine receives digi in ADC (df)
// and sets member variables digifC_ and digiGeV_ as output.
//
void
HFraddamAnalAlgos::processDigi(const HFDataFrame& df)
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

    s2overs1_      [ididx_] = dfC[pkSample_]  /dfC[pkSample_-1];
    s2overs2pluss3_[ididx_] = 1;
  }
  else if (pkSample_ == 0) {
    s0adc_[ididx_] = 0;
    s1adc_[ididx_] = df[pkSample_].adc();
    s2adc_[ididx_] = df[pkSample_+1].adc();

    s2overs1_      [ididx_] = dfC[pkSample_+1]/ dfC[pkSample_];
    s2overs2pluss3_[ididx_] = dfC[pkSample_+1]/(dfC[pkSample_+1]+dfC[pkSample_+2]);
  }
  else if (dfC[pkSample_+1]>dfC[pkSample_-1]) {
    s0adc_[ididx_] = 0;
    s1adc_[ididx_] = df[pkSample_].adc();
    s2adc_[ididx_] = df[pkSample_+1].adc();

    s2overs1_      [ididx_] = dfC[pkSample_+1]/ dfC[pkSample_];
    if (pkSample_ == dfC.size()-2)
      s2overs2pluss3_[ididx_] = 1;
    else
      s2overs2pluss3_[ididx_] = dfC[pkSample_+1]/(dfC[pkSample_+1]+dfC[pkSample_+2]);
  }
  else  {
    if (pkSample_ <= 1)
      s0adc_[ididx_] = 0;
    else
      s0adc_[ididx_] = df[pkSample_-2].adc();
    s1adc_[ididx_] = df[pkSample_-1].adc();
    s2adc_[ididx_] = df[pkSample_].adc();

    s2overs1_      [ididx_] = dfC[pkSample_]/ dfC[pkSample_-1];
      s2overs2pluss3_[ididx_] = dfC[pkSample_]/(dfC[pkSample_]+dfC[pkSample_+1]);
  }
}                                   // HFraddamAnalAlgos::processDigi

//======================================================================

void HFraddamAnalAlgos::processDigis
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

}                                     // HFraddamAnalAlgos::processDigis

//======================================================================

void HFraddamAnalAlgos::processDigis
  (const edm::Handle<HFDigiCollection>& digihandle)
{
  assert(digihandle.isValid());

  myAnalHistos *myAH = m_cuts_["cutNone"]->cuthistos();

  myAH->fill1d<TH1D>(st_digiColSize_,digihandle->size());

  for (unsigned idf = 0; idf < digihandle->size();++idf) {

    const HFDataFrame&  df = (*digihandle)[idf];

    map<int,int>::const_iterator it=channeldetIds_.find(df.id().hashed_index());
    if (it==channeldetIds_.end()) continue;

    ididx_=it->second;

    //cout << it->first << " " << it->second << endl;

    if (df.id().det() == DetId::Hcal) {
      detID_          = HcalDetId(df.id());
      feID_           = lmap_->getHcalFrontEndId(detID_);

      int  absieta = detID_.ietaAbs();
      int    zside = detID_.zside();
      int    depth = detID_.depth();
      int iRMinRBX = feID_.rm();
      //int   ipix = feID_.pixel();
      
      denseID_[ididx_] = it->first;
      ieta_[ididx_]    = detID_.ieta();
      iphi_[ididx_]    = detID_.iphi();
      iRBX_[ididx_]    = atoi(((feID_.rbx()).substr(3,2)).c_str());
      iRM_[ididx_]     = zside * ((iRBX_[ididx_]-1)*4 + iRMinRBX);
      fangle_[ididx_]  = TMath::Pi()*(iphi_[ididx_]-1.)/36.;
      fradius_[ididx_] = hftwrRadii[41-absieta];
    }

    processDigi(df);

    std::map<int,std::pair<int,int> >::const_iterator it2 = 
      m_tdcwins_.find(detID_.denseIndex());

    if (it2 == m_tdcwins_.end()) {
      TDCwinstart_[ididx_] = TDCwinwidth_[ididx_] = 0;
    } else {
      //cout << it2->second.first << " " << it2->second.second << endl;
      TDCwinstart_[ididx_] = it2->second.first;
      TDCwinwidth_[ididx_] = it2->second.second;
    }

    fillHistos4cut(*(m_cuts_["cutNone"]));

    if ((s0adc_[ididx_] < 5)  &&
	(s1adc_[ididx_] > 50) &&
	(s2adc_[ididx_] > 50) &&
	(s2overs2pluss3_[ididx_] > 0.94) &&
	(s2overs2pluss3_[ididx_] < 0.97)   )
      fillHistos4cut(*(m_cuts_["cutamp"]));
      
    if (isWithinWindow()) {
      fillHistos4cut(*(m_cuts_["cutTDCwindow"]));
      fillHistos4cut(*(m_cuts_["cutAll"]));
    }

  } // loop over digis

}                                     // HFraddamAnalAlgos::processDigis

//======================================================================

// ------------ method called to for each event  ------------
void
HFraddamAnalAlgos::process(const myEventData& ed)
{
  bool isLocalHCALrun = ed.hcaltbtrigdata().isValid();
  bool isLaserEvent   = false;
  if (isLocalHCALrun) {
    TDCalgo_->process(ed);
    if (ed.hcaltbtrigdata()->wasLaserTrigger()) isLaserEvent = true;
  }

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
      if(value==hc_HBHEHPD || value==hc_HOHPD || value==hc_HFPMT){ isLaserEvent=true; break;} 
    }
  }   
#endif

  if(!isLaserEvent) return;
  else {
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

  processDigis(ed.hfdigis());

  if (doTree_) tree_->Fill();

  neventsProcessed_++;
}                                       // HFraddamAnalAlgos::process

//======================================================================

void
HFraddamAnalAlgos::beginJob()
{
  neventsProcessed_=0;

  edm::Service<TFileService> fs;

  if (doTree_) {
    tree_ = fs->make<TTree>("mytree","Hcal Results Tree");
    tree_->Branch("lsnum",           &lsnum_,          "lsnum/I");
    tree_->Branch("bxnum",           &bxnum_,          "bxnum/I");
    tree_->Branch("evtnum",          &evtnum_,         "evtnum/I");
    tree_->Branch("runnum",          &runnum_,         "runnum/I");
    tree_->Branch("dayofyear",       &dayofyear_,      "dayofyear/I");
    tree_->Branch("intlumipbofday",  &intlumipbofday_, "intlumipbofday/F");
    tree_->Branch("intlumipbofyear", &intlumipbofyear_,"intlumipbofdyear/F");
    tree_->Branch("TDCphase",        &TDCphase_,       "TDCphase/F");
    tree_->Branch("TDCwinstart",     TDCwinstart_,     "TDCwinstart[56]/I");
    tree_->Branch("TDCwinwidth",     TDCwinwidth_,     "TDCwinwidth[56]/I");
    tree_->Branch("denseID",         denseID_,         "denseID[56]/I");
    tree_->Branch("iRBX",            iRBX_ ,           "iRBX[56]/I");
    tree_->Branch("iRM",             iRM_ ,            "iRM[56]/I");
    tree_->Branch("ieta",            ieta_ ,           "ieta[56]/I");
    tree_->Branch("iphi",            iphi_ ,           "iphi[56]/I");
    tree_->Branch("s0adc",           s0adc_,           "s0adc[56]/I");
    tree_->Branch("s1adc",           s1adc_,           "s1adc[56]/I");
    tree_->Branch("s2adc",           s2adc_,           "s2adc[56]/I");
    tree_->Branch("fCamplitude",     fCamplitude_,     "fCamplitude[56]/F");
    tree_->Branch("s2overs1",        s2overs1_,        "s2overs1[56]/F");
    tree_->Branch("s2overs1means",   s2overs1means_,   "s2overs1means[56]/F");
    tree_->Branch("s2overs2pluss3",  s2overs2pluss3_,  "s2overs2pluss3[56]/F");
    tree_->Branch("HFPmixhi",        HFPmixhi_,        "HFPmixhi[4]/F");
    tree_->Branch("HFPmixlo",        HFPmixlo_,        "HFPmixlo[4]/F");
    tree_->Branch("HFPscpin",        HFPscpin_,        "HFPscpin[4]/F");
    tree_->Branch("HFMmixhi",        HFMmixhi_,        "HFMmixhi[4]/F");
    tree_->Branch("HFMmixlo",        HFMmixlo_,        "HFMmixlo[4]/F");
    tree_->Branch("HFMscpin",        HFMscpin_,        "HFMscpin[4]/F");
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
    "s2overs1meansfile_  = "      << s2overs1meansfile_ << "\n" <<
    "lumiprofilefile_    = "      << lumiprofilefile_   << "\n";

}

//======================================================================

void
HFraddamAnalAlgos::endAnal()
{
  std::cout << neventsProcessed_ << " laser events processed" << std::endl;

  //positivize2dHistos();
}

//======================================================================
