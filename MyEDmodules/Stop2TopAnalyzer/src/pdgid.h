#ifndef PDG_ID_H
#define PDG_ID_H

// QUARKS
static const int downq    = 1;
static const int upq      = 2;
static const int strangeq = 3;
static const int charmq   = 4;
static const int bottomq  = 5;
static const int topq     = 6;

static const int antidownq    = -1;
static const int antiupq      = -2;
static const int antistrangeq = -3;
static const int anticharmq   = -4;
static const int antibottomq  = -5;
static const int antitopq     = -6;

//LEPTONS
static const int electron   =  11;
static const int positron   = -11;
static const int nu_e       =  12;
static const int antinu_e   = -12;
static const int muminus    =  13;
static const int muplus     = -13;
static const int nu_mu      =  14;
static const int antinu_mu  = -14;
static const int tauminus   =  15;
static const int tauplus    = -15;
static const int nu_tau     =  16;
static const int antinu_tau = -16;

// GAUGE AND HIGGS BOSONS
static const int gluon    = 21;
static const int photon   = 22;
static const int Z0boson  = 23;
static const int Wplus    = 24;
static const int Wminus   = -24;
static const int Higgs    = 25;

// DIQUARKS
static const int dd1      = 1103;
static const int ud0      = 2101;
static const int ud1      = 2103;
static const int uu1      = 2203;
static const int sd0      = 3101;
static const int sd1      = 3103;
static const int su0      = 3201;
static const int su1      = 3203;
static const int ss1      = 3303;
static const int cd0      = 4101;
static const int cd1      = 4103;
static const int cu0      = 4201;
static const int cu1      = 4203;
static const int cs0      = 4301;
static const int cs1      = 4303;
static const int cc1      = 4403;
static const int bd0      = 5101;
static const int bd1      = 5103;
static const int bu0      = 5201;
static const int bu1      = 5203;
static const int bs0      = 5301;
static const int bs1      = 5303;
static const int bc0      = 5401;
static const int bc1      = 5403;
static const int bb0      = 5503;

// SUSY PARTICLES
static const int sdownL         =  1000001;
static const int antisdownL     = -1000001;
static const int supL           =  1000002;
static const int antisupL       = -1000002;
static const int sstrangeL      =  1000003;
static const int antisstrangeL  = -1000003;
static const int scharmL        =  1000004;
static const int antischarmL    = -1000004;
static const int sbottom1       =  1000005;
static const int antisbottom1   = -1000005;
static const int stop1          =  1000006;
static const int antistop1      = -1000006;
static const int selectronL     =  1000011;
static const int antiselectronL = -1000011;
static const int snu_eL         =  1000012;
static const int antisnu_eL     = -1000012;
static const int smuonL         =  1000013;
static const int antismuonL     = -1000013;
static const int snu_muL        =  1000014;
static const int antisnu_muL    = -1000014;
static const int stau1          =  1000015;
static const int antistau1      = -1000015;
static const int snu_tauL       =  1000016;
static const int antisnu_tauL   = -1000016;
static const int sdownR         =  2000001;
static const int antisdownR     = -2000001;
static const int supR           =  2000002;
static const int antisupR       = -2000002;
static const int sstrangeR      =  2000003;
static const int antisstrangeR  = -2000003;
static const int scharmR        =  2000004;
static const int antischarmR    = -2000004;
static const int sbottom2       =  2000005;
static const int antisbottom2   = -2000005;
static const int stop2          =  2000006;
static const int antistop2      = -2000006;
static const int selectronR     =  2000011;
static const int antiselectronR = -2000011;
static const int smuonR         =  2000013;
static const int antismuonR     = -2000013;
static const int stau2          =  2000015;
static const int antistau2      = -2000015;
static const int gluino         =  1000021;
static const int neutralino01   =  1000022;
static const int neutralino02   =  1000023;
static const int charginoplus1  =  1000024;
static const int neutralino03   =  1000025;
static const int neutralino04   =  1000035;
static const int charginoplus2  =  1000037;
static const int gravitino      =  1000039;

// LIGHT I=1 MESONS
static const int pi0            =  111;
static const int piplus         =  211;
static const int piminus        = -211;

// STRANGE MESONS
static const int K0_L           =  130;
static const int K0_S           =  310;
static const int K0             =  311;
static const int K0bar          = -311;
static const int Kplus          =  321;
static const int Kminus         = -321;

// CHARMED MESONS
static const int Dplus          =  411;
static const int Dminus         = -411;
static const int D0             =  421;
static const int D0bar          = -421;

static const int B0             =  511;
static const int B0bar          = -511;
static const int Bplus          =  521;
static const int Bminus         = -521;

// LIGHT BARYONS
static const int proton         =  2212;
static const int antiproton     = -2212;
static const int neutron        =  2112;
static const int antineutron    = -2112;

#endif // PDGID_H
