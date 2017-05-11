//
#include <string>
#include "TH1D.h"
#include "TProfile.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TStyle.h"
#include "TF1.h"
#include "TPaveStats.h"

template <class T>
class MyHistoWrapper {
public:
  MyHistoWrapper<T>(const std::string& name,
		    const std::string& title,
		    int nbins, float min, float max);
  MyHistoWrapper<T>(T *h, const std::string& name, const std::string& title);
  MyHistoWrapper<T>(T *h);
  ~MyHistoWrapper() {}
  inline T *histo() const {return h_;}

  void   Draw(void)                       { Draw(drawoption_); }
  void   DrawSame(void)                   { Draw(drawoption_+"same"); }
  void   DrawSames(void)                  { Draw(drawoption_+"sames"); }
  void   Draw(const std::string& drawopt);

  // Only non-default values get set.
  void   SetXaxis(const std::string& title,
		  bool center = false,
		  float titlesize=0.0,
		  float titleoffs=0.0,
		  int   titlefont=0,
		  float labelsize=0.0,
		  int   labelfont=0,
		  float rangeuserMin=1e99,
		  float rangeuserMax=-1e99,
		  int   ndiv=510);
  void   SetYaxis(const std::string& title,
		  bool center = false,
		  float titlesize=0.0,
		  float titleoffs=0.0,
		  int   titlefont=0,
		  float labelsize=0.0,
		  int   labelfont=0,
		  float rangeuserMin=1e99,
		  float rangeuserMax=-1e99,
		  int   ndiv=510);
  void   SetZaxis(const std::string& title,
		  bool center = false,
		  float titlesize=0.0,
		  float titleoffs=0.0,
		  int   titlefont=0,
		  float labelsize=0.0,
		  int   labelfont=0,
		  float rangeuserMin=1e99,
		  float rangeuserMax=-1e99,
		  int   ndiv=510);

  void SetAxes   (void);

  void SetLine   (int color=0,
		  int style=0,
		  int width=0);
  void SetMarker (int color=0,
		  int style=0,
		  int size=0);
  void SetStats  (bool  on=true,
		  int   optstat=0,
		  float x1ndc=0.,
		  float y1ndc=0.,
		  float x2ndc=0.,
		  float y2ndc=0.);
  void DrawStats (void);

  bool         statsAreOn    (void)                      { return statsAreOn_;           }
  int          GetOptStat    (void)                      { return stats_.GetOptStat();   }
  void         SetLegendEntry(const std::string& entry)  { legentry_   = entry;          }
  void         SetDrawOption (const std::string& option) { drawoption_ = option;         }
  void         SetLegDrawOpt (const std::string& option) { legdrawopt_ = option;         }
  void         SetFitOption  (const std::string& option) { fitoption_  = option;         }
  std::string& GetDrawOption (void)                      { return drawoption_;           }
  std::string& GetLegDrawOpt (void)                      { return legdrawopt_;           }
  std::string& GetFitOption  (void)                      { return fitoption_;            }
  void         SetFitRange   (double xmin,double xmax)   { fitxmin_=xmin; fitxmax_=xmax; }
  void         SaveStyle     (TStyle *instyle)           { style2apply_ = instyle;       }
  bool         ApplySavedStyle (void);
  std::string& GetPathInFile (void)                      { return pathInFile_;           }
  void         SetPathInFile (const std::string& inpath) { pathInFile_ = inpath;         }

  void Add2Legend(TLegend *leg) {
    if (!legdrawopt_.size()) {
      if (verbosity_)
	std::cout << "legdrawopt not set, guessing..."<<std::endl;

      if (h_->GetFillColor())                              legdrawopt_ = "F";
      else if (drawoption_.size()) {
	// if      (drawoption_.find("E") != std::string::npos)    legdrawopt_ = "LEP";
	if      (drawoption_.find("HIST") != std::string::npos) legdrawopt_ = "L";
	else if (drawoption_.find("E") != std::string::npos)    legdrawopt_ = "L";
	else if (drawoption_.find("L") != std::string::npos)    legdrawopt_ = "L";
	else if (drawoption_.find("P") != std::string::npos)    legdrawopt_ = "P";
      }
      else                                                 legdrawopt_ = "L";
    }
    if (verbosity_)
      std::cout << "legdrawopt="<<legdrawopt_<<std::endl;

    leg->AddEntry(h_,legentry_.c_str(),legdrawopt_.c_str());
  }

