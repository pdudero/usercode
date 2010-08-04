
// -*- C++ -*-
//
// Package:    HcaTimingAnalAlgos
// Class:      HcaTimingAnalAlgos
// 
/**\class HcaTimingAnalAlgos HcaTimingAnalAlgos.cc MyEDmodules/HcaTimingAnalAlgos/src/HcaTimingAnalAlgos.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalTimingAnalAlgos.cc,v 1.1 2010/06/20 12:48:45 dudero Exp $
//
//


// system include files
#include <set>
#include <string>
#include <vector>
#include <stdlib.h> // itoa
#include <math.h>   // floor

// user include files
#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "RecoLocalCalo/HcalRecAlgos/interface/HcalCaloFlagLabels.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "MyEDmodules/MyAnalUtilities/interface/inSet.hh"

#include "../interface/HcalTimingAnalAlgos.hh"


#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TProfile.h"
#include "TProfile2D.h"

inline string int2str(int i) {
  std::ostringstream ss;
  ss << i;
  return ss.str();
}

static const float wpksamp0_hf = 0.500053;
static const float scale_hf    = 0.999683;
static const int   num_bins_hf = 100;

static const float actual_ns_hf[num_bins_hf] = {
 0.00250, // 0.000-0.010
 0.08000, // 0.010-0.020
 0.16000, // 0.020-0.030
 0.23750, // 0.030-0.040
 0.31750, // 0.040-0.050
 0.39500, // 0.050-0.060
 0.47500, // 0.060-0.070
 0.55500, // 0.070-0.080
 0.63000, // 0.080-0.090
 0.70000, // 0.090-0.100
 0.77000, // 0.100-0.110
 0.84000, // 0.110-0.120
 0.91000, // 0.120-0.130
 0.98000, // 0.130-0.140
 1.05000, // 0.140-0.150
 1.12000, // 0.150-0.160
 1.19000, // 0.160-0.170
 1.26000, // 0.170-0.180
 1.33000, // 0.180-0.190
 1.40000, // 0.190-0.200
 1.47000, // 0.200-0.210
 1.54000, // 0.210-0.220
 1.61000, // 0.220-0.230
 1.68000, // 0.230-0.240
 1.75000, // 0.240-0.250
 1.82000, // 0.250-0.260
 1.89000, // 0.260-0.270
 1.96000, // 0.270-0.280
 2.03000, // 0.280-0.290
 2.10000, // 0.290-0.300
 2.17000, // 0.300-0.310
 2.24000, // 0.310-0.320
 2.31000, // 0.320-0.330
 2.38000, // 0.330-0.340
 2.45000, // 0.340-0.350
 2.52000, // 0.350-0.360
 2.59000, // 0.360-0.370
 2.68500, // 0.370-0.380
 2.79250, // 0.380-0.390
 2.90250, // 0.390-0.400
 3.01000, // 0.400-0.410
 3.11750, // 0.410-0.420
 3.22500, // 0.420-0.430
 3.33500, // 0.430-0.440
 3.44250, // 0.440-0.450
 3.55000, // 0.450-0.460
 3.73250, // 0.460-0.470
 4.02000, // 0.470-0.480
 4.30750, // 0.480-0.490
 4.59500, // 0.490-0.500
 6.97500, // 0.500-0.510
10.98750, // 0.510-0.520
13.03750, // 0.520-0.530
14.39250, // 0.530-0.540
15.39500, // 0.540-0.550
16.18250, // 0.550-0.560
16.85250, // 0.560-0.570
17.42750, // 0.570-0.580
17.91500, // 0.580-0.590
18.36250, // 0.590-0.600
18.76500, // 0.600-0.610
19.11250, // 0.610-0.620
19.46000, // 0.620-0.630
19.76500, // 0.630-0.640
20.03500, // 0.640-0.650
20.30250, // 0.650-0.660
20.57250, // 0.660-0.670
20.79250, // 0.670-0.680
21.00250, // 0.680-0.690
21.21000, // 0.690-0.700
21.42000, // 0.700-0.710
21.62750, // 0.710-0.720
21.79000, // 0.720-0.730
21.95250, // 0.730-0.740
22.11500, // 0.740-0.750
22.27750, // 0.750-0.760
22.44000, // 0.760-0.770
22.60500, // 0.770-0.780
22.73250, // 0.780-0.790
22.86000, // 0.790-0.800
22.98500, // 0.800-0.810
23.11250, // 0.810-0.820
23.23750, // 0.820-0.830
23.36500, // 0.830-0.840
23.49000, // 0.840-0.850
23.61750, // 0.850-0.860
23.71500, // 0.860-0.870
23.81250, // 0.870-0.880
23.91250, // 0.880-0.890
24.01000, // 0.890-0.900
24.10750, // 0.900-0.910
24.20750, // 0.910-0.920
24.30500, // 0.920-0.930
24.40500, // 0.930-0.940
24.50250, // 0.940-0.950
24.60000, // 0.950-0.960
24.68250, // 0.960-0.970
24.76250, // 0.970-0.980
24.84000, // 0.980-0.990
24.92000  // 0.990-1.000
};

static float timeshift_ns_hf(float wpksamp) {
  float flx = (num_bins_hf*(wpksamp - wpksamp0_hf)/scale_hf);
  int index = (int)flx;
  float yval;

  if      (index <    0)        return actual_ns_hf[0];
  else if (index >= num_bins_hf-1) return actual_ns_hf[num_bins_hf-1];

  // else interpolate:
  float y1       = actual_ns_hf[index];
  float y2       = actual_ns_hf[index+1];

  // float delta_x  = 1/(float)num_bins_hf;
  // yval = y1 + (y2-y1)*(flx-(float)index)/delta_x;

  yval = y1 + (y2-y1)*(flx-(float)index);
  return yval;
}

static const double hftwrRadii[] = { // in meters
  1.570-1.423,  // 41
  1.570-1.385,  // 40
  1.570-1.350,  // 39
  1.570-1.308,  // 38
  1.570-1.258,  // 37
  1.570-1.198,  // 36
  1.570-1.127,  // 35
  1.570-1.042,  // 34
  1.570-0.941,  // 33
  1.570-0.821,  // 32
  1.570-0.677,  // 31
  1.570-0.505,  // 30
  1.570-0.344   // 29
};

static const double hftwrEdges[] = {  // in meters
-(1.570- 0.207),
-(1.570-(0.505+0.344)/2.), // 29
-(1.570-(0.677+0.505)/2.), // 30
-(1.570-(0.821+0.677)/2.), // 31
-(1.570-(0.941+0.821)/2.), // 32
-(1.570-(1.042+0.941)/2.), // 33
-(1.570-(1.127+1.042)/2.), // 34
-(1.570-(1.198+1.127)/2.), // 35
-(1.570-(1.258+1.198)/2.), // 36
-(1.570-(1.308+1.258)/2.), // 37
-(1.570-(1.350+1.308)/2.), // 38
-(1.570-(1.385+1.350)/2.), // 39
-(1.570-(1.423+1.385)/2.), // 40
-(1.570-1.445),            // 41
  0.,                      // ieta
  1.570-1.445,             // 41
  1.570-(1.423+1.385)/2.,  // 40
  1.570-(1.385+1.350)/2.,  // 39
  1.570-(1.350+1.308)/2.,  // 38
  1.570-(1.308+1.258)/2.,  // 37
  1.570-(1.258+1.198)/2.,  // 36
  1.570-(1.198+1.127)/2.,  // 35
  1.570-(1.127+1.042)/2.,  // 34
  1.570-(1.042+0.941)/2.,  // 33
  1.570-(0.941+0.821)/2.,  // 32
  1.570-(0.821+0.677)/2.,  // 31
  1.570-(0.677+0.505)/2.,  // 30
  1.570-(0.505+0.344)/2.,  // 29
  1.570- 0.207
};

const int nEbins= 12;
static const double digEbins[] = {
  1.0,2.0,3.0,4.0,5.0,7.0,10.0,20.0,40.0,100.0,300.0,600.0,1000.0
};

//======================================================================
// functions to facilitate auto-fill
//
static bool isPlus   (const DetId& id) {
  return ((id.det() == DetId::Hcal) ? (HcalDetId(id)).zside()>0:(HcalZDCDetId(id).zside()>0));
}
static bool isMinus  (const DetId& id) {
  return ((id.det() == DetId::Hcal) ? (HcalDetId(id)).zside()<0:(HcalZDCDetId(id).zside()<0));
}
static bool isDepth1 (const DetId& id) {
  return ((id.det() == DetId::Hcal) ? (HcalDetId(id)).depth()==1:(HcalZDCDetId(id).depth()==1));
}

static bool isDepth2 (const DetId& id) { return ((HcalDetId(id)).depth()==2); }
static bool isDepth3 (const DetId& id) { return ((HcalDetId(id)).depth()==3); }
static bool isDepth4 (const DetId& id) { return ((HcalDetId(id)).depth()==4); }
static bool isHBHE   (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalBarrel) ||
						 ((HcalDetId(id)).subdet()==HcalEndcap)); }
static bool isHBHEd1 (const DetId& id) { return (((HcalDetId(id)).depth()==1) &&
						 (((HcalDetId(id)).subdet()==HcalBarrel) ||
						  ((HcalDetId(id)).subdet()==HcalEndcap))); }
static bool isHBHEd2 (const DetId& id) { return (((HcalDetId(id)).depth()==2) &&
						 (((HcalDetId(id)).subdet()==HcalBarrel) ||
						  ((HcalDetId(id)).subdet()==HcalEndcap))); }
static bool isHO2M   (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalOuter) &&
						 ((HcalDetId(id)).ieta()>=-15)&&((HcalDetId(id)).ieta()<=-11)); }
static bool isHO1M   (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalOuter) &&
						 ((HcalDetId(id)).ieta()>=-10)&&((HcalDetId(id)).ieta()<=-5));  }
static bool isHO0    (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalOuter) &&
						 ((HcalDetId(id)).ieta()>=-4)&&((HcalDetId(id)).ieta()<=4));    }
static bool isHO1P   (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalOuter) &&
						 ((HcalDetId(id)).ieta()>=5)&&((HcalDetId(id)).ieta()<=10));    }
static bool isHO2P   (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalOuter) &&
						 ((HcalDetId(id)).ieta()>=11)&&((HcalDetId(id)).ieta()<=15));   }
static bool isHFPd1  (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalForward) &&
						 ((HcalDetId(id)).zside()*(HcalDetId(id)).depth()==1)); }
static bool isHFPd2  (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalForward) &&
						 ((HcalDetId(id)).zside()*(HcalDetId(id)).depth()==2)); }
static bool isHFMd1  (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalForward) &&
						 ((HcalDetId(id)).zside()*(HcalDetId(id)).depth()==-1)); }
static bool isHFMd2  (const DetId& id) { return (((HcalDetId(id)).subdet()==HcalForward) &&
						 ((HcalDetId(id)).zside()*(HcalDetId(id)).depth()==-2)); }

//======================================================================

float
HcalTimingAnalAlgos::recoTimeFromAvgPulseHF(TProfile *avgPulse)
{
  float maxval = -9e99;
  int maxbin   = 0;
  // find peak bin within selected window
  for (int ibin=std::max(firstsamp_+1,2);
       ibin<=std::min(firstsamp_+nsamps_+1,avgPulse->GetNbinsX());
       ibin++) {
    float val = avgPulse->GetBinContent(ibin);
    if (val > maxval) {
      maxval = val;
      maxbin = ibin;
    }
  }
  float t0 = avgPulse->GetBinContent(maxbin-1);
  float t2 = avgPulse->GetBinContent(maxbin+1);

  float wpksamp = (t0 + maxval + t2);
  if (wpksamp!=0) wpksamp=(maxval + 2.0f*t2) / wpksamp;
  return (maxbin-1 - presamples_)*25.0f + timeshift_ns_hf(wpksamp);
}

//======================================================================

static void labelHitFlagAxis(const TH1F& h1f, HcalSubdetector subdet)
{
  TAxis *xax = h1f.GetXaxis();
  xax->SetBinLabel(1,(subdet==HcalForward)?"HFLongShort":"HBHEHpdHitMultiplicity");
  xax->SetBinLabel(2,(subdet==HcalForward)?"HFDigiTime":"HBHEPulseShape");
  xax->SetBinLabel(3,"HSCP_R1R2");
  xax->SetBinLabel(4,"HSCP_FracLeader");
  xax->SetBinLabel(5,"HSCP_OuterEnergy");
  xax->SetBinLabel(6,"HSCP_ExpFit");
  if (subdet==HcalForward) {
    xax->SetBinLabel(7,"HFTimingTrust0");
    xax->SetBinLabel(8,"HFTimingTrust1");
  }
  xax->SetBinLabel(9, "HBHETimingShapedCuts0");
  xax->SetBinLabel(10,"HBHETimingShapedCuts1");
  xax->SetBinLabel(11,"HBHETimingShapedCuts2");
  //xax->SetBinLabel(12,"N/A");
  //xax->SetBinLabel(13,"N/A");
  //xax->SetBinLabel(14,"N/A");
  //xax->SetBinLabel(15,"N/A");
  //xax->SetBinLabel(16,"N/A");
  xax->SetBinLabel(17,"Subtracted25ns");
  xax->SetBinLabel(18,"Added25ns");
  xax->SetBinLabel(19,"UncorBCNcapidMismatch");
  xax->SetBinLabel(20,"Saturation");
  xax->SetBinLabel(21,"Fraction2TS_0");
  xax->SetBinLabel(22,"Fraction2TS_1");
  xax->SetBinLabel(23,"Fraction2TS_2");
  xax->SetBinLabel(24,"Fraction2TS_3");
  xax->SetBinLabel(25,"Fraction2TS_4");
  xax->SetBinLabel(26,"Fraction2TS_5");
  xax->SetBinLabel(30,"UserDefinedBit2");
  xax->SetBinLabel(31,"UserDefinedBit1");
  xax->SetBinLabel(32,"UserDefinedBit0");
}

//======================================================================

static void labelSubdetAxis(const TProfile2D& tp)
{
  TAxis *xax = tp.GetXaxis();
  xax->SetBinLabel(1,"HF-");
  xax->SetBinLabel(2,"HE-");
  xax->SetBinLabel(3,"HB-");
  xax->SetBinLabel(4,"HB+");
  xax->SetBinLabel(5,"HE+");
  xax->SetBinLabel(6,"HF+");
}

//======================================================================

static void labelHORingAxis(const TProfile2D& tp)
{
  TAxis *xax = tp.GetXaxis();
  xax->SetBinLabel(1,"HO2-");
  xax->SetBinLabel(2,"HO1-");
  xax->SetBinLabel(3,"HO0");
  xax->SetBinLabel(4,"HO1+");
  xax->SetBinLabel(5,"HO2+");
}

//======================================================================
const std::string HcalTimingAnalAlgos::st_fillDetail_   = "filldetail";
const std::string HcalTimingAnalAlgos::st_perEvtDigi_   = "perEvtDigi";
const std::string HcalTimingAnalAlgos::st_doPerChannel_ = "doPerChannel";

//
// constructors and destructor
//
HcalTimingAnalAlgos::HcalTimingAnalAlgos(const edm::ParameterSet& iConfig) :
  mysubdetstr_(iConfig.getUntrackedParameter<std::string>("subdet")),
  globalToffset_(iConfig.getParameter<double>("globalTimeOffset")),
  globalFlagMask_(iConfig.getParameter<int>("globalRecHitFlagMask")),
  rundescr_(iConfig.getUntrackedParameter<std::string>("runDescription",""))
{
  ampCutsInfC_        = iConfig.getParameter<bool>  ("ampCutsInfC");
  minHitAmplitude_    = iConfig.getParameter<double>("minHit_GeVorfC");
  maxHitAmplitude_    = iConfig.getParameter<double>("maxHit_GeVorfC");
  recHitEscaleMinGeV_ = iConfig.getParameter<double>("recHitEscaleMinGeV");
  recHitEscaleMaxGeV_ = iConfig.getParameter<double>("recHitEscaleMaxGeV");
  recHitTscaleNbins_  = iConfig.getParameter<int>   ("recHitTscaleNbins");
  recHitTscaleMinNs_  = iConfig.getParameter<double>("recHitTscaleMinNs");
  recHitTscaleMaxNs_  = iConfig.getParameter<double>("recHitTscaleMaxNs");
#if 0
  timeWindowMinNS_    = iConfig.getParameter<double>("timeWindowMinNS");
  timeWindowMaxNS_    = iConfig.getParameter<double>("timeWindowMaxNS");
#endif
  maxEventNum2plot_   = iConfig.getParameter<int>   ("maxEventNum2plot");
  normalizeDigis_     = iConfig.getParameter<bool>  ("normalizeDigis");
  doPerChannel_       = iConfig.getParameter<bool>  ("doPerChannel");
  minEvents4avgT_     = iConfig.getParameter<int>   ("minEvents4avgT");
  doTree_             = iConfig.getUntrackedParameter<bool>("doTree",false);

  recHitEscaleNbins_ = (uint32_t)(recHitEscaleMaxGeV_ - recHitEscaleMinGeV_);

  recHitEscaleMinfC_ = 5*recHitEscaleMinGeV_;
  recHitEscaleMaxfC_ = 5*recHitEscaleMaxGeV_;

  edm::ParameterSet TrecoParams =
    iConfig.getParameter<edm::ParameterSet>("TrecoParams");

  firstsamp_  = TrecoParams.getParameter<int>("firstSample");
  nsamps_     = TrecoParams.getParameter<int>("numSamples");
  presamples_ = TrecoParams.getParameter<int>("preSamples");

  std::vector<int> v_maskidnumbers =
    iConfig.getParameter<vector<int> >("detIds2mask");

  if (!buildMaskSet(v_maskidnumbers))
    throw cms::Exception("Invalid detID vector");

  HcalLogicalMapGenerator gen;
  lmap_ = new HcalLogicalMap(gen.createMap());

  if (!mysubdetstr_.compare("HB"))  mysubdet_ = HcalBarrel;  else 	 
  if (!mysubdetstr_.compare("HE"))  mysubdet_ = HcalEndcap;  else 	 
  if (!mysubdetstr_.compare("HO"))  mysubdet_ = HcalOuter;   else 	 
  if (!mysubdetstr_.compare("HF"))  mysubdet_ = HcalForward; else
  if (!mysubdetstr_.compare("ZDC")) mysubdet_ = HcalOther;   else {
    throw cms::Exception("Unknown subdet string: ") << mysubdetstr_ << endl;
  }

  firstEvent_ = true;
}                            // HcalTimingAnalAlgos::HcalTimingAnalAlgos

//======================================================================

bool
HcalTimingAnalAlgos::buildMaskSet(const std::vector<int>& v_idnumbers)
{
  // convert det ID numbers to valid detIds:
  if (v_idnumbers.size()%3) {
    return false;
  }

  for (uint32_t i=0; i<v_idnumbers.size(); i+=3) {
    int ieta  = v_idnumbers[i];
    int iphi  = v_idnumbers[i+1];
    int depth = v_idnumbers[i+2];
    enum HcalSubdetector subdet;
    if      (depth == 4)       subdet = HcalOuter;
    else if (abs(ieta) <= 16)  subdet = (depth==3)? HcalEndcap:HcalBarrel;
    else if (abs(ieta) <= 29)  subdet = HcalEndcap; // gud enuf fer gubmint werk
    else return false;                    // no HF for splash
    
    if (!HcalDetId::validDetId(subdet,ieta,iphi,depth))
      return false;

    detIds2mask_.insert(HcalDetId(subdet,ieta,iphi,depth).hashed_index());
  }
  return true;
}                                   // HcalTimingAnalAlgos::buildMaskSet

//======================================================================

myAnalCut *
HcalTimingAnalAlgos::getCut(const std::string& cutstr)
{
  std::map<std::string,myAnalCut *>::const_iterator it = m_cuts_.find(cutstr);
  if (it == m_cuts_.end()) {
    throw cms::Exception("Cut not found, you numnutz! You changed the name: ") << cutstr << endl;
  }

  return it->second;
}                                         // HcalTimingAnalAlgos::getCut

//======================================================================

void
HcalTimingAnalAlgos::bookHistos4allCuts(void)
{
  std::vector<myAnalHistos::HistoParams_t>   v_hpars1d; 
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars1d_af;
  std::vector<myAnalHistos::HistoParams_t>   v_hpars1dprof;
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars1dprof_af;
  std::vector<myAnalHistos::HistoParams_t>   v_hpars2d; 
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars2d_af;
  std::vector<myAnalHistos::HistoParams_t>   v_hpars2dprof;
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars2dprof_af;

  /*****************************************
   * 1-D HISTOGRAMS FIRST:                 *
   *****************************************/

  st_rhCorTimes_ = "h1d_rhCorTimes" + mysubdetstr_;

  //==================== Basic Run Info ====================

  add1dHisto("h1d_lsnum0-499",
	     "Lumi Section Occupancy, LS=0-499, "+mysubdetstr_+" "+rundescr_+"; LS Number",
	     501,0,500,v_hpars1d);

  st_bxnum_ = "h1d_bxnum";
  add1dHisto(st_bxnum_,
	     "BX# Occupancy, "+mysubdetstr_+" "+rundescr_+"; BX Number",
	     3601,0,3600,v_hpars1d);

  //==================== Collection sizes ====================

  st_digiColSize_ = "h1d_digiCollectionSize" + mysubdetstr_;
  add1dHisto( st_digiColSize_, "Digi Collection Size, "+mysubdetstr_+" "+rundescr_,
	      5201,-0.5, 5200.5, // 72chan/RBX*72RBX = 5184, more than HF or HO
	      v_hpars1d);

  st_rhColSize_ = "h1d_rechitCollectionSize" + mysubdetstr_;
  add1dHisto( st_rhColSize_, "Rechit Collection Size, "+mysubdetstr_+" "+rundescr_,
	      5201,-0.5, 5200.5,  // 72chan/RBX*72RBX = 5184, more than HF or HO
	      v_hpars1d);

  //==================== Total time/energy/flags distros ====================
	     
  add1dAFhisto("h1d_rhUncorTimes" + mysubdetstr_,
	       "RecHit Times (uncorrected), "+mysubdetstr_+" "+rundescr_+"; Rechit Time (ns)",
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&hittime_,NULL,NULL,v_hpars1d_af);

  add1dAFhisto(st_rhCorTimes_,
	       "RecHit Times, "+mysubdetstr_+" "+rundescr_+"; Rechit Time (ns)",
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&corTime_,NULL,NULL,v_hpars1d_af);

  add1dAFhisto("h1d_rhCorTimes" + mysubdetstr_ + "P",
	       "RecHit Times, "+mysubdetstr_+"P, "+rundescr_+"; Rechit Time (ns)",
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&corTime_,NULL,(detIDfun_t)&isPlus,v_hpars1d_af);

  add1dAFhisto("h1d_rhCorTimes" + mysubdetstr_ + "M",
	       "RecHit Times, "+mysubdetstr_+"M, "+rundescr_+"; Rechit Time (ns)",
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&corTime_,NULL,(detIDfun_t)&isMinus,v_hpars1d_af);

  add1dAFhisto("h1d_RHEnergies" + mysubdetstr_,
	       "RecHit Energies, "+mysubdetstr_+" "+rundescr_+"; Rechit Energy (GeV)",
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       (void *)&hitenergy_,NULL,NULL,v_hpars1d_af);

  st_rhFlagBits_ = "h1d_rhFlagBits" + mysubdetstr_;
  add1dHisto( st_rhFlagBits_,
	      "RecHit Quality Flag Bits, "+mysubdetstr_+" "+rundescr_,
	      32,0.5,32.5,v_hpars1d);  // see below (post-booking) for bin labeling
  
  if ((mysubdet_ == HcalBarrel)||
      (mysubdet_ == HcalEndcap)  ) {
      st_rhHBHEtimingShapedCuts_ = "h1d_rhHBHEtimingShapedCuts";
      add1dHisto( st_rhHBHEtimingShapedCuts_,
		  "RecHit Timing Shaped Cuts Bits, ("+mysubdetstr_+") "+rundescr_+"; Quality (0-7)",
		  10,-0.5,9.5,v_hpars1d);
  }

  //==================== ...by Event ====================

  st_totalEperEv_ = "h1d_totalEperEvIn" + mysubdetstr_;
  add1dHisto( st_totalEperEv_,
  "#Sigma RecHit Energy Per Event in "+mysubdetstr_+" "+rundescr_+"; Event Number; Total Energy (GeV)",
//	      (maxEventNum2plot_+1),-0.5,((float)maxEventNum2plot_)+0.5,
		100,-0.5,((float)maxEventNum2plot_)+0.5, v_hpars1d);


  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  string yaxist = normalizeDigis_ ? "Normalized fC" : "fC";

  st_avgPulse_   = "h1d_pulse" + mysubdetstr_;
  add1dHisto(st_avgPulse_,"Average Pulse Shape, "+mysubdetstr_+"; TS;"+yaxist,
	     10,-0.5,9.5,v_hpars1dprof);

  st_avgPulsePlus_   = "h1d_pulse" + mysubdetstr_ + "P";
  add1dHisto(st_avgPulsePlus_,"Average Pulse Shape, "+mysubdetstr_+"P; TS;"+yaxist,
	     10,-0.5,9.5,v_hpars1dprof);

  st_avgPulseMinus_   = "h1d_pulse" + mysubdetstr_ + "M";
  add1dHisto(st_avgPulseMinus_,"Average Pulse Shape, "+mysubdetstr_+"M; TS;"+yaxist,
	     10,-0.5,9.5,v_hpars1dprof);

  st_avgPulseTerr_  = "h1d_pulseTerr" + mysubdetstr_;
  add1dHisto(st_avgPulseTerr_,
	     "Avg Pulse Shape (Pulse Shape Error) "+mysubdetstr_+"; TS;"+yaxist,
	     10, -0.5, 9.5,v_hpars1dprof);

  if ((mysubdet_ != HcalForward) && 
      (mysubdet_ != HcalOther) ) {
    st_avgPulseHPDmult_  = "h1d_pulseHPDmult" + mysubdetstr_;
    add1dHisto(st_avgPulseHPDmult_, "Avg Pulse Shape (HPD noise) "+mysubdetstr_+"; TS;"+yaxist,
	       10, -0.5, 9.5,v_hpars1dprof);
  }

  add1dAFhisto( "h1d_avgTperEvIn" + mysubdetstr_,
"Depth 1 Averaged Time Per Event in "+mysubdetstr_+" "+rundescr_+"; Event Number; Average Time (ns)",
//	      (maxEventNum2plot_+1),-0.5,((float)maxEventNum2plot_)+0.5,
		100,-0.5,((float)maxEventNum2plot_)+0.5,
		(void *)&fevtnum_,(void *)&corTime_,(detIDfun_t)&isDepth1,v_hpars1dprof_af);

  /*****************************************
   * 2-D HISTOGRAMS AFTER:                 *
   *****************************************/

  if (mysubdet_ != HcalOther) {
    add2dAFhisto("h2d_NhitsPerIetaIphi" + mysubdetstr_,
		 "RecHit Occupancy Map, "+mysubdetstr_+" "+rundescr_+"; i#eta; i#phi",
		 83, -41.5,  41.5, 72, 0.5, 72.5,
		 (void *)&fieta_,(void *)&fiphi_,NULL,NULL,v_hpars2d_af);

    add2dAFhisto("p2d_rhEperIetaIphi" + mysubdetstr_,
		 "RecHit Energy Map (#Sigma depths), "+mysubdetstr_+" "+rundescr_+"; i#eta; i#phi",
		 83, -41.5,  41.5, 72,   0.5,  72.5, 
		 (void *)&fieta_,(void *)&fiphi_,(void *)&hitenergy_,NULL,v_hpars2dprof_af);
    
    add2dAFhisto("p2d_rhTvsEtaEnergy" + mysubdetstr_,
"RecHit Time Vs. #eta and Energy, "+mysubdetstr_+" "+rundescr_+"; i#eta; Hit Energy (GeV)",
		 83, -41.5,  41.5, recHitEscaleNbins_, recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
		 (void *)&fieta_,(void *)&hitenergy_,(void *)&corTime_,NULL,v_hpars2dprof_af);

    st_rhTprofRBX_ = "p2d_rhTcorPerRBX";
    if (mysubdet_ != HcalOuter) {
      add2dAFhisto(st_rhTprofRBX_,
		   "RecHit Time RBX Map, "+rundescr_+"; ; iRBX",
		   6, -3.0,  3.0, 18,   0.5,  18.5,
		   (void *)&fsubdet_,(void *)&fRBX_,(void *)&corTime_,
		   NULL,v_hpars2dprof_af);
  
      st_rhFraction2TSvsE_ = "h2d_rhFraction2TSvsE" + mysubdetstr_;
      string titlestr;

      if (mysubdet_ == HcalForward) {
	titlestr =  "RecHit 'Fraction 2TS'  Bits (HF)"+rundescr_+"; E_{hit} (GeV); fC_{peak-1}/fC_{peak}";
      } else {
	titlestr  = "RecHit 'Fraction 2TS'  Bits ("+mysubdetstr_+") "+rundescr_;
	titlestr += "; E_{hit} (GeV); max(#Sigma_{2TS})/#Sigma_{allTS}";
      }
      add2dHisto( st_rhFraction2TSvsE_, titlestr,
		  recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
		  101,0.495,1.505,v_hpars2d);
    }
  }

  add2dAFhisto("h2d_uncorTimingVsE" + mysubdetstr_,
 "RecHit Timing (uncorrected) vs. Energy, "+mysubdetstr_+"; Rechit Energy (GeV); Rechit Time (ns)",
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&hitenergy_,(void *)&hittime_,NULL,NULL, v_hpars2d_af);

  add2dAFhisto("h2d_corTimingVsE" + mysubdetstr_,
"RecHit Timing vs. Energy, "+mysubdetstr_+"; Rechit Energy (GeV); Rechit Time (ns)",
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&hitenergy_,(void *)&corTime_,NULL,NULL, v_hpars2d_af);

  add2dAFhisto("h2d_corTimingVsED1" + mysubdetstr_,
"RecHit Timing vs. Energy, "+mysubdetstr_+" Depth 1; Rechit Energy (GeV); Rechit Time (ns)",
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&hitenergy_,(void *)&corTime_,NULL,(detIDfun_t)&isDepth1, v_hpars2d_af);

  /*****************************************
   * BOOK 'EM, DANNO...                    *
   *****************************************/

  uint32_t total = v_hpars1d.size()+v_hpars1d_af.size()+
    v_hpars1dprof.size()+v_hpars1dprof_af.size()+v_hpars2d_af.size();

  cout<<mysubdetstr_<<": Booking "<<total<<" histos for each of ";
  cout<<m_cuts_.size()<<" cuts..."<<std::endl;

  std::map<string, myAnalCut *>::const_iterator cutit;
  for (cutit = m_cuts_.begin(); cutit != m_cuts_.end(); cutit++) {
    cout << "Booking for cut " << cutit->first << endl;
    myAnalHistos *cutAH = cutit->second->cuthistos();
    cutAH->book1d<TH1F>      (v_hpars1d);
    cutAH->book1d<TH1F>      (v_hpars1d_af);
    cutAH->book1d<TProfile>  (v_hpars1dprof);
    cutAH->book1d<TProfile>  (v_hpars1dprof_af);
    cutAH->book2d<TH2F>      (v_hpars2d);
    cutAH->book2d<TH2F>      (v_hpars2d_af);
    cutAH->book2d<TProfile2D>(v_hpars2dprof);
    cutAH->book2d<TProfile2D>(v_hpars2dprof_af);
    if (mysubdet_==HcalForward)
      bookHFbasicProfiles(cutAH);
    if ((mysubdet_ != HcalOuter) &&
	(mysubdet_ != HcalOther)   ) {
      TProfile2D*tp=cutAH->get<TProfile2D>(st_rhTprofRBX_.c_str());
      TH1F     *h1f=cutAH->get<TH1F>      (st_rhFlagBits_.c_str());
      if(tp)labelSubdetAxis(*tp);
      if(h1f)labelHitFlagAxis(*h1f,mysubdet_);
    }

    // Digi pulses binned by energy
    st_pulsePerEbin_  = "p2d_pulsePerEbin" + mysubdetstr_;

    string titlestr;
    titlestr =
      "Average Pulse Shape vs E, "+mysubdetstr_+"; Sample Number; E_{hit} (GeV); fC";
    cutAH->book2d<TProfile2D>(st_pulsePerEbin_, titlestr.c_str(),
			      10,-0.5,9.5, nEbins, digEbins);

    if (cutit->second->doInverted()) {
      myAnalHistos *invAH = cutit->second->invhistos();
      invAH->book1d<TH1F>      (v_hpars1d);
      invAH->book1d<TH1F>      (v_hpars1d_af);
      invAH->book1d<TProfile>  (v_hpars1dprof);
      invAH->book1d<TProfile>  (v_hpars1dprof_af);
      invAH->book2d<TH2F>      (v_hpars2d);
      invAH->book2d<TH2F>      (v_hpars2d_af);
      invAH->book2d<TProfile2D>(v_hpars2dprof);
      invAH->book2d<TProfile2D>(v_hpars2dprof_af);
      if (mysubdet_==HcalForward)
	bookHFbasicProfiles(invAH);
      if ((mysubdet_ != HcalOuter) &&
	  (mysubdet_ != HcalOther)   ) {
	TProfile2D*tp =invAH->get<TProfile2D>(st_rhTprofRBX_.c_str());
	TH1F      *h1f=invAH->get<TH1F>      (st_rhFlagBits_.c_str());
	if(tp) labelSubdetAxis(*tp);
	if(h1f)labelHitFlagAxis(*h1f,mysubdet_);
      }

      titlestr =
	"Average Pulse Shape vs E, "+mysubdetstr_+"; Sample Number; E_{hit} (GeV); fC";
      invAH->book2d<TProfile2D>(st_pulsePerEbin_, titlestr.c_str(),
				10,-0.5,9.5, nEbins, digEbins);
    }

    if (cutit->second->flagSet(st_fillDetail_))
      bookDetailHistos4cut(*(cutit->second));
      
  } // cut loop

  cout<<"Done."<<std::endl;
}                           // HcalTimingAnalAlgos::bookHistos4allCuts

