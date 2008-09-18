#ifndef _LINEARIZERLUT_HH
#define _LINEARIZERLUT_HH

#include <string>
#include <map>
#include <vector>
#include <set>

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"

#include "TH1F.h"
#include "TH1S.h"
#include "TH2F.h"
#include "TFile.h"

class linearizerLUT {
public:

  linearizerLUT(const edm::ParameterSet& iConfig,
		std::vector<HcalDetId>& detIds2mask);

// Currently the read function assumes a lot about the input format of the
// text file: it assumes that:
// 1) all columns have min/max iphis covering 1-72; i.e., no phi dependence
// 2) there is no double coverage or implied overwriting of represented detIds
//    in the LUT; no subsequent column is intended to overwrite data in a previous
//    column.

// Additional columns supplied for masking channels should be removed from the file
// and represented instead on the list of masked channels input to the constructor;
// this saves memory so you don't have to store all those zeros - and also keeps
// from having to index the LUT by individual detId, which would increase the LUT
// size unnecessarily from 2*2*128 to 26*72*2*128.
//

  bool     readFromTextFile(void);
  void     dump            (void);
  uint32_t lookupVal       (HcalDetId& detId, int rawadc);

private:
  std::string lutFileName_;

  // ---------- internal types ---------------------------
  struct columnSpec_t {
    std::set<int> includedIetas;
    std::set<int> includedDepths;
  };

  // ---------- internal methods ---------------------------

  void initColumn      (int icol,
			int ietamin, int ietamax,
			int depthmin, int depthmax);
  void insertElement   (int icol, uint32_t lutval);
  bool maskedId        (const HcalDetId& id);


  // ----------member data ---------------------------

  // parameters
  std::vector<columnSpec_t>     columns_;
  std::vector<HcalDetId>        detIds2mask_;

  
  std::vector<std::vector<uint32_t> > vv_LUT_;
};

#endif // _HFTRIGANALALGOS_HH
