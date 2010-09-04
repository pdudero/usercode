// -*- C++ -*-
//
// Package:    HcalDelayTuner
// Class:      HcalDelayTuner
// 
/**\class HcalDelayTuner HcalDelayTuner.cc MyEDmodules/HcalDelayTuner/src/HcalDelayTuner.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalDelayTuner.cc,v 1.2 2010/03/01 06:52:33 dudero Exp $
//
//


// system include files
#include <limits.h>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerInput.hh"
#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerXML.hh"

//
// class declaration
//

class HcalDelayTuner : public edm::EDAnalyzer {
public:
  explicit HcalDelayTuner(const edm::ParameterSet&);
  ~HcalDelayTuner();

private:
  //virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  void   shiftBySubdet       (const std::string subdet);
  int    detSetting4Channel  (const HcalFrontEndId& feID,
			      float  hittime,
			      float  mintime,
			      int oldsetting);
  void   determineSettings   (const TimesPerFEchan& hittimes,
			      const DelaySettings& oldsettings);

  // ----------member data ---------------------------

  HcalDelayTunerInput   *inp_;
  HcalSubdetector        mysubdet_;
  std::string            mysubdetstr_;
  HcalDelayTunerXML     *xml_;
  DelaySettings          oldsettings_;
  DelaySettings          newsettings_;

  bool writeBricks_;
  bool firstEvent_;
  bool clipAtLimits_;
  int  nchanMissingOldSettings_;
  int  nchanMissingData_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//
//======================================================================

//
// constructors and destructor
//
HcalDelayTuner::HcalDelayTuner(const edm::ParameterSet& iConfig)
{
  std::cerr << "-=-=-=-=-=HcalDelayTuner Constructor=-=-=-=-=-" << std::endl;

  writeBricks_ = iConfig.getUntrackedParameter<bool>("writeBricks",false);
  mysubdetstr_ = iConfig.getUntrackedParameter<std::string>("subdet");
  clipAtLimits_= iConfig.getUntrackedParameter<bool>("clipSettingsAtLimits",false);

  inp_ = new HcalDelayTunerInput(iConfig);
  xml_ = new HcalDelayTunerXML();
  if (writeBricks_ && !clipAtLimits_) {
    for (unsigned i=0; i<10; i++) {
      std::cout << "BOY, YOU IN A HEAP'A'TROUBLE!" << std::endl;
    }
  }
}

HcalDelayTuner::~HcalDelayTuner() {
  std::cerr << "-=-=-=-=-=HcalDelayTuner Destructor=-=-=-=-=-" << std::endl;
}

//======================================================================

//
// member functions
//
//======================================================================

static const int maxlimit=26;

int
HcalDelayTuner::detSetting4Channel(const HcalFrontEndId& feID,
					float hittime,
					float mintime,
					int oldsetting)
{
  // no rounding.
  // int ideltatime = (int)(floor(hittime-mintime));
  int ideltatime = (int)round(hittime);

  int setting = oldsetting + ideltatime;

  return setting;
}

//======================================================================

void
HcalDelayTuner::shiftBySubdet(const std::string subdet)
{
  // 1. Iterate through the new settings to see what the maxima and minima are.

  // Find the max/min setting across all channels
  int minsetting    = INT_MAX;
  int maxsetting    = INT_MIN;

  DelaySettings::iterator itnew;
  for (itnew =newsettings_.begin();
       itnew!=newsettings_.end(); itnew++) {
    HcalFrontEndId feID = itnew->first;

    // allows use of "HBHE" as a subdet:
    if (subdet.find(feID.rbx().substr(0,2)) == std::string::npos) continue;

    int setting = itnew->second;
    if (setting < minsetting) minsetting = setting;
    if (setting > maxsetting) maxsetting = setting;
  }

  std::cout << "For subdet    " << subdet<<":"<< std::endl;
  std::cout << "Max setting = " << maxsetting << std::endl;
  std::cout << "Min setting = " << minsetting << std::endl;
  int glshift = std::min((maxsetting-maxlimit),minsetting);

  std::cout<< "Shifting by " << glshift << "ns" << std::endl;
  std::cout<< "FrontEnd ID\t\t\tNew (shifted) Setting" << std::endl;
  for (itnew  = newsettings_.begin();
       itnew != newsettings_.end(); itnew++) {

    HcalFrontEndId feID = itnew->first;
    if (feID.rbx().find(subdet) == std::string::npos) continue;

    int setting = itnew->second - glshift;
    std::cout << itnew->first << '\t' <<std::setw(2)<<setting;
    if (setting > maxlimit) {
      if (clipAtLimits_) {
	std::cout << " <-- clipped at " << maxlimit << std::endl;
	setting = maxlimit;
      } else
	std::cout << " <--!!! " << std::endl;
    }
    else if (setting < 0) {
      if (clipAtLimits_) {
	std::cout << " <-- clipped at 0." << std::endl;
	setting = 0;
      } else {
	std::cout << " <--!!! " << std::endl;
      }
    } else
      std::cout << std::endl;

    itnew->second = setting;
  }
}                                       // HcalDelayTuner::shiftBySubdet

//======================================================================

void
HcalDelayTuner::determineSettings(const TimesPerFEchan& timecors,
				       const DelaySettings& oldsettings)
{
  bool fromscratch = false;

  if (!timecors.size()) {
    edm::LogWarning("determineSettings") <<
      "No new data received! You must want me to XMLize the old settings";
    newsettings_ = oldsettings;
    return;
  }

  if (!oldsettings.size()) {
    edm::LogWarning("determineSettings") <<
      "No old settings received. You must want me to cook some up from scratch";
    fromscratch=true;
  }

  // Find the minimum time across all channels
  float mintime = 1e99;
  TimesPerFEchan::const_iterator itcor;
  for (itcor = timecors.begin(); itcor != timecors.end(); itcor++)
    if (itcor->second < mintime) mintime = itcor->second;

  nchanMissingOldSettings_ = 0;
  nchanMissingData_ = 0;

  newsettings_.clear();
  DelaySettings::const_iterator itold;
  DelaySettings::iterator  lastins = newsettings_.begin();

  /******************************************************
   * First perform the tune, handling partial data sets
   * either in the data or in the old settings, and ignoring
   * the range limits
   ******************************************************/

  std::cout<< "\t\t\t\t\tChannel\tOld\tNew" << std::endl;
  std::cout<< "FrontEnd ID\t\t\t\tTime (ns)\tSetting\tSetting" << std::endl;
  for (itcor   = timecors.begin(),   itold  = oldsettings.begin();
       (itcor != timecors.end()) || (itold != oldsettings.end());
       ) {

    // handle gaps in either dataset and notate
    //
    while((itcor != timecors.end()) &&           // gap in the old settings
	  ((itold == oldsettings.end()) ||
	   (itcor->first < itold->first)  )  ) {
      HcalFrontEndId feID = itcor->first;
      float       timecor = itcor->second;
      if (!fromscratch) {
	std::cout << feID << '\t' << std::fixed << std::setprecision(1) << timecor<<'\t';
	std::cout << std::setw(2) << "XXX Old setting missing - skip XXX" << std::endl;
	nchanMissingOldSettings_++;
      } else {
	int setting = detSetting4Channel(itcor->first,itcor->second, mintime, 0);
	lastins =
	  newsettings_.insert(lastins,
			      std::pair<HcalFrontEndId,int>(itold->first,setting));
      }
      itcor++;
    }
    while( (itold  != oldsettings.end()) &&       // gap in the time corrections
	   ((itcor == timecors.end()) ||
	    (itold->first < itcor->first) )   ) {
      HcalFrontEndId feID = itold->first;
#if 0

      lastins = newsettings_.insert(lastins, *itold);

      std::cout << itold->first << "\t--\t" << std::setw(2);
      std::cout << itold->second << '\t' << itold->second << std::endl;

#endif /* HACK FOR HO (FIXME!): PEND INSERTION OF OLD SETTINGS FOR
	  CHANNELS WITH NO DATA(HO1M04,HO1M10, SiPM BOXES)
	  UNTIL AFTER WE'VE SHIFTED AROUND THE NEW ONES */

      std::cout << itold->first << "\t--\tWAIT FOR IT..." << std::endl;

      itold++;
      nchanMissingData_++;
    }
    if (itcor->first == itold->first) {
      HcalFrontEndId feID = itcor->first;

      /* >>>>>>>>>>>>>>>>>>>> NORMAL CASE <<<<<<<<<<<<<<<<<<<< */

      int setting = detSetting4Channel(feID,itcor->second,
				       mintime, itold->second);

      std::cout << feID << '\t' << std::fixed << std::setprecision(1) << itcor->second<<'\t';
      std::cout << std::setw(2) << itold->second << '\t' << setting;

      if ((setting < 0) || (setting > maxlimit)) {
	std::cout << " <--!!! " << std::endl;
      } else
	std::cout << std::endl;

      lastins = newsettings_.insert(lastins,std::pair<HcalFrontEndId,int>(itold->first,setting));

      itcor++; itold++;
    }
  }  // loop iterating over old settings / time corrections

  /*****************************************************************
   * CHECK TO SEE IF WE HAVE TO SHIFT THE WHOLE SET TO MAKE IT FIT
   *****************************************************************/

  // For HO the shift is split up into 3 sets of rings: 0, pm1 and pm2.
  // HO has a timing spread of 37ns, well over a time sample.
  //   so we split it up and let the pieces float.


  if (mysubdetstr_ == "HO") {
    shiftBySubdet("HO0");
    shiftBySubdet("HO1");
    shiftBySubdet("HO2");
  } else {
    shiftBySubdet(mysubdetstr_);
  }

  for (itold =oldsettings.begin();
       itold!=oldsettings.end(); itold++) {
    // insertion will fail if the new setting already exists in its place
    lastins = newsettings_.insert(lastins,*itold);
  }
}                                   // HcalDelayTuner::determineSettings

//======================================================================

// ------------ method called to for each event  ------------
void
HcalDelayTuner::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalDelayTuner::endJob() {
  DelaySettings oldsettings;
  TimesPerFEchan chtimes;
  inp_->getTimeCorrections(chtimes);
  inp_->getSamplingDelays(oldsettings);
  determineSettings(chtimes,oldsettings);
  if (writeBricks_)
    xml_->writeDelayBricks(newsettings_);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalDelayTuner);