//======================================================================

void
HcalTimingAnalAlgos::bookHFbasicProfiles(myAnalHistos *myAH)
{
  /*****************************************
   * HF 2-D POLAR PROFILES:                *
   *****************************************/
  std::string titlestr;

  // Variable size binned histos for HF
  //
  st_rhTprofHFPd1_    = "p2d_rhTcorPolarHFPd1";
  st_rhTprofHFPd2_    = "p2d_rhTcorPolarHFPd2";
  st_rhTprofHFMd1_    = "p2d_rhTcorPolarHFMd1";
  st_rhTprofHFMd2_    = "p2d_rhTcorPolarHFMd2";

  st_rhEmapHFPd1_     = "p2d_rhEmapPolarHFPd1";
  st_rhEmapHFPd2_     = "p2d_rhEmapPolarHFPd2";
  st_rhEmapHFMd1_     = "p2d_rhEmapPolarHFMd1";
  st_rhEmapHFMd2_     = "p2d_rhEmapPolarHFMd2";

  st_rhOccMapHFPd1_   = "h2d_rhOccMapPolarHFPd1";
  st_rhOccMapHFPd2_   = "h2d_rhOccMapPolarHFPd2";
  st_rhOccMapHFMd1_   = "h2d_rhOccMapPolarHFMd1";
  st_rhOccMapHFMd2_   = "h2d_rhOccMapPolarHFMd2";
    
  // variable bin timing maps for HF
  myAH->book2d<TProfile2D>(st_rhTprofHFPd1_,
			   "RecHit Timing, HFP Depth 1; radius; i#phi",
			   28, hftwrEdges,36, 0.0, 6.28);
  myAH->book2d<TProfile2D>(st_rhTprofHFPd2_,
			   "RecHit Timing, HFP Depth 2; radius; i#phi",
			   28, hftwrEdges,36, 0.0, 6.28);
  myAH->book2d<TProfile2D>(st_rhTprofHFMd1_,
			   "RecHit Timing, HFM Depth 1; radius; i#phi",
			   28, hftwrEdges,36, 0.0, 6.28);
  myAH->book2d<TProfile2D>(st_rhTprofHFMd2_,
			   "RecHit Timing, HFM Depth 2; radius; i#phi",
			   28, hftwrEdges,36, 0.0, 6.28);

  titlestr = "RecHit Energy Map, HFP depth 1, "+rundescr_+"; radius; i#phi";
  myAH->book2d<TProfile2D>(st_rhEmapHFPd1_,titlestr.c_str(),
			   28, hftwrEdges,36, 0.0, 6.28);
  
  titlestr = "RecHit Energy Map, HFP depth 2, "+rundescr_+"; radius; i#phi";
  myAH->book2d<TProfile2D>(st_rhEmapHFPd2_,titlestr.c_str(),
			   28, hftwrEdges,36, 0.0, 6.28);
  
  titlestr = "RecHit Energy Map, HFM depth 1, "+rundescr_+"; radius; i#phi";
  myAH->book2d<TProfile2D>(st_rhEmapHFMd1_,titlestr.c_str(),
			   28, hftwrEdges,36, 0.0, 6.28);
  
  titlestr = "RecHit Energy Map, HFM depth 2, "+rundescr_+"; radius; i#phi";
  myAH->book2d<TProfile2D>(st_rhEmapHFMd2_,titlestr.c_str(),
			   28, hftwrEdges,36, 0.0, 6.28);
  
  titlestr = "RecHit Occupancy Map, HFP depth 1, "+rundescr_+"; radius; i#phi";
  myAH->book2d<TH2F>(st_rhOccMapHFPd1_,titlestr.c_str(),
		     28, hftwrEdges,36, 0.0, 6.28);
  
  titlestr = "RecHit Occupancy Map, HFP depth 2, "+rundescr_+"; radius; i#phi";
  myAH->book2d<TH2F>(st_rhOccMapHFPd2_,titlestr.c_str(),
		     28, hftwrEdges,36, 0.0, 6.28);
  
  titlestr = "RecHit Occupancy Map, HFM depth 1, "+rundescr_+"; radius; i#phi";
  myAH->book2d<TH2F>(st_rhOccMapHFMd1_,titlestr.c_str(),
		     28, hftwrEdges,36, 0.0, 6.28);
    
  titlestr = "RecHit Occupancy Map, HFM depth 2, "+rundescr_+"; radius; i#phi";
  myAH->book2d<TH2F>(st_rhOccMapHFMd2_,titlestr.c_str(),
		     28, hftwrEdges,36, 0.0, 6.28);

  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars1d_af;

  // I want these for all cuts, just for HF
  //
  titlestr = "RecHit Energies, "+mysubdetstr_+"D1, "+rundescr_+"; Rechit Energy (GeV)";
  add1dAFhisto("h1d_RHEnergies" + mysubdetstr_+"D1",titlestr,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       (void *)&hitenergy_,NULL,(detIDfun_t)&isDepth1,v_hpars1d_af);

  titlestr = "RecHit Energies, "+mysubdetstr_+"D2, "+rundescr_+"; Rechit Energy (GeV)";
  add1dAFhisto("h1d_RHEnergies" + mysubdetstr_+"D2",titlestr,
	       recHitEscaleNbins_,recHitEscaleMinGeV_,recHitEscaleMaxGeV_,
	       (void *)&hitenergy_,NULL,(detIDfun_t)&isDepth2,v_hpars1d_af);

  myAH->book1d<TH1F>(v_hpars1d_af);

}                            // HcalTimingAnalAlgos::bookHFbasicProfiles