  const std::string& GetLegendEntry(void)   { return legentry_;  }

  void loadFitFunction(TF1 *fitfn);

  void DrawFits(const std::string& drawopt="",
		const std::string& fitopt="",
		double fitrangemin=0.,
		double fitrangemax=0.);

  void ShutUpAlready() { verbosity_ = false; }

  MyHistoWrapper<T> *Clone(const std::string& newname,
			   const std::string& newtitle);
private:
  void   SetAxis(TAxis& ax,
		 const std::string& title,
		 bool center = true,
		 float titlesize=0.0,
		 float titleoffs=0.0,
		 int   titlefont=0,
		 float labelsize=0.0,
		 int   labelfont=0,
		 float rangeuserMin=1e99,
		 float rangeuserMax=-1e99,
		 int   ndiv=510);
  T *h_;
  TAxis xax,yax,zax; /* have to save separately since the internal graph
			axes are not created until after being drawn. */

  std::string pathInFile_;
  std::string legentry_;
  std::string drawoption_;
  std::string legdrawopt_;
  std::string fitoption_;
  bool        statsAreOn_;
  bool        verbosity_;
  double      fitxmin_,fitxmax_;
  TPaveStats  stats_; // placeholder for storing stats options.

  TStyle     *style2apply_;

  std::vector<TF1 *> v_fits_;
};

//======================================================================

template<typename T>
MyHistoWrapper<T>::MyHistoWrapper(const std::string& name,
				  const std::string& title,
				  int nbins, float min, float max)
{
  cout << "MyHistWrapper::MyHistWrapper(nm,t,nb,mn,mx)"<<endl;

  h_ = new T(name.c_str(),title.c_str(),nbins,min,max);
  drawoption_=std::string("");
  style2apply_ = NULL;
  statsAreOn_ = false;
  verbosity_=true;
  fitxmin_=fitxmax_=0;

  xax.ResetAttAxis("X");
  yax.ResetAttAxis("Y");
  zax.ResetAttAxis("Z");

  h_->GetXaxis()->Copy(xax);
  h_->GetYaxis()->Copy(yax);
  h_->GetZaxis()->Copy(zax);

  xax.SetLimits(-1,-1);
  yax.SetLimits(-1,-1);
  zax.SetLimits(-1,-1);

#if 0
  if(h_->GetXaxis()->GetXbins()->GetArray())
    xax.Set(h_->GetNbinsX(),h_->GetXaxis()->GetXbins()->GetArray());
  else
    xax.Set(h_->GetNbinsX(),h_->GetXaxis()->GetXmin(),h_->GetXaxis()->GetXmax());
  if(h_->GetYaxis()->GetXbins()->GetArray())
    yax.Set(h_->GetNbinsY(),h_->GetYaxis()->GetXbins()->GetArray());
  else
    yax.Set(h_->GetNbinsY(),h_->GetYaxis()->GetXmin(),h_->GetYaxis()->GetXmax());
  if(h_->GetZaxis()->GetXbins()->GetArray())
    zax.Set(h_->GetNbinsZ(),h_->GetZaxis()->GetXbins()->GetArray());
  else
    zax.Set(h_->GetNbinsZ(),h_->GetZaxis()->GetXmin(),h_->GetZaxis()->GetXmax());

  cout<<"xax = "<<xax.GetNbins()<<","<<xax.GetXmin()<<","<<xax.GetXmax()<<endl;
  cout<<"yax = "<<yax.GetNbins()<<","<<yax.GetXmin()<<","<<yax.GetXmax()<<endl;
  cout<<"zax = "<<zax.GetNbins()<<","<<zax.GetXmin()<<","<<zax.GetXmax()<<endl;
#endif
}           //MyHistoWrapper<T>::MyHistoWrapper(name,title, nbins, min, max)

//======================================================================

