#include <vector>
#include <string>
#include "../interface/myAnalHistos.hh"

//======================================================================

void
add1dHisto(const std::string& name, const std::string& title,
	   int nbinsx, double minx, double maxx,
	   std::vector<myAnalHistos::HistoParams_t>& v_hpars1d)
{
  myAnalHistos::HistoParams_t hpars1d(name,title,nbinsx,minx,maxx);
  v_hpars1d.push_back(hpars1d);
}                                                          // add1dHisto

//======================================================================

void
add2dHisto(const std::string& name, const std::string& title,
	   int nbinsx, double minx, double maxx,
	   int nbinsy, double miny, double maxy,
	   std::vector<myAnalHistos::HistoParams_t>& v_hpars2d)
{
  myAnalHistos::HistoParams_t hpars2d(name,title,nbinsx,minx,maxx,nbinsy,miny,maxy);
  v_hpars2d.push_back(hpars2d);
}                                                          // add2dHisto

//======================================================================

void
add3dHisto(const std::string& name, const std::string& title,
	   int nbinsx, double minx, double maxx,
	   int nbinsy, double miny, double maxy,
	   int nbinsz, double minz, double maxz,
	   std::vector<myAnalHistos::HistoParams_t>& v_hpars3d)
{
  myAnalHistos::HistoParams_t hpars3d(name,title,
				      nbinsx,minx,maxx,
				      nbinsy,miny,maxy,
				      nbinsz,minz,maxz);
  v_hpars3d.push_back(hpars3d);
}                                                          // add3dHisto

//======================================================================

void
add1dAFhisto(const std::string& name, const std::string& title,
	     int nbinsx, double minx, double maxx,
	     void *filladdrx, void *filladdrw, detIDfun_t detIDfun,
	     std::vector<myAnalHistos::HistoAutoFill_t>& v_hpars1d)
{
  myAnalHistos::HistoParams_t   hpars(name,title,nbinsx,minx,maxx);
  myAnalHistos::AutoFillPars_t  afpars(filladdrx,NULL,NULL,filladdrw,detIDfun);
  myAnalHistos::HistoAutoFill_t haf1d(hpars,afpars);
  v_hpars1d.push_back(haf1d);
}                                                        // add1dAFHisto

//======================================================================

void
add2dAFhisto(const std::string& name, const std::string& title,
	     int nbinsx, double minx, double maxx,
	     int nbinsy, double miny, double maxy,
	     void *filladdrx, void *filladdry, void *filladdrw,
	     detIDfun_t detIDfun,
	     std::vector<myAnalHistos::HistoAutoFill_t>& v_hpars2d)
{
  myAnalHistos::HistoParams_t   hpars(name,title,nbinsx,minx,maxx,nbinsy,miny,maxy);
  myAnalHistos::AutoFillPars_t  afpars(filladdrx,filladdry,NULL,filladdrw,detIDfun);
  myAnalHistos::HistoAutoFill_t haf2d(hpars,afpars);
  v_hpars2d.push_back(haf2d);
}                                                        // add2dAFhisto

//======================================================================

void
add3dAFhisto(const std::string& name, const std::string& title,
	     int nbinsx, double minx, double maxx,
	     int nbinsy, double miny, double maxy,
	     int nbinsz, double minz, double maxz,
	     void *filladdrx, void *filladdry, void *filladdrz,
	     void *filladdrw,detIDfun_t detIDfun,
	     std::vector<myAnalHistos::HistoAutoFill_t>& v_hpars3d)
{
  myAnalHistos::HistoParams_t   hpars(name,title,nbinsx,minx,maxx,nbinsy,miny,maxy,nbinsz,minz,maxz);
  myAnalHistos::AutoFillPars_t  afpars(filladdrx,filladdry,filladdrz,filladdrw,detIDfun);
  myAnalHistos::HistoAutoFill_t haf3d(hpars,afpars);
  v_hpars3d.push_back(haf3d);
}                                                        // add3dAFhisto
