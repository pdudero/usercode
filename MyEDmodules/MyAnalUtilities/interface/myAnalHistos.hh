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
// $Id: myAnalHistos.hh,v 1.13 2010/05/05 00:44:12 dudero Exp $
//
//


// system include files
#include <string>
#include <vector>
#include <map>
#include <ext/hash_map>
#include <iostream>
#include <boost/iterator/transform_iterator.hpp>

// user include files

#include "TH1.h"
#include "TProfile.h"
#include "TH2.h"
#include "TH3.h"
#include "TProfile2D.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

//
// class declaration
//
typedef bool (*detIDfun_t)(const DetId&);

struct eqstr{
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1,s2)==0;
  }
};

struct eqint{
  bool operator()(uint32_t i1, uint32_t i2) const {
    return i1==i2;
  }
};

template <class Tkey>
class myAnalHistosTC {
public:

  // set nbinsy,nbinsz to 0 for 1-D, nbinz=0 for 2-D
  struct HistoParams_t {
    HistoParams_t() {}
    HistoParams_t(const std::string& inname, const std::string& intit,
		  uint32_t innbinsx,double inminx, double inmaxx,
		  uint32_t innbinsy=0,double inminy=0.0, double inmaxy=0.0,
		  uint32_t innbinsz=0,double inminz=0.0, double inmaxz=0.0) :
      name(inname), title(intit),
      nbinsx(innbinsx), minx(inminx), maxx(inmaxx),
      nbinsy(innbinsy), miny(inminy), maxy(inmaxy),
      nbinsz(innbinsz),minz(inminz),maxz(inmaxz) {}
    std::string name;
    std::string title;
    uint32_t    nbinsx;
    double      minx;
    double      maxx;
    uint32_t    nbinsy;
    double      miny;
    double      maxy;
    uint32_t    nbinsz;
    double      minz;
    double      maxz;
  };

  struct AutoFillPars_t {
    AutoFillPars_t() : filladdrx(0),filladdry(0),filladdrz(0),filladdrw(0),detIDfun(0) {}
    void         *filladdrx;
    void         *filladdry;
    void         *filladdrz;
    void         *filladdrw; // weight
    detIDfun_t    detIDfun;
  };

  struct HistoAutoFill_t {
    HistoParams_t   hpars;
    AutoFillPars_t afpars;
  };

  typedef struct {
    TH1            *ptr;
    HistoParams_t   hpars;
    AutoFillPars_t  afpars;
  }
  Histo_t;

  class histoFromIterator // return key/histo pointer pair with parameters stripped
  {
  public:
    typedef std::pair<Tkey,TH1 *> result_type; // required by the machinery
    template <typename U>
    result_type operator()(std::pair<U,Histo_t>& element) const
    {
      return result_type(element.first,element.second.ptr);
    }
  };

  //typedef  __gnu_cxx::hash_map<Tkey, Histo_t> myHashmap_t;
  typedef  std::map<Tkey, Histo_t> myHashmap_t;

  typedef boost::transform_iterator<
    histoFromIterator, typename myHashmap_t::iterator> iterator;

  iterator begin()  {
    return boost::make_transform_iterator(hm_histos_->begin(),
					  histoFromIterator());
  }

  iterator end()  {
    return boost::make_transform_iterator(hm_histos_->end(),
					  histoFromIterator());
  }

  explicit myAnalHistosTC(const std::string& dirdescr);
  explicit myAnalHistosTC(const std::string& dirdescr,
			  TFileDirectory& subdir);
  ~myAnalHistosTC() {}

  template<class T> T *book1d(const HistoParams_t& pars, bool verbose=true);
  template<class T> T *book2d(const HistoParams_t& pars, bool verbose=true);
  template<class T> T *book3d(const HistoParams_t& pars, bool verbose=true);

  template<class T> T *book1d(Tkey key, const HistoParams_t& pars, bool verbose=true);
  template<class T> T *book2d(Tkey key, const HistoParams_t& pars, bool verbose=true);
  template<class T> T *book3d(Tkey key, const HistoParams_t& pars, bool verbose=true);

