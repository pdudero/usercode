#ifndef _MYANALCUT
#define _MYANALCUT

// -*- C++ -*-
//
// Package:    myAnalCut
// Class:      myAnalCut
// 
/**\class myAnalCut myAnalCut.cc 

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: myAnalCut.hh,v 1.1 2009/04/09 22:12:43 dudero Exp $
//
//

// system include files
#include <string>
#include <vector>

// user include files

#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"

//
// class declaration
//
class myAnalCut {
public:
  myAnalCut(const int cutnum,const std::string& descr) :
    active_(false), cutnum_(cutnum), evtCount_(0), cutdescr_(descr)
  { pHistos_ = new myAnalHistos(descr); }

  inline bool                 isActive()    const { return active_;   }
  inline int                  nEvents()     const { return evtCount_; }
  inline myAnalHistos        *histos()      const { return pHistos_;  }
  inline const std::string&   description() const { return cutdescr_; }

  inline void Activate(bool val) { active_ = val; }
  
private:
  bool           active_;
  int            cutnum_;
  uint32_t       evtCount_;
  std::string    cutdescr_;
  myAnalHistos  *pHistos_;
};

#endif // _MYANALCUT