template<typename T>
MyHistoWrapper<T>::MyHistoWrapper(T *h, const std::string& name, const std::string& title) : h_(h)
{
  cout << "MyHistWrapper::MyHistWrapper(h)"<<endl;

  if (name.size())  h_->SetName (name.c_str());
  if (title.size()) h_->SetTitle(title.c_str());
  style2apply_ = NULL;
  statsAreOn_ = false;
  verbosity_=true;
  fitxmin_=fitxmax_=0;

  xax.ResetAttAxis("X");
  yax.ResetAttAxis("Y");
  zax.ResetAttAxis("Z");

  h->GetXaxis()->Copy(xax);
  h->GetYaxis()->Copy(yax);
  h->GetZaxis()->Copy(zax);

  xax.SetLimits(-1,-1);
  yax.SetLimits(-1,-1);
  zax.SetLimits(-1,-1);

#if 0
  if(h_->GetXaxis()->GetXbins()->GetArray())
    xax.Set(h_->GetNbinsX(),h_->GetXaxis()->GetXbins()->GetArray());
  else
    xax.Set(h_->GetNbinsX(),h_->GetXaxis()->GetXmin(),h_->GetXaxis()->GetXmax());
  if(h_->GetYaxis()->GetXbins()->GetArray())
    yax.Set(h_->GetNbinsY(),h_->GetYaxis()->GetXbins()->GetArray());
  else
    yax.Set(h_->GetNbinsY(),h_->GetYaxis()->GetXmin(),h_->GetYaxis()->GetXmax());
  if(h_->GetZaxis()->GetXbins()->GetArray())
    zax.Set(h_->GetNbinsZ(),h_->GetZaxis()->GetXbins()->GetArray());
  else
    zax.Set(h_->GetNbinsZ(),h_->GetZaxis()->GetXmin(),h_->GetZaxis()->GetXmax());

  cout<<"xax = "<<xax.GetNbins()<<","<<xax.GetXmin()<<","<<xax.GetXmax()<<endl;
  cout<<"yax = "<<yax.GetNbins()<<","<<yax.GetXmin()<<","<<yax.GetXmax()<<endl;
  cout<<"zax = "<<zax.GetNbins()<<","<<zax.GetXmin()<<","<<zax.GetXmax()<<endl;
#endif
}                  //MyHistoWrapper<T>::MyHistoWrapper(T *h, name,title)

//======================================================================

template <typename T>
MyHistoWrapper<T>:: MyHistoWrapper(T *h) : h_(h)
{
  assert(h);
  style2apply_=NULL;
  verbosity_=true;
  statsAreOn_=false;
  fitxmin_=fitxmax_=0; 

  xax.ResetAttAxis("X");
  yax.ResetAttAxis("Y");
  zax.ResetAttAxis("Z");

  h->GetXaxis()->Copy(xax);
  h->GetYaxis()->Copy(yax);
  h->GetZaxis()->Copy(zax);

  xax.SetLimits(-1,-1);
  yax.SetLimits(-1,-1);
  zax.SetLimits(-1,-1);

}                              //MyHistoWrapper<T>::MyHistoWrapper(T *h)

//======================================================================

template <typename T>
MyHistoWrapper<T> *
MyHistoWrapper<T>::Clone(const std::string& newname,
			 const std::string& newtitle)
{
  cout << "MyHistWrapper::Clone"<<endl;
  T *newh = (T *)h_->Clone(newname.c_str());
  newh->SetTitle(newtitle.c_str());

  return new MyHistoWrapper<T>(newh);
}

//======================================================================

template<typename T>
void   
MyHistoWrapper<T>::SetXaxis(const std::string& t,bool c,
			    float ts, float to, int tf,
			    float ls, int lf,
			    float ruMin, float ruMax,int nd) {
  SetAxis(xax,t,c,ts,to,tf,ls,lf,ruMin,ruMax,nd);
}

//======================================================================

template<typename T>
void   
MyHistoWrapper<T>::SetYaxis(const std::string& t,bool c,
			    float ts, float to, int tf,
			    float ls, int lf,
			    float ruMin, float ruMax,int nd) {
  SetAxis(yax,t,c,ts,to,tf,ls,lf,ruMin,ruMax,nd);
}

//======================================================================

template<typename T>
void   
MyHistoWrapper<T>::SetZaxis(const std::string& t,bool c,
			    float ts, float to, int tf,
			    float ls, int lf,
			    float ruMin, float ruMax,int nd) {
  SetAxis(zax,t,c,ts,to,tf,ls,lf,ruMin,ruMax,nd);
}

//======================================================================

