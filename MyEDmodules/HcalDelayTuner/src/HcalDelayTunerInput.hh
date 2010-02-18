#ifndef  _MYEDMODULESHCALDELAYTUNERINPUT
#define  _MYEDMODULESHCALDELAYTUNERINPUT

// -*- C++ -*-
//
// Package:    HcalDelayTunerInput
// Class:      HcalDelayTunerInput
// 
/**\class HcalDelayTunerInput HcalDelayTunerInput.cc MyEDmodules/HcalDelayTunerInput/src/HcalDelayTunerInput.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalDelayTunerInput.hh,v 1.3 2009/12/04 14:40:28 dudero Exp $
//
//


// system include files
#include <map>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalDetId/interface/HcalFrontEndId.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"


typedef std::map<HcalFrontEndId,int>   DelaySettings;
typedef std::map<HcalFrontEndId,float> TimesPerFEchan;
typedef std::map<HcalDetId,float>      TimesPerDetId;

//
// class declaration
//

class HcalDelayTunerInput {
public:

  struct TTCrxSettings {
    int fine;
    int coarse1;
    int coarse2;
  };

  // ParameterSet only if you identify a parameter you think
  // should be configurable from the py file.
  //
  explicit HcalDelayTunerInput(const edm::ParameterSet&);
  ~HcalDelayTunerInput() {}

  // Parses a list of xml files and returns a map of delay settings,
  // one for each channel
  //
  void getSamplingDelays  (DelaySettings& delays);

  // Parses a list of xml files and returns a map of delay settings,
  // one for each channel
  //
  void getTimeCorrections (TimesPerFEchan& timecorrs);
  void getTimeCorrections (TimesPerDetId&  timecorrs);

  // Parses a single xml file that has one phase setting per RBX
  // and returns a map of these settings, one per RBX name.
  //
  void getTTCrxPhases      (const std::string& xmlFileNames,
			    std::map<std::string,TTCrxSettings>& phases);

private:

  // Add private methods and data members as needed
  std::vector<std::string> xmlfileNames_;
  std::vector<std::string> timecorrFileNames_;
  std::string              timecorrScanFmt_;
  std::string              settingScanFmt_;
					
};

#endif // _MYEDMODULESHCALDELAYTUNERINPUT
