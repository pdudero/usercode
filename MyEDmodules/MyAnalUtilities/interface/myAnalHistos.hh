#ifndef _MYANALHISTOS
#define _MYANALHISTOS

// -*- C++ -*-
//
// Package:    myAnalHistos
// Class:      myAnalHistos
// 
/**\class myAnalHistos myAnalHistos.cc 

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: myAnalAlgos.cc,v 1.4 2009/04/03 16:35:31 dudero Exp $
//
//


// system include files
#include <string>
#include <vector>
#include <map>
#include <ext/hash_map>

// user include files

#include "TH1.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"

//
// class declaration
//

class myAnalHistos {
public:
  // set nbinsy to 0 for 1-D
  typedef struct {
    std::string name;
    std::string title;
    uint32_t    nbinsx;
    double      minx;
    double      maxx;
    uint32_t    nbinsy;
    double      miny;
    double      maxy;
  }
  HistoParams_t;

  explicit myAnalHistos(const std::string& dirdescr);
  ~myAnalHistos() {}
  void book(const std::vector<HistoParams_t>& pars);

  void fill1d(std::map<std::string,double>& vals);
  void fill1d(std::string& hname,double val,double weight=1.0);
  void fill2d(std::map<std::string,std::pair<double,double> >& vals);
  void fill2d(std::string& hname,double valx,double valy,double weight=1.0);

private:

  typedef struct {
    TH1          *ptr;
    HistoParams_t pars;
  }
  Histo_t;

  typedef  __gnu_cxx::hash_map<const char*, Histo_t> myHashmap_t;

  template<class T>
  T *book1d(const HistoParams_t& pars);
  template<class T>
  T *book2d(const HistoParams_t& pars);

  // ----------member data ---------------------------

  TFileDirectory *dir_;
  myHashmap_t hm_histos_;
};

#endif // _MYANALHISTOS
