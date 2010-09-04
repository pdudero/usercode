
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
// $Id: HcalDelayTunerInput.cc,v 1.6 2010/04/12 10:27:52 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "HcalDelayTunerInput.hh"

using namespace std;


// ======================================================================

HcalDelayTunerInput::HcalDelayTunerInput(const edm::ParameterSet& iConfig)
{
  std::vector<std::string> empty;

  xmlfileNames_        = iConfig.getUntrackedParameter<vector<string> >("fileNames",empty);
  overwriteDuplicates_ = iConfig.getUntrackedParameter<bool>("overwriteDuplicates",false);
  timecorrFileNames_   = iConfig.getUntrackedParameter<vector<string> >("timecorrFilenames",empty);
  timecorrScanFmt_     = iConfig.getUntrackedParameter<string>("timecorrScanFmt","");
  settingScanFmt_      = iConfig.getUntrackedParameter<string>("settingScanFmt","");

  cout << "Opening " << xmlfileNames_.size() << " file(s) for reading: " << endl;
  for (size_t i=0; i<xmlfileNames_.size(); i++)
    cout << xmlfileNames_[i] << endl;
}

// ======================================================================

// Parses a list of xml files and returns a map of delay settings,
// one for each channel
//
void
HcalDelayTunerInput::getSamplingDelays (DelaySettings& delays)
{
  if (!xmlfileNames_.size()) {
    edm::LogWarning("Called getSamplingDelays without defining filename vector!");
    return; // nothing to do...
  }

  if (!xmlfileNames_[0].size()) {
    edm::LogWarning("Filename 0 is empty, aborting");
    return; // nothing to do...
  }

  int rm;  // 1,2,3,4 (max 3 for HF)
  int card;  // 1,2,3 (max 4 for HF)
  int qie;   // 0,1,2,3,4,5
  int setting; // 0-24

  char rbx[40];
  char line[128];

  delays.clear();

  if ((xmlfileNames_.size() > 1) &&
      overwriteDuplicates_) {
    cout << "************************************************************************" << endl;
    cout << "WARNING: overwriteDuplicates flag is set with multiple files to read in." << endl;
    cout << "************************************************************************" << endl;
  }

  for (size_t i=0; i<xmlfileNames_.size(); i++) {
    // Here the files are really just white-space-separated columns of
    // RBX RM card qie delay settings
    //
    FILE *fp = fopen(xmlfileNames_[i].c_str(),"r");
    if (!fp) {
      throw cms::Exception("File doesn't exist, "+xmlfileNames_[i]);
    }
    
    while (!feof(fp) && fgets(line,128,fp)) {
      if (line[0]=='#') continue;
      int num = sscanf(line,settingScanFmt_.c_str(),
		       rbx, &rm, &card, &qie, &setting);
      if (num != 5) 
	throw cms::Exception("ScanFmt string doesn't match input")
	  << line << settingScanFmt_ << endl;

      if (rbx[0] == 'Z') continue; // ZDC
      HcalFrontEndId feID(std::string(rbx),rm,0,1,0,card,qie);
      std::pair<DelaySettings::iterator,bool> ret = 
	delays.insert(std::pair<HcalFrontEndId,int>(feID,setting));

      if (!ret.second) {
	if (overwriteDuplicates_) {
	  ret.first->second = setting;
	} else {
	  stringstream str;
	  str << "Duplicate setting found for feID " << feID;
	  str << ", current value is " << ret.first->second;
	  str << ", set 'overwriteDuplicates=True' to overwrite";
	  throw cms::Exception(str.str());
	}
      }
      cout << "read in " << feID << " old setting = " << setting << endl;
    }

    cout << "Read in sampling delay settings from " << xmlfileNames_[i] << endl;
  }

#if 0
  // Dump for test.
  cout<< "FrontEnd ID\t\tSetting" << endl;
  DelaySettings::const_iterator it;
  for (it=delays.begin(); it!=delays.end(); it++) {
    cout << it->first << '\t' << it->second << endl;
  }
#endif
}                               // HcalDelayTunerInput::getSamplingDelays

// ======================================================================

