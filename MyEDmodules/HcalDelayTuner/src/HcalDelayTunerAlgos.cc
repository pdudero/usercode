
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
// $Id: HcalDelayTunerAlgos.cc,v 1.1 2009/07/27 15:56:53 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>
#include <math.h>

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
  // cut string vector initialized in order
  writeBricks_ = iConfig.getUntrackedParameter<bool>("writeBricks",false);
  mysubdetstr_ = iConfig.getUntrackedParameter<std::string>("subdet");

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

static const int maxsetting=24;

int
HcalDelayTunerAlgos::detSetting4Channel(const HcalFrontEndId& feID,
					double hittime,
					double mintime,
					int oldsetting)
{
  // no rounding.
  int ideltatime = (int)(floor(hittime-mintime));

  int setting = oldsetting + ideltatime;

  if (setting > maxsetting) {
    edm::LogWarning("detSetting4Channel") <<
      "Setting is pegged to maximum for channel " << feID;
    setting = maxsetting;
  }

  return setting;
}

//======================================================================

void
HcalDelayTunerAlgos::determineSettings(const ChannelTimes& hittimes,
				       const DelaySettings& oldsettings)
{
  bool fromscratch = false;

  if (!hittimes.size()) {
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
  double mintime = 1e99;
  std::map<HcalFrontEndId,double>::const_iterator ittime;
  for (ittime = hittimes.begin(); ittime != hittimes.end(); ittime++)
    if (ittime->second < mintime) mintime = ittime->second;

  nchanMissingOldSettings_ = 0;
  nchanMissingData_ = 0;

  newsettings_.clear();
  DelaySettings::const_iterator itold;
  DelaySettings::iterator       lastins = newsettings_.begin();
  cout<< "FrontEnd ID\tChannelTime (ns)\tOld Setting\tNew Setting" << endl;
  for (ittime   = hittimes.begin(),   itold  = oldsettings.begin();
       (ittime != hittimes.end()) || (itold != oldsettings.end());
       ) {
    // first check if the iterators are tracking - any channel present
    // in one map and missing in the other generates an error message
    //
    while( ((itold  == oldsettings.end()) &&
	    (ittime != hittimes.end())) ||
	   (ittime->first < itold->first)    ) {
      if (!fromscratch) {
	edm::LogWarning("determineSettings") <<
	  "No old setting found for channel " << itold->first << ", skipping";
	nchanMissingOldSettings_++;
      } else {
	int setting = detSetting4Channel(ittime->first,ittime->second, mintime, 0);
	lastins = newsettings_.insert(lastins,std::pair<HcalFrontEndId,int>(itold->first,setting));
      }
      ittime++;
    }
    while( ((ittime == hittimes.end()) &&
	    (itold  != oldsettings.end())) ||
	   (itold->first < ittime->first)    ) {
      edm::LogWarning("determineSettings") <<
	"No data found for channel " << itold->first << ", using old setting";
      lastins = newsettings_.insert(lastins, *itold);
      itold++;
      nchanMissingData_++;
    }
    if (ittime->first == itold->first) {
      int setting = detSetting4Channel(ittime->first,ittime->second, mintime, itold->second);

      cout << ittime->first << '\t' << fixed << setprecision(1) << ittime->second<<'\t';
      cout << setw(2) << itold->second << '\t' << setting << endl;

      lastins = newsettings_.insert(lastins,std::pair<HcalFrontEndId,int>(itold->first,setting));
      ittime++; itold++;
    }
  }
}                              // HcalDelayTunerAlgos::determineSettings

//======================================================================

void
HcalDelayTunerAlgos::endJob(const ChannelTimes& hittimes,
			    const DelaySettings& oldsettings)
{
  determineSettings(hittimes,oldsettings);
  xml_->writeDelayBricks(newsettings_);
}

//======================================================================
