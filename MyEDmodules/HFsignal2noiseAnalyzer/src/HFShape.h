#ifndef HcalSimAlgos_HFShape_h
#define HcalSimAlgos_HFShape_h
#include<vector>
  
#include "SimCalorimetry/CaloSimAlgos/interface/CaloVShape.h"

/**
  
   \class HFShape
  
   \brief  shaper for HF
     
*/

class HFShape : public CaloVShape
{
 public:
  
  HFShape();
  HFShape(const HFShape&d);

  virtual ~HFShape(){}

  virtual double timeToRise() const { return 0.0; }
  virtual double operator () (double time) const;
  double derivative (double time) const;
  double getTpeak () const;

 protected:
  virtual void computeShapeHF();
  virtual double fcon(double x);
  
  int nbin_;
  std::vector<float> nt_;
  
};

#endif
