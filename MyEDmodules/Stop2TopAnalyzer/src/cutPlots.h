#ifndef CUTPLOTS_H
#define CUTPLOTS_H

using namespace std;

#include <map>
#include <vector>

#include "MyAnalyzers/Stop2TopAnalyzer/src/myParticleRecord.h"
#include "TH1D.h"
#include "TH2F.h"

/** \class cutPlots
 *  
 *  $Id$
 */


class cutPlots {
 public:
  typedef struct {
    double absmet;
    double ht;
    double htplusmet;
    double leadingjetET;
    double maxElectronET;
    vector<double> v_electronETs;
    int    eventclass;
    int    numjets;
    int    numjetsoverthresh;
    myParticleRecord *myprec;

    double recoWmass;
    double recoTopMass;
    double recoTopEt;
    double recoTopPt;
    double recomassChi2;
    double recoTopBoost;
    double recoTop_e_dr;
    double recoTop_e_dphi;
    double recoTop_eplusbjet_dphi;
    double metsignificance;

    double eplusbjetmass;
  } plotVars_t;

  // Constructor
  cutPlots(double maxplottedETgev,
	   int    cutnumber,
	   const char *cutstr);

  void fill(plotVars_t& vars);

 private:
  TH1D  *h1d_electronET;
  TH1D  *h1d_leadingjetET;
  TH1D  *h1d_met;
  TH1D  *h1d_ht;
  TH1D  *h1d_htplusmet;
  TH1D  *h1d_recoWmass;
  TH1D  *h1d_recoTopMass;
  TH1D  *h1d_recoTopEt;
  TH1D  *h1d_recoTopPt;
  TH1D  *h1d_recomassChi2;
  TH1D  *h1d_recoTopBoost;
  TH1D  *h1d_recoTop_e_dr;
  TH1D  *h1d_recoTop_e_dphi;
  TH1D  *h1d_recoTop_eplusbjet_dphi;
  TH1D  *h1d_eplusbjetmass;

  TH1D  *h1d_metsignificance;

  TH1F  *h1f_class;
  TH2F  *h2f_numElecs;
  TH2F  *h2f_numElecTaus;
  TH2F  *h2f_numJets;
  TH2F  *h2f_numJetsOverThresh;
};

#endif