//======================================================================

void
HcalTimingAnalAlgos::bookD1D2detail(myAnalHistos *myAH)
{
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars1d_af;
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars1dprof_af;
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars2dprof_af;

  std::string titlestr;

  st_avgTimePerPhid1_  = "p1d_avgTimePerPhid1"     + mysubdetstr_;
  st_avgTimePerPhid2_  = "p1d_avgTimePerPhid2"     + mysubdetstr_;
  st_rhTcorProfd1_     = "p2d_rhTcorPerIetaIphiD1" + mysubdetstr_;
  st_rhTcorProfd2_     = "p2d_rhTcorPerIetaIphiD2" + mysubdetstr_;
  st_avgTcorPerIetad1_ = "p1d_avgTcorPerIetad1"    + mysubdetstr_;
  st_avgTcorPerIetad2_ = "p1d_avgTcorPerIetad2"    + mysubdetstr_;
  st_avgTimePerRMd1_   = "p1d_avgTimePerRMd1"      + mysubdetstr_;

  std::string st_avgTuncPerIetad1_ = "p1d_avgTuncPerIetad1"    + mysubdetstr_;
  std::string st_avgTuncPerIetad2_ = "p1d_avgTuncPerIetad2"    + mysubdetstr_;
  std::string st_avgTimePerRMd2_   = "p1d_avgTimePerRMd2"      + mysubdetstr_;
  std::string st_rhTuncProfd1_     = "p2d_rhTuncPerIetaIphiD1" + mysubdetstr_;
  std::string st_rhTuncProfd2_     = "p2d_rhTuncPerIetaIphiD2" + mysubdetstr_;

  /*****************************************
   * 1-D HISTOGRAMS:                       *
   *****************************************/

  add1dAFhisto("h1d_rhCorTimesD1" + mysubdetstr_,
	       "Depth 1 RecHit Times, "+mysubdetstr_+" "+rundescr_+"; Rechit Time (ns)",
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&corTime_,NULL,(detIDfun_t)&isDepth1,v_hpars1d_af);

  add1dAFhisto("h1d_rhCorTimesD2" + mysubdetstr_,
	       "Depth 2 RecHit Times, "+mysubdetstr_+" "+rundescr_+"; Rechit Time (ns)",
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,
	       (void *)&corTime_,NULL,(detIDfun_t)&isDepth2,v_hpars1d_af);

  /*****************************************
   * 1-D PROFILES:                         *
   *****************************************/

  titlestr =  "Avg. Time, "+mysubdetstr_+" Depth 1 ( "+rundescr_+"); i#phi; Time (ns)";
  add1dAFhisto( st_avgTimePerPhid1_,titlestr, 145,-72.5, 72.5,
		(void *)&fiphisigned_,(void *)&corTime_,(detIDfun_t)&isDepth1,v_hpars1dprof_af);

  titlestr =  "Avg. Time, "+mysubdetstr_+" Depth 2 ( "+rundescr_+"); i#phi; Time (ns)";
  add1dAFhisto( st_avgTimePerPhid2_, titlestr, 145,-72.5, 72.5,
		(void *)&fiphisigned_,(void *)&corTime_,(detIDfun_t)&isDepth2,v_hpars1dprof_af);
  titlestr = 
    "Avg. Time (Uncorrected), "+mysubdetstr_+" Depth 1 ( "+rundescr_+"); i#eta; Time (ns)";
  add1dAFhisto( st_avgTuncPerIetad1_, titlestr, 83,-41.5, 41.5,
		(void *)&fieta_,(void *)&hittime_,(detIDfun_t)&isDepth1,v_hpars1dprof_af);
  titlestr = 
    "Avg. Time (Uncorrected), "+mysubdetstr_+" Depth 2 ( "+rundescr_+"); i#eta; Time (ns)";
  add1dAFhisto( st_avgTuncPerIetad2_, titlestr, 83,-41.5, 41.5,
		(void *)&fieta_,(void *)&hittime_,(detIDfun_t)&isDepth2,v_hpars1dprof_af);

  titlestr =  "Avg. Time, "+mysubdetstr_+" Depth 1 ( "+rundescr_+"); i#eta; Time (ns)";
  add1dAFhisto( st_avgTcorPerIetad1_, titlestr, 83,-41.5, 41.5,
		(void *)&fieta_,(void *)&corTime_,(detIDfun_t)&isDepth1,v_hpars1dprof_af);
  
  titlestr =  "Avg. Time, "+mysubdetstr_+" Depth 2 ( "+rundescr_+"); i#eta; Time (ns)";
  add1dAFhisto( st_avgTcorPerIetad2_, titlestr, 83,-41.5, 41.5,
		(void *)&fieta_,(void *)&corTime_,(detIDfun_t)&isDepth2,v_hpars1dprof_af);

  titlestr =  "Avg. Time, "+mysubdetstr_+" Depth 1 ( "+rundescr_+"); i#RM; Time (ns)";
  add1dAFhisto( st_avgTimePerRMd1_, titlestr, 145,-72.5, 72.5, 
		(void *)&fRM_,(void *)&corTime_,(detIDfun_t)&isDepth1,v_hpars1dprof_af);

  titlestr =  "Avg. Time, "+mysubdetstr_+" Depth 2 ( "+rundescr_+"); i#RM; Time (ns)";
  add1dAFhisto( st_avgTimePerRMd2_,titlestr, 145,-72.5, 72.5, 
		(void *)&fRM_,(void *)&corTime_,(detIDfun_t)&isDepth2,v_hpars1dprof_af);

  /*****************************************
   * 2-D PROFILES:                         *
   *****************************************/

  titlestr =
    "Depth 1 RecHit Time Map (uncorrected), "+mysubdetstr_+" "+rundescr_+"; i#eta; i#phi";
  add2dAFhisto(st_rhTuncProfd1_, titlestr, 83, -41.5,  41.5, 72, 0.5, 72.5, 
	       (void *)&fieta_,(void *)&fiphi_,(void *)&hittime_,
	       (detIDfun_t)&isDepth1,v_hpars2dprof_af);

  titlestr =
    "Depth 2 RecHit Time Map (uncorrected), "+mysubdetstr_+" "+rundescr_+"; i#eta; i#phi";
  add2dAFhisto(st_rhTuncProfd2_, titlestr, 83, -41.5,  41.5, 72,   0.5,  72.5,
	       (void *)&fieta_,(void *)&fiphi_,(void *)&hittime_,
	       (detIDfun_t)&isDepth2,v_hpars2dprof_af);

  titlestr = "Depth 1 RecHit Time Map, "+mysubdetstr_+" "+rundescr_+"; i#eta; i#phi";
  add2dAFhisto(st_rhTcorProfd1_, titlestr,  83, -41.5,  41.5, 72, 0.5, 72.5,
	       (void *)&fieta_,(void *)&fiphi_,(void *)&corTime_,
	       (detIDfun_t)&isDepth1,v_hpars2dprof_af);

  titlestr = "Depth 2 RecHit Time Map, "+mysubdetstr_+" "+rundescr_+"; i#eta; i#phi";
  add2dAFhisto(st_rhTcorProfd2_, titlestr,  83, -41.5,  41.5, 72, 0.5, 72.5,
	       (void *)&fieta_,(void *)&fiphi_,(void *)&corTime_,
	       (detIDfun_t)&isDepth2,v_hpars2dprof_af);

  if (mysubdet_ != HcalForward) {
    std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof;
    st_rhTprofd1hpdMult_  = "p2d_rhTperIetaIphiDepth1hpdMult";
    titlestr = 
      "HBHE (Depth 1) RecHit Time Map-Profile for HPD Noise events " + rundescr_ + "; ieta; iphi";
    add2dHisto(st_rhTprofd1hpdMult_, titlestr,
	       83,-41.5, 41.5, 72, 0.5, 72.5,v_hpars2dprof);
    myAH->book2d<TProfile2D>(v_hpars2dprof);
  }

  myAH->book1d<TH1F>      (v_hpars1d_af);
  myAH->book1d<TProfile>  (v_hpars1dprof_af);
  myAH->book2d<TProfile2D>(v_hpars2dprof_af);

}                                 // HcalTimingAnalAlgos::bookD1D2detail

