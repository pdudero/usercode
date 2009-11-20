
// -*- C++ -*-
//
// Package:    HcalDelayTunerAlgos
// Class:      HcalDelayTunerAlgos
// 
/**\class HcalDelayTunerAlgos HcalDelayTunerAlgos.cc MyEDmodules/HcalDelayTunerAlgos/src/HcalDelayTunerAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalDelayTunerAlgos.cc,v 1.1 2009/11/09 00:57:58 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>
#include <math.h>
#include <limits.h>

// user include files
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"

#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"

#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerXML.hh"

#include "MyEDmodules/HcalDelayTuner/interface/HcalDelayTunerAlgos.hh"


#include "TH1D.h"
#include "TH2D.h"
#include "TF2.h"
#include "TProfile.h"
#include "TProfile2D.h"

inline string int2str(int i) {
  std::ostringstream ss;
  ss << i;
  return ss.str();
}

//
// constructors and destructor
//
HcalDelayTunerAlgos::HcalDelayTunerAlgos(const edm::ParameterSet& iConfig)
{
  writeBricks_ = iConfig.getUntrackedParameter<bool>("writeBricks",false);
  mysubdetstr_ = iConfig.getUntrackedParameter<std::string>("subdet");
  clipAtLimits_= iConfig.getUntrackedParameter<bool>("clipSettingsAtLimits",false);

  if (!mysubdetstr_.compare("HB")) mysubdet_ = HcalBarrel;  else
  if (!mysubdetstr_.compare("HE")) mysubdet_ = HcalEndcap;  else
  if (!mysubdetstr_.compare("HO")) mysubdet_ = HcalOuter;   else
  if (!mysubdetstr_.compare("HF")) mysubdet_ = HcalForward; else {
    mysubdet_ = HcalOther;
    edm::LogWarning("Warning: subdetector set to 'other', ") << mysubdetstr_;
  }

  xml_ = new HcalDelayTunerXML();
}

//======================================================================

static const int maxlimit=24;

int
HcalDelayTunerAlgos::detSetting4Channel(const HcalFrontEndId& feID,
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
HcalDelayTunerAlgos::shiftBySubdet(const std::string subdet)
{
  // 1. Iterate through the new settings to see what the maxima and minima are.

  // Find the max/min setting across all channels
  int minsetting    = INT_MAX;
  int maxsetting    = INT_MIN;

  DelaySettings::iterator itnew;
  for (itnew =newsettings_.begin();
       itnew!=newsettings_.end(); itnew++) {
    HcalFrontEndId feID = itnew->first;
    if (feID.rbx().find(subdet) == std::string::npos) continue;

    int setting = itnew->second;
    if (setting < minsetting) minsetting = setting;
    if (setting > maxsetting) maxsetting = setting;
  }

  cout << "For subdet    " << subdet<<":"<< endl;
  cout << "Max setting = " << maxsetting << endl;
  cout << "Min setting = " << minsetting << endl;
  int glshift = std::min((maxsetting-maxlimit),minsetting);

  cout<< "Shifting by " << glshift << "ns" << endl;
  cout<< "FrontEnd ID\t\t\tNew (shifted) Setting" << endl;
  for (itnew  = newsettings_.begin();
       itnew != newsettings_.end(); itnew++) {

    HcalFrontEndId feID = itnew->first;
    if (feID.rbx().find(subdet) == std::string::npos) continue;

    int setting = itnew->second - glshift;
    cout << itnew->first << '\t' <<setw(2)<<setting;
    if (setting > maxlimit) {
      if (clipAtLimits_) {
	cout << " <-- clipped at " << maxlimit << endl;
	setting = maxlimit;
      } else
	cout << " <--!!! " << endl;
    }
    else if (setting < 0) {
      if (clipAtLimits_) {
	cout << " <-- clipped at 0." << endl;
	setting = 0;
      } else {
	cout << " <--!!! " << endl;
      }
    } else
      cout << endl;

    itnew->second = setting;
  }
}                                  // HcalDelayTunerAlgos::shiftBySubdet

//======================================================================

void
HcalDelayTunerAlgos::determineSettings(const TimesPerFEchan& timecors,
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

  cout<< "\t\t\t\t\tChannel\tOld\tNew" << endl;
  cout<< "FrontEnd ID\t\t\t\tTime (ns)\tSetting\tSetting" << endl;
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
	cout << feID << '\t' << fixed << setprecision(1) << timecor<<'\t';
	cout << setw(2) << "XXX Old setting missing - skip XXX" << endl;
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

      cout << itold->first << "\t--\t" << setw(2);
      cout << itold->second << '\t' << itold->second << endl;

#endif /* HACK FOR HO (FIXME!): PEND INSERTION OF OLD SETTINGS FOR
	  CHANNELS WITH NO DATA(HO1M04,HO1M10, SiPM BOXES)
	  UNTIL AFTER WE'VE SHIFTED AROUND THE NEW ONES */

      cout << itold->first << "\t--\tWAIT FOR IT..." << endl;

      itold++;
      nchanMissingData_++;
    }
    if (itcor->first == itold->first) {
      HcalFrontEndId feID = itcor->first;

      /* >>>>>>>>>>>>>>>>>>>> NORMAL CASE <<<<<<<<<<<<<<<<<<<< */

      int setting = detSetting4Channel(feID,itcor->second,
				       mintime, itold->second);

      cout << feID << '\t' << fixed << setprecision(1) << itcor->second<<'\t';
      cout << setw(2) << itold->second << '\t' << setting;

      if ((setting < 0) || (setting > maxlimit)) {
	cout << " <--!!! " << endl;
      } else
	cout << endl;

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


  switch (mysubdet_) {
  case HcalBarrel:  shiftBySubdet("HB"); break; // unsatisfactory...we
  case HcalEndcap:  shiftBySubdet("HE"); break; // really want these 2 together
  case HcalForward: shiftBySubdet("HF"); break;
  case HcalOuter:   shiftBySubdet("HO0");
                    shiftBySubdet("HO1");
		    shiftBySubdet("HO2");break;
  default: break;
  }

  for (itold =oldsettings.begin();
       itold!=oldsettings.end(); itold++) {
    // insertion will fail if the new setting already exists in its place
    lastins = newsettings_.insert(lastins,*itold);
  }

}                              // HcalDelayTunerAlgos::determineSettings

//======================================================================

void
HcalDelayTunerAlgos::endJob(const TimesPerFEchan& hittimes,
			    const DelaySettings& oldsettings)
{
  determineSettings(hittimes,oldsettings);
  if (writeBricks_)
    xml_->writeDelayBricks(newsettings_);
}

//======================================================================
