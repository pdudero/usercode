#ifndef _HCALDELAYTUNERXML_H
#define _HCALDELAYTUNERXML_H

// -*- C++ -*-
//
// Package:    HcalDelayTunerXML
// Class:      HcalDelayTunerXML
// 
/**\class HcalDelayTunerXML HcalDelayTunerXML.cc MyEDmodules/HcalDelayTuner/src/HcalDelayTunerXML.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalDelayTunerXML.hh,v 1.1 2009/11/09 00:57:58 dudero Exp $
//
//

#include <string>
#include <map>
#include <stdlib.h>
#include "DataFormats/HcalDetId/interface/HcalFrontEndId.h"
#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerInput.hh" // "DelaySettings"

//
// class declaration
//

class HcalDelayTunerXML {
public:
  explicit HcalDelayTunerXML() {}
  ~HcalDelayTunerXML() {}
  void getTimeStamp        (char *timestamp);
  void writeDelayBricks    (const DelaySettings& settings);
private:
  void writeHeader    (FILE *xmlout, const char *timestamp, const char *tagstr, const char *rbxname);
  void writeFooter    (FILE *xmlout);
  void writeLine      (FILE *xmlout,
		       const int& rm, const int& card, const int& chan,
		       const int& setting);
};

#endif // _HCALDELAYTUNERXML_H
