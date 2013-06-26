#ifndef _HWWCHANNELS_H
#define _HWWCHANNELS_H

#define NUMCHAN 1

// must be put in lexicographical order
//
const char *channames[NUMCHAN] = {
  "mu2jet"
};

const char *channames2[NUMCHAN] = {
  "Muon"
};

const double mutrigeff      = 0.975;
const double eltrigeff      = 0.821;
const double sigtrigeffunc  = 0.01;
const double siglumiunc     = 0.045;

const bool isHiggs          = true;

//const double intlumipbinv = 1200.0;
//const double intlumipbinv = 2100.0;
const double intlumipbinv = 4700.0;

const int NUMMASSPTS=3;
const int masspts[NUMMASSPTS] = {
  180, 250, 400
};

const int xminpermass[NUMMASSPTS] = {
  170, 200, 260
};

const int xmaxpermass[NUMMASSPTS] = {
  250, 500, 780
};

const int binwpermass[NUMMASSPTS] = {
  10, 20, 40
};

// inputs are normalized to 1/nent

// FILE/HISTO STRUCTURE: assumed same name objects contained in different files for the different channels

// ordered the same as the channels
//
const char *dir =
  "/uscms_data/d2/andersj/Wjj/CMSSW_4_2_3_patch5/src/ElectroWeakAnalysis/VPlusJets/test/HWWCutCount/";
//"/uscms_data/d2/andersj/Wjj/CMSSW_4_2_3_patch5/src/ElectroWeakAnalysis/VPlusJets/test/HWWPreSel/";

const char *inputfiles[NUMMASSPTS*NUMCHAN] = {
  "H180_Muon_2Jets_Fit_Shapes.root",
  "H250_Muon_2Jets_Fit_Shapes.root",
  "H400_Muon_2Jets_Fit_Shapes.root"
};

const char *dataobjname = "theData";
const char *bkgdobjname = "h_total";

double backnormerr[NUMMASSPTS*NUMCHAN] = {
  /*        mu2j */
  /*180*/  1.04,
  /*250*/  1.04,
  /*400*/  1.04
};

double sigselefferrpct[NUMMASSPTS*NUMCHAN] = {
  /*       mu2j */
  /*180*/  3.62,
  /*250*/  2.53,
  /*400*/  0.55
};

// taken from the larger qq->H numbers
//
double sigaccptsyst[NUMMASSPTS] = {
  /*180*/ 1.02,
  /*250*/ 1.0151469,
  /*400*/ 1.024166
};


#if 0
const double sumwinmin = 150-15*2;
const double sumwinmax = 150+15*2;
#elif 0
const double sumwinmin = 140;
const double sumwinmax = 170;
#else
const double sumwinmin = 60;
const double sumwinmax = 300;
#endif

#endif // _HWWCHANNELS_H
