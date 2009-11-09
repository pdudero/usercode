
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
// $Id: HcalDelayTunerInput.hh,v 1.5 2009/05/21 09:52:41 dudero Exp $
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
#include "DataFormats/HcalDetId/interface/HcalFrontEndId.h"
#include "HcalDelayTunerInput.hh"

using namespace std;

HcalDelayTunerInput::HcalDelayTunerInput(const edm::ParameterSet& iConfig)
{
  xmlfileNames_ = iConfig.getUntrackedParameter<vector<string> >("oldSettingFilenames");
}

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

  char line[128];
  char rbx[10];
  int rm;  // 1,2,3,4 (max 3 for HF)
  int card;  // 1,2,3 (max 4 for HF)
  int qie;   // 0,1,2,3,4,5
  int setting; // 0-24

  delays.clear();

  // Here the files are really just white-space-separated columns of
  // RBX RM card qie delay settings
  //
  FILE *fp = fopen(xmlfileNames_[0].c_str(),"r");
  if (!fp) {
    throw cms::Exception("File doesn't exist") << xmlfileNames_[0];
  }

  while (!feof(fp) && fgets(line,128,fp)) {
    sscanf(line,"%s %d %d %d %d", rbx, &rm, &card, &qie, &setting);
    if (rbx[0] == 'Z') continue; // ZDC
    HcalFrontEndId feID(std::string(rbx),rm,0,1,0,card,qie);
    delays.insert(std::pair<HcalFrontEndId,int>(feID,setting));
    //cout << "read in " << feID << " old setting = " << setting << endl;
  }
}