//======================================================================

void
HcalTimingAnalAlgos::bookHEdetail(myAnalHistos *myAH)
{
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars1dprof_af;
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars2dprof_af;

  st_rhTcorProfd3_     = "p2d_rhTcorPerIetaIphiD3HE";
  st_avgTcorPerIetad3_ = "p1d_avgTcorPerIetad3HE";
  st_avgTimePerPhid3_  = "p1d_avgTimePerPhid3HE";

  //
  // Detail histos for the last cut only
  //
  add1dAFhisto( "p1d_avgTimePerRMd3HE",
		"Avg. Time (Depth 3), HE, "+rundescr_+"; iRM; Time (ns)",
		145,-72.5, 72.5,
		(void *)&fRM_,(void *)&corTime_,(detIDfun_t)&isDepth3,v_hpars1dprof_af);

  add1dAFhisto( st_avgTimePerPhid3_,
		"Avg. Time (Depth 3), HE, "+rundescr_+"; i#phi; Time (ns)",
		145,-72.5, 72.5,
		(void *)&fiphisigned_,(void *)&corTime_,(detIDfun_t)&isDepth3,v_hpars1dprof_af);

  add1dAFhisto( "p1d_avgTuncPerIetad3HE",
		"Avg. Time (Depth 3), HE, "+rundescr_+"; i#eta; Time (ns)",
		61,-30.5, 30.5,
		(void *)&fieta_,(void *)&hittime_,(detIDfun_t)&isDepth3,v_hpars1dprof_af);

  add1dAFhisto( st_avgTcorPerIetad3_,
		"Avg. Time (Depth 3), HE, "+rundescr_+"; i#eta; Time (ns)",
		61,-30.5, 30.5,
		(void *)&fieta_,(void *)&corTime_,(detIDfun_t)&isDepth3,v_hpars1dprof_af);

  add2dAFhisto("p2d_rhTuncPerIetaIphiD3HE",
	       "Depth 3 RecHit Time Map (uncorrected), HE, "+rundescr_+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       (void *)&fieta_,(void *)&fiphi_,(void *)&hittime_,
	       (detIDfun_t)&isDepth3,v_hpars2dprof_af);

  add2dAFhisto(st_rhTcorProfd3_,
	       "Depth 3 RecHit Time Map, HE, "+rundescr_+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       (void *)&fieta_,(void *)&fiphi_,(void *)&corTime_,
	       (detIDfun_t)&isDepth3,v_hpars2dprof_af);

  myAH->book1d<TProfile>  (v_hpars1dprof_af);
  myAH->book2d<TProfile2D>(v_hpars2dprof_af);

}                                   // HcalTimingAnalAlgos::bookHEdetail

