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
#include <iostream>
#include <vector>

// user include files
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "MyEDmodules/MyAnalUtilities/interface/myAnalHistos.hh"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"

#include "TH1D.h"
#include "TH2D.h"

//======================================================================

//
// constructors and destructor
//
myAnalHistos::myAnalHistos(const std::string& dirdescr)
{
  edm::Service<TFileService> fs;
  dir_ = new TFileDirectory(fs->mkdir(dirdescr));
}

//======================================================================

//
// member functions
//
template<class T>
T *myAnalHistos::book1d(const HistoParams_t& pars)
{
  std::cout << "booking histogram " << pars.name << std::endl;
  return dir_->make <T> (pars.name.c_str(), pars.title.c_str(),
			pars.nbinsx, pars.minx, pars.maxx);
}
//======================================================================

template<class T>
T *myAnalHistos::book2d(const HistoParams_t& pars)
{
  std::cout << "booking histogram " << pars.name << std::endl;
  return dir_->make <T> (pars.name.c_str(), pars.title.c_str(),
			pars.nbinsx, pars.minx, pars.maxx,
			pars.nbinsy, pars.miny, pars.maxy);
}
//======================================================================

void
myAnalHistos::book(const std::vector<HistoParams_t>& v_pars)
{
  for (uint32_t i=0; i<v_pars.size(); i++) {
    Histo_t histo;
    histo.pars = v_pars[i];
    if (histo.pars.nbinsy) histo.ptr = (TH1 *)book2d<TH2D>(histo.pars);
    else                   histo.ptr = (TH1 *)book1d<TH1D>(histo.pars);
    hm_histos_[histo.pars.name.c_str()] = histo;
  }
}

//======================================================================

void
myAnalHistos::fill1d(std::string& hname,double val,double weight)
{
  myHashmap_t::const_iterator ith;

  ith = hm_histos_.find(hname.c_str());
  if (ith != hm_histos_.end()) {
    TH1D *p = (TH1D *)ith->second.ptr;
    p->Fill(val,weight);
  }
}

//======================================================================

void
myAnalHistos::fill1d(std::map<std::string,double>& vals)
{
  std::map<std::string,double>::const_iterator itv;
  for (itv = vals.begin(); itv != vals.end(); itv++) {
    myHashmap_t::const_iterator ith;

    ith = hm_histos_.find(itv->first.c_str());
    if (ith != hm_histos_.end()) {
      TH1D *p = (TH1D *)ith->second.ptr;
      p->Fill(itv->second);
    }
  }
}

//======================================================================

void
myAnalHistos::fill2d(std::string& hname,double valx,double valy,double weight)
{
  myHashmap_t::const_iterator ith;

  ith = hm_histos_.find(hname.c_str());
  if (ith != hm_histos_.end()) {
    TH2D *p = (TH2D *)ith->second.ptr;
    p->Fill(valx,valy,weight);
  }
}

//======================================================================

void
myAnalHistos::fill2d(std::map<std::string,std::pair<double,double> >& vals)
{
  std::map<std::string,std::pair<double,double> >::const_iterator itv;
  for (itv = vals.begin(); itv != vals.end(); itv++) {
    myHashmap_t::const_iterator ith;

    ith = hm_histos_.find(itv->first.c_str());
    if (ith != hm_histos_.end()) {
      TH2D *p = (TH2D *)ith->second.ptr;
      p->Fill(itv->second.first,itv->second.second);
    }
  }
}
