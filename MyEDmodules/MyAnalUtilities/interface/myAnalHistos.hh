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
// $Id: myAnalHistos.hh,v 1.7 2009/11/09 01:10:56 dudero Exp $
//
//


// system include files
#include <string>
#include <vector>
#include <map>
#include <ext/hash_map>
#include <iostream>

// user include files

#include "TH1D.h"
#include "TProfile.h"
#include "TH2D.h"
#include "TProfile2D.h"

#include "PhysicsTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

//
// class declaration
//

class myAnalHistos {
public:
  // set nbinsy to 0 for 1-D
  struct HistoParams_t {
    HistoParams_t() {}
    HistoParams_t(const std::string& inname, const std::string& intit,
		  uint32_t innbinsx,double inminx, double inmaxx,
		  uint32_t innbinsy=0,double inminy=0.0, double inmaxy=0.0) :
      name(inname), title(intit), nbinsx(innbinsx), minx(inminx), maxx(inmaxx),
      nbinsy(innbinsy), miny(inminy), maxy(inmaxy) {}
    std::string name;
    std::string title;
    uint32_t    nbinsx;
    double      minx;
    double      maxx;
    uint32_t    nbinsy;
    double      miny;
    double      maxy;
  };

  explicit myAnalHistos(const std::string& dirdescr);
  explicit myAnalHistos(const std::string& dirdescr,
			TFileDirectory& subdir);
  ~myAnalHistos() {}

  template<class T> void book1d(const HistoParams_t& pars);
  template<class T> void book2d(const HistoParams_t& pars);
  template<class T> void book1d(const std::vector<HistoParams_t>& v_pars);
  template<class T> void book2d(const std::vector<HistoParams_t>& v_pars);
  template<class T> void book2dvarx(const std::string& name,
				    const char *title,
				    int nbinsx, const double xbins[],
				    int nbinsy, double ymin, double ymax);
  template<class T> void book2dvary(const std::string& name,
				    const char *title,
				    int nbinsx, double xmin, double xmax,
				    int nbinsy, const double ybins[]);
  template<class T> T *get(const std::string& hname);

  template<class T> void fill1d(const std::map<std::string,double>& vals);
  template<class T> void fill1d(const std::string& hname,double val,double weight=1.0);
  template<class T> void fill2d(const std::map<std::string,std::pair<double,double> >& vals);
  template<class T> void fill2d(const std::string& hname,double valx,double valy,double weight=1.0);

  template<class T> void bookClone(const std::string& cloneName,const T& h);