//======================================================================

void
HcalTimingAnalAlgos::bookHOdetail(myAnalHistos *myAH)
{
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars1dprof_af;
  std::vector<myAnalHistos::HistoAutoFill_t> v_hpars2dprof_af;

  st_avgTimePerPhiRing2M_ = "p1d_avgTimePerPhiHORing2M";
  st_avgTimePerPhiRing1M_ = "p1d_avgTimePerPhiHORing1M";
  st_avgTimePerPhiRing0_  = "p1d_avgTimePerPhiHORing0";
  st_avgTimePerPhiRing1P_ = "p1d_avgTimePerPhiHORing1P";
  st_avgTimePerPhiRing2P_ = "p1d_avgTimePerPhiHORing2P";
  st_rhTcorProfd4_        = "p2d_rhTcorPerIetaIphiD4HO";

  add1dAFhisto( "p1d_avgTimePerRMd4HO",
		"Avg. Time/iRM, HO, "+rundescr_+"; iRM; Time (ns)",
		145,-72.5, 72.5,
		(void *)&fRM_,(void *)&corTime_,(detIDfun_t)&isDepth4,v_hpars1dprof_af);

  //==================== ...by Ring ====================

  add1dAFhisto( st_avgTimePerPhiRing0_,
		"Avg. Time/i#phi (YB0), HO, "+rundescr_+"; i#phi; Time (ns)",
		72, 0.5, 72.5,
		(void *)&fiphi_,(void *)&corTime_,(detIDfun_t)&isHO0,v_hpars1dprof_af);

  add1dAFhisto( st_avgTimePerPhiRing1M_,
		"Avg. Time/i#phi (YB-1), HO, "+rundescr_+"; i#phi; Time (ns)",
		72, 0.5, 72.5,
		(void *)&fiphi_,(void *)&corTime_,(detIDfun_t)&isHO1M,v_hpars1dprof_af);

  add1dAFhisto( st_avgTimePerPhiRing1P_,
		"Avg. Time/i#phi (YB+1), HO, "+rundescr_+"; i#phi; Time (ns)",
		72, 0.5, 72.5,
		(void *)&fiphi_,(void *)&corTime_,(detIDfun_t)&isHO1P,v_hpars1dprof_af);

  add1dAFhisto( st_avgTimePerPhiRing2M_,
		"Avg. Time/i#phi (YB-2), HO, "+rundescr_+"; i#phi; Time (ns)",
		72, 0.5, 72.5,
		(void *)&fiphi_,(void *)&corTime_,(detIDfun_t)&isHO2M,v_hpars1dprof_af);

  add1dAFhisto( st_avgTimePerPhiRing2P_,
		"Avg. Time/i#phi (YB+2), HO, "+rundescr_+"; i#phi; Time (ns)",
		72, 0.5, 72.5,
		(void *)&fiphi_,(void *)&corTime_,(detIDfun_t)&isHO2P,v_hpars1dprof_af);

  add1dAFhisto( "p1d_avgTuncPerIetad4HO",
		"Avg. Time/i#eta (Uncorrected), HO, "+rundescr_+"; i#eta; Time (ns)",
		61,-30.5, 30.5,
		(void *)&fieta_,(void *)&hittime_,(detIDfun_t)&isDepth4,v_hpars1dprof_af);

  add1dAFhisto( "p1d_avgTcorPerIetad4HO",
		"Avg. Time/i#eta, HO, "+rundescr_+"; i#eta; Time (ns)",
		61,-30.5, 30.5,
		(void *)&fieta_,(void *)&corTime_,(detIDfun_t)&isDepth4,v_hpars1dprof_af);

  add2dAFhisto("p2d_rhTuncPerIetaIphiD4HO",
	       "Depth 4 RecHit Time Map (uncorrected), HO, "+rundescr_+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       (void *)&fieta_,(void *)&fiphi_,(void *)&hittime_,
	       (detIDfun_t)&isDepth4,v_hpars2dprof_af);

  add2dAFhisto(st_rhTcorProfd4_,
	       "Depth 4 RecHit Time Map, HO, "+rundescr_+"; i#eta; i#phi",
	       83, -41.5,  41.5, 72,   0.5,  72.5,
	       (void *)&fieta_,(void *)&fiphi_,(void *)&corTime_,
	       (detIDfun_t)&isDepth4,v_hpars2dprof_af);

  add2dAFhisto("p2d_rhTperRBXD4HO",
	       "Depth 4 RecHit Time RBX Map, HO, "+rundescr_+"; ; iRBX",
	       6, -3.0,  3.0, 18,   0.5,  18.5,
	       (void *)&fsubdet_,(void *)&fRBX_,(void *)&corTime_,
	       (detIDfun_t)&isDepth4,v_hpars2dprof_af);

  add2dAFhisto(st_rhTprofRBX_,
	       "RecHit Time RBX Map, "+rundescr_+"; ; iRBX",
	       5, -2.5, 2.5, 12,   0.5,  12.5,
	       (void *)&fring_,(void *)&fRBX_,(void *)&corTime_,
	       NULL,v_hpars2dprof_af);

  myAH->book1d<TProfile>  (v_hpars1dprof_af);
  myAH->book2d<TProfile2D>(v_hpars2dprof_af);

  TProfile2D *tp = myAH->get<TProfile2D>(st_rhTprofRBX_.c_str());
  if (tp) labelHORingAxis(*tp);
}                                   // HcalTimingAnalAlgos::bookHOdetail

