
// -*- C++ -*-
//
// Package:    SplashHitTimeCorrector
// Class:      SplashHitTimeCorrector
// 
/**\class SplashHitTimeCorrector SplashHitTimeCorrector.cc MyEDmodules/SplashHitTimeCorrector/src/SplashHitTimeCorrector.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: SplashHitTimeCorrector.cc,v 1.1 2009/07/27 15:56:53 dudero Exp $
//
//


// system include files
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <math.h> // floor

// user include files

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "MyEDmodules/HcalDelayTuner/interface/SplashHitTimeCorrector.hh"

#include "TProfile.h"

//
// constructors and destructor
//
SplashHitTimeCorrector::SplashHitTimeCorrector(bool plusZsideSplash) :
  splashPlusZside_(plusZsideSplash)
{
  std::cout<<"SPLASH ZSIDE IS SET TO: "<<(splashPlusZside_?"PLUS":"MINUS")<<std::endl;
  edm::Service<TFileService> fs;
  rootdir_ = new TFileDirectory(fs->mkdir("HitTimeCorrections"));
  bookHistos();
}                      // SplashHitTimeCorrector::SplashHitTimeCorrector

//======================================================================

void
SplashHitTimeCorrector::bookHistos(void)
{
  // Initialize the cuts for the run and add them to the global map
  for (int i=1; i<=4; i++) {
    char name[40];
    char title[128];
    sprintf (name, "p1d_TimeCorrsVsIetaD%d",i);
    sprintf (title,
     "Splash Time Correction (Depth %d); i#eta; Time Correction (ns)",
	     i);
    v_profiles_.push_back(rootdir_->make<TProfile>(name,title,
						   83,-41.5,41.5));
  }
}                                                          // bookHistos

//======================================================================
// COURTESY OF JORDAN DAMGOV, jdamgov@fnal.gov
//
GlobalPoint
SplashHitTimeCorrector::lookupTwrCtr4(const HcalDetId& id)
{
  double R=0,Z=0;
  GlobalPoint position = geo_->getPosition(id());
  int absieta = abs(id.ieta());
  int depth   = id.depth();

  switch(id.subdet()) {
  case HcalBarrel:
    if(absieta<15)              R=233;
    if(absieta==15 && depth==1) R=216;
    if(absieta==15 && depth==2) R=267;
    if(absieta==16 && depth==1) R=188;
    if(absieta==16 && depth==2) R=206;
    Z=R/tan(position.theta());
    break;
  case HcalEndcap:
    if(absieta==16)                            Z=445;
    if(absieta==17)                            Z=462;
    if(absieta==18 && depth==1)                Z=410;
    if(absieta==18 && depth==2)                Z=484;
    if(absieta>=19 && absieta<=26 && depth==1) Z=418;
    if(absieta>=19 && absieta<=26 && depth==2) Z=493;
    if(absieta>=27 && absieta<=29 && depth==1) Z=410;
    if(absieta>=27 && absieta<=29 && depth==2) Z=428;
    if(absieta>=27 && absieta<=29 && depth==3) Z=493;
    R=Z*tan(position.theta());
    if(id.ieta()<0)Z=-Z;
    break;
  case HcalOuter:
    if(absieta<=4) R=395;
    if(absieta>=5) R=407;
    Z=R/tan(position.theta());
    break;
  default: break;
  }
  return  GlobalPoint(0,R,Z);
}                               // SplashHitTimeCorrector::lookupTwrCtr4

//======================================================================
// COURTESY OF JORDAN DAMGOV, jdamgov@fnal.gov
//
float
SplashHitTimeCorrector::calcRawTcorrect4(const HcalDetId& id)
{
  GlobalPoint twrctrFromIP = lookupTwrCtr4(id);

  double colz = twrctrFromIP.z()-(15000*(splashPlusZside_ ? 1.:-1.));

  GlobalPoint twrctrFromCollimator = GlobalPoint(twrctrFromIP.x(),
						 twrctrFromIP.y(),
						 colz);
// vector mags are in cm, 30cm=1ns
  double distdiffcm=twrctrFromCollimator.mag()-twrctrFromIP.mag();

  float toffs = (float)(distdiffcm/30.);
  return toffs;
}                            // SplashHitTimeCorrector::calcRawTcorrect4

//======================================================================

float
SplashHitTimeCorrector::getTcor4(const HcalDetId& id)
{
  int ieta   = id.ieta();
  int depth  = id.depth();
  int index  = (10*ieta)+depth;

  float thecor = 0.0;

  std::map<int,float>::const_iterator mapit;
  mapit = m_corrs_.find(index);
  if (mapit == m_corrs_.end()) {
    thecor = calcRawTcorrect4(id)  - globalOffset_;
    (v_profiles_[depth-1])->Fill(ieta,thecor);
    m_corrs_.insert(std::pair<int,float>(index,thecor));
  } else {
    thecor = mapit->second;
  }
  return thecor;
}                                    // SplashHitTimeCorrector::getTcor4

//======================================================================

void
SplashHitTimeCorrector::init(const edm::EventSetup& iSetup)
{
// geometry
  edm::ESHandle < CaloGeometry > pG;
  iSetup.get < CaloGeometryRecord > ().get(pG);
  geo_ = pG.product();

  // since the time difference between collimator and IP
  // is large, offset it initially by the value calculated for
  // the center off the barrel
  //
  HcalDetId idminus1(HcalBarrel,-1,1,1);
  HcalDetId idplus1 (HcalBarrel, 1,1,1);

  float corminus1 = calcRawTcorrect4(idminus1);
  float corplus1  = calcRawTcorrect4(idplus1);

  globalOffset_ = (corminus1+corplus1)/2.;
}                                       // SplashHitTimeCorrector::init
