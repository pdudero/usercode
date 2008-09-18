#include <iostream>
#include "MyEDmodules/HFtrigAnal/src/linearizerLUT.hh"
#include "MyEDmodules/HFtrigAnal/src/inSet.hh"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "math.h"
#include "TMath.h"
#include "TF1.h"

using namespace std;

//======================================================================
// Got this from
// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html

void Tokenize(const string& str,
	      vector<string>& tokens,
	      const string& delimiters = " ")
{
  // Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos) {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}                                                            // Tokenize

//======================================================================

linearizerLUT::linearizerLUT(const edm::ParameterSet& iConfig,
		std::vector<HcalDetId>& detIds2mask) :
  detIds2mask_(detIds2mask)
{
  lutFileName_  = iConfig.getUntrackedParameter<string>("lutFileName");

}                                        // linearizerLUT::linearizerLUT

//======================================================================

bool linearizerLUT::maskedId(const HcalDetId& id)
{
  for (uint32_t i=0; i<detIds2mask_.size(); i++)
    if (id == detIds2mask_[i]) return true;
  return false;
}

//======================================================================

void linearizerLUT::initColumn(int icol,
			       int ietamin, int ietamax,
			       int depthmin, int depthmax)
{
  columnSpec_t& cs = columns_[icol];

  for (int ie = ietamin;  ie <= ietamax;  ie++) cs.includedIetas.insert(ie);
  for (int dp = depthmin; dp <= depthmax; dp++) cs.includedDepths.insert(dp);
}

//======================================================================
//
void linearizerLUT::insertElement(int icol, uint32_t lutval)
{
  vv_LUT_[icol].push_back(lutval);
}                                        // linearizerLUT::insertElement

//======================================================================

bool linearizerLUT::readFromTextFile(void)
{
  char linein[512];
  FILE *fp = fopen(lutFileName_.c_str(),"r");
  int nline=0;
  uint32_t ntokens=0;
  vector<int> iemin,iemax,dpmin,dpmax;

  while (!feof(fp) && fgets(linein, 512, fp))  {
    char *ptr = strchr(linein, '\n'); if (ptr) *ptr = 0; // null the newline
    nline++;

    vector<string> tokens;
    Tokenize(linein, tokens," \t");

    if (nline <= 8) {
      switch (nline) {
      case 1: break; // empty line
      case 2:        // subdet
	ntokens = tokens.size();
	columns_.resize(ntokens);
	vv_LUT_.resize(ntokens);
	for (uint32_t itok=0;itok<tokens.size();itok++)
	  if (!tokens[itok].compare("HF"))
	    throw cms::Exception("readLUTfromTextFile accepts only 'HF' labeled columns");
	break;
      case 3:        // ieta min
	for (uint32_t itok=0;itok<tokens.size();itok++)
	  iemin.push_back(atoi(tokens[itok].c_str()));
	break;
      case 4:        // ieta max
	for (uint32_t itok=0;itok<tokens.size();itok++)
	  iemax.push_back(atoi(tokens[itok].c_str()));
	break;
      case 5:        // iphi min
      case 6:        // iphi max
	break;
      case 7:        // depth min
	for (uint32_t itok=0;itok<tokens.size();itok++)
	  dpmin.push_back(atoi(tokens[itok].c_str()));
	break;
      case 8:        // depth max
	for (uint32_t itok=0;itok<tokens.size();itok++) {
	  dpmax.push_back(atoi(tokens[itok].c_str()));
	  initColumn(itok,iemin[itok],iemax[itok],dpmin[itok],dpmax[itok]);
	}
	break;
      }
      continue;
    }

    if (tokens.size() < ntokens) {
      cerr << "# of entries in line " << nline << " = ";
      cerr << tokens.size() << " - incorrect" << endl;
      return false;
    }

    for (uint32_t itok=0; itok < tokens.size(); itok++)
      insertElement(itok,atoi(tokens[itok].c_str()));

  } // loop over lines in text file

  return (nline == 136);
}                                     // linearizerLUT::readFromTextFile

//======================================================================

void linearizerLUT::dump(void)
{
  for (uint32_t icol = 0; icol < vv_LUT_.size(); icol++) {
    cout << "ID: " << icol << " ietas: (";
    set<int>::const_iterator it;
    for (it  = columns_[icol].includedIetas.begin();
	 it != columns_[icol].includedIetas.end(); it++) {
      cout << (*it) << ", ";
    }
    cout << ") depths: (";
    for (it  = columns_[icol].includedDepths.begin();
	 it != columns_[icol].includedDepths.end(); it++) {
      cout << (*it) << ", ";
    }
    cout << ") LUT: ";
    vector<uint32_t>& column = vv_LUT_[icol];
    for (uint32_t ival=0; ival<column.size(); ival++)
      cout << " " << column[ival];
    cout << endl;
  }
}                                                 // linearizerLUT::dump

//======================================================================

uint32_t linearizerLUT::lookupVal(HcalDetId& detId, int rawadc)
{
#if 0
  int capid = frame[isample].capid ();
  double linear_ADC = frame[isample].nominal_fC();
  double nominal_fC = detId.subdet () == HcalForward ? 2.6 *  linear_ADC : linear_ADC;
#endif

  if (maskedId(detId)) return 0;

  uint32_t icol;
  for (icol = 0; icol<columns_.size(); icol++) {
    set<int>& ietas  = columns_[icol].includedIetas;
    set<int>& depths = columns_[icol].includedDepths;
    if ( (inSet<int>(ietas, detId.ieta())) &&
	 (inSet<int>(depths,detId.depth()))  )
      break;
  }
  if (icol == columns_.size()) {
    cout << "LUT entry for id " << detId << " not found." << endl;
    throw cms::Exception("LUT entry for id not found");
  }

  return (vv_LUT_[icol][rawadc]);
}                                            // linearizerLUT::lookupVal

//======================================================================