//======================================================================

void
HcalTimingAnalAlgos::bookPerChanDetail(myAnalHistos *myAH)
{
  std::vector<myAnalHistos::HistoParams_t> v_hpars1d;

  // make digi subfolders in the final cut folder
  myAH->mkSubdir<uint32_t>("_DigisfCperID");
  //myAH->mkSubdir<uint32_t>("_DigisfCvsEperID");
  myAH->mkSubdir<uint32_t>("_DigisGeVperID");
  //myAH->mkSubdir<uint32_t>("_DigisGeVvsEperID");
  myAH->mkSubdir<uint32_t>("_corTimesPerID");

  myAH->mkSubdir<uint32_t>("_frac2TSperID");
  myAH->mkSubdir<uint32_t>("_digisGeVperEvID");

  myAnalHistos *postAH = myAH->mkSubdir("_postAnal");

  // Time reco histos from average pulse shapes
  //
  st_rhTavgCorPlus_ = "h1d_rhTavgCor" + mysubdetstr_ + "P";
  add1dHisto( st_rhTavgCorPlus_,
"T_{reco} Distro from Avg. Pulse/Channel, "+mysubdetstr_+"P, "+rundescr_+"; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  st_rhTavgCorMinus_ = "h1d_rhTavgCor" + mysubdetstr_ + "M";
  add1dHisto( st_rhTavgCorMinus_,
"T_{reco} Distro from Avg. Pulse/Channel, "+mysubdetstr_+"M, "+rundescr_+"; Rechit Time (ns)",
	      recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_,v_hpars1d);

  postAH->book1d<TH1F>(v_hpars1d);

  if (mysubdet_==HcalForward) {

    myAH->mkSubdir<uint32_t>("_ts012perTS4charge");
    myAH->mkSubdir<uint32_t>("_ts3perTS4charge");
    myAH->mkSubdir<uint32_t>("_ts5perTS4charge");

    // times from average pulse shapes (booked above) determined in endJob
    std::string title;
    std::string basestr;
    std::vector<myAnalHistos::HistoParams_t> v_hpars2dprof;

    st_rhTavgCorProfHFPd1_ = "p2d_rhTavgCorProfHFPd1";
    st_rhTavgCorProfHFPd2_ = "p2d_rhTavgCorProfHFPd2";
    st_rhTavgCorProfHFMd1_ = "p2d_rhTavgCorProfHFMd1";
    st_rhTavgCorProfHFMd2_ = "p2d_rhTavgCorProfHFMd2";

    st_ts43ratioProfHFPd1_ = "p2d_ts43ratioProfHFPd1";
    st_ts43ratioProfHFPd2_ = "p2d_ts43ratioProfHFPd2";
    st_ts43ratioProfHFMd1_ = "p2d_ts43ratioProfHFMd1";
    st_ts43ratioProfHFMd2_ = "p2d_ts43ratioProfHFMd2";

    st_ts43ratioPolarProfHFPd1_ = "p2d_ts43ratioPolarProfHFPd1";
    st_ts43ratioPolarProfHFPd2_ = "p2d_ts43ratioPolarProfHFPd2";
    st_ts43ratioPolarProfHFMd1_ = "p2d_ts43ratioPolarProfHFMd1";
    st_ts43ratioPolarProfHFMd2_ = "p2d_ts43ratioPolarProfHFMd2";

    basestr  = " T_{reco} Map from Avg. Pulse/Channel, "+rundescr_+"("+myAH->name()+"); radius; #phi";

    title = "HFP Depth 1"+basestr;
    postAH->book2d<TProfile2D>(st_rhTavgCorProfHFPd1_, title.c_str(),
			       28, hftwrEdges,36, 0.0, 6.28);

    title = "HFP Depth 2"+basestr;
    postAH->book2d<TProfile2D>(st_rhTavgCorProfHFPd2_, title.c_str(),
			       28, hftwrEdges,36, 0.0, 6.28);

    title = "HFM Depth 1"+basestr;
    postAH->book2d<TProfile2D>(st_rhTavgCorProfHFMd1_, title.c_str(),
			       28, hftwrEdges,36, 0.0, 6.28);

    title = "HFM Depth 2"+basestr;
    postAH->book2d<TProfile2D>(st_rhTavgCorProfHFMd2_, title.c_str(),
			       28, hftwrEdges,36, 0.0, 6.28);
    
    basestr = " TS4/(TS3+TS4) Map from Average Pulse Shapes, "+rundescr_+"("+myAH->name()+");";

    title = "HFP Depth 1"+basestr+"i#eta; i#phi";
    add2dHisto(st_ts43ratioProfHFPd1_,title,13,28.5,41.5,72,0.5,72.5,v_hpars2dprof);
    title = "HFP Depth 1"+basestr+"meters; meters";
    postAH->book2d<TProfile2D>(st_ts43ratioPolarProfHFPd1_,title.c_str(),28,hftwrEdges,36,0.0,6.28);

    title = "HFP Depth 2"+basestr+"i#eta; i#phi";
    add2dHisto(st_ts43ratioProfHFPd2_,title,13,28.5,41.5,72,0.5,72.5,v_hpars2dprof);
    title = "HFP Depth 2"+basestr+"meters; meters";
    postAH->book2d<TProfile2D>(st_ts43ratioPolarProfHFPd2_,title.c_str(),28,hftwrEdges,36,0.0,6.28);

    title = "HFM Depth 1"+basestr+"i#eta; i#phi";
    add2dHisto(st_ts43ratioProfHFMd1_,title,13,-41.5,-28.5,72,0.5,72.5,v_hpars2dprof);
    title = "HFM Depth 1"+basestr+"meters; meters";
    postAH->book2d<TProfile2D>(st_ts43ratioPolarProfHFMd1_, title.c_str(),28,hftwrEdges,36,0.0,6.28);

    title = "HFM Depth 2"+basestr+"i#eta; i#phi";
    add2dHisto(st_ts43ratioProfHFMd2_,title,13,-41.5,-28.5,72,0.5,72.5,v_hpars2dprof);
    title = "HFM Depth 2"+basestr+"meters; meters";
    postAH->book2d<TProfile2D>(st_ts43ratioPolarProfHFMd2_, title.c_str(),28,hftwrEdges,36,0.0,6.28);

    postAH->book2d<TProfile2D>(v_hpars2dprof);
  }
}                              // HcalTimingAnalAlgos::bookPerChanDetail

//======================================================================

void
HcalTimingAnalAlgos::bookDetailHistos4cut(myAnalCut& cut)
{
  cout<<"Detail histos for cut "<<cut.description()<<std::endl;

  bool doinv = cut.doInverted();

  myAnalHistos *cutAH = cut.cuthistos();
  myAnalHistos *invAH = cut.invhistos();

  switch (mysubdet_) {
  case HcalOuter:   bookHOdetail(cutAH);  if (doinv) bookHOdetail(invAH);   break;
  case HcalEndcap:  bookHEdetail(cutAH);  if (doinv) bookHEdetail(invAH); //no break!
  case HcalBarrel:  
  case HcalForward: bookD1D2detail(cutAH);if (doinv) bookD1D2detail(invAH); break;
  default:break;
  }

  if (cut.flagSet(st_doPerChannel_))
    bookPerChanDetail(cutAH); // but not for the inverted!!

  cout<<"Done."<<std::endl;
}                           // HcalTimingAnalAlgos::bookDetailHistos4cut

//======================================================================
inline int    sign   (double x) { return (x>=0) ? 1 : -1; }

void
HcalTimingAnalAlgos::fillHistos4cut(myAnalCut& thisCut)
{
  //edm::LogInfo("Filling histograms for subdet ") << mysubdetstr_ << std::endl;

  myAnalHistos *myAH;
  if (thisCut.isActive()) {
    if (thisCut.doInverted()) { myAH = thisCut.invhistos(); }
    else                       return;
  } else
    myAH = thisCut.cuthistos();

  int  absieta = detID_.ietaAbs();
  int    zside = detID_.zside();
  int    depth = detID_.depth();

  fieta_    = (float)detID_.ieta();
  fiphi_    = (float)detID_.iphi();

  float frac2ts = 0.;

  if (mysubdet_ == HcalOther) {
    zside = zdcDetID_.zside();
  }

  // need front end id: 
  int iRBX    = atoi(((feID_.rbx()).substr(3,2)).c_str());
  fRBX_       = (float)iRBX;
  int        iRMinRBX = feID_.rm();
  //int   ipix = feID_.pixel();
  fRM_         = (float)(zside * ((iRBX-1)*4 + iRMinRBX));
  fiphisigned_ = zside*fiphi_;
    

  fsubdet_ = fring_ = 0.;
  switch (mysubdet_) {
  case HcalBarrel:  fsubdet_ = zside*0.5; break;
  case HcalEndcap:  fsubdet_ = zside*1.5; break;
  case HcalForward: fsubdet_ = zside*2.5; break;
  case HcalOuter:   fring_   = zside*atoi(((feID_.rbx()).substr(2,1)).c_str());
  default: break;
  }

  /* Takes care of most histos! */
  if (mysubdet_ == HcalOther) myAH->setCurDetId(zdcDetID_);
  else                        myAH->setCurDetId(detID_);

  // make sure all autofill variables are set by this point!
  myAH->autofill<float>();

  if ((mysubdet_ == HcalBarrel)||
      (mysubdet_ == HcalEndcap)||
      (mysubdet_ == HcalForward) ) {
    // global rechit info
    for (int ibit=0; ibit<32; ibit++) {
      int flagshift = (hitflags_>>ibit);
      if (mysubdet_ != HcalForward)
	if (ibit==8) {
	  int timingCutQuality = flagshift & 7;
	  myAH->fill1d<TH1F>(st_rhHBHEtimingShapedCuts_,timingCutQuality);
	}
      if (flagshift & 1) {
	myAH->fill1d<TH1F>(st_rhFlagBits_,ibit+1);
      }
    }

    frac2ts = (float)((hitflags_>>((int)HcalCaloFlagLabels::Fraction2TS))&0x3f);

    if  (mysubdet_==HcalForward) {
      fangle_   = TMath::Pi()*(fiphi_-1.)/36.;
      fradius_  = hftwrRadii[41-absieta];

      //cout << ieta << " --> " << fradius_ << endl;
      switch (zside*depth) {
      case -2:
	myAH->fill2d<TProfile2D>(st_rhTprofHFMd2_,  fradius_,fangle_,corTime_);
	myAH->fill2d<TProfile2D>(st_rhEmapHFMd2_,   fradius_,fangle_,hitenergy_);
	myAH->fill2d<TProfile2D>(st_rhOccMapHFMd2_, fradius_,fangle_);
	break;
      case -1:
	myAH->fill2d<TProfile2D>(st_rhTprofHFMd1_,  fradius_,fangle_,corTime_);
	myAH->fill2d<TProfile2D>(st_rhEmapHFMd1_,   fradius_,fangle_,hitenergy_);
	myAH->fill2d<TProfile2D>(st_rhOccMapHFMd1_, fradius_,fangle_);
	break;
      case  1:
	myAH->fill2d<TProfile2D>(st_rhTprofHFPd1_,  fradius_,fangle_,corTime_);
	myAH->fill2d<TProfile2D>(st_rhEmapHFPd1_,   fradius_,fangle_,hitenergy_);
	myAH->fill2d<TProfile2D>(st_rhOccMapHFPd1_, fradius_,fangle_);
	break;
      case  2:
	myAH->fill2d<TProfile2D>(st_rhTprofHFPd2_,  fradius_,fangle_,corTime_);
	myAH->fill2d<TProfile2D>(st_rhEmapHFPd2_,   fradius_,fangle_,hitenergy_);
	myAH->fill2d<TProfile2D>(st_rhOccMapHFPd2_, fradius_,fangle_);
	break;
      }

      // Fraction2TS:
      frac2ts = (frac2ts-1.f)/50.f;

    } // is HF
    else { // is HBHE

      // Fraction2TS:
      frac2ts = 0.5f + ((frac2ts-1.f)/100.f);
    }

    myAH->fill2d<TH2F>(st_rhFraction2TSvsE_,hitenergy_,frac2ts);

  } // if HB/HE/HF


  CaloSamples filldigifC;
  if (doDigis_) {
    filldigifC = digifC_;
    int digisize = std::min(10,digifC_.size());

    if (normalizeDigis_) {
      double sum = 0.0;
      for (int its=0; its<digisize; ++its) sum += digifC_[its];
      if (sum > 0.0)  filldigifC *= 1.0/sum;
      
      // normalize the digi in GeV to the hitenergy that was reconstructed
      if (hitenergy_ != 0.0) 
	for (int its=0; its<digiGeV_.size(); its++) digiGeV_[its] /= hitenergy_;
    }

    TProfile *avgPulse = myAH->get<TProfile>(st_avgPulse_.c_str());
    for (int its=0; its<digisize; ++its)
      avgPulse->Fill(its,filldigifC[its]);

    // segregate +side/-side digis for timing comparison
    TProfile *avgPulsePerZside =
      myAH->get<TProfile>(((zside>0)?
			   st_avgPulsePlus_.c_str():
			   st_avgPulseMinus_.c_str()));

    for (int its=0; its<digisize; ++its)
      avgPulsePerZside->Fill(its,filldigifC[its]);

    TProfile2D *avgPulsePerE = myAH->get<TProfile2D>(st_pulsePerEbin_.c_str());
    for (int its=0; its<digisize; ++its)
      avgPulsePerE->Fill(its,hitenergy_,filldigifC[its]);

  }

  if (thisCut.flagSet(st_fillDetail_)) {
    //cout << detId << "\t" << feID.rbx() << "\t" << feID.rbx().substr(3,2) << "\t";
    //cout << feID.rm() << " maps to RBX/RM# " << iRBX << "/" << iRM << endl;

#if 0
    // per-channel histograms
    std::map<uint32_t,TH1F *>::const_iterator it=m_perChHistos_.find(denseId);
    if (it==m_perChHistos_.end())
      cerr << "No histo booked for detId " << detID_ << "!!!" << endl;
    else
      it->second->Fill(corTime_);
#endif

    // Now per channel...
    if (   // (myAH != thisCut.invhistos()) &&
	thisCut.flagSet(st_doPerChannel_)) {
      uint32_t     dix;
      stringstream name;

      if (mysubdet_ == HcalOther) {
	dix = zdcDetID_.denseIndex(); name << zdcDetID_;
      } else {
	dix = detID_.denseIndex(); name << detID_;
      }

      //========== Pulse shape per channel ==========

      bool perchOverThresh = ampCutsInfC_ ?
	(fCamplitude_ > std::max(25.0,minHitAmplitude_)) :
	(hitenergy_   > std::max(5.0,minHitAmplitude_));

      if (doDigis_ && perchOverThresh) {
	fillDigiPulseHistos(myAH->getAttachedHisto<uint32_t>("_digisfCperID"),
			    dix,name.str(),filldigifC);
	fillDigiPulseHistos(myAH->getAttachedHisto<uint32_t>("_digisGeVperID"),
			    dix,name.str(),digiGeV_);
	if  (mysubdet_==HcalForward)
	  fillTSdistros (myAH,dix,name.str());

	if (thisCut.flagSet(st_perEvtDigi_)) {
	  perChanHistos *digPerEvID = myAH->getAttachedHisto<uint32_t>("_digisGeVperEvID");
	  uint32_t dix = (digPerEvID->last() == digPerEvID->rend()) ? 0 : digPerEvID->last()->first;

	  stringstream evidname;
	  evidname << name.str() << " Run" << runnum_ << " Ev" << evtnum_;
	  fillDigiPulseHistos(digPerEvID,++dix,evidname.str(),digiGeV_);
	}
      }

      //========== Timing per channel ==========

      perChanHistos *rhperidHistos =
	myAH->getAttachedHisto<uint32_t>("_corTimesPerID");

      perChanHistos::HistoParams_t
	hpars1(name.str(),name.str(),
	       recHitTscaleNbins_,recHitTscaleMinNs_,recHitTscaleMaxNs_);

      TH1F *htcor = rhperidHistos->exists(dix) ? // Corrected hit time per channel
	rhperidHistos->get<TH1F>(dix) :
	rhperidHistos->book1d<TH1F>(dix,hpars1,false);

      if (htcor && perchOverThresh)
	htcor->Fill(corTime_);

      //========== Frac2TS per channel ==========

      perChanHistos *frac2tsHistos =
	myAH->getAttachedHisto<uint32_t>("_frac2TSperID");

      perChanHistos::HistoParams_t hpars2(name.str(),name.str(),101,0.495,1.505);

      TH1F *hfrac2ts = frac2tsHistos->exists(dix) ? // maxsum(2ts)/sum(10ts)
	frac2tsHistos->get<TH1F>(dix) :
	frac2tsHistos->book1d<TH1F>(dix,hpars2,false);

      if (hfrac2ts && perchOverThresh)
	hfrac2ts->Fill(frac2ts);

    } // if doPerChannel
  } // if filldetail
}                                 // HcalTimingAnalAlgos::fillHistos4cut

//======================================================================

void
HcalTimingAnalAlgos::fillDigiPulseHistos(perChanHistos *digiFolder,
					 uint32_t       hkey,
					 const std::string& name,
					 const CaloSamples& filldigi)
{
  perChanHistos::HistoParams_t hpars(name,name,10,-0.5,9.5);

  int digisize = std::min(10,filldigi.size());
  
  // pulses in fC
  if (!digiFolder) return;

  TProfile *hpulse = digiFolder->exists(hkey)   ?
    digiFolder->get<TProfile>(hkey) :
    digiFolder->book1d<TProfile>(hkey,hpars,false);

  if (hpulse) {
    for (int its=0; its<digisize; ++its)
      hpulse->Fill(its,filldigi[its]);
  }

#if 0
  // ...binned by energy...
  hpars.name  += "vsE";
  hpars.title += "; ; E_{hit}(GeV)";

  perChanHistos *digifCvsEhistos = myAH->getAttachedHisto<uint32_t>("_DigisfCvsEperID");

  TProfile2D *hpulsefCperE=digifCvsEhistos->exists(hkey)? // Digis in fC, binned energies
    digifCvsEhistos->get<TProfile2D>(hkey) :
    digifCvsEhistos->book2d<TProfile2D>
    (hkey,hpars.name,hpars.title.c_str(),10,-0.5,9.5,nEbins,digEbins,false);

  if (hpulsefCPerE) {
    for (int its=0; its<digisize; ++its)
      hpulsefCPerE->Fill(its, hitenergy_, filldigifC[its] );
  }

  perChanHistos *digiGeVvsEhistos = myAH->getAttachedHisto<uint32_t>("_DigisGeVvsEperID");

  TProfile2D *hpulseGeVperE=digiGeVvsEhistos->exists(hkey)? // Digis in GeV, binned energies
    digiGeVvsEhistos->get<TProfile2D>(hkey) :
    digiGeVvsEhistos->book2d<TProfile2D>
    (hkey,hpars.name,hpars.title.c_str(),10,-0.5,9.5,nEbins,digEbins,false);

  if (hpulseGeVPerE)
    for (unsigned its=0; its<digiGeV_.size(); ++its)
      hpulseGeVPerE->Fill(its, hitenergy_, digiGeV_[its] );
#endif
}                            // HcalTimingAnalAlgos::fillDigiPulseHistos

//======================================================================

void
HcalTimingAnalAlgos::fillTSdistros(myAnalHistos *myAH,
				   uint32_t      hkey,
				   const std::string& name)
{
  perChanHistos *ts012perTS4charge = myAH->getAttachedHisto<uint32_t>("_ts012perTS4charge");
  perChanHistos *ts3perTS4charge   = myAH->getAttachedHisto<uint32_t>("_ts3perTS4charge");
  perChanHistos *ts5perTS4charge   = myAH->getAttachedHisto<uint32_t>("_ts5perTS4charge");

  perChanHistos::HistoParams_t hpars(name,name, 41,-25.,2025.,151,-50.5,100.5);

  assert (ts012perTS4charge && ts3perTS4charge && ts5perTS4charge);

  TH2F *ts012 = ts012perTS4charge->exists(hkey) ?
    ts012perTS4charge->get<TH2F>(hkey) :
    ts012perTS4charge->book2d<TH2F>(hkey,hpars,false);

  TH2F *ts3 = ts3perTS4charge->exists(hkey) ?
    ts3perTS4charge->get<TH2F>(hkey) :
    ts3perTS4charge->book2d<TH2F>(hkey,hpars,false);

  TH2F *ts5 = ts5perTS4charge->exists(hkey) ?
    ts5perTS4charge->get<TH2F>(hkey) :
    ts5perTS4charge->book2d<TH2F>(hkey,hpars,false);

  ts012->Fill(digifC_[0],digifC_[4],0.3333);
  ts012->Fill(digifC_[1],digifC_[4],0.3333);
  ts012->Fill(digifC_[2],digifC_[4],0.3333);

  ts3->Fill(digifC_[3],digifC_[4]);
  ts5->Fill(digifC_[5],digifC_[4]);
}                                  // HcalTimingAnalAlgos::fillTSdistros

//======================================================================

// ------------ method called to for each event  ------------
void
HcalTimingAnalAlgos::process(const myEventData& ed)
{
  if (firstEvent_) {
    runnum_    = ed.runNumber();

    if (!rundescr_.size())
      rundescr_ = "Run "+int2str(runnum_);

    bookHistos4allCuts();
    firstEvent_ = false;
  }

  lsnum_  = ed.lumiSection();
  bxnum_  = ed.bxNumber();
  evtnum_ = ed.evtNumber();
  fevtnum_ = (float)evtnum_;

  // leave the rest to the child class?
}                                        // HcalTimingAnalAlgos::process

//======================================================================

void
HcalTimingAnalAlgos::beginJob(const edm::EventSetup& iSetup,
			      const     myEventData& ed)
{
  neventsProcessed_=0;

  edm::Service<TFileService> fs;
  
  if (ed.hbhedigis().isValid() ||
      ed.hodigis().isValid() ||
      ed.hfdigis().isValid() ||
      ed.zdcdigis().isValid()) {
    iSetup.get<HcalDbRecord>().get( conditions_ );
  }

  if (doTree_) {
    tree_ = fs->make<TTree>("mytree","Hcal Results Tree");
    tree_->Branch("feID",         &feID_,  32000, 1);
    tree_->Branch("detID",        &detID_, 32000, 1);
    tree_->Branch("bxnum",        &bxnum_, 32000, 1);
    tree_->Branch("lsnum",        &lsnum_, 32000, 1);
    tree_->Branch("evtnum",       &evtnum_,32000, 1);
    tree_->Branch("hittime",      &hittime_);
    tree_->Branch("hitenergy",    &hitenergy_);
    tree_->Branch("hitflags",     &hitflags_);
    tree_->Branch("corhittime_ns",&corTime_);
    tree_->Branch("correction_ns",&correction_ns_);
  }

  std::cout << "----------------------------------------"  << "\n" <<
  std::cout << "Parameters being used: "               << "\n" <<
    "subdet_            = "      << mysubdetstr_       << "\n" << 
    "globalToffset_     = "      << globalToffset_     << "\n" << 
    "globalFlagMask_    = "<<hex << globalFlagMask_    << "\n" << 
    "ampCutsInfC_       = "      << ampCutsInfC_       << "\n" << 
    "minHitAmplitude_   = "<<dec << minHitAmplitude_   << "\n" << 
    "maxHitAmplitude_   = "      << maxHitAmplitude_   << "\n" << 
    "normalizeDigis_    = "      << normalizeDigis_    << "\n" <<
    "doPerChannel_      = "      << doPerChannel_      << "\n" <<
    "doTree_            = "      << doTree_            << "\n" <<
    "minEvents4avgT_    = "      << minEvents4avgT_    << "\n" <<
    "firstsamp_         = "      << firstsamp_         << "\n" <<
    "nsamps_            = "      << nsamps_            << "\n" <<
    "presamples_        = "      << presamples_        << "\n" <<
#if 0
    "timeWindowMinNS_   = "      << timeWindowMinNS_   << "\n" << 
    "timeWindowMaxNS_   = "      << timeWindowMaxNS_   << "\n" <<
#endif

  std:: cout << "badEventSet_      = "<<dec;
  std::set<int>::const_iterator it;
  for (it=badEventSet_.begin(); it!=badEventSet_.end(); it++)
    std::cout << *it << ",";
  std::cout << std::endl;

  std::cout << "acceptedBxNums_   = ";

  for (it=acceptedBxNums_.begin(); it!=acceptedBxNums_.end(); it++)
    std::cout << *it << ",";
  std::cout << std::endl;
  std::cout << "----------------------------------------" << std::endl;

}                                       // HcalTimingAnalAlgos::beginJob

//======================================================================

void
HcalTimingAnalAlgos::computeChannelTimingFromAvgDigis(myAnalHistos *myAH)
{
  myAnalHistos *postAH = myAH->getAttachedHisto("_postAnal");

  TH1F *h1plus =postAH->get<TH1F>(st_rhTavgCorPlus_.c_str());
  TH1F *h1minus=postAH->get<TH1F>(st_rhTavgCorMinus_.c_str());

  //perChanHistos *digiGeVHistos = myAH->getAttachedHisto<uint32_t>("_DigisGeVperID");
  perChanHistos *digifCHistos  = myAH->getAttachedHisto<uint32_t>("_DigisfCperID");

  perChanHistos::iterator it;
  //for (it = digiGeVHistos->begin(); it != digiGeVHistos->end(); it++) {
  for (it = digifCHistos->begin(); it != digifCHistos->end(); it++) {
    int zside,depth,absieta;
    float fieta,fiphi;
    float fradius,fangle;
    float calibcorr;
    if (mysubdet_ == HcalOther) {
      HcalZDCDetId detID = HcalZDCDetId::detIdFromDenseIndex(it->first);
      zside              = detID.zside();
      depth              = detID.depth();
      calibcorr          = (conditions_->getHcalCalibrations(detID)).timecorr();
    }else{
      HcalDetId detID = HcalDetId::detIdFromDenseIndex(it->first);
      zside           = detID.zside();
      depth           = detID.depth();
      absieta         = detID.ietaAbs();
      fieta           = (float)detID.ieta();
      fiphi           = (float)detID.iphi();
      calibcorr       = (conditions_->getHcalCalibrations(detID)).timecorr();
    }
    TProfile *tp = (TProfile *)it->second;
    if ((tp->GetEntries()/10) < minEvents4avgT_) continue;
    float tRecoFromAvgPulse = 0.0;
    if ((mysubdet_ == HcalForward) ||
	(mysubdet_ == HcalOther)     )
      tRecoFromAvgPulse = recoTimeFromAvgPulseHF(tp) - calibcorr;

    tRecoFromAvgPulse -= globalToffset_;

    if (zside > 0) h1plus->Fill(tRecoFromAvgPulse);
    else          h1minus->Fill(tRecoFromAvgPulse);

    if (mysubdet_ == HcalForward) {
      // Find the map to put the result in
      std::string st_rhTavgCorProf, st_ts43ratioProf, st_ts43ratioPolarProf;
      switch (zside*depth) {
      case -2:
	st_rhTavgCorProf=st_rhTavgCorProfHFMd2_;
	st_ts43ratioProf=st_ts43ratioProfHFMd2_;
	st_ts43ratioPolarProf=st_ts43ratioPolarProfHFMd2_;
	break;
      case -1:
	st_rhTavgCorProf=st_rhTavgCorProfHFMd1_;
	st_ts43ratioProf=st_ts43ratioProfHFMd1_;
	st_ts43ratioPolarProf=st_ts43ratioPolarProfHFMd1_;
	break;
      case  1:
	st_rhTavgCorProf=st_rhTavgCorProfHFPd1_;
	st_ts43ratioProf=st_ts43ratioProfHFPd1_;
	st_ts43ratioPolarProf=st_ts43ratioPolarProfHFPd1_;
	break;
      case  2:
	st_rhTavgCorProf=st_rhTavgCorProfHFPd2_;
	st_ts43ratioProf=st_ts43ratioProfHFPd2_;
	st_ts43ratioPolarProf=st_ts43ratioPolarProfHFPd2_;
	break;
      default: break;
      }

      fangle   = TMath::Pi()*(fiphi-1.)/36.;
      fradius  = hftwrRadii[41-absieta];

      postAH->fill2d<TProfile2D>(st_rhTavgCorProf,fradius,fangle,tRecoFromAvgPulse);

      float ratio43= 0.0f;
      float ts3 = tp->GetBinContent(4);
      float ts4 = tp->GetBinContent(5);

      if ((ts3+ts4) > 0.0f)
	ratio43 = ts4/(ts3+ts4);
      else
	ratio43 = -1.0; 

      postAH->fill2d<TProfile2D>(st_ts43ratioProf,fieta,fiphi,ratio43);
      postAH->fill2d<TProfile2D>(st_ts43ratioPolarProf,fradius,fangle,ratio43);

    } // if HF
  } // digi histo loop

}               // HcalTimingAnalAlgos::computeChannelTimingFromAvgDigis

//======================================================================

void
HcalTimingAnalAlgos::compilePerTimeSampleStatistics(myAnalHistos *myAH)
{
#if 0
  perChanHistos *ts012perTS4charge = myAH->getAttachedHisto<uint32_t("_ts012perTS4charge");
  perChanHistos *ts3perTS4charge   = myAH->getAttachedHisto<uint32_t("_ts3perTS4charge");
  perChanHistos *ts5perTS4charge   = myAH->getAttachedHisto<uint32_t("_ts5perTS4charge");

  perChanHistos::iterator it = ts012perTS4charge->begin();

  myAnalHistos *postAH = myAH->getAttachedHisto("_postAnal");

  TH2F *h2f = (TH2F *)it->second;

  // average over all channels
  TH2F *ts012vsts4avg =  myAH->bookClone<TH2F>("ts012vsts4avg", *h2f);
  myAnalHistos::HistoParams_t
    hpars("ts012meanVsE",
	  "TS0,1,or 2 fC per-channel means vs TS4 fC; TS4 (fC); TS0,1, or 2 (fC)",
	  200, recHitEscaleMinfC_, recHitEscaleMaxfC_);
				    
  TProfile *ts012meanVsE = postAH->book1d<TProfile>(hpars);
  

  for (++it; it != ts012perTS4charge->end(); it++) {
    TH2F *h2f = (TH2F *)it->second;

    ts012vsts4avg->Add(*h2f);
  }
#endif
}                 // HcalTimingAnalAlgos::compilePerTimeSampleStatistics

//======================================================================

void
HcalTimingAnalAlgos::endJob()
{
  cout << neventsProcessed_ << " events processed for subdet " << mysubdetstr_ << endl;
  if (!neventsProcessed_) return;

  myAnalHistos *lastAH = getCut(st_lastCut_)->cuthistos();

  if (mysubdet_ == HcalBarrel) {
    TProfile  *tp=lastAH->get<TProfile>(st_avgTcorPerIetad1_.c_str());
    float TetaMinus1 = tp->GetBinContent(tp->GetXaxis()->FindBin(-1));
    float TetaPlus1  = tp->GetBinContent(tp->GetXaxis()->FindBin( 1));

    cout << "Global offset calculated from HB center is: ";
    cout << (TetaMinus1+TetaPlus1)/2. << endl;
  }

  if (doPerChannel_ && doDigis_) {
    computeChannelTimingFromAvgDigis(lastAH);
    if (mysubdet_ == HcalForward)
      compilePerTimeSampleStatistics(lastAH);
  }
}                                         // HcalTimingAnalAlgos::endJob