// Parses a list of xml files and returns a map of delay settings,
// one for each channel
//
void
HcalDelayTunerInput::getSamplingDelays (SettingsPerDetId& delays)
{
  if (!xmlfileNames_.size()) {
    edm::LogWarning("Called getSamplingDelays without defining filename vector!");
    return; // nothing to do...
  }

  if (!xmlfileNames_[0].size()) {
    edm::LogWarning("Filename 0 is empty, aborting");
    return; // nothing to do...
  }

  int ieta;
  int iphi;
  int depth;
  int setting; // 0-24

  char line[128];

  delays.clear();

  // Here the files are really just white-space-separated columns of
  // RBX RM card qie delay settings
  //
  FILE *fp = fopen(xmlfileNames_[0].c_str(),"r");
  if (!fp) {
    throw cms::Exception("File doesn't exist") << xmlfileNames_[0];
  }

  while (!feof(fp) && fgets(line,128,fp)) {
    if (line[0]=='#') continue;
    int num = sscanf(line,settingScanFmt_.c_str(),
		     &iphi, &ieta, &depth, &setting);
    if (num != 4) 
      throw cms::Exception("ScanFmt string doesn't match input")
	<< line << settingScanFmt_ << endl;

    HcalSubdetector subdet;
    if (depth == 4) subdet = HcalOuter;
    else if (abs(ieta)  > 29) subdet = HcalForward;
    else if (abs(ieta)  < 16) subdet = HcalBarrel;
    else if (abs(ieta) == 16) subdet = (depth==3) ? HcalEndcap : HcalBarrel;
    else if (abs(ieta) == 29) {
      if (depth == 3)  subdet = HcalEndcap;
      else {
	cerr << "subdet HF is assumed, ieta=" << ieta << ", depth=" << depth << endl;
	subdet = HcalForward;
      }
    }
    else subdet = HcalEndcap;

    if (HcalDetId::validDetId(subdet,ieta,iphi,depth)) {
      HcalDetId detId(subdet,ieta,iphi,depth);
      delays.insert(std::pair<HcalDetId,int>(detId,setting));
    }
  }

  cout << "Read in sampling delay settings from " << xmlfileNames_[0] << endl;

#if 0
  // Dump for test.
  cout<< "FrontEnd ID\t\tSetting" << endl;
  DelaySettings::const_iterator it;
  for (it=delays.begin(); it!=delays.end(); it++) {
    cout << it->first << '\t' << it->second << endl;
  }
#endif
}                               // HcalDelayTunerInput::getSamplingDelays

// ======================================================================

// Parses a list of xml files and returns a map of delay settings,
// one for each channel
//
void
HcalDelayTunerInput::getTimeCorrections (TimesPerFEchan& timecorrs)
{
  if (!timecorrFileNames_.size()) {
    edm::LogWarning("Called getTimeCorrections without defining filename vector!");
    return; // nothing to do...
  }

  char  line[128];
  char  rbx[40];
  int   rm;  // 1,2,3,4 (max 3 for HF)
  int   card;  // 1,2,3 (max 4 for HF)
  int   qie;   // 0,1,2,3,4,5
  float tcor;

  timecorrs.clear();

  // Here the files are really just white-space-separated columns of
  // RBX RM card qie delay settings
  //
  for (uint32_t i=0; i<timecorrFileNames_.size();i++) {
    FILE *fp = fopen(timecorrFileNames_[i].c_str(),"r");
    if (!fp) {
      throw cms::Exception("File doesn't exist") << timecorrFileNames_[0];
    }
    
    while (!feof(fp) && fgets(line,128,fp)) {
      if (line[0]=='#') continue;
      int num=sscanf(line,timecorrScanFmt_.c_str(),rbx,&rm,&card,&qie,&tcor);
      if (num != 5) 
	throw cms::Exception("ScanFmt string doesn't match input")
	  << line << timecorrScanFmt_ << endl;
      
      if (rbx[0] == 'Z') continue; // ZDC
      HcalFrontEndId feID(std::string(rbx),rm,0,1,0,card,qie);
      timecorrs.insert(std::pair<HcalFrontEndId,float>(feID,tcor));
    }
    fclose(fp);
    cout << "Read in time corrections from " << timecorrFileNames_[i] << endl;
  }
}                             // HcalDelayTunerInput::getTimeCorrections

//======================================================================

// Parses a list of xml files and returns a map of delay settings,
// one for each channel
//
void
HcalDelayTunerInput::getTimeCorrections (TimesPerDetId& timecorrs)
{
  if (!timecorrFileNames_.size()) {
    edm::LogWarning("Called getTimeCorrections without defining filename vector!");
    return; // nothing to do...
  }

  char  line[128];
  char  subdet[10];
  int   ieta;  // -41 to 41
  int   iphi;  // 1-72
  int   depth;   // 1,2,3,4
  float tcor;

  HcalSubdetector sd;

  timecorrs.clear();

  // Here the files are really just white-space-separated columns of
  // RBX RM card qie delay settings
  //
  FILE *fp = fopen(timecorrFileNames_[0].c_str(),"r");
  if (!fp) {
    throw cms::Exception("File doesn't exist") << timecorrFileNames_[0];
  }

  while (!feof(fp) && fgets(line,128,fp)) {
    if (line[0]=='#') continue;
    int num=sscanf(line,timecorrScanFmt_.c_str(),subdet,&ieta,&iphi,&depth,&tcor);
    if (num != 5) 
      throw cms::Exception("ScanFmt string doesn't match input") << line;

    if      (string(subdet) == "HB")  sd = HcalBarrel;
    else if (string(subdet) == "HE")  sd = HcalEndcap;
    else if (string(subdet) == "HO")  sd = HcalOuter;
    else if (string(subdet) == "HF")  sd = HcalForward;
    else {
      throw cms::Exception("Invalid/unsupported subdetector") << subdet;
    }

    if (HcalDetId::validDetId(sd,ieta,iphi,depth)) {
      HcalDetId detId(sd,ieta,iphi,depth);
      timecorrs.insert(std::pair<HcalDetId,float>(detId,tcor));
    }
  }
  cout << "Read in time corrections from " << timecorrFileNames_[0] << endl;
}