  template<class T> T *book2d(const std::string& name,
			      const char *title,
			      int nbinsx, const double xbins[],
			      int nbinsy, double ymin, double ymax,
			      bool verbose=true);
  template<class T> T *book2d(const std::string& name,
			      const char *title,
			      int nbinsx, double xmin, double xmax,
			      int nbinsy, const double ybins[],
			      bool verbose=true);

  template<class T> T *book2d(Tkey key,
			      const std::string& name,
			      const char *title,
			      int nbinsx, const double xbins[],
			      int nbinsy, double ymin, double ymax,
			      bool verbose=true);
  template<class T> T *book2d(Tkey key,
			      const std::string& name,
			      const char *title,
			      int nbinsx, double xmin, double xmax,
			      int nbinsy, const double ybins[],
			      bool verbose=true);

  template<class T> void book1d(const std::vector<HistoParams_t>& v_pars);
  template<class T> void book2d(const std::vector<HistoParams_t>& v_pars);
  template<class T> void book3d(const std::vector<HistoParams_t>& v_pars);

  // Auto-fill booking routines:
  template<class T> T   *book1d(const HistoAutoFill_t& haf,bool verbose=true);
  template<class T> T   *book2d(const HistoAutoFill_t& haf,bool verbose=true);
  template<class T> T   *book3d(const HistoAutoFill_t& haf,bool verbose=true);
  template<class T> void book1d(const std::vector<HistoAutoFill_t>& v_haf);
  template<class T> void book2d(const std::vector<HistoAutoFill_t>& v_haf);
  template<class T> void book3d(const std::vector<HistoAutoFill_t>& v_haf);

  template<class T> T *bookClone(const std::string& cloneName,const T& h,
				 bool verbose=true);

  template<class T> T *get(Tkey key);

  template<class T> void fill1d(const std::map<std::string,double>& vals);
  template<class T> void fill1d(const std::string& hname,double val,double weight=1.0);
  template<class T> void fill2d(const std::map<std::string,std::pair<double,double> >& vals);
  template<class T> void fill2d(const std::string& hname,double valx,double valy,double weight=1.0);
  template<class T> void fill3d(const std::string& hname,
				double valx,double valy,double valz,double weight=1.0);

  void mkSubdir(const std::string& dirdescr);

  // Can attach another myAnalHistosTC object to this one that manages the histos in a subdir
  // The histos can be mapped with a different key type.
  //
  template<class Tsubkey>
  void mkSubdir(const std::string& dirdescr);

  template<class Tsubkey>
  myAnalHistosTC<Tsubkey> *getAttachedHisto(const std::string& name);

  bool exists(Tkey key);

  template <class Tdata> // e.g., double or float
  void autofill(void);

  void dump(void);

  void setCurDetId(DetId detId) { curDetID_ = detId; }

  TFileDirectory     *dir(void) { return dir_;    }
  const std::string& name(void) { return myname_; }

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

  // ----------member data ---------------------------

  std::string myname_;

  TFileDirectory *dir_;
  myHashmap_t *hm_histos_;
  myHashmap_t *hm_histos_af_;
  DetId        curDetID_;

  std::map<std::string,void *> m_attachedHistos_;
};

//======================================================================
typedef myAnalHistosTC<std::string> myAnalHistos; // most common use case
typedef myAnalHistosTC<uint32_t>    perChanHistos; // most common use case
//======================================================================

//
// member functions
//

//======================================================================

//
// constructors and destructor
//
template <class Tkey>
myAnalHistosTC<Tkey>::myAnalHistosTC(const std::string& dirdescr)
{
  edm::Service<TFileService> fs;
  std::cout << "Making histo folder " << dirdescr << std::endl;
  dir_ = new TFileDirectory(fs->mkdir(dirdescr));
  myname_ = dirdescr;

  hm_histos_    = new myHashmap_t;
  hm_histos_af_ = new myHashmap_t;
}