template<typename T>
void   
MyHistoWrapper<T>::SetAxis(TAxis& ax,
			   const std::string& title,
			   bool center,
			   float titlesize,
			   float titleoffs,
			   int   titlefont,
			   float labelsize,
			   int   labelfont,
			   float min,
			   float max,
			   int   ndiv)
{
  if (title.size())    ax.SetTitle(title.c_str());
  if (center)          ax.CenterTitle();
  if (titlesize > 0.0) ax.SetTitleSize(titlesize);
  if (titleoffs > 0.0) ax.SetTitleOffset(titleoffs);
  if (titlefont > 0)   ax.SetTitleFont(titlefont);
  if (labelsize > 0.0) ax.SetLabelSize(labelsize);
  if (labelfont > 0)   ax.SetLabelFont(labelfont);

  if (min < max) {
    //ax.SetLimits(min,max);
    ax.SetRangeUser(min,max);
    if (verbosity_) {
      cout << "Setting limits for histo "<< h_->GetName();
      cout << " to "<<min<<"-"<<max<<endl;
    }
  }

  //if (ndiv != 510)     ax.SetNdivisions(ndiv,kFALSE);
  if (ndiv != 510)     ax.SetNdivisions(ndiv);

}                                          // MyHistoWrapper<T>::SetAxis

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::SetAxes(void)
{
  h_->GetXaxis()->ImportAttributes(&xax);
  h_->GetYaxis()->ImportAttributes(&yax);
  h_->GetZaxis()->ImportAttributes(&zax);

#if 1
  if( xax.GetXmax()>xax.GetXmin() ) h_->GetXaxis()->SetRangeUser(xax.GetXmin(),xax.GetXmax());
  if( yax.GetXmax()>yax.GetXmin() ) h_->GetYaxis()->SetRangeUser(yax.GetXmin(),yax.GetXmax());
  if( zax.GetXmax()>zax.GetXmin() ) h_->GetZaxis()->SetRangeUser(zax.GetXmin(),zax.GetXmax());
#endif
}                                          // MyHistoWrapper<T>::SetAxes

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::SetLine(int color, int style, int width)
{
  if (color >  0) h_->SetLineColor(color);
  if (style)      h_->SetLineStyle(style);
  if (width)      h_->SetLineWidth(width);
}

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::SetMarker(int color, int style, int size)
{
  if (color > 0)  h_->SetMarkerColor(color);
  if (style)      h_->SetMarkerStyle(style);
  if (size)       h_->SetMarkerSize(size);
}

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::SetStats(bool  on,
			    int   optstat,
			    float x1ndc,
			    float y1ndc,
			    float x2ndc,
			    float y2ndc)
{
  statsAreOn_ = on;
  h_->SetStats(on);
  if (on && optstat)
    stats_.SetOptStat(optstat);
  if (x1ndc > 0.0) stats_.SetX1NDC(x1ndc);
  if (y1ndc > 0.0) stats_.SetY1NDC(y1ndc);
  if (x2ndc > 0.0) stats_.SetX2NDC(x2ndc);
  if (y2ndc > 0.0) stats_.SetY2NDC(y2ndc);
}

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::Draw(const std::string& drawopt)
{
  if (drawopt.find("COL") != std::string::npos)
    gStyle->SetPalette(1,0); // always!

  if (verbosity_) {
    std::cout << "MyHistoWrapper::Draw: Drawing "<<h_->GetName();
    std::cout << " with option(s) "<<drawopt<<std::endl;
  }
  h_->Print();
  h_->Draw(drawopt.c_str());
}

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::loadFitFunction(TF1 *fitfn)
{
  if (fitfn) {
    // check the range of the function
    double fitmin,fitmax,hmin,hmax;
    fitfn->GetRange(fitmin,fitmax);
    hmin = h_->GetXaxis()->GetXmin();
    hmax = h_->GetXaxis()->GetXmax();
    if ((fitmax < hmin) || (fitmax > hmax) ||
	(fitmin < hmin) || (fitmin > hmax)) {
      fitmin = hmin;
      fitmax = hmax;
    }
    if (verbosity_) {
      std::cout << "MyHistoWrapper::loadFitFunction: ";
      std::cout << "resetting TF1 " << fitfn->GetName() <<" range ";
      std::cout << fitmin << " - " << fitmax << std::endl;
    }
    fitfn->SetRange(fitmin,fitmax);

    v_fits_.push_back(fitfn);
  }
}                                                     // loadFitFunction

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::DrawFits(const std::string& drawopt,
			    const std::string& fitopt,
			    double fitrangemin,
			    double fitrangemax)
{
  std::string option;
  if (fitopt.size())
    option = fitopt; // overrides stored option
  else
    option = fitoption_;

  for (size_t i=0; i<v_fits_.size(); i++)  {
    TF1 *f1 = v_fits_[i];

    double xmin=fitrangemin,xmax=fitrangemax; // overrides stored ranges
    if (xmin >= xmax) {
      if (fitxmin_ < fitxmax_) {
	xmin = fitxmin_; xmax = fitxmax_;
      } else {
	f1->GetRange(xmin,xmax);
      }
    }
    if (verbosity_) {
      std::cout << "MyHistoWrapper::DrawFits: Drawing fit "<<f1->GetName();
      std::cout << " with fitoption(s) "<<option;
      std::cout << " and drawoption(s) "<<drawoption_+drawopt;
      std::cout << " and range " << xmin << " to " << xmax << std::endl;
    } else 
      option+=" Q";

    h_->Fit(f1,option.c_str(),"same",xmin,xmax); // (drawoption_+drawopt).c_str(),xmin,xmax);

    if (verbosity_) {
      double *pars = f1->GetParameters();
      printf("*Precision* printout of fit parameters for TF1 %s:\n",
	     f1->GetName());
      for (int j=0; j<f1->GetNpar(); j++) {
	printf("Par %2d:  %.18g\n", j, pars[j]);
      }
      printf("Chi^2/NDF = %f/%d\n",f1->GetChisquare(),f1->GetNDF());
    }
    //f1->Draw(drawopt.c_str());
  }
}                                                            // DrawFits

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::DrawStats(void)
{
  TPaveStats *st1 = (TPaveStats*)h_->GetListOfFunctions()->FindObject("stats");
  if (!st1) {
    std::cerr << "Error, couldn't find stats object for ";
    std::cerr << h_->GetName() << std::endl;
    return;
  }

  st1->UseCurrentStyle();
  //st1->SetTextSize(0.03);
  st1->SetTextFont(42);

  double x1 = stats_.GetX1NDC();
  double x2 = stats_.GetX2NDC();
  double y1 = stats_.GetY1NDC();
  double y2 = stats_.GetY2NDC();

  if (verbosity_) {
    std::cout << "MyHistoWrapper::DrawStats: ";
    std::cout << "Drawing Stats Object for " << h_->GetName() << std::endl;
    std::cout << "with OptStat = " << stats_.GetOptStat() << std::endl;
    std::cout << "at (X=";
    std::cout << x1 << "-" << x2 << ", Y=" << y1 << "-" << y2 << ")";
  }

  if ((x1 > 0)  && (x1 < 1.) &&
      (x2 > x1) && (x2 < 1.) &&
      (y1 > 0)  && (y1 < 1.) &&
      (y2 > y1) && (y2 < 1.)) {
    st1->SetX1NDC(x1); st1->SetX2NDC(x2);
    st1->SetY1NDC(y1); st1->SetY2NDC(y2);
    if (verbosity_)
    std::cout << std::endl;
  } else if (verbosity_)
    std::cout<<"Sorry, Charlie"<<std::endl;


  if (drawoption_ == "P") {
    st1->SetLineColor(h_->GetMarkerColor());
    st1->SetTextColor(h_->GetMarkerColor());
  } else {
    st1->SetLineColor(h_->GetLineColor());
    st1->SetTextColor(h_->GetLineColor());
  }
  st1->SetOptStat(stats_.GetOptStat());
  st1->Draw();
}                                                           // DrawStats

//======================================================================

template<typename T>
bool
MyHistoWrapper<T>::ApplySavedStyle (void)
{
  if (!style2apply_) return false;
  TStyle *temp = gStyle;
  style2apply_->cd();
  if (verbosity_) {
    std::cout << "MyHistoWrapper::ApplySavedStyle: ";
    std::cout << "Applying style " << gStyle->GetName() << std::endl;
    std::cout << gStyle->GetOptStat() << std::endl;
  }
  h_->UseCurrentStyle();
  temp->cd();
  return true;
}

typedef MyHistoWrapper<TH1> wTH1;
typedef MyHistoWrapper<TH1D> wTH1D;
typedef MyHistoWrapper<TProfile> wTProf;
