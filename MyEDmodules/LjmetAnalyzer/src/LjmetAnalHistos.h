#ifndef _LJMETANALHISTOS_H
#define _LJMETANALHISTOS_H

#include "TH1.h"
#include "TH2F.h"
#include <string>
#include <vector>
#include <map>
#include "MyEDmodules/LjmetAnalyzer/src/GenEvtClass.h"

//======================================================================
/** \class LjmetAnalHistos specification
      
$Date: 2008/04/08 19:06:53 $
$Revision: 1.2 $
\author P. Dudero - Minnesota
*/
class LjmetAnalHistos {
public:
  typedef struct {
    int nbins;
    double min;
    double max;
  }
  HistoParams_t;

  typedef struct {
    HistoParams_t ethtmet;
    HistoParams_t numobj;
    HistoParams_t dR;
    HistoParams_t dphi;
    HistoParams_t signif;
  }
  AllHistoParams_t;

  typedef struct {
    double absmet;
    double ht;
    double htplusmet;
    double leadingjetET;
    double leadingjetEta;
    double leadingjetPhi;
    double maxElectronET;
    double maxElectronEta;
    double maxElectronPhi;
    std::vector<double> v_electronETs;
    std::vector<double> v_jetETs;
    std::vector<double> v_ejetdR;   // 0th element for leading jet
    std::vector<double> v_ejetdphi; // 0th element for leading jet
    GenEvtClass::EnumSample_t     eventclass;
    GenEvtClass::EnumSignature_t  signatureclass;
    int    genElectronCount;
    int    genQuarkCount;
    int    numElecs;
    int    numJets;
    int    numJetsCoHemi;
    int    numJetsAntiHemi;
    double metsignificance;

#if 0
    double recoWmass;
    double recoTopMass;
    double recoTopEt;
    double recoTopPt;
    double recomassChi2;
    double recoTopBoost;
    double recoTop_e_dr;
    double recoTop_e_dphi;
    double recoTop_eplusbjet_dphi;
    double eplusbjetmass;
#endif

  } HistoVars_t;

  LjmetAnalHistos() {}

  void bookHistos(AllHistoParams_t& pars,
		  std::string setid,
		  std::string setdescr);

  void fill(HistoVars_t& vars);

private:
  TH1D  *h1d_leET;
  TH1D  *h1d_leEta;
  TH1D  *h1d_lePhi;
  TH1D  *h1d_ljET;
  TH1D  *h1d_ljEta;
  TH1D  *h1d_ljPhi;
  TH1D  *h1d_alleET;
  TH1D  *h1d_met;
  TH1D  *h1d_ht;
  TH1D  *h1d_htplusmet;
#if 0
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
#endif
  TH1D  *h1d_metsignificance;

  TH2F  *h2f_numElecs;
  TH2F  *h2f_numJets;
  TH2F  *h2f_numElecsVsNumJets;
  TH2F  *h2f_jetetbynumber;
  TH1F  *h1f_numJetsCoHemi;
  TH1F  *h1f_numJetsAntiHemi;
  TH2F  *h2f_numJetsCoHemiVsAntiHemi;
  TH1F  *h1f_leljdR;
  TH1F  *h1f_leljdphi;
  TH1F  *h1f_lealljdR;
  TH1F  *h1f_lealljdphi;
  TH1F  *h1f_lecjdR;
};

#endif // _LJMETANALHISTOS_H
