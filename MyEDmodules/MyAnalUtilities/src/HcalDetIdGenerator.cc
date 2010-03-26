
// -*- C++ -*-
//
// Package:    HcalDetIdGenerator
// Class:      HcalDetIdGenerator
// 
/**\class HcalDetIdGenerator HcalDetIdGenerator.cc MyEDmodules/HcalDetIdGenerator/src/HcalDetIdGenerator.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalDetIdGenerator.cc,v 1.1 2009/11/09 00:57:58 dudero Exp $
//
//


#include <set>
#include <string>
#include <vector>
#include <math.h>

// user include files
#include "FWCore/Utilities/interface/Exception.h"
#include "MyEDmodules/MyAnalUtilities//interface/HcalDetIdGenerator.hh"

//======================================================================
//
// member functions
//
//======================================================================

void
HcalDetIdGenerator::appendDetIds4subdet(HcalSubdetector         subdet,
					std::vector<uint32_t>& v_denseIds)
{
  // internal to the HcalDetId type, the major/minor indices go
  // minor:  phi->eta->zside->depth : major
  //
  switch (subdet) {

  case HcalBarrel:
    for (int zside=-1; zside <=1; zside+=2)
      for (int ieta=1; ieta<=16; ieta++)
	for (int iphi=1; iphi<=72; iphi++) {
	  v_denseIds.push_back(HcalDetId(HcalBarrel,(zside*ieta),iphi,1).denseIndex());
	}
    for (int zside=-1; zside <=1; zside+=2)
      for (int ieta=15; ieta<=16; ieta++)
	for (int iphi=1; iphi<=72; iphi++) {
	  v_denseIds.push_back(HcalDetId(HcalBarrel,(zside*ieta),iphi,2).denseIndex());
	}
    break;

  case HcalEndcap:
    for (int zside=-1; zside <=1; zside+=2)
      for (int ieta=17; ieta<=29; ieta++)
	for (int iphi=1; iphi<=72; iphi++) {
	  v_denseIds.push_back(HcalDetId(HcalEndcap,(zside*ieta),iphi,1).denseIndex());
	}
    for (int zside=-1; zside <=1; zside+=2)
      for (int ieta=18; ieta<=29; ieta++)
	for (int iphi=1; iphi<=72; iphi++) {
	  v_denseIds.push_back(HcalDetId(HcalEndcap,(zside*ieta),iphi,2).denseIndex());
	}
    for (int zside=-1; zside <=1; zside+=2)
      for (int iphi=1; iphi<=72; iphi++) {
	v_denseIds.push_back(HcalDetId(HcalEndcap,(zside*16),iphi,3).denseIndex());
      }
    for (int zside=-1; zside <=1; zside+=2)
      for (int ieta=27; ieta<=29; ieta++)
	for (int iphi=1; iphi<=72; iphi++) {
	  v_denseIds.push_back(HcalDetId(HcalEndcap,(zside*ieta),iphi,3).denseIndex());
	}
    break;

  case HcalOuter:
    for (int zside=-1; zside <=1; zside+=2)
      for (int ieta=1; ieta<=15; ieta++)
	for (int iphi=1; iphi<=72; iphi++) {
	  v_denseIds.push_back(HcalDetId(HcalOuter,(zside*ieta),iphi,4).denseIndex());
	}
    break;

  case HcalForward:
    for (int zside=-1; zside <=1; zside+=2)
      for (int ieta=29; ieta<=41; ieta++)
	for (int iphi=1; iphi<=71; iphi+=2) {
	  v_denseIds.push_back(HcalDetId(HcalForward,(zside*ieta),iphi,1).denseIndex());
	}
    for (int zside=-1; zside <=1; zside+=2)
      for (int ieta=29; ieta<=41; ieta++)
	for (int iphi=1; iphi<=71; iphi+=2) {
	  v_denseIds.push_back(HcalDetId(HcalForward,(zside*ieta),iphi,2).denseIndex());
	}
    break;

  default:
    throw cms::Exception("HcalDetIdGenerator") << "BAD SUBDET: " << subdet;
  }
}

//======================================================================
