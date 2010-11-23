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
  MyHistoWrapper<T>(T *h) : h_(h){ style2apply_ = NULL; }
  ~MyHistoWrapper() {}
  inline T *histo() const {return h_;}

  void   Draw(void)                       { Draw(drawoption_); }
  void   DrawSame(void)                   { Draw(drawoption_+"same"); }
  void   DrawSames(void)                  { Draw(drawoption_+"sames"); }
  void   Draw(const std::string& drawopt);

  // Only non-default values get set.
  TAxis *SetXaxis(const std::string& title,
		  bool center = false,
		  float titlesize=0.0,
		  float titleoffs=0.0,
		  int   titlefont=0,
		  float labelsize=0.0,
		  int   labelfont=0,
		  float rangeuserMin=1e99,
		  float rangeuserMax=-1e99,
		  int   ndiv=510);
  TAxis *SetYaxis(const std::string& title,
		  bool center = false,
		  float titlesize=0.0,
		  float titleoffs=0.0,
		  int   titlefont=0,
		  float labelsize=0.0,
		  int   labelfont=0,
		  float rangeuserMin=1e99,
		  float rangeuserMax=-1e99,
		  int   ndiv=510);
  TAxis *SetZaxis(const std::string& title,
		  bool center = false,
		  float titlesize=0.0,
		  float titleoffs=0.0,
		  int   titlefont=0,
		  float labelsize=0.0,
		  int   labelfont=0,
		  float rangeuserMin=1e99,
		  float rangeuserMax=-1e99,
		  int   ndiv=510);
  void SetLine   (int color=0,
		  int style=0,
		  int width=0);
  void SetMarker (int color=0,
		  int style=0,
		  int size=0);
  void SetStats  (bool  on=true,
		  float x1ndc=0.0,
		  float y1ndc=0.0,
		  float x2ndc=0.0,
		  float y2ndc=0.0);
  void DrawStats (void);

  bool statsAreOn    (void)                       { return statsAreOn_; }
  void SetLegendEntry(const std::string& entry)   { legentry_   = entry;  }
  void SetDrawOption (const std::string& option)  { drawoption_ = option; }

  void SaveStyle       (TStyle *instyle)          { style2apply_ = instyle; }
  bool ApplySavedStyle (void);

  void Add2Legend(TLegend *leg) {
    std::string legdrawopt;
    if (h_->GetFillColor())                              legdrawopt = "F";
    else if (drawoption_.size()) {
      if (drawoption_.find("L") != string::npos)
	if (drawoption_.find("E") != string::npos)       legdrawopt = "LE";
	else                                             legdrawopt = "L";
      else if (drawoption_.find("HIST") != string::npos) legdrawopt = "L";
      else if (drawoption_.find("P") != string::npos)    legdrawopt = "P";
    }
    else                                                 legdrawopt = "L";

    cout << "legdrawopt="<<legdrawopt<<endl;
    leg->AddEntry(h_,legentry_.c_str(),legdrawopt.c_str());
  }

  const std::string& GetLegendEntry(void)   { return legentry_;  }

  void loadFitFunction(TF1 *fitfn) {if (fitfn) v_fits_.push_back(fitfn);}

  void DrawFits(const std::string& drawopt="");

  MyHistoWrapper<T> *Clone(const std::string& newname,
			   const std::string& newtitle);
private:
  TAxis *SetAxis(TAxis *ax,
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
  std::string legentry_;
  std::string drawoption_;
  bool        statsAreOn_;
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
  h_ = new T(name.c_str(),title.c_str(),nbins,min,max);
  drawoption_=std::string("");
  style2apply_ = NULL;
}

//======================================================================

template<typename T>
MyHistoWrapper<T>::MyHistoWrapper(T *h, const std::string& name, const std::string& title) : h_(h)
{
  if (name.size())  h_->SetName (name.c_str());
  if (title.size()) h_->SetTitle(title.c_str());
  style2apply_ = NULL;
}

//======================================================================

template <typename T>
MyHistoWrapper<T> *
MyHistoWrapper<T>::Clone(const std::string& newname,
			 const std::string& newtitle)
{
  T *newh = (T *)h_->Clone(newname.c_str());
  newh->SetTitle(newtitle.c_str());
  return new MyHistoWrapper<T>(newh);
}

//======================================================================

template<typename T>
TAxis *
MyHistoWrapper<T>::SetXaxis(const std::string& t,bool c,
			    float ts, float to, int tf,
			    float ls, int lf,
			    float ruMin, float ruMax,int nd) {
  return (SetAxis(h_->GetXaxis(),t,c,ts,to,tf,ls,lf,ruMin,ruMax,nd));
}

