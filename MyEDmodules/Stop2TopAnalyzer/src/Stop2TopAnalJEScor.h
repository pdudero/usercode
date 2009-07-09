#ifndef _STOP2TOPANALJESCOR_H
#define _STOP2TOPANALJESCOR_H

#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/JetReco/interface/GenJet.h"

#include <vector>
#include <map>
#include "TH1F.h"
#include "TH2D.h"
#include "TFile.h"

//======================================================================

using namespace std;

//======================================================================

//======================================================================
/** \class Stop2TopAnalJEScor specification
      
$Date: 2005/10/06 22:21:33 $
$Revision: 1.2 $
\author P. Dudero - Minnesota
*/
class Stop2TopAnalJEScor {
public:
  Stop2TopAnalJEScor(bool           verbosity,
		     string         outRootFileName,
		     double         maxdR,
		     vector<double> v_genETbins,
		     vector<double> v_recETbins,
		     double         emfractioncut);

  void beginJob(void);
  void endJob();
  void processEvent(GenJetCollection&  genjets2,
		    GenJetCollection&  genjets5,
		    GenJetCollection&  genjets7,
		    CaloJetCollection& recjets2,
		    CaloJetCollection& recjets5,
		    CaloJetCollection& recjets7);
private:
  // internal methods
  void dumpJets(CaloJetCollection& jetc, const char *intro);
  void dumpJets(GenJetCollection&  jetc, const char *intro);
  void correlateJets(GenJetCollection&  genjetc,
		     CaloJetCollection& calojetc,
		     int conesize,
		     const char *intro,
		     vector<pair<GenJet,CaloJet> >& v_jetCorr,
		     vector<GenJet>& v_genjetUncorr);
  void collectJetStats(vector<pair<GenJet,CaloJet> >& v_jetCorr,
		       vector<GenJet>& v_genjetUncorr,
		       int conesize);

  // user-configurable parameters
  bool   verbosity_;
  string outRootFileName_;
  double maxdR_;
  vector<double> v_genETbins_; 
  vector<double> v_recETbins_; 
  double emfractioncut_;
  double genjetETgevcut_;
  double recjetETgevcut_;

  TH2D *jetETscale_vs_phi_cone5;
  TH2D *jetETscale_vs_phi_cone7;
  map<double,TH2D *> m_jetETscale_vs_geta_et_hists_cone5;
  map<double,TH2D *> m_jetETscale_vs_geta_et_hists_cone7;
  map<double,TH2D *> m_jetETscale_vs_reta_et_hists_cone5;
  map<double,TH2D *> m_jetETscale_vs_reta_et_hists_cone7;
  TH1F *emRecoJet_vs_geta_cone5;
  TH1F *emRecoJet_vs_geta_cone7;
  TFile *outf;
};

#endif // _STOP2TOPANALJESCOR_H
