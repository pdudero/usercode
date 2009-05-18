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
// $Id: myAnalHistos.hh,v 1.2 2009/05/06 19:49:14 dudero Exp $
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
#include "FWCore/MessageLogger/interface/MessageLogger.h"

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

  template<class T>
  void book1d(const std::vector<HistoParams_t>& v_pars);
  template<class T>
  void book2d(const std::vector<HistoParams_t>& v_pars);

  template <class T>
  void fill1d(std::map<std::string,double>& vals);
  template <class T>
  void fill1d(std::string& hname,double val,double weight=1.0);
  template <class T>
  void fill2d(std::map<std::string,std::pair<double,double> >& vals);
  template <class T>
  void fill2d(std::string& hname,double valx,double valy,double weight=1.0);

private:

  typedef struct {
    TH1          *ptr;
    HistoParams_t pars;
  }
  Histo_t;

  typedef  __gnu_cxx::hash_map<const char*, Histo_t> myHashmap_t;

  // ----------member data ---------------------------

  TFileDirectory *dir_;
  myHashmap_t hm_histos_;
};

//======================================================================

//
// member functions
//
template<class T>
void myAnalHistos::book1d(const std::vector<HistoParams_t>& v_pars)
{
  for (uint32_t i=0; i<v_pars.size(); i++) {
    Histo_t histo;
    histo.pars = v_pars[i];

    std::cout << "booking histogram " << histo.pars.name << std::endl;
    histo.ptr = dir_->make <T> (histo.pars.name.c_str(), histo.pars.title.c_str(),
				histo.pars.nbinsx, histo.pars.minx, histo.pars.maxx);
    hm_histos_[histo.pars.name.c_str()] = histo;
  }
}
//======================================================================

template<class T>
void myAnalHistos::book2d(const std::vector<HistoParams_t>& v_pars)
{
  for (uint32_t i=0; i<v_pars.size(); i++) {
    Histo_t histo;
    histo.pars = v_pars[i];

    std::cout << "booking histogram " << histo.pars.name << std::endl;
    histo.ptr = dir_->make <T> (histo.pars.name.c_str(), histo.pars.title.c_str(),
				histo.pars.nbinsx, histo.pars.minx, histo.pars.maxx,
				histo.pars.nbinsy, histo.pars.miny, histo.pars.maxy);
    hm_histos_[histo.pars.name.c_str()] = histo;
  }
}

//======================================================================

template<class T>
void
myAnalHistos::fill1d(std::string& hname,double val,double weight)
{
  myHashmap_t::const_iterator ith;

  ith = hm_histos_.find(hname.c_str());
  if (ith != hm_histos_.end()) {
    T *p = (T *)ith->second.ptr;
    p->Fill(val,weight);
  } else {
    edm::LogError("Couldn't find hash for " + hname + "!") << std::endl;
  }
}

//======================================================================

template<class T>
void
myAnalHistos::fill1d(std::map<std::string,double>& vals)
{
  std::map<std::string,double>::const_iterator itv;
  for (itv = vals.begin(); itv != vals.end(); itv++) {
    myHashmap_t::const_iterator ith;

    ith = hm_histos_.find(itv->first.c_str());
    if (ith != hm_histos_.end()) {
      T *p = (T *)ith->second.ptr;
      p->Fill(itv->second);
    } else {
      edm::LogError("Couldn't find hash for " + itv->first + "!") << std::endl;
    }
  }
}

//======================================================================

template<class T>
void
myAnalHistos::fill2d(std::string& hname,double valx,double valy,double weight)
{
  myHashmap_t::const_iterator ith;

  ith = hm_histos_.find(hname.c_str());
  if (ith != hm_histos_.end()) {
    T *p = (T *)ith->second.ptr;
    p->Fill(valx,valy,weight);
  } else {
    edm::LogError("Couldn't find hash for " + hname + "!") << std::endl;
  }
}

//======================================================================

template<class T>
void
myAnalHistos::fill2d(std::map<std::string,std::pair<double,double> >& vals)
{
  std::map<std::string,std::pair<double,double> >::const_iterator itv;
  for (itv = vals.begin(); itv != vals.end(); itv++) {
    myHashmap_t::const_iterator ith;

    ith = hm_histos_.find(itv->first.c_str());
    if (ith != hm_histos_.end()) {
      T *p = (T *)ith->second.ptr;
      p->Fill(itv->second.first,itv->second.second);
    } else {
      edm::LogError("Couldn't find hash for " + itv->first + "!") << std::endl;
    }
  }
}

#endif // _MYANALHISTOS
