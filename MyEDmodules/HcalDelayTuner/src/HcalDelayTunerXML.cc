
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
// $Id: HcalDelayTunerXML.cc,v 1.1 2009/07/27 15:56:53 dudero Exp $
//
//

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MyEDmodules/HcalDelayTuner/src/HcalDelayTunerXML.hh"

using namespace std;

//======================================================================

void
HcalDelayTunerXML::writeDelayBricks(const DelaySettings& settings)
{
  FILE *xmlout = NULL;

  char xmlfilename[80];
  char timestamp[30];
  std::string oldrbxname;
				    
  getTimeStamp(timestamp);

  const std::string tagstr = "USC55 RBX tuned delays";

  DelaySettings::const_iterator it;
  for (it  = settings.begin();
       it != settings.end();
       it++) {
    const HcalFrontEndId& feID = it->first;
    std::string rbxname        = feID.rbx();
    int   setting              = it->second;

    if (!oldrbxname.size() || oldrbxname.compare(rbxname)) {
      // new RBX, finish off the brick, start a new one.

      // This works because of the way HcalFrontEndId is constructed;
      // as such it is assumed that the map is sorted such that all
      // channels in an RBX are consecutive within it.
      //
      if (xmlout) {
	writeFooter(xmlout);
	fclose(xmlout);
      }
      sprintf(xmlfilename, "del_%s.xml", rbxname.c_str());
      if ((xmlout = fopen(xmlfilename, "w")) == NULL) {
	fprintf(stderr, "Unable to open file %s", xmlfilename);
	exit(EXIT_FAILURE);
      }
      cout << "Writing " << xmlfilename << endl;
      writeHeader(xmlout, timestamp, tagstr.c_str(), rbxname.c_str());
    }

    writeLine(xmlout, feID.rm(), feID.qieCard(), feID.adc(), setting);
    oldrbxname = rbxname;
  }

  if (xmlout) {
    writeFooter(xmlout);
    fclose(xmlout);
  }
}                             // HcalDelayTunerXML::writeDelayBrick4rbx

//======================================================================

void
HcalDelayTunerXML::getTimeStamp(char *timestamp)
{
  time_t t = time(NULL);
  struct tm *tmp = localtime(&t);

  if (tmp == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }

  if (strftime(timestamp, 30, "%F %T", tmp) == 0) {
    fprintf(stderr, "strftime returned 0\n");
    exit(EXIT_FAILURE);
  }
}

//======================================================================

void
HcalDelayTunerXML::writeHeader(FILE *xmlout,
			       const char *timestamp,
			       const char *tagstr,
			       const char *rbxname)
{
  fprintf(xmlout, "<?xml version='1.0'?>\n");
  fprintf(xmlout, "<CFGBrick>\n");
  fprintf(xmlout, "   <Parameter name='RBX' type='string'>%s</Parameter>\n", rbxname);
  fprintf(xmlout, "   <Parameter name='INFOTYPE' type='string'>DELAY</Parameter>\n");
  fprintf(xmlout, "   <Parameter name='CREATIONTAG' type='string'>%s</Parameter>\n", tagstr);
  fprintf(xmlout, "   <Parameter name='CREATIONSTAMP' type='string'>%s</Parameter>\n", timestamp);
}

//======================================================================

void
HcalDelayTunerXML::writeFooter(FILE *xmlout)
{
  fprintf(xmlout, "</CFGBrick>\n");
}

//======================================================================

void
HcalDelayTunerXML::writeLine  (FILE *xmlout,
				const int& rm, const int& card, const int& chan,
				const int& setting)
{
  fprintf (xmlout,
	   "   <Data elements=\"1\" encoding=\"dec\" rm=\"%d\" card=\"%d\" qie=\"%d\">%d</Data>\n",
	   rm, card, chan, setting);
}
