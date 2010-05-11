// -*- C++ -*-
//
// Package:    HFsignal2noiseAnalyzer
// Class:      HFsignal2noiseAnalyzer
// 
/**\class HFsignal2noiseAnalyzer HFsignal2noiseAnalyzer.cc MyEDmodules/HFsignal2noiseAnalyzer/src/HFsignal2noiseAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Phillip Russell Dudero,40 R-A08,+41227675312,
//         Created:  Fri May  7 18:09:05 CEST 2010
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"

#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"

#include "CalibFormats/CaloObjects/interface/CaloSamples.h"
#include "MyEDmodules/MyAnalUtilities/interface/myEventData.hh"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "HFShape.h"
#include "TH2F.h"
#include "TRandom.h"

#define BINSIZE_NS         25.0
#define INT_DELTA_NS        0.0025
#define SHIFT_SCALE_NS      0.0025
#define NUM_SHIFTS      10000
#define NUM_BINS          100

//
// class declaration
//

class HFsignal2noiseAnalyzer : public edm::EDAnalyzer {
public:
  explicit HFsignal2noiseAnalyzer(const edm::ParameterSet&);
  ~HFsignal2noiseAnalyzer();


private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
  double       determine5050minus7phase(void);
  double       determineAmpScale(double t0);
  void         addSignalAndHisto(float ts5noiseSample, float jitterPhase);
  void         processDigis(const edm::Handle<HFDigiCollection>& hfdigihandle);

      // ----------member data ---------------------------
  typedef   HFDigiCollection::const_iterator HFDigiIt;

  edm::ESHandle<HcalDbService>  conditions_;
  myEventData *eventData_;
  bool         firstEvent_;
  int          numdigis_;
  TRandom      myRandom_;

  TH1F *h1d_jitterPhaseNS_;
  TH1F *h1d_noiseSOIplus1fC_;
  TH2F *h2d_signalSOIfC_;
  TH2F *h2d_signalSOIplus1fC_;
  TH2F *h2d_signalSOIminus1fC_;
  TH2F *h2d_ts5sigPlusNoiseVsAmp_;
  TH2F *h2d_ts5sigPlusNoiseVsNormAmp_;
};

typedef double (*DoubleFunc_t)(double);
double integrate(double xmin, double xmax,
		 double stepsize, DoubleFunc_t func2integrate) {
  int    nsteps = 0;
  double intval = 0.0;
  for (double x = xmin; x < xmax; x+= stepsize) {
    double loedge = func2integrate(x);
    double hiedge = func2integrate(x+stepsize);
    intval += (loedge+hiedge)*stepsize/2.0;
    nsteps++;
  }

  return intval;
}

HFShape HFshape;
float pulseAmpScale_;
double scaledHFshape(double time) {return pulseAmpScale_*HFshape(time);}

//
// constants, enums and typedefs
//

//
// static data member definitions
//

static const double jitterRMS_ns  = 1.0;
static const double jitterMean_ns = 0.0;

//
// constructors and destructor
//
HFsignal2noiseAnalyzer::HFsignal2noiseAnalyzer(const edm::ParameterSet& iConfig)
{
  edm::ParameterSet edPset =
    iConfig.getUntrackedParameter<edm::ParameterSet>("eventDataPset");

  eventData_ = new myEventData(edPset);
}


HFsignal2noiseAnalyzer::~HFsignal2noiseAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}

// this is actually the HBHE table. HF values are roughly 2.6x
//
static const float gl_inv_conversionTable[128] = {
  -0.5f,0.5f,1.5f,2.5f,3.5f,4.5f,5.5f,6.5f,7.5f,8.5f,9.5f,10.5f,11.5f,12.5f,13.5f,
  15.0f,17.0f,19.0f,21.0f,23.0f,25.0f,27.0f,
  29.5f,32.5f,35.5f,38.5f,
  42.0f,46.0f,50.0f,
  54.5f,59.5f,64.5f,
  59.5f,64.5f,69.5f,74.5f,79.5f,84.5f,89.5f,94.5f,99.5f,104.5f,109.5f,114.5f,119.5f,124.5f,129.5f,
  137.0f,147.0f,157.0f,167.0f,177.0f,187.0f,197.0f,
  209.5f,224.5f,239.5f,254.5f,
  272.0f,292.0f,312.0f,
  334.5f,359.5f,384.5f,
  359.5f,384.5f,409.5f,434.5f,459.5f,484.5f,509.5f,534.5f,559.5f,584.5f,609.5f,634.5f,659.5f,684.5f,709.5f,
  747.0f,797.0f,847.0f,897.0f,947.0f,997.0f,1047.0f,
  1109.5f,1184.5f,1259.5f,1334.5f,
  1422.0f,1522.0f,1622.0f,
  1734.5f,1859.5f,1984.5f,
  1859.5f,1984.5f,2109.5f,2234.5f,2359.5f,2484.5f,2609.5f,2734.5f,2859.5f,2984.5f,3109.5f,3234.5f,3359.5f,3484.5f,3609.5f,
  3797.0f,4047.0f,4297.0f,4547.0f,4797.0f,5047.0f,5297.0f,
  5609.5f,5984.5f,6359.5f,6734.5f,
  7172.0f,7672.0f,8172.0f,
  8734.5f,9359.5f,9984.5f
};

double digitize(double rawfC) {
  double digifC = 2.6*gl_inv_conversionTable[0];

  if (rawfC > 2.6*gl_inv_conversionTable[127])
    digifC =  2.6*gl_inv_conversionTable[127];
  else {
    for (int j=1; j<128; j++)
      if (rawfC < 2.6*gl_inv_conversionTable[j]) {
	digifC = 2.6*gl_inv_conversionTable[j-1];
	break;
      }
  }
  return digifC;
}

//
// member functions
//

//============================================================================
// Determines the bin0 left edge value in ns from the start of the pulse (t=0)
// such that the pulse is 7ns into bin1 from perfect 50/50 sharing in
// bin0/bin1. So bin1 in signal simulation counts as the SOI (peak) bin
// and bin2 counts as peak+1.
//
double HFsignal2noiseAnalyzer::determine5050minus7phase(void)
{
  pulseAmpScale_ = 1.0;

  int time5050shift=0;
  for (int shift=0; shift<NUM_SHIFTS; shift++) {
    // Digitize by integrating to find all time sample
    // bin values for this shift. Also find bin with peak.
    //
    double tmin = (-SHIFT_SCALE_NS)*(double)shift;
    double bin0val = integrate(tmin, tmin+BINSIZE_NS,INT_DELTA_NS,&scaledHFshape);
    double bin1val = integrate(tmin+BINSIZE_NS, tmin+2*BINSIZE_NS,INT_DELTA_NS,&scaledHFshape);

    if (bin1val > bin0val) {
      time5050shift = shift;
      cout << "time5050shift=" << time5050shift << endl;
      break;
    }
  }

  return ((-SHIFT_SCALE_NS*(double)time5050shift) - 7.0);
}                          // HFsignal2noiseAnalyzer::determine5050minus7phase

//============================================================================
// In case pulse shape is not normalized to unit area, determine the
// amplitude scale of the pulse
//
double HFsignal2noiseAnalyzer::determineAmpScale(double t0)
{
  edm::Service<TFileService> fs;

  pulseAmpScale_ = 1.0;
  double integral = integrate(0.,200.,INT_DELTA_NS,&scaledHFshape);

  double cum =0.0;

  TH1F *h1d_intpulse = fs->make<TH1F>("h1d_intpulse","h1d_intpulse",
				      (int)(200./INT_DELTA_NS),0.,200.);

  int ibin=1;
  for (double t=t0; t<200.; t+=INT_DELTA_NS) {
    // Digitize by integrating to find all time sample
    // bin values for this shift. Also find bin with peak.
    //
    double slice  = integrate(t,t+INT_DELTA_NS,INT_DELTA_NS,&scaledHFshape);
    cum += slice;

    h1d_intpulse->SetBinContent(ibin++,cum);
  }

  return 1./integral;
}                    // HFsignal2noiseAnalyzer::determine5050minus7phase

//======================================================================

void
HFsignal2noiseAnalyzer::addSignalAndHisto(float ts5noiseSample,
					  float jitterPhase)
{
  h1d_noiseSOIplus1fC_->Fill(ts5noiseSample);
  h1d_jitterPhaseNS_->Fill(jitterPhase);

  int biny = h2d_signalSOIfC_->GetYaxis()->FindFixBin(jitterPhase);

  float miny     = h2d_ts5sigPlusNoiseVsAmp_->GetYaxis()->GetXmin();
  float maxy     = h2d_ts5sigPlusNoiseVsAmp_->GetYaxis()->GetXmax();
  //float minnormy = h2d_ts5sigPlusNoiseVsNormAmp_->GetYaxis()->GetXmin();
  //float maxnormy = h2d_ts5sigPlusNoiseVsNormAmp_->GetYaxis()->GetXmax();
  float minnormy = h2d_ts5sigPlusNoiseVsNormAmp_->GetYaxis()->GetBinCenter(1);
  float maxnormy =
    h2d_ts5sigPlusNoiseVsNormAmp_->GetYaxis()->GetBinCenter(h2d_ts5sigPlusNoiseVsNormAmp_->GetNbinsY());

  // Add peak signal to each time bin
  //
  int numAmplitudeBins = h2d_ts5sigPlusNoiseVsAmp_->GetNbinsX();
  for (int binx=1; binx<=numAmplitudeBins; binx++) {
    float pulseAmpfC     = h2d_signalSOIplus1fC_->GetXaxis()->GetBinCenter(binx);
    float signalAmpTS5fC = h2d_signalSOIplus1fC_->GetBinContent(binx,biny);

    float sigplusnoise     = ts5noiseSample+signalAmpTS5fC;

    float sigplusnoisenorm;
    if (pulseAmpfC <= 0.f)
      sigplusnoisenorm = 0.f;
    else {
      sigplusnoisenorm = sigplusnoise/pulseAmpfC;
      sigplusnoisenorm = std::max(minnormy,std::min(sigplusnoisenorm,maxnormy));
    }

    h2d_ts5sigPlusNoiseVsAmp_->Fill(pulseAmpfC,
				    std::max(miny,std::min(sigplusnoise,maxy)));
    h2d_ts5sigPlusNoiseVsNormAmp_->Fill(pulseAmpfC,sigplusnoisenorm);
  }
}                           // HFsignal2noiseAnalyzer::addSignalAndHisto

//======================================================================

void
HFsignal2noiseAnalyzer::processDigis
(const edm::Handle<HFDigiCollection>& hfdigihandle)
{
  assert (hfdigihandle.isValid());

  edm::Service<TFileService> fs;

  const HFDigiCollection& hfdigis   = *hfdigihandle;
  CaloSamples dfC;

  HFDigiIt digit;

  for (digit = hfdigis.begin(); digit != hfdigis.end(); digit++) {
    const HFDataFrame& df = *digit;
    const HcalCalibrations& calibs = conditions_->getHcalCalibrations(df.id());
    const HcalQIECoder *qieCoder   = conditions_->getHcalCoder( df.id() );
    const HcalQIEShape *qieShape   = conditions_->getHcalShape();
    HcalCoderDb coder( *qieCoder, *qieShape );
    coder.adc2fC( df, dfC);

    for (int i=0; i<dfC.size(); i++) {
      int capid  = df[i].capid();
      dfC[i]    -= calibs.pedestal(capid);     // pedestal subtraction
    }

    // One noise sample, one shift value per event
    double trueshift;
    do {
      trueshift= myRandom_.Gaus(jitterMean_ns, jitterRMS_ns);
    } while (fabs(trueshift) > 5.0);

    addSignalAndHisto(dfC[1],trueshift); // TS1 stands in for TS5

    bool keephist = !(numdigis_ % 10000);

    stringstream diginame;
    diginame << "digi" << setw(4) << setfill('0') << numdigis_/1000;
    if (keephist) {
      TH1F *trialpulse = fs->make<TH1F>(diginame.str().c_str(),diginame.str().c_str(),
					10,-0.5,9.5);
      for (int i=0; i<10; i++) {
	trialpulse->Fill(i,dfC[i]);
      }
    }
    numdigis_++;
  } // digi loop
}                                // HFsignal2noiseAnalyzer::processDigis

//======================================================================

// ------------ method called to for each event  ------------
void
HFsignal2noiseAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  if (firstEvent_) {
    iSetup.get<HcalDbRecord>().get( conditions_ );
    firstEvent_ = false;
  }

  eventData_->get(iEvent,iSetup);

  processDigis(eventData_->hfdigis());
}


// ------------ method called once each job just before starting event loop  ------------
void 
HFsignal2noiseAnalyzer::beginJob()
{
  numdigis_=0;

  edm::Service<TFileService> fs;

  h2d_ts5sigPlusNoiseVsAmp_ = fs->make<TH2F>("h2d_ts5sig+noisevsAmp",
"HF TS_{Peak+1} signal+noise vs. Signal Amplitude; #scale[0.7]{#int} SimPulse(t)dt (fC); TS_{Peak+1} (fC)",
					     41, -25.,2025., 151,-50.5,100.5);

  h2d_ts5sigPlusNoiseVsNormAmp_ = fs->make<TH2F>("h2d_ts5sig+noisevsNormAmp",
"HF TS_{Peak+1} Fractional signal+noise vs. Signal Amplitude; #scale[0.7]{#int} SimPulse(t)dt; TS_{Peak+1} / #scale[0.7]{#int} SimPulse(t)dt",
					     41, -25.,2025., 5001,-0.0005,5.0005);

  h2d_signalSOIplus1fC_ = fs->make<TH2F>("h1d_signalSOIplus1fC",
"HF TS_{Peak+1} (fC) vs. total Pulse Amplitude; Total Pulse Amplitude (fC); Jitter phase (ns)",
					 41,-25.,2025.,101,-5.05,5.05);

  h1d_noiseSOIplus1fC_ = fs->make<TH1F>("h1d_noiseSOIplus1fC",
					"HF TS_{Peak+1} noise distribution; TS_{Peak+1} (fC)",
					101,-50.5,50.5);

  h2d_signalSOIfC_ = fs->make<TH2F>("h1d_signalSOIfC",
"HF TS_{Peak} (fC) vs. total Pulse Amplitude; Total Pulse Amplitude (fC); Jitter phase (ns)",
				    41,-25.,2025.,101,-5.05,5.05);

  h2d_signalSOIminus1fC_ = fs->make<TH2F>("h1d_signalSOIminus1fC",
 "HF TS_{Peak-1} (fC) vs. total Pulse Amplitude; Total Pulse Amplitude (fC); Jitter phase (ns)",
				    41,-25.,2025.,101,-5.05,5.05);

  h1d_jitterPhaseNS_ = fs->make<TH1F>("h1d_jitterPhaseNS_",
				      "HF signal jitter phase Distribution; Jitter phase (ns)",
				      101,-5.05,5.05);

  firstEvent_ = true;

  double t0 = determine5050minus7phase();  cout << "t0 = "       << t0       << endl;
  double ampscale = determineAmpScale(t0); cout << "ampscale = " << ampscale << endl;

  // Calculate the "numAmplitudeBins" number of signal values once,
  // to be used over and over again for each noise sample collected.
  //
  cout << "Pre-calculating TS5 signal values for all amplitudes/jitter phases..." << endl;

  int numAmplitudeBins = h2d_ts5sigPlusNoiseVsAmp_->GetNbinsX();
  for (int i=1; i<=numAmplitudeBins; i++) {
    double pulseAmpfC = h2d_ts5sigPlusNoiseVsAmp_->GetXaxis()->GetBinCenter(i);
    pulseAmpScale_    = ampscale*pulseAmpfC;

    for (double tjit=-5.0; tjit <= 5.0; tjit += 0.1) {
      // integrate SOI(peak) and SOI (peak) + 1 portions of the signal for various
      // signal amplitudes
      //
      double signalAmpSOIm1fC = 0.;
      double signalAmpSOIfC   = 0.;
      double signalAmpSOIp1fC = 0.;

      cout << i << "\t" << tjit << "\r" << flush;

      if (pulseAmpScale_ > 0.) {
	// integrate SOI(peak) and SOI (peak) + 1 portions of the signal for various
	// signal amplitudes
	//
	double t = t0 + tjit;
	signalAmpSOIm1fC = integrate(t,    t+25.,INT_DELTA_NS,&scaledHFshape);
	signalAmpSOIfC   = integrate(t+25.,t+50.,INT_DELTA_NS,&scaledHFshape);
	signalAmpSOIp1fC = integrate(t+50.,t+75.,INT_DELTA_NS,&scaledHFshape);
#if 0
	signalAmpSOIm1fC = digitize(signalAmpSOIm1fC);
	signalAmpSOIfC   = digitize(signalAmpSOIfC);
	signalAmpSOIp1fC = digitize(signalAmpSOIp1fC);
#endif
      }
      int biny = h2d_signalSOIfC_->GetYaxis()->FindFixBin(tjit);
      h2d_signalSOIfC_      ->SetBinContent(i,biny,signalAmpSOIfC);
      h2d_signalSOIplus1fC_ ->SetBinContent(i,biny,signalAmpSOIp1fC);
      h2d_signalSOIminus1fC_->SetBinContent(i,biny,signalAmpSOIm1fC);

    } // jitter phase loop

  } // amplitude loop

  cout << "Done." << endl;
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HFsignal2noiseAnalyzer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(HFsignal2noiseAnalyzer);
