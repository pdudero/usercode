#ifndef _LASERDELAYTUNERXML_H
#define _LASERDELAYTUNERXML_H

// -*- C++ -*-
//
// Package:    LaserDelayTunerXML
// Class:      LaserDelayTunerXML
// 
/**\class LaserDelayTunerXML LaserDelayTunerXML.cc MyEDmodules/LaserDelayTuner/src/LaserDelayTunerXML.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: LaserDelayTunerXML.hh,v 1.5 2009/05/21 09:52:41 dudero Exp $
//
//

#include <string>
#include <map>
#include <stdlib.h>

//
// class declaration
//

class LaserDelayTunerXML {
public:
  explicit LaserDelayTunerXML() {}
  ~LaserDelayTunerXML() {}
  void getTimeStamp           (char *timestamp);
  void writePhiDelayBrick4rbx (const std::string& subdet,
			       int  signedrbxnum,
			       char *timestamp,
			       char *tagstr,
			       const std::map<int,int>& m_settingsPerRM);

private:
  void writeHeader    (FILE *xmlout, char *timestamp, char *tagstr, char *rbxname);
  void writeFooter    (FILE *xmlout);
  void writeLine      (FILE *xmlout,
		       const int& rm, const int& card, const int& chan,
		       const int& setting);
};

#endif // _LASERDELAYTUNERXML_H