//======================================================================

template<typename T>
TAxis *
MyHistoWrapper<T>::SetYaxis(const std::string& t,bool c,
			    float ts, float to, int tf,
			    float ls, int lf,
			    float ruMin, float ruMax,int nd) {
  return (SetAxis(h_->GetYaxis(),t,c,ts,to,tf,ls,lf,ruMin,ruMax,nd));
}

//======================================================================

template<typename T>
TAxis *
MyHistoWrapper<T>::SetZaxis(const std::string& t,bool c,
			    float ts, float to, int tf,
			    float ls, int lf,
			    float ruMin, float ruMax,int nd) {
  return (SetAxis(h_->GetZaxis(),t,c,ts,to,tf,ls,lf,ruMin,ruMax,nd));
}

//======================================================================

template<typename T>
TAxis *
MyHistoWrapper<T>::SetAxis(TAxis *ax,
			   const std::string& title,
			   bool center,
			   float titlesize,
			   float titleoffs,
			   int   titlefont,
			   float labelsize,
			   int   labelfont,
			   float rangeuserMin,
			   float rangeuserMax,
			   int   ndiv)
{
  if (title.size())    ax->SetTitle(title.c_str());
  if (center)          ax->CenterTitle();
  if (titlesize > 0.0) ax->SetTitleSize(titlesize);
  if (titleoffs > 0.0) ax->SetTitleOffset(titleoffs);
  if (titlefont > 0)   ax->SetTitleFont(titlefont);
  if (labelsize > 0.0) ax->SetLabelSize(labelsize);
  if (labelfont > 0)   ax->SetLabelFont(labelfont);
  if (rangeuserMin < rangeuserMax) {
    ax->SetLimits(rangeuserMin,rangeuserMax);
  }
  if (ndiv != 510)     ax->SetNdivisions(ndiv,kFALSE);

  return ax;
}

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
			    float x1ndc,
			    float y1ndc,
			    float x2ndc,
			    float y2ndc)
{
  statsAreOn_ = on;
  h_->SetStats(on);
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

  cout << "Drawing " << h_->GetName() << " with " << drawopt << endl;

  h_->Draw(drawopt.c_str());
}

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::DrawFits(const std::string& drawopt)
{
  for (size_t i=0; i<v_fits_.size(); i++)  {
    TF1 *f1 = v_fits_[i];
    cout << "Drawing fit " << f1->GetName() << " with " << drawopt << endl;
    double xmin,xmax;
    f1->GetRange(xmin,xmax);
    h_->Fit(f1,"",drawopt.c_str(),xmin,xmax);
    //f1->Draw(drawopt.c_str());
  }
}

//======================================================================

template<typename T>
void
MyHistoWrapper<T>::DrawStats(void)
{
  TPaveStats *st1 = (TPaveStats*)h_->GetListOfFunctions()->FindObject("stats");
  if (!st1) {
    std::cout << "Error, couldn't find stats object for ";
    std::cout << h_->GetName() << std::endl;
    return;
  }

  double x1 = stats_.GetX1NDC();
  double x2 = stats_.GetX2NDC();
  double y1 = stats_.GetY1NDC();
  double y2 = stats_.GetY2NDC();

  if ((x1 > 0)  && (x1 < 1.) &&
      (x2 > x1) && (x2 < 1.) &&
      (y1 > 0)  && (y1 < 1.) &&
      (y2 > y1) && (y2 < 1.)) {
    st1->SetX1NDC(x1); st1->SetX2NDC(x2);
    st1->SetY1NDC(y1); st1->SetY2NDC(y2);
#if 1
    std::cout << "Setting Stats Object for " << h_->GetName() << " to ";
    std::cout << x1 << ", " << x2 << ", " << y1 << ", " << y2 << std::endl;
#endif

  }
  if (drawoption_ == "P") {
    st1->SetLineColor(h_->GetMarkerColor());
    st1->SetTextColor(h_->GetMarkerColor());
  } else {
    st1->SetLineColor(h_->GetLineColor());
    st1->SetTextColor(h_->GetLineColor());
  }
}

//======================================================================

template<typename T>
bool
MyHistoWrapper<T>::ApplySavedStyle (void)
{
  if (!style2apply_) return false;
  TStyle *temp = gStyle;
  style2apply_->cd();
  std::cout << "Applying style " << gStyle->GetName() << endl;
  std::cout << gStyle->GetOptStat() << std::endl;
  h_->UseCurrentStyle();
  temp->cd();
  return true;
}

typedef MyHistoWrapper<TH1> wTH1;
typedef MyHistoWrapper<TH1D> wTH1D;
typedef MyHistoWrapper<TProfile> wTProf;
