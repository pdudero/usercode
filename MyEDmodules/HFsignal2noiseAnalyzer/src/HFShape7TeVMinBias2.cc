#include "HFShape.h"
#include <cmath>

HFShape::HFShape()
: nbin_(256),
  nt_(nbin_, 0.)
{   
  computeShapeHF();
}


HFShape::HFShape(const HFShape&d)
: CaloVShape(d),
  nbin_(d.nbin_),
  nt_(d.nt_)
{
}

//========================================
// This was reconstructed for >=40GeV hits
//
double HFShape::fcon(double x) {
  double r,xx;

  xx=x;
  r = -1.2964e-01*pow(1.0+exp(16.2624-2.1343*xx)+exp(1.242266-0.194862*xx),2);
  return (-2.1343*exp(16.2624-2.1343*xx)-0.194862*exp(1.242266-0.194862*xx))/r;
}

void HFShape::computeShapeHF()
{

  //  cout << endl << " ===== computeShapeHF  !!! " << endl << endl;

  int j;
  float norm;

  // HF SHAPE
  norm = 0.0;
  for( j = 0; j<nbin_; j++){
    nt_[j] = fcon((double)j/2); // 0.5ns steps
    norm += nt_[j];
  }
  // normalize pulse area to 1.0
  for( j = 0; j < nbin_; j++){
    nt_[j] /= norm;
  }
}

double HFShape::operator () (double time) const
{

  // return pulse amplitude for request time in ns
  int jtime;
  jtime = static_cast<int>(2.*time+0.5);

  if(jtime >= 0 && jtime < nbin_)
    return nt_[jtime];
  else 
    return 0.0;
}

double HFShape::derivative (double time) const
{
  return 0.0;
}
