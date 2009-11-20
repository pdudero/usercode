#ifndef  _MYEDMODULESHCALDELAYTUNERALGOS
#define  _MYEDMODULESHCALDELAYTUNERALGOS

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
// $Id: HcalDelayTunerAlgos.hh,v 1.1 2009/11/09 00:58:33 dudero Exp $
//
//


// system include files
#include <string>
#include <vector>

// user include files
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"
#include "DataFormats/Common/interface/SortedCollection.h"
#include "DataFormats/HcalDetId/interface/HcalFrontEndId.h"

#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalCut.hh"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"

class HcalDelayTunerXML;

//
// class declaration
//
typedef std::map<HcalFrontEndId,int>   DelaySettings;
typedef std::map<HcalFrontEndId,float> TimesPerFEchan;
typedef std::map<HcalDetId,float>      TimesPerDetId;

class HcalDelayTunerAlgos {
public:
  HcalDelayTunerAlgos(const edm::ParameterSet&);
  virtual ~HcalDelayTunerAlgos() {}
  virtual void process (const myEventData& ed) {}
  virtual void beginJob(const edm::EventSetup&) {}
  virtual void endJob(const TimesPerFEchan& chtimes,
		      const DelaySettings& oldsettings);

protected:
  void   shiftBySubdet       (const std::string subdet);
  int    detSetting4Channel  (const HcalFrontEndId& feID,
			      float  hittime,
			      float  mintime,
			      int oldsetting);
  void   determineSettings   (const TimesPerFEchan& hittimes,
			      const DelaySettings& oldsettings);

  HcalSubdetector              mysubdet_;
  std::string                  mysubdetstr_;
  HcalDelayTunerXML           *xml_;
  DelaySettings                oldsettings_;
  DelaySettings                newsettings_;

private:

  // ----------member data ---------------------------

  bool writeBricks_;
  bool firstEvent_;
  bool clipAtLimits_;
  int  nchanMissingOldSettings_;
  int  nchanMissingData_;
};

#endif // _MYEDMODULESHCALDELAYTUNERALGOS
