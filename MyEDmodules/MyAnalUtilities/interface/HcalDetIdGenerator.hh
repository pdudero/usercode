#ifndef _MYANALUTILITIESHCALDETIDGENERATOR_H
#define _MYANALUTILITIESHCALDETIDGENERATOR_H

// -*- C++ -*-
//
// Package:    HcalDetIdGenerator
// Class:      HcalDetIdGenerator
// 
/**\class HcalDetIdGenerator HcalDetIdGenerator.cc MyEDmodules/HcalDetIdGenerator/src/HcalDetIdGenerator.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: HcalDetIdGenerator.cc,v 1.1 2009/11/09 00:57:58 dudero Exp $
//
//


#include <string>
#include <vector>
#include <math.h>

// user include files
#include "DataFormats/HcalDetId/interface/HcalDetId.h"

//
// class declaration
//

class HcalDetIdGenerator {
public:
  HcalDetIdGenerator() {}
  ~HcalDetIdGenerator() {}

  void appendDetIds4subdet(HcalSubdetector        subdet,
			   std::vector<uint32_t>& v_denseIds);
};

#endif // _MYANALUTILITIESHCALDETIDGENERATOR_H
