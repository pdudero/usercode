
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
// $Id: LaserDelayTunerXML.cc,v 1.5 2009/05/21 09:52:41 dudero Exp $
//
//

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MyEDmodules/LaserDelayTuner/src/LaserDelayTunerXML.hh"

using namespace std;

//======================================================================

void
LaserDelayTunerXML::writePhiDelayBrick4rbx(const string& subdetstr,
					   int    signedrbxnum,
					   char  *timestamp,
					   char  *tagstr,
					   const map<int,int>& m_settingsPerRM)
{
  FILE *xmlout;

  char xmlfilename[80];
  char rbxname[20];

  sprintf(rbxname, "%s%c%02d",
	  subdetstr.c_str(),
	  (signedrbxnum > 0) ? 'P' : 'M',
	  abs(signedrbxnum));

  sprintf(xmlfilename, "del_%s.xml", rbxname);

  if ((xmlout = fopen(xmlfilename, "w")) == NULL) {
    fprintf(stderr, "Unable to open file %s", xmlfilename);
    exit(EXIT_FAILURE);
  }

  writeHeader(xmlout, timestamp, tagstr, rbxname);

  map<int,int>::const_iterator it;
  for (it  = m_settingsPerRM.begin();
       it != m_settingsPerRM.end();
       it++) {
    const int iRM     = it->first;
    const int setting = it->second;
    for (int icard=1; icard<=3; icard++)
      for (int qie=0; qie<6; qie++)
	writeLine(xmlout, iRM, icard, qie, setting);
  }

  writeFooter(xmlout);
  fclose(xmlout);
}                          // LaserDelayTunerXML::writePhiDelayBrick4rbx

//======================================================================

void
LaserDelayTunerXML::getTimeStamp(char *timestamp)
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
LaserDelayTunerXML::writeHeader(FILE *xmlout,
				char *timestamp,
				char *tagstr,
				char *rbxname)
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
LaserDelayTunerXML::writeFooter(FILE *xmlout)
{
  fprintf(xmlout, "</CFGBrick>\n");
}

//======================================================================

void
LaserDelayTunerXML::writeLine  (FILE *xmlout,
				const int& rm, const int& card, const int& chan,
				const int& setting)
{
  fprintf (xmlout,
	   "   <Data elements=\"1\" encoding=\"dec\" rm=\"%d\" card=\"%d\" qie=\"%d\">%d</Data>\n",
	   rm, card, chan, setting);
}
