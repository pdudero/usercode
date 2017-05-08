#include <iostream>
#include <map>
#include "TH1.h"
#include "TPad.h"
#include "TGraph.h"
#include "TCanvas.h"
#include <glob.h>
#include <cassert>
#include <cstdlib>

#include "tdrstyle.C"

//string fileglob = "CLEAN_NODUPEDAYS/_HE16/*.txt";
//string fileglob = "_HE16/*.txt";
string fileglob = "_HE-16/*.txt";
//string fileglob = "_HE-16/fcamps2012_114090194*.txt";

bool gl_verbose=false;

using namespace std;

#define LINELEN 80

//======================================================================

void loadMapFromFile(const char *filename, 
		     std::map<int, float>& m_ietaiphiratio)
{
  char linein[LINELEN];

  FILE *fp = fopen(filename, "r");

  if (!fp) {
    cerr << "File failed to open, " << filename << endl;
    exit(-1);
  }

  if (gl_verbose)
    cout << "Loading map from file " << filename << endl;

  while (!feof(fp) && fgets(linein,LINELEN,fp)) {
    int ieta, iphi;
    float ratio;
    if( linein[0]=='#' ) continue;                // comments are welcome
    if( sscanf(linein, "%d %d %f", &ieta, &iphi, &ratio) != 3 ) {
      cerr << "scan failed, file " << filename << ", line = " << linein;
      cerr << ", skipping" << endl;
      //return;
      continue;
    }
    else {
      int ietaiphi = ieta*100 + iphi;
      m_ietaiphiratio.insert(std::pair<int,float>(ietaiphi, ratio));
    }
  }

  fclose(fp);
}                                                     // loadMapFromFile

//======================================================================

void makeGraph4ietas(int ieta1, int ieta2, TVirtualPad *pad,
		     const std::map<int, float>& m_ietaiphiratio)
{
  TGraph *result = new TGraph();
  result->SetNameTitle(Form("ieta%dvs%d",ieta1,ieta2),
		       Form("i#eta %d vs. %d; i#eta %d; i#eta %d",
			    ieta2,ieta1,ieta1,ieta2));
  int ipt = 0;
  for (int iphi=1; iphi<72; iphi+=2) {
    int ietaiphi1 = 100*ieta1 + iphi;
    int ietaiphi2 = 100*ieta2 + iphi;
    std::map<int,float>::const_iterator it1 = m_ietaiphiratio.find(ietaiphi1);
    std::map<int,float>::const_iterator it2 = m_ietaiphiratio.find(ietaiphi2);
    if (it1 != m_ietaiphiratio.end() && it1 != m_ietaiphiratio.end() )
      result->SetPoint(ipt++, it1->second, it2->second);
    else
      cerr << "One of the points " << ietaiphi1 << ", "
	   << ietaiphi2 << "not found in map" << endl;
  }

  result->GetXaxis()->SetLimits(0.3,1.2);
  result->GetYaxis()->SetRangeUser(0.3,1.2);

  result->SetMarkerStyle(8);

  double corfactor = result->GetCorrelationFactor();
  TLatex *ltx = new TLatex();

  result->Draw("AP");
  ltx->DrawLatexNDC(.4,.8,Form("R = %5.2lf",corfactor));

}                                                     // makeGraph4ietas

//======================================================================

void plotCorrelations()
{
  std::map<int,float> m_ietaiphiratio;

  setTDRStyle();

  // pick out day of year and mean(sum(fC))
  loadMapFromFile("ampsoverphilumi40_4dan.txt",m_ietaiphiratio);

  TCanvas *c1 = new TCanvas("c1","c1",1000,750);
  c1->Divide(4,3);

  makeGraph4ietas(22,24,c1->cd(1),m_ietaiphiratio);
  makeGraph4ietas(24,26,c1->cd(2),m_ietaiphiratio);
  makeGraph4ietas(26,28,c1->cd(3),m_ietaiphiratio);
  makeGraph4ietas(28,29,c1->cd(4),m_ietaiphiratio);
  makeGraph4ietas(21,23,c1->cd(5),m_ietaiphiratio);
  makeGraph4ietas(23,25,c1->cd(6),m_ietaiphiratio);
  makeGraph4ietas(25,27,c1->cd(7),m_ietaiphiratio);
  makeGraph4ietas(27,29,c1->cd(8),m_ietaiphiratio);
  makeGraph4ietas(22,23,c1->cd(9),m_ietaiphiratio);
  makeGraph4ietas(24,25,c1->cd(10),m_ietaiphiratio);
  makeGraph4ietas(26,27,c1->cd(11),m_ietaiphiratio);
  makeGraph4ietas(28,29,c1->cd(12),m_ietaiphiratio);

  c1->SaveAs("correlations_hep.png");

  makeGraph4ietas(-22,-24,c1->cd(1),m_ietaiphiratio);
  makeGraph4ietas(-24,-26,c1->cd(2),m_ietaiphiratio);
  makeGraph4ietas(-26,-28,c1->cd(3),m_ietaiphiratio);
  makeGraph4ietas(-28,-29,c1->cd(4),m_ietaiphiratio);
  makeGraph4ietas(-21,-23,c1->cd(5),m_ietaiphiratio);
  makeGraph4ietas(-23,-25,c1->cd(6),m_ietaiphiratio);
  makeGraph4ietas(-25,-27,c1->cd(7),m_ietaiphiratio);
  makeGraph4ietas(-27,-29,c1->cd(8),m_ietaiphiratio);
  makeGraph4ietas(-22,-23,c1->cd(9),m_ietaiphiratio);
  makeGraph4ietas(-24,-25,c1->cd(10),m_ietaiphiratio);
  makeGraph4ietas(-26,-27,c1->cd(11),m_ietaiphiratio);
  makeGraph4ietas(-28,-29,c1->cd(12),m_ietaiphiratio);

  c1->SaveAs("correlations_hem.png");
}