#if 0
template <class Tkey>
myAnalHistosTC<Tkey>::~myAnalHistosTC()
{
  delete dir_;
  delete hm_histos_;
  delete hm_histos_af_;
}
#endif

//======================================================================

//
// constructors and destructor
//
template <class Tkey>
myAnalHistosTC<Tkey>::myAnalHistosTC(const std::string& dirdescr,
				     TFileDirectory& subdir)
{
  std::cout << "Making histo folder " << dirdescr << " in subdir" << std::endl;

  dir_ = new TFileDirectory(subdir.mkdir(dirdescr));
  myname_ = dirdescr;

  hm_histos_    = new myHashmap_t;
  hm_histos_af_ = new myHashmap_t;
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book1d(const HistoParams_t& pars,bool verbose)
{
  Histo_t histo;
  histo.hpars = pars;

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << pars.name << std::endl;
  //std::cout << ", this=" << (int)this << std::endl;
  histo.ptr = dir_->make <T> (histo.hpars.name.c_str(),
			      histo.hpars.title.c_str(),
			      histo.hpars.nbinsx, histo.hpars.minx, histo.hpars.maxx);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(histo.hpars.name,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << pars.name << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template <class T>
T *myAnalHistosTC<Tkey>::book1d(Tkey key, const HistoParams_t& pars,bool verbose)
{
  Histo_t histo;
  histo.hpars = pars;

  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << histo.hpars.name << std::endl;
  //std::cout << ", this=" << (int)this << std::endl;
  histo.ptr = dir_->make <T> (histo.hpars.name.c_str(),
			      histo.hpars.title.c_str(),
			      histo.hpars.nbinsx, histo.hpars.minx, histo.hpars.maxx);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(key,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << key << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

//
// member functions
//
template <class Tkey>
template<class T>
void myAnalHistosTC<Tkey>::book1d(const std::vector<HistoParams_t>& v_pars)
{
  for (uint32_t i=0; i<v_pars.size(); i++)
    book1d<T>(v_pars[i]);
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book2d(const HistoParams_t& pars,bool verbose)
{
  Histo_t histo;
  histo.hpars = pars;

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << histo.hpars.name << std::endl;
  histo.ptr = dir_->make <T> (histo.hpars.name.c_str(), histo.hpars.title.c_str(),
			      histo.hpars.nbinsx, histo.hpars.minx, histo.hpars.maxx,
			      histo.hpars.nbinsy, histo.hpars.miny, histo.hpars.maxy);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(histo.hpars.name,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << pars.name << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book2d(Tkey key, const HistoParams_t& pars,bool verbose)
{
  Histo_t histo;
  histo.hpars = pars;

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << histo.hpars.name << std::endl;
  histo.ptr = dir_->make <T> (histo.hpars.name.c_str(), histo.hpars.title.c_str(),
			      histo.hpars.nbinsx, histo.hpars.minx, histo.hpars.maxx,
			      histo.hpars.nbinsy, histo.hpars.miny, histo.hpars.maxy);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(key,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << key << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
void myAnalHistosTC<Tkey>::book2d(const std::vector<HistoParams_t>& v_pars)
{
  for (uint32_t i=0; i<v_pars.size(); i++)
    book2d<T>(v_pars[i]);
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book2d(const std::string& name,
				const char *title,
				int nbinsx, const double xbins[],
				int nbinsy, double ymin, double ymax,
				bool verbose)
{
  Histo_t histo;
  histo.hpars.name = name;
  histo.hpars.title=std::string(title);
  histo.hpars.nbinsx=nbinsx;
  histo.hpars.nbinsy=nbinsy;
  histo.hpars.miny=ymin;
  histo.hpars.maxy=ymax;

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << name << std::endl;
  histo.ptr = dir_->make <T> (name.c_str(), title,
			      nbinsx, xbins,
			      nbinsy, ymin, ymax);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(histo.hpars.name,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << name << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book2d(const std::string& name,
				const char *title,
				int nbinsx, double xmin, double xmax,
				int nbinsy, const double ybins[],
				bool verbose)
{
  Histo_t histo;
  histo.hpars.name=name;
  histo.hpars.title=std::string(title);
  histo.hpars.nbinsx=nbinsx;
  histo.hpars.minx=xmin;
  histo.hpars.maxx=xmax;
  histo.hpars.nbinsy=nbinsy;

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << name << std::endl;
  histo.ptr = dir_->make <T> (name.c_str(), title,
			      nbinsx, xmin, xmax,
			      nbinsy, ybins);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(histo.hpars.name,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << name << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book2d(Tkey key,
				const std::string& name,
				const char *title,
				int nbinsx, const double xbins[],
				int nbinsy, double ymin, double ymax,
				bool verbose)
{
  Histo_t histo;
  histo.hpars.name = name;
  histo.hpars.title=std::string(title);
  histo.hpars.nbinsx=nbinsx;
  histo.hpars.nbinsy=nbinsy;
  histo.hpars.miny=ymin;
  histo.hpars.maxy=ymax;

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << name << std::endl;
  histo.ptr = dir_->make <T> (name.c_str(), title,
			      nbinsx, xbins,
			      nbinsy, ymin, ymax);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(key,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << key << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book2d(Tkey key,
				const std::string& name,
				const char *title,
				int nbinsx, double xmin, double xmax,
				int nbinsy, const double ybins[],
				bool verbose)
{
  Histo_t histo;
  histo.hpars.name=name;
  histo.hpars.title=std::string(title);
  histo.hpars.nbinsx=nbinsx;
  histo.hpars.minx=xmin;
  histo.hpars.maxx=xmax;
  histo.hpars.nbinsy=nbinsy;

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << name << std::endl;
  histo.ptr = dir_->make <T> (name.c_str(), title,
			      nbinsx, xmin, xmax,
			      nbinsy, ybins);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(key,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << key << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book3d(const HistoParams_t& pars,bool verbose)
{
  Histo_t histo;
  histo.hpars = pars;

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << histo.hpars.name << std::endl;
  histo.ptr = dir_->make <T> (histo.hpars.name.c_str(), histo.hpars.title.c_str(),
			      histo.hpars.nbinsx, histo.hpars.minx, histo.hpars.maxx,
			      histo.hpars.nbinsy, histo.hpars.miny, histo.hpars.maxy,
			      histo.hpars.nbinsz, histo.hpars.minz, histo.hpars.maxz);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(histo.hpars.name,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << pars.name << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book3d(Tkey key, const HistoParams_t& pars,bool verbose)
{
  Histo_t histo;
  histo.hpars = pars;

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking standard histogram " << histo.hpars.name << std::endl;
  histo.ptr = dir_->make <T> (histo.hpars.name.c_str(), histo.hpars.title.c_str(),
			      histo.hpars.nbinsx, histo.hpars.minx, histo.hpars.maxx,
			      histo.hpars.nbinsy, histo.hpars.miny, histo.hpars.maxy,
			      histo.hpars.nbinsz, histo.hpars.minz, histo.hpars.maxz);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(key,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << key << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
void myAnalHistosTC<Tkey>::book3d(const std::vector<HistoParams_t>& v_pars)
{
  for (uint32_t i=0; i<v_pars.size(); i++)
    book3d<T>(v_pars[i]);
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book1d(const HistoAutoFill_t& haf,bool verbose)
{
  Histo_t histo;
  histo.hpars  = haf.hpars;
  histo.afpars = haf.afpars;

  if (!histo.afpars.filladdrx) {
    throw cms::Exception("Can't book auto-fill histo with null filladdrx "+haf.hpars.name+"! ")
      << std::endl;
  }

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking autofill histogram " << histo.hpars.name << std::endl;
  //std::cout << ", this=" << (int)this << std::endl;
  histo.ptr = dir_->make <T> (histo.hpars.name.c_str(),
			      histo.hpars.title.c_str(),
			      histo.hpars.nbinsx, histo.hpars.minx, histo.hpars.maxx);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_af_->insert(std::pair<Tkey,Histo_t>(histo.hpars.name,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << haf.hpars.name << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book2d(const HistoAutoFill_t& haf,bool verbose)
{
  Histo_t histo;
  histo.hpars  = haf.hpars;
  histo.afpars = haf.afpars;

  if (!histo.afpars.filladdrx) {
    throw cms::Exception("Can't book auto-fill histo with null filladdrx "+haf.hpars.name+"! ")
      << std::endl;
  }

  if (!histo.afpars.filladdry) {
    throw cms::Exception("Can't book 2d auto-fill histo with null filladdry "+haf.hpars.name+"! ")
      << std::endl;
  }

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking autofill histogram " << histo.hpars.name << std::endl;
  histo.ptr = dir_->make <T> (histo.hpars.name.c_str(), histo.hpars.title.c_str(),
			      histo.hpars.nbinsx, histo.hpars.minx, histo.hpars.maxx,
			      histo.hpars.nbinsy, histo.hpars.miny, histo.hpars.maxy);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_af_->insert(std::pair<Tkey,Histo_t>(histo.hpars.name,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << haf.hpars.name << std::endl;
  }
  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
T *myAnalHistosTC<Tkey>::book3d(const HistoAutoFill_t& haf,bool verbose)
{
  Histo_t histo;
  histo.hpars  = haf.hpars;
  histo.afpars = haf.afpars;

  if (!histo.afpars.filladdrx) {
    throw cms::Exception("Can't book auto-fill histo with null filladdrx "+haf.hpars.name+"! ")
      << std::endl;
  }

  if (!histo.afpars.filladdry) {
    throw cms::Exception("Can't book 2d auto-fill histo with null filladdry "+haf.hpars.name+"! ")
      << std::endl;
  }

  if (!histo.afpars.filladdrz) {
    throw cms::Exception("Can't book 2d auto-fill histo with null filladdrz "+haf.hpars.name+"! ")
      << std::endl;
  }

  //edm::LogInfo("booking histogram ") << histo.hpars.name << std::endl;
  if (verbose)
    std::cout << myname_ << ": booking autofill histogram " << histo.hpars.name << std::endl;
  histo.ptr = dir_->make <T> (histo.hpars.name.c_str(), histo.hpars.title.c_str(),
			      histo.hpars.nbinsx, histo.hpars.minx, histo.hpars.maxx,
			      histo.hpars.nbinsy, histo.hpars.miny, histo.hpars.maxy,
			      histo.hpars.nbinsz, histo.hpars.minz, histo.hpars.maxz);

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_af_->insert(std::pair<Tkey,Histo_t>(histo.hpars.name,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << haf.hpars.name << std::endl;
  }
  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
void myAnalHistosTC<Tkey>::book1d(const std::vector<HistoAutoFill_t>& v_haf)
{
  for (uint32_t i=0; i<v_haf.size(); i++)
    book1d<T>(v_haf[i]);
}

template <class Tkey>
template<class T>
void myAnalHistosTC<Tkey>::book2d(const std::vector<HistoAutoFill_t>& v_haf)
{
  for (uint32_t i=0; i<v_haf.size(); i++)
    book2d<T>(v_haf[i]);
}

template <class Tkey>
template<class T>
void myAnalHistosTC<Tkey>::book3d(const std::vector<HistoAutoFill_t>& v_haf)
{
  for (uint32_t i=0; i<v_haf.size(); i++)
    book3d<T>(v_haf[i]);
}

//======================================================================

template <class Tkey>
template<class T>
T *
myAnalHistosTC<Tkey>::bookClone(const std::string& cloneName,const T& h,
				bool verbose)
{
  if (verbose)
    std::cout << myname_ << ": booking clone histogram " << cloneName;
  Histo_t histo;
  histo.ptr = (TH1 *)dir_->make <T, T> (h);
  histo.ptr->SetName(cloneName.c_str());

  std::pair<typename myHashmap_t::iterator,bool> retpair =
    hm_histos_->insert(std::pair<Tkey,Histo_t>(cloneName,histo));

  if (!retpair.second) {
    throw cms::Exception("booking failed, key already taken: ") << cloneName << std::endl;
  }

  return (T *)histo.ptr;
}

//======================================================================

template <class Tkey>
template<class T>
void
myAnalHistosTC<Tkey>::fill1d(const std::string& hname,double val,double weight)
{
  typename myHashmap_t::const_iterator ith;

  ith = hm_histos_->find(hname);

  if (ith != hm_histos_->end()) {
    T *p = (T *)ith->second.ptr;
    if (!p) std::cout << myname_ << ": NULL POINTER for histo " << hname << "!!!" << std::endl;
    else
      p->Fill(val,weight);
  } else {
    //    edm::LogError("Couldn't find hash for " + hname + "! ") 
    throw cms::Exception("Couldn't find hash for " + hname)  <<"! val="<< val << "\tweight=" << weight << std::endl;
    //dump();
  }
}

//======================================================================

template <class Tkey>
template<class T>
void
myAnalHistosTC<Tkey>::fill1d(const std::map<std::string,double>& vals)
{
  std::map<std::string,double>::const_iterator itv;
  for (itv = vals.begin(); itv != vals.end(); itv++) {
    typename myHashmap_t::const_iterator ith;

    ith = hm_histos_.find(itv->first);
    if (ith != hm_histos_->end()) {
      T *p = (T *)ith->second.ptr;
      p->Fill(itv->second);
    } else {
      edm::LogError("Couldn't find hash for " + itv->first + "!");
      //dump();
    }
  }
}

//======================================================================

template <class Tkey>
template<class T>
void
myAnalHistosTC<Tkey>::fill2d(const std::string& hname,double valx,double valy,double weight)
{
  typename myHashmap_t::const_iterator ith;

  ith = hm_histos_->find(hname);
  if (ith != hm_histos_->end()) {
    T *p = (T *)ith->second.ptr;
    if (!p) std::cout << myname_ << ": NULL POINTER for histo " << hname << "!!!" << std::endl;
    p->Fill(valx,valy,weight);
  } else {
    std::cout << myname_ << ": couldn't find hash for "+hname << "! valx="
      << valx << "\tvaly=" << valy << "\tweight=" << weight << std::endl;
    //dump();
  }
}

//======================================================================

template <class Tkey>
template<class T>
void
myAnalHistosTC<Tkey>::fill2d(const std::map<std::string,std::pair<double,double> >& vals)
{
  std::map<std::string,std::pair<double,double> >::const_iterator itv;
  for (itv = vals.begin(); itv != vals.end(); itv++) {
    typename myHashmap_t::const_iterator ith;;

    ith = hm_histos_->find(itv->first);
    if (ith != hm_histos_->end()) {
      T *p = (T *)ith->second.ptr;
      p->Fill(itv->second.first,itv->second.second);
    } else {
      std::cout << myname_ << ": couldn't find hash for "+itv->first+"!" << std::endl;
      //dump();
    }
  }
}

//======================================================================

template <class Tkey>
template<class T>
void
myAnalHistosTC<Tkey>::fill3d(const std::string& hname,
			     double valx,double valy,double valz,double weight)
{
  typename myHashmap_t::const_iterator ith;

  ith = hm_histos_->find(hname);
  if (ith != hm_histos_->end()) {
    T *p = (T *)ith->second.ptr;
    if (!p) std::cout << myname_ << ": NULL POINTER for histo " << hname << "!!!" << std::endl;
    p->Fill(valx,valy,valz,weight);
  } else {
    std::cout << myname_ << ": couldn't find hash for "+hname << "! valx="
      << valx << "\tvaly=" << valy << "\tvalz=" << valz << "\tweight=" << weight << std::endl;
    //dump();
  }
}

//======================================================================

template <class Tkey>
template <class T>
T *
myAnalHistosTC<Tkey>::get(Tkey key)
{
  T *p = (T *)NULL;
  typename myHashmap_t::const_iterator ith = hm_histos_->find(key);
  if (ith != hm_histos_->end())
    p = (T *)ith->second.ptr;
  else {
    ith = hm_histos_af_->find(key);
    if (ith != hm_histos_af_->end())
      p = (T *)ith->second.ptr;
    else {
      std::cout << myname_ << ": couldn't get key " << key << std::endl;
      //dump();
    }
  }
  return p;
}

//======================================================================

template<class Tkey>
void
myAnalHistosTC<Tkey>::mkSubdir(const std::string& dirdescr)
{
  myAnalHistosTC<Tkey> *subdir = new myAnalHistosTC<Tkey>(dirdescr,*dir_);
  std::pair<std::string,void *> pair(dirdescr,(void *)subdir);

  m_attachedHistos_.insert(pair);
}

//======================================================================

template<class Tkey>
template<class Tsubkey>
void
myAnalHistosTC<Tkey>::mkSubdir(const std::string& dirdescr)
{
  myAnalHistosTC<Tsubkey> *subdir = new myAnalHistosTC<Tsubkey>(dirdescr,*dir_);
  std::pair<std::string,void *> subpair(dirdescr,(void *)subdir);

  m_attachedHistos_.insert(subpair);
}

//======================================================================

template <class Tkey>
template<class Tsubkey>
myAnalHistosTC<Tsubkey> *
myAnalHistosTC<Tkey>::getAttachedHisto(const std::string& name)
{
  typename std::map<std::string,void *>::const_iterator it;
  it = m_attachedHistos_.find(name);
  if (it != m_attachedHistos_.end())
    return (myAnalHistosTC<Tsubkey> *)it->second;
  return NULL;
}

//======================================================================

template <class Tkey>
bool
myAnalHistosTC<Tkey>::exists(Tkey key)
{
  return ((hm_histos_->find(key)    != hm_histos_->end()) ||
	  (hm_histos_af_->find(key) != hm_histos_af_->end()));
}

//======================================================================

template <class Tkey>
void
myAnalHistosTC<Tkey>::dump(void)
{
  typename myHashmap_t::const_iterator ith;
  std::cout << myname_ << ": ";
  for (ith = hm_histos_->begin(); ith != hm_histos_->end(); ith++)
    std::cout << ith->first << " ";
  std::cout << std::endl;
}

//======================================================================

template <class Tkey>
template <class Tdata>
void
myAnalHistosTC<Tkey>::autofill(void)
{
  typename myHashmap_t::const_iterator ith;
  for (ith  = hm_histos_af_->begin();
       ith != hm_histos_af_->end();
       ith++) {
    const Histo_t& h = ith->second;
    if (h.hpars.nbinsz) {                   // 3d histos
      TH3 *h3 = (TH3 *)h.ptr;
      if (!h.afpars.detIDfun || (*h.afpars.detIDfun)(curDetID_)) {
	h3->Fill(*(Tdata *)h.afpars.filladdrx,
		 *(Tdata *)h.afpars.filladdry,
		 *(Tdata *)h.afpars.filladdrz,
		 (h.afpars.filladdrw ? (*(Tdata *)h.afpars.filladdrw) : 1));
      }
    }
    else if (h.hpars.nbinsy) {              // 2d histos
      TH2 *h2 = (TH2 *)h.ptr;
      if (!h.afpars.detIDfun || (*h.afpars.detIDfun)(curDetID_)) {
	h2->Fill(*(Tdata *)h.afpars.filladdrx,
		 *(Tdata *)h.afpars.filladdry,
		 (h.afpars.filladdrw ? (*(Tdata *)h.afpars.filladdrw) : 1));
      }
    } else {                               // 1d histos
      TH1 *h1 = (TH1 *)h.ptr;
      if (!h.afpars.detIDfun || (*h.afpars.detIDfun)(curDetID_))
	h1->Fill(*(Tdata *)h.afpars.filladdrx,
		 (h.afpars.filladdrw ? (*(Tdata *)h.afpars.filladdrw) : 1));
    }
  } // histo loop
}

#endif // _MYANALHISTOS
