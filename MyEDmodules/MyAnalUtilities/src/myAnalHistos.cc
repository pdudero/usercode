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
}                                     // add1dHisto

//======================================================================

void
add2dHisto(const std::string& name, const std::string& title,
	   int nbinsx, double minx, double maxx,
	   int nbinsy, double miny, double maxy,
	   std::vector<myAnalHistos::HistoParams_t>& v_hpars2d)
{
  myAnalHistos::HistoParams_t hpars2d(name,title,nbinsx,minx,maxx,nbinsy,miny,maxy);
  v_hpars2d.push_back(hpars2d);
}                                     // add2dHisto

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
}                                     // add3dHisto

//======================================================================

void
add1dAFhisto(const std::string& name, const std::string& title,
	     int nbinsx, double minx, double maxx,
	     void *filladdrx, void *filladdrw, detIDfun_t detIDfun,
	     std::vector<myAnalHistos::HistoAutoFill_t>& v_hpars1d)
{
  myAnalHistos::HistoAutoFill_t haf1d;
  haf1d.hpars.name       = name;
  haf1d.hpars.title      = title;
  haf1d.hpars.nbinsx     = nbinsx;
  haf1d.hpars.minx       = minx;
  haf1d.hpars.maxx       = maxx;
  haf1d.hpars.nbinsy     = 0;
  haf1d.hpars.nbinsz     = 0;
  haf1d.afpars.filladdrx = filladdrx;
  haf1d.afpars.filladdry = NULL;
  haf1d.afpars.filladdrw = filladdrw;
  haf1d.afpars.detIDfun  = detIDfun;
  v_hpars1d.push_back(haf1d);
}                                     // add1dHisto

//======================================================================

void
add2dAFhisto(const std::string& name, const std::string& title,
	     int nbinsx, double minx, double maxx,
	     int nbinsy, double miny, double maxy,
	     void *filladdrx, void *filladdry, void *filladdrw,
	     detIDfun_t detIDfun,
	     std::vector<myAnalHistos::HistoAutoFill_t>& v_hpars2d)
{
  myAnalHistos::HistoAutoFill_t haf2d;
  haf2d.hpars.name       = name;
  haf2d.hpars.title      = title;
  haf2d.hpars.nbinsx     = nbinsx;
  haf2d.hpars.minx       = minx;
  haf2d.hpars.maxx       = maxx;
  haf2d.hpars.nbinsy     = nbinsy;
  haf2d.hpars.nbinsz     = 0;
  haf2d.hpars.miny       = miny;
  haf2d.hpars.maxy       = maxy;
  haf2d.afpars.filladdrx = filladdrx;
  haf2d.afpars.filladdry = filladdry;
  haf2d.afpars.filladdrw = filladdrw;
  haf2d.afpars.detIDfun  = detIDfun;
  v_hpars2d.push_back(haf2d);
}                                   // add2dAFhisto