  TFileDirectory *dir(void) { return dir_; }

#if 0
  // common usages:
  void bookTH1D(const std::vector<HistoParams_t>& v_pars) {
    book1d<TH1D>(v_pars);
  } 
  void bookTProfile(const std::vector<HistoParams_t>& v_pars) {
    book1d<TProfile>(v_pars);
  }
  void bookTH2D(const std::vector<HistoParams_t>& v_pars) {
    book1d<TH2D>(v_pars);
  }
  void bookTProfile2D(const std::vector<HistoParams_t>& v_pars) {
    book1d<TProfile2D>(v_pars);
  }
#endif

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
void myAnalHistos::book1d(const HistoParams_t& pars)
{
  Histo_t histo;
  histo.pars = pars;

  //edm::LogInfo("booking histogram ") << histo.pars.name << std::endl;
  std::cout << "booking histogram " << histo.pars.name << std::endl;
  //std::cout << ", this=" << (int)this << std::endl;
  histo.ptr = dir_->make <T> (histo.pars.name.c_str(),
			      histo.pars.title.c_str(),
			      histo.pars.nbinsx, histo.pars.minx, histo.pars.maxx);
  hm_histos_[histo.pars.name.c_str()] = histo;
  //std::cout << "hm_histos_.size() = " << hm_histos_.size() << std::endl;
}

//======================================================================

//
// member functions
//
template<class T>
void myAnalHistos::book1d(const std::vector<HistoParams_t>& v_pars)
{
  for (uint32_t i=0; i<v_pars.size(); i++)
    book1d<T>(v_pars[i]);
}

//======================================================================

template<class T>
void myAnalHistos::book2d(const HistoParams_t& pars)
{
  Histo_t histo;
  histo.pars = pars;

  //edm::LogInfo("booking histogram ") << histo.pars.name << std::endl;
  std::cout << "booking histogram " << histo.pars.name << std::endl;
  histo.ptr = dir_->make <T> (histo.pars.name.c_str(), histo.pars.title.c_str(),
			      histo.pars.nbinsx, histo.pars.minx, histo.pars.maxx,
			      histo.pars.nbinsy, histo.pars.miny, histo.pars.maxy);
  hm_histos_[histo.pars.name.c_str()] = histo;
}

//======================================================================

template<class T>
void myAnalHistos::book2d(const std::vector<HistoParams_t>& v_pars)
{
  for (uint32_t i=0; i<v_pars.size(); i++)
    book2d<T>(v_pars[i]);
}

//======================================================================

template<class T>
void myAnalHistos::book2dvarx(const std::string& name,
			      const char *title,
			      int nbinsx, const double xbins[],
			      int nbinsy, double ymin, double ymax)
{
  Histo_t histo;
  histo.pars.name = name;
  histo.pars.title=std::string(title);
  histo.pars.nbinsx=nbinsx;
  histo.pars.nbinsy=nbinsy;
  histo.pars.miny=ymin;
  histo.pars.maxy=ymax;

  //edm::LogInfo("booking histogram ") << histo.pars.name << std::endl;
  std::cout << "booking histogram " << name << std::endl;
  histo.ptr = dir_->make <T> (name.c_str(), title,
			      nbinsx, xbins,
			      nbinsy, ymin, ymax);
  hm_histos_[name.c_str()] = histo;
}

//======================================================================

template<class T>
void myAnalHistos::book2dvary(const std::string& name,
			      const char *title,
			      int nbinsx, double xmin, double xmax,
			      int nbinsy, const double ybins[])
{
  Histo_t histo;
  histo.pars.name=name;
  histo.pars.title=std::string(title);
  histo.pars.nbinsx=nbinsx;
  histo.pars.minx=xmin;
  histo.pars.maxx=xmax;
  histo.pars.nbinsy=nbinsy;

  //edm::LogInfo("booking histogram ") << histo.pars.name << std::endl;
  std::cout << "booking histogram " << name << std::endl;
  histo.ptr = dir_->make <T> (name.c_str(), title,
			      nbinsx, xmin, xmax,
			      nbinsy, ybins);
  hm_histos_[name.c_str()] = histo;
}

//======================================================================

template<class T>
void
myAnalHistos::bookClone(const std::string& cloneName,const T& h)
{
  //edm::LogInfo("booking clone histogram ") << cloneName << std::endl;
  std::cout << "booking clone histogram " << cloneName;
  std::cout << ", this=" << (int)this << std::endl;
  Histo_t histo;
  histo.ptr = (TH1 *)dir_->make <T, T> (h);
  //std::cout << "Before SetName: " << histo.ptr->GetName() << std::endl;
  histo.ptr->SetName(cloneName.c_str());
  //std::cout << "After  SetName: " << histo.ptr->GetName() << std::endl;
  hm_histos_[cloneName.c_str()] = histo;
  //std::cout << "hm_histos_.size() = " << hm_histos_.size() << std::endl;
}

//======================================================================

template<class T>
void
myAnalHistos::fill1d(const std::string& hname,double val,double weight)
{
  myHashmap_t::const_iterator ith;
  ith = hm_histos_.find(hname.c_str());

  if (ith != hm_histos_.end()) {
    T *p = (T *)ith->second.ptr;
    if (!p) std::cout << "NULL POINTER for histo " << hname << "!!!" << std::endl;

    p->Fill(val,weight);
  } else {
    //    edm::LogError("Couldn't find hash for " + hname + "! ") 
     throw cms::Exception("Couldn't find hash for " + hname + "! ") 
      << val << "\t" << weight << std::endl;
  }
}

//======================================================================

template<class T>
void
myAnalHistos::fill1d(const std::map<std::string,double>& vals)
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
myAnalHistos::fill2d(const std::string& hname,double valx,double valy,double weight)
{
  myHashmap_t::const_iterator ith;

  ith = hm_histos_.find(hname.c_str());
  if (ith != hm_histos_.end()) {
    T *p = (T *)ith->second.ptr;
    if (!p) std::cout << "NULL POINTER for histo " << hname << "!!!" << std::endl;
    p->Fill(valx,valy,weight);
  } else {
    edm::LogError("Couldn't find hash for " + hname + "!")
      << valx << "\t" << valy << "\t" << weight << std::endl;
  }
}

//======================================================================

template<class T>
void
myAnalHistos::fill2d(const std::map<std::string,std::pair<double,double> >& vals)
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

//======================================================================

template<class T>
T *
myAnalHistos::get(const std::string& hname)
{
  T *p = (T *)NULL;
  myHashmap_t::const_iterator ith = hm_histos_.find(hname.c_str());
  if (ith != hm_histos_.end())
    p = (T *)ith->second.ptr;
  return p;
}

#endif // _MYANALHISTOS
